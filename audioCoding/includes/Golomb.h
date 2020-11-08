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
    /* TODO: put here methods for unary and truncated binary codes handling */

public:

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
