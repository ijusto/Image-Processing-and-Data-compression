#include "BitStream.hpp"

BitStream :: BitStream(char *file, char mode){

    if(mode == 'r'){
        ifstream infile (file);

        if(!infile.is_open()){
            throw("FAILED to open file ");
        }
        unsigned char in;
        while(infile >> in){
            readFileInfo.push_back(in);
            len++;
        }
        infile.close();
    } else if(mode == 'w'){
        outfile.open(file, ofstream::binary);
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

    //cout << "val read = "<< val << endl;
    int bit, position;
    position = pow(2, r_pos);
    bit =((int)val & position);
    bit = bit >> r_pos;
    r_pos--;

    if(r_pos < 0){
        //cout << "Acabei as contagens" << endl;
        r_pos = 7;
        cr_pos++;
    }

    return bit != 0;
}

vector<bool> BitStream::readNbits(unsigned int N){
    vector<bool> bits;

    for(int i = 0; i < N; i++){
        if(i>len*8){
            cout << "ERROR = This file don't have more information"<< endl;
            break;
        }

        try {
            bits.push_back(readBit());
        } catch (string mess){
            cout << mess << endl;
            break;
        }
    }
    return bits;
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
    for(int i = 0 ; i < bits.size(); i++){
        writeBit(bits.at(i));
    }
}

void BitStream::endWriteFile(){
    if(w_pos != 7){
        this->outfile.put(buffer);
    }
    this->outfile.close();
}

BitStream::~BitStream(){
    delete this;
}