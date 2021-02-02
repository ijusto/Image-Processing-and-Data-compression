#include "BitStream.hpp"

BitStream :: BitStream(char *file, char mode){

    if(mode == 'r'){
        ifstream infile(file, std::ios::binary);

        if(!infile.is_open()){
            throw("FAILED to open file ");
        }
        unsigned char in;
        while(infile >> noskipws >> in){ /* reads whitespaces in file*/
            readFileInfo.push_back(in);
            len++;
        }
        infile.close();
    } else if(mode == 'w'){
        outfile.open(file, std::ios::binary);
        if(!outfile.is_open()){
            cout << "FAILED to open file " << file << endl;
        }
    } else {
        throw "Mode not allowed. Modes available: r (read) or w (write).";
    }
    //Init var need
    cr_pos = 0;
    r_pos = 7;
    w_pos = 7;
    result = 0;
}

unsigned char BitStream::getC(char mode){
    unsigned char c;
    if(mode == 'r'){
        c = readFileInfo[cr_pos];
    } else if(mode == 'w'){
        c = writeFileInfo[w_pos];
    } else {
        throw "BitStream::getC(char mode): mode not allowed.";
    }
    return c;
}


bool BitStream :: readBit(){

    if(cr_pos >= readFileInfo.size()){
        throw("Read all file.");
    }
    unsigned char val = getC('r');

    int bit, position;
    position = pow(2, r_pos);
    bit =((int)val & position);
    bit = bit >> r_pos;
    r_pos--;

    if(r_pos < 0){
        r_pos = 7;
        cr_pos++;
    }

    return bit != 0;
}

vector<bool> BitStream::readNbits(unsigned int N){
    vector<bool> bits;

    for(int i = 0; i < N; i++){
        try {
            bits.push_back(readBit());
        } catch (string mess){
            cout << mess << endl;
            break;
        }
    }
    return bits;
}

void BitStream::readToEnd(vector<bool> &outBits) {
    while(true){
        try {
            outBits.push_back(readBit());
        } catch (string mess){
            cout << mess << endl;
            break;
        }
    }
}

void BitStream :: writeBit(bool bit){
    unsigned int b;
    if(bit){ b = 1; } else { b = 0; }
    result |= b << w_pos;
    buffer = result;
    w_pos--;

    if(w_pos < 0){
        this->outfile.put(buffer);
        result = 0;
        w_pos = 7;
    }
}

void BitStream :: writeNbits(vector<bool> bits){
    for(bool bit : bits){
        writeBit(bit);
    }
}

void BitStream::endWriteFile(){
    if(w_pos != 7){
        this->outfile.put(buffer);
    }
    this->outfile.close();
}

unsigned int BitStream::size(){
    return len;
}

BitStream::~BitStream(){
    delete this;
}