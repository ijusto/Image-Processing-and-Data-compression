#include "../includes/Golomb.h"
#include "BitStream.cpp"
#include <cmath>
#include <vector>
#include <iostream>
#include <tuple>

Golomb::Golomb(unsigned int _m, BitStream _bitStream): m(_m), bitStream(_bitStream), useBS(true){}

Golomb::Golomb(unsigned int _m): m(_m), bitStream(nullptr), useBS(false){}

vector<bool> Golomb::encode(int n) {
    /* a positive value x is mapped to x'=2|x|=2x,x>0 and a negative value y is mapped to y'=2|y|-1=-2y-1,y<0*/
    int nMapped = 0;
    if (n >= 0){
        nMapped = 2 * n;
    } else {
        nMapped = -2*n -1;
    }
    auto q = (unsigned int) (nMapped / this->m);
    unsigned int r = n % this->m; /* <=> n-q*m */
    vector<bool> unary = Golomb::encodeUnary(q);
    vector<bool> truncBin = this->encodeTruncatedBinary(r);

    if(this->useBS){
        unsigned char u[unary.size()];
        for(int i=0; i<unary.size(); i++){
            if(unary.at(i)){ u[i] = '1';
            } else { u[i] = '0'; }
        }

        unsigned char tb[truncBin.size()];
        for(int i=0; i<truncBin.size(); i++){
            if(truncBin.at(i)){ tb[i] = '1';
            } else { tb[i] = '0'; }
        }
        this->bitStream.writeNbits(unary.size(), u);
        this->bitStream.writeNbits(truncBin.size(), tb);
    }

    vector<bool> encoded_n = unary;
    encoded_n.insert( encoded_n.end(), truncBin.begin(), truncBin.end() );

    return encoded_n;
}

vector<bool> Golomb::encodeUnary(unsigned int q) {
    /* unary comma code where the end mark is '1'*/
    vector<bool> unary;
    for(int i=0; i<q; i++){ unary.push_back(false); }
    unary.push_back(true);
    return unary;
}

vector<bool> Golomb::encodeTruncatedBinary(unsigned int r) {
    vector<bool> truncatedBin;
    auto b = (unsigned int) ceil(log2(this->m));
    /* Encode the first 2**b − m values of r using the first 2**b−m binary codewords of b−1 bits */
    unsigned int codeNumber = r;
    unsigned int nBits = b - 1;
    /* Encode the remainder values of r by coding the number r+2**b−m in binary codewords of b bits. */
    if(r >= pow(2, b) - this->m) { codeNumber += (int)pow(2, b) - this->m; nBits += 1; }

    /* Conversion of decimal code number to binary*/
    for(int i = 0; codeNumber > 0; codeNumber /= 2, i++) {
        truncatedBin.push_back(codeNumber % 2 == 0);
    }
    vector<bool> res;
    while(truncatedBin.size() < nBits){
        res.push_back(false);
        nBits--;
    }
    res.insert( res.end(), truncatedBin.begin(), truncatedBin.end() );

    return res;
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
    unsigned int q, r;
    while(!encoded_n.empty()){
        /* Decode unary */
        q = 0;
        for(bool bit : encoded_n){
            if(!bit){
                q++;
            } else {
                break;
            }
        }
        for(int i = 0; i <= q; i++){
            encoded_n.erase(encoded_n.begin());
        }

        /* Decode truncated binary */
        r = 0;
        auto b = (unsigned int) ceil(log2(this->m));

        vector<bool> nBitsRead;

        for(int i = 0; i < b - 1; i++){
            nBitsRead.push_back(encoded_n.at(i));
        }
        for(int i = 0; i < b - 1; i++){
            encoded_n.erase(encoded_n.begin());
        }

        // convert the b-1 bits read to dec/int
        int readInt = 0;
        for (int i = 0; i < b - 1; i++){
            if(nBitsRead.at(b - 2 - i)){
                readInt += pow(2,i);
            }
        }

        if(readInt < (pow(2,b) - this->m)) {
            r = readInt;
        } else {
            bool lastBitRead = encoded_n.at(0);
            encoded_n.erase(encoded_n.begin());

            // covert the b-1 firstly read bits "concatenated" with the last bit read to dec/int
            unsigned int bitReadInt = 0;
            if(lastBitRead){
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

        std::cout << "decode number " << n << std::endl;
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
