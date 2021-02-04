#ifndef VIDEOCODING_VIDEODECODER_HPP
#define VIDEOCODING_VIDEODECODER_HPP

//!  VideoDecoder
/*!
 *  This class is useful to decode a video.
 *  @author Agostinho Pires
 *  @author Bruno Pereira
 *  @author Inês Justo
*/

#include    "Golomb.h"
#include    "LosslessJPEGPredictors.hpp"
#include    "JPEGQuantization.hpp"
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

    //! 0 - lossless, 1 - lossy
    int lossy;

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

    //! used for quantization in lossy mode
    std::vector<std::vector<int>> prevDCs; // y, u, v
    JPEGQuantization* quantization;

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

    /*!
     *
     * @param data
     * @param indexPtr
     * @param n_residuals
     * @param golomb
     * @param m_rate
     * @param outRes
     */
    void getResAndUpdate(int n_residuals, Golomb *golomb, int m_rate, vector<int> &outRes);

    /*!
     *
     * @param data
     * @param indexPtr
     * @param n_residuals
     * @param golomb
     * @param m_rate
     * @param outRes
     * @param f_rows
     * @param f_cols
     * @param channel
     */
    void getResAndUpdate(Golomb *golomb, vector<int> &outRes, int f_rows, int f_cols, int channel);
        //!
    /*!
     * @param
     * @param
     * @param
     * @param
     * @param
     * @param
     */
    void decodeRes_intra(vector<int> &residualValues, vector<uchar> &outPlanarValues, int f_rows, int f_cols);

    /*!
     *
     * @param prev_frame
     * @param currFrameResiduals
     * @param grid_h
     * @param grid_w
     * @param outPlanarValues
     * @param block_size
     * @param search_size
     */
    void decodeRes_inter(Mat &prev_frame, vector<int> &currFrameResiduals, int grid_h, int grid_w, vector<uchar> &outPlanarValues, int block_size, int search_size);

    //! Write decoded video
    /*!
     * @param destFileName destination decoded video bitstream file name/path.
     */
    void write(char* fileName);

};

#endif //VIDEOCODING_VIDEODECODER_HPP

