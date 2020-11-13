#include    <iostream>
#include "../src/BitStream.cpp"

/*
 * Implement a simple program to test the Bitstream class.
 * Unitary tests should be considered as well.
 */


using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 5){
        cout << "Usage: " << argv[0] << " src_path dst_path Nbits number of times to read" << endl;
        cout << "src_path:\n\tpath to test_file_to_read" << endl;
        cout << "dst_path:\n\tpath to new_text_file" << endl;
        cout << "Nbits:\n\tnumber of bits" << endl;
        cout << "Nbits:\n\tnumber of times to read" << endl;
        return 0;
    }

    auto * BS = new BitStream(argv[argc-4]);
    int NBITS = stoi(argv[argc-2]);
    unsigned char bits[NBITS];

    //construct read_N_bits
    for (int i, i < stoi(argv[argc-1], i++ ){
        BS->readNbits(NBITS,bits);
        //cout << "Bits read = "<< bits << endl;
        BS -> writeNbits(NBITS,bits);
    }

    BS ->writeOnfile( argv[argc-3]);

    return 0;
}
