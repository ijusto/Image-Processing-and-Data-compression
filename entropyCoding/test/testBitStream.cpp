#include    <iostream>
#include "../src/BitStream.cpp"

/*
 * Implement a simple program to test the Bitstream class.
 * Unitary tests should be considered as well.
 */

#define     NBITS   3
using namespace std;

int main(int argc, char* argv[]) {
    auto * BS = new BitStream(argv[argc-2]);

    unsigned char bits[NBITS];
    //cout << "Char 1 do file = " << BS->getC() << endl;
    //construct read_N_bits
    BS->readNbits(NBITS,bits);
    cout << "Bits read = "<< bits << endl;

    BS -> writeNbits(NBITS,bits);

    //BS ->writeOnfile(NBITS, argv[argc-1]);

    BS->readNbits(NBITS,bits);
    cout << "Bits read = "<< bits << endl;

    BS -> writeNbits(NBITS,bits);

    //BS ->writeOnfile(NBITS, argv[argc-1]);

    BS->readNbits(NBITS,bits);
    cout << "Bits read = "<< bits << endl;

    BS -> writeNbits(NBITS,bits);

    BS ->writeOnfile( argv[argc-1]);

    return 0;
}
