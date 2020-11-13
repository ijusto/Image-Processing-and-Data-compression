#include "../includes/Golomb.h"
#include "BitStream.cpp"
#include <cmath>
#include <vector>
#include <iostream>
#include <tuple>
#include <functional>

Golomb::Golomb(unsigned int _m, BitStream _bitStream): m(_m), bitStream(_bitStream){}

void Golomb::encode(int n) {
    /* a positive value x is mapped to x'=2|x|=2x,x>0 and a negative value y is mapped to y'=2|y|-1=-2y-1,y<0*/
    int nMapped = 0;
    if (n >= 0){
        nMapped = 2 * n;
    } else {
        nMapped = -2*n -1;
    }
    auto q = (unsigned int) floor(nMapped / this->m);
    unsigned int r = n % this->m; /* <=> n-q*m  - TODO: what is more optimal */
    unsigned char unary = Golomb::encodeUnary(q);
    std::tuple<unsigned char*, unsigned int> binaryRes = this->encodeTruncatedBinary(r);
    unsigned char* binary = get<0>(binaryRes);
    unsigned int nBinBits = get<1>(binaryRes);

    this->bitStream.writeNbits(q + 1, &unary);
    this->bitStream.writeNbits(nBinBits, binary);
}

unsigned char Golomb::encodeUnary(unsigned int q) {
    /* unary comma code where the end mark is '1'*/
    return 0x80 >> q; // Ex: q = 2; mask = 00100000
}

std::tuple<unsigned char*, unsigned int> Golomb::encodeTruncatedBinary(unsigned int r) {
    auto b = (unsigned int) ceil(log2(this->m));
    /* Encode the first 2**b − m values of r using the first 2**b−m binary codewords of b−1 bits */
    unsigned int codeNumber = r;
    unsigned int nBits = b - 1;
    /* Encode the remainder values of r by coding the number r+2**b−m in binary codewords of b bits. */
    if(r >= pow(2, b) - this->m) { codeNumber += (int)pow(2, b) - this->m; nBits += 1; }

    /* Conversion of decimal code number to binary*/
    std::vector<int> binInt;
    for(int i = 0; codeNumber > 0; codeNumber /= 2, i++) {
        binInt.push_back((codeNumber % 2));
    }
    unsigned char *binChar = nullptr;
    for (int i = 0; i < binInt.size(); i++){
        if(binInt.at(i) == 0){
            *(binChar + binInt.size() -1 - i) = '0';
        } else{
            *(binChar + binInt.size() -1 - i) = '1';
        }
    }

    return std::make_tuple(binChar, nBits);
}

int Golomb::decode() {
    unsigned int q = this->decodeUnary();
    unsigned int r = this->decodeTruncatedBinary();
    int nMapped = this->m*q + r;
    /* a positive value x is mapped to x'=2|x|=2x,x>0 and a negative value y is mapped to y'=2|y|-1=-2y-1,y<0*/
    int n;
    if((nMapped % 2) == 0){
        n = nMapped/2;
    } else {
        n = -(nMapped + 1)/2;
    }
    return n;
}


//! Decodes the quotient of the coded number, that is, the comma code (unary) part of the Golomb code.
/*!
 * @return q quotient of the coded number by m.
*/
unsigned int Golomb::decodeUnary() {
    unsigned int q = 0;
    while(this->bitStream.readBit() == '0'){ q++; }
    return q;
}

//! Decodes the remainder of the division of the coded number by m, that is, the truncated binary code part of the
//! Golomb code.
/*!
 * @return r remainder of the division of the coded number by m.
*/
unsigned int Golomb::decodeTruncatedBinary() {
    unsigned int r = 0;
    auto b = (unsigned int) ceil(log2(this->m));

    unsigned char nBitsRead[b - 1];
    this->bitStream.readNbits(b - 1, nBitsRead);
    // convert the b-1 bits read to dec/int
    int readInt = 0;
    for (int i = 0; i < b - 1; i++){
        if(nBitsRead[b - 2 - i] == '1'){
            readInt += pow(2,i);
        }
    }

    if(readInt < (pow(2,b) - this->m)) {
          r = readInt;
    } else {
          unsigned char bitRead = this->bitStream.readBit();
          // covert the b-1 firstly read bits "concatenated" with the last bit read to dec/int
          unsigned int bitReadInt = 0;
          if(bitRead == '1'){
              bitReadInt = 1;
          }
          unsigned int newCodeRead = readInt*2 + bitReadInt;
          r = newCodeRead - (int)pow(2,b) + this->m;
    }
    return r;
}

void getAlpham(){
    /* The Golomb code is optimum for an information source following a distribution P(n) = α^n(1 − α), n = 0, 1, 2, . . .
     * where m = ceil(-1/log2α)*/

}


Golomb::~Golomb(){
    delete this;
}
