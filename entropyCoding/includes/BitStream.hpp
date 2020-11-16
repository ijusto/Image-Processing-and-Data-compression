//
// Created by iotmaster on 11/11/20.
//

#ifndef ENTROPYCODING_BITSTREAM_HPP
#define ENTROPYCODING_BITSTREAM_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

using namespace std;

//! BitStream interfaces
/*!
 *  @author Agostinho Pires.
 *  @author Inês Justo. (modified some logic of returns)
 *  This class is useful to creat a bitstream to right and read bits on a file
 */

class BitStream{
private:
    vector<unsigned char> readFileInfo, writeFileInfo;
    unsigned char  buffer;
    int cr_pos, cw_pos, len,r_pos ,w_pos,result;
    ofstream outfile;

public:
    //! A constructor.
    /*!
     * This class constructor have the propose of read the entire file and save on a vector structure
     *  @param file is the pointer of the path where the file is located
     *  @param mode read or write ("r" or "w")
     *  @warning need to be an existing file
     *  @see writeOnfile(int N, char* file_w), getC(void)
     */
    explicit BitStream(char *file, char mode);

    //! BitStream class destructor.
    ~BitStream();

    //! Read 1 bit
    /*!
        This class function has the propose of read just 1 bit
        @return returns a boolean that represents the bit read
        @see readNbits(int N)
    */
    bool readBit();

    //! Read N bit
    /*!
        This class function has the propose of read N bits
        @param N number of bits to read
        @return returns a vector of booleans representing the n bits read
        @see readBit()
    */
    vector<bool> readNbits(unsigned int N);

    //! Write 1 bit
    /*!
        This class function has the propose of writing just 1 bit
        @param b boolean representing the value of the bit to write
        @see writeNbits(vector<bool> bits)
    */
    void writeBit(bool b);

    //! Write N bits
    /*!
        This class function has the propose of writing more than one bit at once
        @param bits vector of booleans representing the bits to write
        @see writeBit(bool b)
    */
    void writeNbits(vector<bool> bits);

    //! Get a unsigned char
    /*!
     * This class function have the propose of get a unsigned char from the vector
     * @param mode read or write ("r" or "w")
     * @return returns a char that contain one 8 bits a byte
     * @see  BitStream(char *file_r), writeOnfile(int N, char* file_w)
    */
    unsigned char getC(char mode);

    void endWriteFile();
};

#endif //ENTROPYCODING_BITSTREAM_HPP
