#ifndef VIDEOCODING_HUFFMANCODE_HPP
#define VIDEOCODING_HUFFMANCODE_HPP

//!  HuffmanCode
/*!
 *
 *  @author Inês Justo
*/
class HuffmanCode {
private:


public:
    //! HuffmanCode constructor.
    /*!
     * @param frame
    */
    explicit HuffmanCode();

    void encode();

    void decode();
};


#endif //VIDEOCODING_HUFFMANCODE_HPP
