#ifndef ENTROPYCODING_GOLOMB_H
#define ENTROPYCODING_GOLOMB_H

#include "BitStream.hpp"

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
    static unsigned char encodeUnary(unsigned int q);
    tuple<unsigned char *, unsigned int> encodeTruncatedBinary(unsigned int r);
    unsigned int decodeUnary();
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
