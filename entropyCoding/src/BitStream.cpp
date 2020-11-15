//! BitStream functions
/*!
 *  @author Agostinho Pires.
 *  @author Inês Justo. (modified some logic of returns)
 *  This file have the code of the functions
 */

#include "../includes/BitStream.hpp"

BitStream :: BitStream(char *file_r){

    //Init var need
    cr_pos = 0;
    cw_pos = 0;
    len = 0;
    r_pos = 7;
    w_pos = 0;
    result = 0;

    ifstream infile (file_r);

    if(infile.is_open()){
        cout << "SUCCESS : File "<<file_r<<" is Opened " << endl;
    } else {
        cout << "FAILED to open file " << endl;
    }
    unsigned char in;
    while(infile >> in){
        info_file.push_back(in);
        len++;
    }

    infile.close();

}

unsigned char BitStream :: getC(void){
    unsigned char in;
    in = info_file[cr_pos];
    return in;
}


bool BitStream :: readBit(){
    unsigned char val = getC();

    //cout << "val read = "<< val << endl;
    int bit, position;
    position = pow(2, r_pos);
    bit =((int)val & position);
    bit = bit >> r_pos;
    // cout << "Bit read = "<< bit << endl;
    r_pos--;

    if(r_pos < 0){
        //cout << "Acabei as contagens" << endl;
        r_pos = 7;
        cr_pos++;
        //cout << "Read buffer = " <<read_buffer << endl;
        //write on file, do that on read bit on readBit
    }
    return bit != 0;
}

vector<bool> BitStream :: readNbits (unsigned int N){
    vector<bool> bits;

    for(int i = 0; i<N;i++){
        if(i>len*8){
            cout << "ERROR = This file don't have more information"<< endl;
            break;
        }

        //cout << "Buffer position = " <<(int)r_pos << endl;

        bits.push_back(readBit());
        //cout << "Bit read = "<< readBit() << endl;
        //cout << "Read buffer = " << buffer << endl;
        //r_pos = r_pos<<1;
    }
    return bits;
}

void BitStream :: writeBit(bool bit){
    unsigned int b;
    if(bit){ b = 1; } else { b = 0; }

    result = (result | (b << w_pos));
    //cout << "Write bit = " << result << endl;
    w_pos++;

    buffer = result;

    if(w_pos >7){
        new_file.push_back(buffer);
        //cout << "Write Buffer = " << buffer << endl;
        result = 0;
        w_pos = 0;
        cw_pos++;
        //openfile << buffer;
    }
}

void BitStream :: writeNbits(vector<bool> bits){
    for(int i = 0 ; i<bits.size(); i++){
        writeBit(bits.at(i));
    }
}

void BitStream :: writeOnfile(char* file_w){
    ofstream openfile (file_w, ofstream::binary);


    if(openfile.is_open()){
        cout << "SUCCESS : File "<<file_w<<" is Opened " << endl;
    }
    if(w_pos%8!=0){
        // cout << "Entrei aqui "<< endl;
        new_file.push_back(buffer);
    }
    for(char inputs : new_file){
        openfile << inputs;
    }

    openfile.close();
}

BitStream::~BitStream(){
    delete this;
}