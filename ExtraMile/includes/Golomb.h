#ifndef EXTRAMILE_GOLOMB_H
#define EXTRAMILE_GOLOMB_H

#include "BitStream.hpp"
#include <cmath>
#include <vector>
#include <iostream>
#include <tuple>

//!  Golomb
/*!
 *  This class is useful to encodeAudio and decodeAudio with the Golomb Code.
 *  @author Inês Justo
*/
class Golomb {

private:

    //! Golomb integer parameter.
    unsigned int m;

    //! Mode on to operate: encodeAudio and/or decodeAudio ('e', 'd', 'b')
    char mode;

    //! BitStream used for reading the decoded values.
    BitStream *readBitStream;

    //! BitStream used for writing the encoded values.
    BitStream *writeBitStream;

public:
    //! Golomb constructor for encoding and decoding without writing to file
    /*!
     * @param _m Golomb integer parameter.
     */
    Golomb(unsigned int _m);

    //! Golomb constructor for encoding and decoding.
    /*!
     * @param _m Golomb integer parameter.
     * @param _encodeFile Path to the file where the values to decodeAudio are read from.
     * @param _decodeFile Path to the file where the encoded values are written.
    */
    Golomb(unsigned int _m, char *_encodeFile, char *_decodeFile);


    //! Golomb constructor for only encoding or only decoding.
    /*!
     * @param _m Golomb integer parameter.
     * @param _file Path to the file used to get the values for the operation performed.
     * @param mode encodeAudio or decodeAudio ("e" or "d")
    */
    Golomb(unsigned int _m, char *_file, char mode);

    //! Golomb class destructor.
    ~Golomb();

    /*!
     * Setter for m parameter.
     * @param _m Golomb integer parameter.
     */
    void setM(unsigned int _m);

    //! Encodes and writes with the bitStream.
    /*!
     * @param n number to encodeAudio.
     * @returns vector of bool values representing 0s and 1s of the coded number.
    */
    vector<bool> encode(int n);

    //! Encodes and returns bits.
    /*!
     * @param n number to encodeAudio.
     * @returns vector of bool values representing 0s and 1s of the coded number.
    */
    vector<bool> encode2(int n);

    //! Encodes and returns bits.
    /*!
     * @param n number to encodeAudio.
     * @param encoded_n encoded number as a vector of bits
    */
    void encode3(int n, vector<bool> &encoded_n);

    //! Reads with the bitStream and decodes.
    /*!
     * @param numbers pointer to vector of decoded numbers.
    */
    void decode(vector<int> *numbers);

    //! Reads bits of bool vector until it decodes a number and updates pointer.
    /*!
     * @param encodedBits vector containing bits to decodeAudio
     * @param index next bit pointer
     * @param count number of numbers to decodeAudio
     * @return
     */
    vector<int> decode2(vector<bool> encodedBits, unsigned int *index, unsigned int count);

    //! Reads bits of bool vector until it decodes a number and updates pointer.
    /*!
     * @param encodedBits vector containing bits to decode
     * @param decoded vector decoded integers
     * @param index next bit pointer
     * @param count number of numbers to decode
     * @return
     */
    void decode3(vector<bool> &encodedBits, vector<int> &decoded, unsigned int *index, unsigned int count);

    void setEncodeFile(char *_encodeFile);

    void setDecodeFile(char *_decodeFile);

    void closeEncodeFile();
};

#endif //EXTRAMILE_GOLOMB_H
