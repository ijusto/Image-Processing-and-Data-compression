#ifndef VIDEOCODING_VIDEODECODER_HPP
#define VIDEOCODING_VIDEODECODER_HPP

//!  VideoDecoder
/*!
 *  This class is useful to decode a video.
 *  @author Agostinho Pires
 *  @author Bruno Pereira
 *  @author Inês Justo
*/

#include "../../entropyCoding/includes/Golomb.h"
#include    "LosslessJPEGPredictors.hpp"
#include    <opencv2/opencv.hpp>
#include    <vector>

class VideoDecoder{

private:
    //!
    BitStream *sourceFile;

    //!
    unsigned int headerSize;

    //!
    char *dst;

    //!
    int initial_m;

    //!
    int predictor;

    //!
    int subsampling;

    //!
    int mode;

    //!
    int fps1;

    //!
    int fps2;

    //!
    int totalFrames;

    //!
    int rows;

    //!
    int cols;

    //!
    vector<vector<uchar>> frames;

    //! Asserts that vector contains bits stored as least significant bit at the biggest address.
    /*!
     * @param vec
     * @return
     */
    static int boolvec2int(vector<bool> vec);

public:
    //! VideoDecoder constructor.
    /*!
     * @param encodedFileName encoded video bitstream file name/path.
     */
    explicit VideoDecoder(char* encodedFileName);

    //! Decodes residuals for each channel.
    void decode();

    //!
    /*!
     * @param
     * @param
     * @param
     */
    void update_m(vector<int> residuals, Golomb *golomb, int m_rate);

    //!
    /*!
     * @param
     * @param
     * @param
     * @param
     * @param
     * @param
     */
    void decodeRes_intra(vector<int> &residualValues, vector<uchar> &planarValues, int f_rows, int f_cols);

    //! Write decoded video
    /*!
     * @param destFileName destination decoded video bitstream file name/path.
     */
    void write(char* fileName);

};

#endif //VIDEOCODING_VIDEODECODER_HPP

