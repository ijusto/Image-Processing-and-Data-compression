#include "../includes/Golomb.h"
#include "BitStream.cpp"
#include <cmath>
#include <vector>
#include <iostream>
#include <tuple>

Golomb::Golomb(unsigned int _m, BitStream _bitStream): m(_m), bitStream(_bitStream){}

Golomb::Golomb(unsigned int _m): m(_m), bitStream(nullptr){}

vector<bool> Golomb::encode(int n) {
    /* a positive value x is mapped to x'=2|x|=2x,x>0 and a negative value y is mapped to y'=2|y|-1=-2y-1,y<0*/
    int nMapped = 0;
    if (n >= 0){
        nMapped = 2 * n;
    } else {
        nMapped = -2*n -1;
    }
    auto q = (unsigned int) floor(nMapped / this->m);
    unsigned int r = n % this->m; /* <=> n-q*m */
    unsigned char* unary = Golomb::encodeUnary(q);
    std::tuple<unsigned char*, unsigned int> binaryRes = this->encodeTruncatedBinary(r);
    unsigned char* binary = get<0>(binaryRes);
    unsigned int nBinBits = get<1>(binaryRes);

    if(this->bitStream != nullptr){
        this->bitStream.writeNbits(q + 1, unary);
        this->bitStream.writeNbits(nBinBits, binary);
    }

    vector<bool> encoded_n;
    for(int i = 0; i < q + 1; i++){
        if(*(unary + i) == '1'){
            encoded_n.push_back(true);
        } else if(*(unary + i) == '0'){
            encoded_n.push_back(false);
        }
    }

    for(int i = 0; i < nBinBits; i++){
        if(*(binary + i) == '1'){
            encoded_n.push_back(true);
        } else if(*(binary + i) == '0'){
            encoded_n.push_back(false);
        }
    }

    return encoded_n;
}

unsigned char* Golomb::encodeUnary(unsigned int q) {
    /* unary comma code where the end mark is '1'*/
    unsigned char unary[q+1];
    for(int i=0; i<q; i++){
        unary[i] = '0';
    }
    unary[q] = '1';
    unsigned char *unaryPtr = unary;
    return unaryPtr;
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
    unsigned char binChar[binInt.size()];
    for (int i = 0; i < binInt.size(); i++){
        if(binInt.at(i) == 0){
            binChar[binInt.size() -1 - i] = '0';
        } else{
            binChar[binInt.size() -1 - i] = '1';
        }
    }
    unsigned char *binCharPtr = binChar;

    return std::make_tuple(binCharPtr, nBits);
}

int Golomb::decode() {
    unsigned int q = this->decodeUnary();
    unsigned int r = this->decodeTruncatedBinary();
    unsigned int nMapped = this->m*q + r;
    /* a positive value x is mapped to x'=2|x|=2x,x>0 and a negative value y is mapped to y'=2|y|-1=-2y-1,y<0*/
    int n;
    if((nMapped % 2) == 0){
        n = (int) nMapped/2;
    } else {
        n = (int) -(nMapped + 1)/2;
    }
    return n;
}

vector<int> Golomb::decode(vector<bool> encoded_n) {
    vector<int> numbers;
    //vector<bool> decoded;
    unsigned int q = 0;
    unsigned int r = 0;
    while(!encoded_n.empty()){
        /* Decode unary */
        q = 0;
        for(auto it=encoded_n.begin(); *it!=true; ++it){
            q++;
            //decoded.push_back(*it);
        }
        for(int i = 0; i < q; i++){
            encoded_n.erase(encoded_n.begin());
        }

        /* Decode truncated binary */
        r = 0;
        auto b = (unsigned int) ceil(log2(this->m));

        unsigned char nBitsRead[b - 1];

        for(int i = 0; i < b - 1; i++){
            if(encoded_n.at(i)){
                nBitsRead[i] = '1';
            } else {
                nBitsRead[i] = '0';
            }
            //decoded.push_back(encoded_n.at(i));
        }
        for(int i = 0; i < b - 1; i++){
            encoded_n.erase(encoded_n.begin());
        }

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
            unsigned char bitRead;
            if(encoded_n.at(0)){
                bitRead = '1';
            } else {
                bitRead = '0';
            }
            //decoded.push_back(encoded_n.at(0));
            encoded_n.erase(encoded_n.begin());

            // covert the b-1 firstly read bits "concatenated" with the last bit read to dec/int
            unsigned int bitReadInt = 0;
            if(bitRead == '1'){
                bitReadInt = 1;
            }
            unsigned int newCodeRead = readInt*2 + bitReadInt;
            r = newCodeRead - (int)pow(2,b) + this->m;
        }

        /* Get n */
        unsigned int nMapped = this->m*q + r;
        /* a positive value x is mapped to x'=2|x|=2x,x>0 and a negative value y is mapped to y'=2|y|-1=-2y-1,y<0*/
        int n;
        if((nMapped % 2) == 0){
            n = (int) nMapped/2;
        } else {
            n = (int) -(nMapped + 1)/2;
        }
        numbers.push_back(n);
    }

    return numbers;
}

unsigned int Golomb::decodeUnary() {
    unsigned int q = 0;
    while(this->bitStream.readBit() == '0'){ q++; }
    return q;
}

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

void Golomb::changeM(unsigned int _m) {
    this->m = _m;
}
