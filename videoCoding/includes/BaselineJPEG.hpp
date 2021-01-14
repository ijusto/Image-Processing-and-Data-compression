#ifndef VIDEOCODING_BASELINEJPEG_HPP
#define VIDEOCODING_BASELINEJPEG_HPP

//!  BaselineJPEG
/*!
 *
 *  @author Inês Justo
*/
class BaselineJPEG {

private:

    // frame

public:
    //! BaselineJPEG constructor.
    /*!
     * @param frame
    */
    explicit BaselineJPEG();

    void calculateDCT();

    void quantizeDCTCoefficients();
};


#endif //VIDEOCODING_BASELINEJPEG_HPP
