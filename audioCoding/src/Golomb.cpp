#include "../includes/Golomb.h"
#include <cmath>

Golomb::Golomb(unsigned int _m): m(_m){}

void Golomb::uEncode(unsigned int n) {
    unsigned int q = n / this->m;
    unsigned int r = n % m; /* <=> n-q*m  - TODO: what is more optimal */
    // bitStream.writeNBits(unary, q+1)
    char unary = this->encodeUnary(q);
    // bitStream.writeNBits(binary, nBinBits)
    std::tuple<char,int> binaryRes = this->encodeTruncatedBinary(r);
    char binary = get<0>(res);
    int nBinBits = get<1>(res);

}

void Golomb::sEncode(int n) {

}

char Golomb::encodeUnary(unsigned int q) {
    /* unary comma code where the end mark is '1'*/
    // char mask = 0x80 >> q; // Ex: q = 2; mask = 00100000
    char mask = 0xFF >> q; // Ex: q = 2; mask = 00111111

    return mask;
}

std::tuple<char, int> char Golomb::encodeTruncatedBinary(unsigned int r) {
    unsigned int b = ceil(log2(m));
    /* Encode the first 2**b − m values of r using the first 2**b−m binary codewords of b−1 bits */
    int nBits;
    char bin = 0x00;
    int codeNumber;
    if(r < (2**b - m)){
        codeNumber = r;
        nBits = b - 1;
    } else {
        /* Encode the remainder values of r by coding the number r+2**b−m in binary codewords of b bits. */
        codeNumber = r + 2**b - m;
        nBits = b;
    }

    /* Conversion of decimal code number to binary*/
    for(int i = 0; codeNumber > 0; codeNumber /= 2, i++) {
        bin |= ((codeNumber % 2) << i);
    }

    /* Shift the truncated binary code to the most significant bits */
    bin <<= (8 - nBits);
    return std::make_tuple(bin, nBits);
}

void Golomb::decodeUnary() {
    // bitStream.readBit()
}

void Golomb::decodeTruncatedBinary() {
    /* In the truncated binary part, the decoder reads floor(log2(m)) bits at once
     * if the floor(log2(m)) bits are a shift code, then it has to read more bit(s) */
    unsigned int b = floor(log2(m));

    // bitStream.readNBits() -> n = b
}

unsigned Golomb::uDecode(unsigned int q, unsigned int r) {

}

int Golomb::sDecode(unsigned int q, unsigned int r) {

}