#include "../includes/Golomb.h"
#include <cmath>

Golomb::Golomb(unsigned int _m): m(_m){}

void Golomb::uEncode(unsigned int n) {
    unsigned int q = (unsigned int) floor(n / this.m);
    unsigned int r = n % this.m; /* <=> n-q*m  - TODO: what is more optimal */
    unsigned char unary = this.encodeUnary(q);
    std::tuple<unsigned char, unsigned int> binaryRes = this.encodeTruncatedBinary(r);
    unsigned char binary = get<0>(res);
    unsigned int nBinBits = get<1>(res);

    /*
     * bitStream.writeNBits(unary, q + 1);
     * bitStream.writeNBits(binary, nBinBits);
     *
     * OR (if unsigned chars):
     */

    unsigned char code = unary | (binary>>(q+1));
    // bitStream.writeNBits(code, q + 1 + nBinBits);

    // number of bits from the truncated binary lost in the shift operation
    int nLostBits = 8 - (q + 1 + nBinBits);
    if(nLostBits){
        unsigned char lostBits = 0x00;
        unsigned char tempBin = binary;
        for(int i = q; i >= 0; i--){
            bool ret = tempBin & 1;
            tempBin >>= 1;
            lostBits = (lostBits | (ret<<(7-i)));
        }
        // bitStream.writeNBits(lostBits, nLostBits);
    }
}

void Golomb::sEncode(int n) {

}

unsigned char Golomb::encodeUnary(unsigned int q) {
    /* unary comma code where the end mark is '1'*/
    return 0x80 >> q; // Ex: q = 2; mask = 00100000
}

std::tuple<unsigned char, unsigned int> Golomb::encodeTruncatedBinary(unsigned int r) {
    unsigned int b = (unsigned int) ceil(log2(this.m));
    unsigned int nBits, codeNumber;
    /* Encode the first 2**b − m values of r using the first 2**b−m binary codewords of b−1 bits */
    if(r < (pow(2,b) - this.m)){
        codeNumber = r;
        nBits = b - 1;
    } else {
        /* Encode the remainder values of r by coding the number r+2**b−m in binary codewords of b bits. */
        codeNumber = r + pow(2,b) - this.m;
        nBits = b;
    }

    /* Conversion of decimal code number to binary*/
    unsigned  char bin = 0x00;
    for(int i = 0; codeNumber > 0; codeNumber /= 2, i++) {
        bin |= ((codeNumber % 2) << i);
    }

    /* Shift the truncated binary code to the most significant bits */
    bin <<= (8 - nBits);
    return std::make_tuple(bin, nBits);
}

unsigned int Golomb::uDecode() {
    unsigned int q = this.decodeUnary();
    unsigned int r = this.decodeTruncatedBinary();
    return this.m*q + r;
}

int Golomb::sDecode() {

}

unsigned int Golomb::decodeUnary() {
    unsigned int q = 0;
    /*
     * while(!bitStream.readBit()){ // if readBit returns the bit or the boolean associated
     *      q++;
     * }*/
    return q;
}

unsigned int Golomb::decodeTruncatedBinary() {
    unsigned int r = 0;
    unsigned int b = (unsigned int) ceil(log2(this.m));

    // bitStream.readNBits(b - 1)
    // convert the b-1 bits read to dec/int (into the variable readInt)
    // if(readInt < (pow(2,b) - this.m)) {
    //      r = readInt;
    // } else {
    //      bitStream.readBit();
    //      covert the b-1 firstly read bits "concatenated" with the last bit read to dec/int (into the variable readInt)
    //      r = readInt - pow(2,b) + this.m;
    // }

    return r;
}