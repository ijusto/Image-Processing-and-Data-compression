/**
 * Audio encoder/decoder
 */

#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    if((argc < 4) | (string(argv[1]) != "encode" && string(argv[1]) != "decode")){
        cout << "usage: " << argv[0] << " <operation> SOURCE DEST" << endl;
        cout << "operations:" << endl;
        cout << "\t encode\t encode SOURCE .wav sound file into DEST compressed file" << endl;
        cout << "\t decode\t decode SOURCE compressed file into SOURCE .wav sound file" << endl;
        return 0;
    }
    string op = argv[1];
    string src = argv[2];
    string dst = argv[3];

    if (op == "encode"){
        // init encoder with sound file
        // encode
        // write compressed file
    }else if (op == "decode"){
        // init decoder with compressed file
        // decode
        // write sound file
    }
}