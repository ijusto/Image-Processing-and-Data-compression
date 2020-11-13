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
 *  This class is useful to creat a bitstream to right and read bits on a file
 */

class BitStream{
private:
    vector<unsigned char> info_file, new_file;
    unsigned char  buffer;
    int cr_pos ,cw_pos , len,r_pos ,w_pos,result;

public:
    //! A constructor.
    /*!
     * This class constructor have the propose of read the entire file and save on a vector structure
     *  @param file_r is the pointer of the path where the file to read is located
     *  @warning need to be an existing file
     *  @see writeOnfile(int N, char* file_w), getC(void)
     */
    explicit BitStream(char *file_r);
    //! Read 1 bit
    /*!
     *  This class function have the propose of read just 1 bit
     *  @return returns a char that contain one 1 bit read
     *  @see readNbits(int N, unsigned char* bits)
    */
    unsigned char readBit(void);
    //! Read N bit
    /*!
     * This class function have the propose of read N bits
     * @param N number of bits to read
     * @param bits pointer for the buffer to return the N bits
     * @return returns a char that contain one 1 bit read
     * @see readBit()
    */
    unsigned char* readNbits(int N, unsigned char* bits);
    //! Write 1 bit
    /*!
     * This class function have the propose of write just 1 bit
     * @param b the bit to write
     * @see writeNbits(int N, unsigned char* bits)
    */
    void writeBit(unsigned char b);
    //! Write N bits
    /*!
     * This class function have the propose of write N bits
     * @param N number of bits to write
     * @param bits pointer for the buffer of N bits to write
     * @see writeNbits(int N, unsigned char* bits)
    */
    void writeNbits(int N,unsigned char* bits);
    //! Get a unsigned char
    /*!
     * This class function have the propose of get a unsigned char from the vector
     * @return returns a char that contain one 8 bits a byte
     * @see  BitStream(char *file_r), writeOnfile(int N, char* file_w)
    */
    unsigned char getC(void);
    //! Write on File
    /*!
     * This class function have the propose write the vector of bits on the file
     * @param  file_w is the pointer of the path where the file to write is located or will be created
     * @see  BitStream(char *file_r), getC(void)
    */
    void writeOnfile(char* file_w);

};

#endif //ENTROPYCODING_BITSTREAM_HPP
