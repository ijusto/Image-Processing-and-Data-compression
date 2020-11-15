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

    //! BitStream used for reading or writing the encoded or the decoded values.
    BitStream bitStream;

public:

    //! Golomb constructor.
    /*!
     * @param _m Golomb integer parameter.
     * @param _bitStream BitStream used for reading or writing the encoded or the decoded values.
    */
    Golomb(unsigned int _m, BitStream _bitStream);

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
     * @return vector of decoded numbers.
    */
    vector<int> decode();

};

#endif //ENTROPYCODING_GOLOMB_H
