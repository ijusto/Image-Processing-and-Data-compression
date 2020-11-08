#include "../includes/Golomb.h"

void Golomb::uEncode(unsigned int n) {
    unsigned int q = n / this->m;
    unsigned int r = n % m; /* <=> n-q*m  - TODO: what is more optimal */
    /* TODO: invoke private method to encode unary and truncated binary codes*/
}

void Golomb::sEncode(int n) {

}

unsigned Golomb::uDecode(unsigned int q, unsigned int r) {

}

int Golomb::sDecode(unsigned int q, unsigned int r) {

}