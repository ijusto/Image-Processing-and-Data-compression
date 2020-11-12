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
    static unsigned char encodeUnary(unsigned int q); /* TODO: change return type if necessary */
    tuple<unsigned char *, unsigned int> encodeTruncatedBinary(unsigned int r); /* TODO: change return type if necessary */
    unsigned int decodeUnary(); /* TODO: change return type if necessary */
    unsigned int decodeTruncatedBinary(); /* TODO: change return type if necessary */

public:

    Golomb(unsigned int _m, BitStream _bitStream);

    ~Golomb();


    //! Unsigned encode.
    /*!
     * @param n
    */
    void uEncode(unsigned int n); /* TODO: return type ?? */

    //! Signed encode.
    /*!
     * @param n
    */
    void sEncode(int n); /* TODO: return type ?? */

    //! Unsigned decode.
    /*!
     * @param n
    */
    unsigned int uDecode();

    //! Signed decode.
    /*!
     * @param n
    */
    int sDecode();
};

#endif //ENTROPYCODING_GOLOMB_H
