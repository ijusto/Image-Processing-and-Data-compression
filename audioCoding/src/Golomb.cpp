#include "../includes/Golomb.h"
#include <cmath>

Golomb::Golomb(unsigned int _m): m(_m){}

void Golomb::uEncode(unsigned int n) {
    unsigned int q = n / this->m;
    unsigned int r = n % m; /* <=> n-q*m  - TODO: what is more optimal */
    /* TODO: invoke private method to encode unary and truncated binary codes*/
    this->encodeUnary(q);
    this->encodeTruncatedBinary(r);
}

void Golomb::sEncode(int n) {

}

void Golomb::encodeUnary(unsigned int q) {
    // bitStream.writeNBits()
}

void Golomb::encodeTruncatedBinary(unsigned int r) {
    unsigned int b = ceil(log2(m));
    /* Encode the first 2**b − m values of r using the first 2**b−m binary codewords of b−1 bits */
    nfirstValR = 2**b - m;
    nBits = b - 1;
    /* Encode the remainder values of r by coding the number r+2**b−m in binary codewords of b bits. */
    nRemainValR = r + 2**b - m;
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