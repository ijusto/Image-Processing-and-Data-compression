#ifndef AUDIOCODING_GOLOMB_H
#define AUDIOCODING_GOLOMB_H

//!  Golomb
/*!
 *  This class is useful to encode and decode with the Golomb Code.
 *  @author Inês Justo
 *  @author ...
*/
class Golomb {

private:
    unsigned int m;
    void encodeUnary(); /* TODO: change return type if necessary */
    void encodeTruncatedBinary(); /* TODO: change return type if necessary */
    void decodeUnary(); /* TODO: change return type if necessary */
    void decodeTruncatedBinary(); /* TODO: change return type if necessary */

public:

    Golomb(unsigned int _m);

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
    unsigned uDecode(unsigned int q, unsigned int r);

    //! Signed decode.
    /*!
     * @param n
    */
    int sDecode(unsigned int q, unsigned int r);
}

#endif //AUDIOCODING_GOLOMB_H
