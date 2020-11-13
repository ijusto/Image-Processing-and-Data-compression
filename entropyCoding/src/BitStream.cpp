//! BitStream functions
/*!
 *  @author Agostinho Pires.
 *  This file have the code of the functions
 */

#include "BitStream.hpp"

BitStream :: BitStream(char *file_r){

    //Init var need
    cr_pos = 0;
    cw_pos = 0;
    len = 0;
    r_pos = 0;
    w_pos = 0;
    result = 0;

    ifstream infile (file_r);

    if(infile.is_open()){
        cout << "SUCCESS : File "<<file_r<<" is Opened " << endl;
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


unsigned char BitStream :: readBit (void){
    unsigned char val = getC();
    int bit, position;
    position = pow(2, r_pos);
    bit =((int)val & position);
    bit = bit >> r_pos;
    //cout << "Bit read = "<< bit << endl;
    r_pos++;

    if(r_pos > 7){
        //cout << "Acabei as contagens" << endl;
        r_pos = 0;
        cr_pos++;
        //cout << "Read buffer = " <<read_buffer << endl;
        //write on file, do that on read bit on readBit
    }
    return bit;
}

unsigned char* BitStream :: readNbits (unsigned int N, unsigned char* bits){

    for(int i = 0; i<N;i++){
        if(i>len*8){
            cout << "ERROR = This file don't have more information"<< endl;
            break;
        }

        //cout << "Buffer position = " <<(int)r_pos << endl;

        bits[i]=readBit();
        //cout << "Bit read = "<< readBit() << endl;
        //cout << "Read buffer = " << buffer << endl;
        //r_pos = r_pos<<1;
    }
    return bits;
}

void BitStream :: writeBit(unsigned char bit){

    result = (result | ((int)bit << w_pos));
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

void BitStream :: writeNbits(unsigned  int N, unsigned char* bits){
    for(int i = 0 ; i<N; i++){
        writeBit(bits[i]);
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