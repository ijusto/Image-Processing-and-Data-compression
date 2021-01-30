#include "../includes/Golomb.h"
#include "BitStream.cpp"
#include <cmath>
#include <vector>
#include <iostream>
#include <tuple>
#include <algorithm>

using namespace std;

Golomb::Golomb(unsigned int _m){
    this->m = _m;
}

Golomb::Golomb(unsigned int _m, char *_encodeFile, char *_decodeFile){
    this->m = _m;
    this->readBitStream = new BitStream(_decodeFile, 'r');
    this->writeBitStream = new BitStream(_encodeFile,'w');
    this->mode = 'b';
}

Golomb::Golomb(unsigned int _m, char *_file, char mode) {
    this->m = _m;
    if(mode == 'e'){
        this->writeBitStream = new BitStream(_file,'w');
        this->mode = 'e';
    } else if(mode == 'd'){
        this->readBitStream = new BitStream(_file, 'r');
        this->mode = 'd';
    } else {
        throw "Mode not allowed. Modes available: e (encode) or d (decode).\nIf you want the two modes use Golomb::"
              "Golomb(unsigned int _m, char *_encodeFile, char *_decodeFile);";
    }
}

void Golomb::setM(unsigned int _m){
    this->m = _m;
}

vector<bool> Golomb::encode(int n) {
    vector<bool> encoded_n;
    encode2(n, encoded_n);
    this->writeBitStream->writeNbits(encoded_n);
    return encoded_n;
}

void Golomb::encode2(int n, vector<bool> &encoded_n) {
    /* a positive value x is mapped to x'=2|x|=2x,x>0 and a negative value y is mapped to y'=2|y|-1=-2y-1,y<0*/

    int nMapped = 2 * n;
    if (n < 0){ nMapped = -nMapped -1; }

    unsigned int q = nMapped / this->m;
    unsigned int r = nMapped % this->m; /* <=> nMapped-q*m */

    /* Encode unary */
    for(int i=0; i<q; i++){ encoded_n.push_back(false); }
    /* unary comma code where the end mark is '1'*/
    encoded_n.push_back(true);

    /* Encode Truncated Binary */
    vector<bool> truncatedBinTmp;
    auto b = (unsigned int) ceil(log2(this->m));

    /* Encode the first 2**b − m values of r using the first 2**b−m binary codewords of b−1 bits */
    unsigned int codedR = r;
    unsigned int nBits = b - 1;

    /* Encode the remainder values of r by coding the number r+2**b−m in binary codewords of b bits. */
    if(r >= ((int)pow(2, b) - this->m)) {
        codedR += ((int)pow(2, b) - this->m);
        nBits += 1;
    }

    /* Conversion of decimal code number to binary*/
    for(int i = 0; codedR > 0; codedR /= 2, i++) {
        truncatedBinTmp.push_back(codedR % 2);
    }
    std::reverse(truncatedBinTmp.begin(),truncatedBinTmp.end());

    vector<bool> truncatedBin;
    for(;truncatedBinTmp.size() < nBits; nBits--){ truncatedBin.push_back(false); }
    truncatedBin.insert( truncatedBin.end(), truncatedBinTmp.begin(), truncatedBinTmp.end() );
    encoded_n.insert( encoded_n.end(), truncatedBin.begin(), truncatedBin.end() );
}

void Golomb::closeEncodeFile(){
    this->writeBitStream->endWriteFile();
}

void Golomb::decode(vector<int> *numbers) {
    while(true){
        /* Decode unary */
        unsigned int q = 0;
        try{
            while(!this->readBitStream->readBit()){ q++; }
        } catch( string mess){
            break;
        }

        /* Decode truncated binary */
        unsigned int r = 0;
        auto b = (unsigned int) ceil(log2(this->m));

        vector<bool> nBitsRead;
        try{
            nBitsRead = this->readBitStream->readNbits(b - 1);
        } catch( string mess){
            std::cout << mess << std::endl;
            std::exit(0);
        }

        // convert the b-1 bits read to dec/int
        int readInt = 0;
        for(int i = 0; i < b - 1; i++){
            if(nBitsRead.at(b - 2 - i)){
                readInt += pow(2, i);
            }
        }
        r = readInt;

        /* If the bits read are an encoded value less than 2**b-m, decoding is complete.*/
        if(readInt >= ((int) pow(2,b) - this->m)) {
            bool bitRead;
            try{
                bitRead = this->readBitStream->readBit();
            } catch( string mess){
                std::cout << mess << std::endl;
                std::exit(0);
            }

            // covert the b-1 firstly read bits "concatenated" with the last bit read to dec/int
            unsigned int bitReadInt = 0;
            if(bitRead){ bitReadInt = 1; }

            /* Otherwise, read an additional bit and subtract 2**b-m from the result. */
            unsigned int newCodeRead = readInt*2 + bitReadInt;
            r = newCodeRead + this->m - (int)pow(2,b);
        }

        unsigned int nMapped = this->m*q + r;

        /* a positive value x is mapped to x'=2|x|=2x,x>0 and a negative value y is mapped to y'=2|y|-1=-2y-1,y<0*/
        int n = nMapped;
        if(nMapped % 2){ n = -(n+1);}
        n /= 2;
        (*numbers).push_back(n);
    }
}

void Golomb::decode3(vector<bool> &encodedBits, vector<int> &decoded) {
    unsigned int b = (unsigned int) ceil(log2(this->m));
    int index = 0;
    while(true){
        /* Decode unary */
        unsigned int q = 0;
        while (!encodedBits.at(index + q)) {
            q++;
        }

        // update index
        index = index + (q + 1);

        /* Decode truncated binary */
        unsigned int r = 0;
        // auto b = (unsigned int) ceil(log2(this->m));

        vector<bool> nBitsRead;
        for (int i = 0; i < b - 1; i++) {
            bool val = encodedBits.at(index + i);
            nBitsRead.push_back(val);
        }

        // update index
        index = index + (b - 1);

        // convert the b-1 bits read to dec/int
        int readInt = 0;
        for(int i = 0; i < b - 1; i++){
            if(nBitsRead.at(b - 2 - i)){
                readInt += pow(2, i);
            }
        }
        r = readInt;

        /* If the bits read are an encoded value less than 2**b-m, decoding is complete.*/
        if(readInt >= ((int) pow(2,b) - this->m)) {
            bool bitRead = encodedBits.at(index);
            // update index
            index = index + 1;

            // covert the b-1 firstly read bits "concatenated" with the last bit read to dec/int
            unsigned int bitReadInt = 0;
            if(bitRead){ bitReadInt = 1; }

            /* Otherwise, read an additional bit and subtract 2**b-m from the result. */
            unsigned int newCodeRead = readInt*2 + bitReadInt;
            r = newCodeRead + this->m - (int)pow(2,b);
        }

        unsigned int nMapped = this->m*q + r;

        /* a positive value x is mapped to x'=2|x|=2x,x>0 and a negative value y is mapped to y'=2|y|-1=-2y-1,y<0*/
        int n = nMapped;
        if(nMapped % 2){ n = -(n+1);}
        n /= 2;
        decoded.push_back(n);

        if(encodedBits.size() <= index){
            break;
        }
    }
}


void Golomb::decode2(vector<bool> &encodedBits, vector<int> &decoded, unsigned int *index, unsigned int count) {
    unsigned int b = (unsigned int) ceil(log2(this->m));

    int n_decoded = 0;
    while(n_decoded < count) {
        /* Decode unary */
        unsigned int q = 0;
        while (!encodedBits.at(*index + q)) {
            q++;
        }

        // update index
        *index = *index + (q + 1);

        /* Decode truncated binary */
        unsigned int r = 0;
        // auto b = (unsigned int) ceil(log2(this->m));

        vector<bool> nBitsRead;
        for (int i = 0; i < b - 1; i++) {
            bool val = encodedBits.at(*index + i);
            nBitsRead.push_back(val);
        }

        // update index
        *index = *index + (b - 1);

        // convert the b-1 bits read to dec/int
        int readInt = 0;
        for (int i = 0; i < b - 1; i++) {
            if (nBitsRead.at(b - 2 - i)) {
                readInt += pow(2, i);
            }
        }
        r = readInt;

        /* If the bits read are an encoded value less than 2**b-m, decoding is complete.*/
        if (readInt >= ((int) pow(2, b) - this->m)) {
            bool bitRead = encodedBits.at(*index);
            // update index
            *index = *index + 1;

            // covert the b-1 firstly read bits "concatenated" with the last bit read to dec/int
            unsigned int bitReadInt = 0;
            if (bitRead) { bitReadInt = 1; }

            /* Otherwise, read an additional bit and subtract 2**b-m from the result. */
            unsigned int newCodeRead = readInt * 2 + bitReadInt;
            r = newCodeRead + this->m - (int) pow(2, b);
        }

        unsigned int nMapped = this->m * q + r;
        /* a positive value x is mapped to x'=2|x|=2x,x>0 and a negative value y is mapped to y'=2|y|-1=-2y-1,y<0*/
        int n = nMapped;
        if (nMapped % 2) { n = -(n + 1); }
        n /= 2;

        decoded.push_back(n);

        n_decoded++;
    }
}

Golomb::~Golomb(){
    delete this;
}

void Golomb::setEncodeFile(char *_encodeFile) {
    this->writeBitStream = new BitStream(_encodeFile,'w');
}

void Golomb::setDecodeFile(char *_decodeFile) {
    this->readBitStream = new BitStream(_decodeFile, 'r');
}
