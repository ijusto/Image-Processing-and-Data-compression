
//! BitStream functions
/*!
 *  @author Agostinho Pires.
 *  This file have the code of the functions
 */

#include "../includes/BitStream.hpp"

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
    bit =((int)val-48 & position);
    bit = bit >> r_pos;
    r_pos++;

    if(r_pos > 7){
        r_pos = 0;
        cr_pos++;
    }

    return bit+48;
}

unsigned char* BitStream :: readNbits (int N, unsigned char* bits){

    for(int i = 0; i<N;i++){
        if(i>len*8){
            cout << "ERROR = This file don't have more information"<< endl;
            break;
        }
        bits[i]=readBit();
    }
    return bits;
}

void BitStream :: writeBit(unsigned char bit){

    result = (result | ((int)bit-48) << w_pos);
    w_pos++;

    buffer = result+48;

    if(w_pos >7){
        new_file.push_back(buffer);
        result = 0;
        w_pos = 0;
        cw_pos++;
    }
}

void BitStream :: writeNbits(int N, unsigned char* bits){
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