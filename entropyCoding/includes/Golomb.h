#ifndef ENTROPYCODING_GOLOMB_H
#define ENTROPYCODING_GOLOMB_H

#include "BitStream.hpp"
#include <cmath>
#include <vector>
#include <iostream>
#include <tuple>

//!  Golomb
/*!
 *  This class is useful to encode and decode with the Golomb Code.
 *  @author Inês Justo
*/
class Golomb {

private:

    //! Golomb integer parameter.
    unsigned int m;

    //! BitStream used for reading the decoded values.
    BitStream *readBitStream;

    //! BitStream used for writing the encoded values.
    BitStream *writeBitStream;

public:

    //! Golomb constructor for encoding and decoding.
    /*!
     * @param _m Golomb integer parameter.
     * @param _encodeFile Path to the file where the values to decode are read from.
     * @param _decodeFile Path to the file where the encoded values are written.
    */
    Golomb(unsigned int _m, char *_encodeFile, char *_decodeFile);


    //! Golomb constructor for only encoding or only decoding.
    /*!
     * @param _m Golomb integer parameter.
     * @param _file Path to the file used to get the values for the operation performed.
     * @param mode encode or decode ("e" or "d")
    */
    Golomb(unsigned int _m, char *_file, char mode);

    //! Golomb class destructor.
    ~Golomb();

    //! Encodes and writes with the bitStream.
    /*!
     * @param n number to encode.
     * @returns vector of bool values representing 0s and 1s of the coded number.
    */
    vector<bool> encode(int n);

    //! Reads with the bitStream and decodes.
    /*!
     * @param numbers pointer to vector of decoded numbers.
    */
    void decode(vector<int> *numbers);

    void setEncodeFile(char *_encodeFile);

    void setDecodeFile(char *_decodeFile);

    void closeEncodeFile();
};

#endif //ENTROPYCODING_GOLOMB_H
