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
 *  @author ...
*/
class Golomb {

private:
    unsigned int m;
    BitStream bitStream;

    //! Encodes the quotient of the to be coded number, that is, the comma code (unary) part of the Golomb code.
    /*!
     * @param q quotient of the to be coded number by m.
     * @return quotient in unary code.
    */
    static unsigned char encodeUnary(unsigned int q);

    //! Encodes the remainder of the division of the to be coded number by m, that is, the truncated binary code part of
    //! the Golomb code.
    /*!
     * @param remainder of the division of the coded number by m.
     * @returns tuple with a pointer to the truncated binary code and its the number of bits.
    */
    tuple<unsigned char *, unsigned int> encodeTruncatedBinary(unsigned int r);

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

    //! Golomb class destructor.
    ~Golomb();

    //! Encode.
    /*!
     * @param n
    */
    void encode(int n);

    //! Decode.
    /*!
     * @param n
    */
    int decode();
};

#endif //ENTROPYCODING_GOLOMB_H
