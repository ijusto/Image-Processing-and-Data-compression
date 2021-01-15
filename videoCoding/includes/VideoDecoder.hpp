#ifndef VIDEOCODING_VIDEODECODER_HPP
#define VIDEOCODING_VIDEODECODER_HPP

//!  VideoDecoder
/*!
 *  This class is useful to ...
 *  @author Inês Justo
*/

#include "../../entropyCoding/includes/Golomb.h"
#include    "LosslessJPEGPredictors.hpp"
#include    <opencv2/opencv.hpp>
#include    <vector>

class VideoDecoder{

private:
    BitStream *sourceFile;
    unsigned int headerSize;
    char *dst;
    int initial_m;
    int predictor;
    int subsampling;
    int mode;
    int fps1;
    int fps2;
    int totalFrames;
    int rows;
    int cols;
    vector<vector<uchar>> frames;

    /**
     * Asserts that vector contains bits stored as least significant bit at the biggest address.
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

    /**
     * Decodes residuals for each channel.
     */
    void decode();

    void update_m(vector<int> residuals, Golomb *golomb, int m_rate);

    void decodeRes_intra(vector<int> &residualValues, vector<uchar> &planarValues, int f_rows, int f_cols, Golomb *golomb, int m_rate);

    /**
    * Write decodified video
    * @param destFileName destination decoded video bitstream file name/path.
    */
    void write(char* fileName);

};

#endif //VIDEOCODING_VIDEODECODER_HPP

