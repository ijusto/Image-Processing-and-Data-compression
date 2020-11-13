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

    //! Flag to know if use the bitStream or not.
    bool useBS;

    //! Golomb integer parameter.
    unsigned int m;

    //! BitStream used for reading or writing the encoded or the decoded values.
    BitStream bitStream;

    //! Encodes the quotient of the to be coded number, that is, the comma code (unary) part of the Golomb code.
    /*!
     * @param q quotient of the to be coded number by m.
     * @return vector of bool values representing '0's and '1's of the coded quotient.
    */
    static vector<bool> encodeUnary(unsigned int q);

    //! Encodes the remainder of the division of the to be coded number by m, that is, the truncated binary code part of
    //! the Golomb code.
    /*!
     * @param remainder of the division of the coded number by m.
     * @returns vector of bool values representing '0's and '1's of the truncated binary code.
    */
    vector<bool> encodeTruncatedBinary(unsigned int r);

    //! Decodes the quotient of the coded number, that is, the comma code (unary) part of the Golomb code.
    /*!
     * @return quotient of the coded number by m.
    */
    unsigned int decodeUnary();

    //! Decodes the remainder of the division of the coded number by m, that is, the truncated binary code part of the
    //! Golomb code.
    /*!
     * @return remainder of the division of the coded number by m.
    */
    unsigned int decodeTruncatedBinary();

public:

    //! Golomb constructor.
    /*!
     * @param _m Golomb integer parameter.
     * @param _bitStream BitStream used for reading or writing the encoded or the decoded values.
    */
    Golomb(unsigned int _m, BitStream _bitStream);

    //! Golomb constructor.
    /*!
     * @param _m Golomb integer parameter.
    */
    Golomb(unsigned int _m);

    //! Golomb class destructor.
    ~Golomb();

    //! Encodes and writes with the bitStream.
    /*!
     * @param n number to encode.
     * @returns vector of bool values representing '0's and '1's of the coded number.
    */
    vector<bool> encode(int n);

    //! Reads with the bitStream and decodes.
    /*!
     * @return Decoded number.
    */
    int decode();

    //! Decodes the coded number passed as argument as a vector of booleans.
    /*!
     * @param encoded_n vector of bool values representing '0's and '1's of all the coded numbers.
     * @return vector of the decoded numbers.
    */
    vector<int> decode(vector<bool> encoded_n);

    //! Changes the Golomb integer parameter.
    /*!
     * @param _m Golomb integer parameter.
    */
    void changeM(unsigned int _m);

};

#endif //ENTROPYCODING_GOLOMB_H
