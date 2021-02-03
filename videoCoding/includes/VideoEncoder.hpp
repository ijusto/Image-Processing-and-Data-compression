#ifndef VIDEOCODING_VIDEOENCODER_HPP
#define VIDEOCODING_VIDEOENCODER_HPP

//!  VideoEncoder
/*!
 *  This class is useful to encode a video.
 *  @author Agostinho Pires
 *  @author Bruno Pereira
 *  @author Inês Justo
*/

#include    "Golomb.h"
#include    <opencv2/opencv.hpp>
#include    "../includes/JPEGQuantization.hpp"

class VideoEncoder{

private:
    int initial_m, subsampling, predictor, totalFrames, rows, cols;
    int fps1, fps2;
    bool mode, lossy, calcHist;
    std::vector<bool> encodedRes;
    vector<vector<char>> *res_hists;
    vector<vector<char>> *sample_hists;
    //! used for quantization in lossy mode
    cv::Mat last_res;
    std::vector<std::vector<int>> prevDCs; // y, u, v
    JPEGQuantization* quantization;

    //! Converts integer to bool vector representing bits.
    /*!
     * @param n
     * @return
     */
    static std::vector<bool> int2boolvec(int n);

public:
    //! VideoEncoder constructor.
    /*!
     * @param srcFileName source video file name/path.
     * @param predictor JPEG linear predictors [1,7] or non linear predictor 8
     * @param initial m used in Golomb encoder
     * @param mode intra (0) or hybrid (1)
     * @param lossy bool true to use quantization false otherwise
     * @param calchist save residuals to compute histogram
    */
    explicit VideoEncoder(char* srcFileName, int pred, int init_m, int mode, bool lossy, bool calcHist);

    //! Use linear and non-linear predictors to compute residuals, encode them using the Golomb encoder and add them to
    //! the bitstream.
    /*!
     * @param frame matrix containing single channel values.
     * @param golomb Golomb entropy encoder
     * @param m_rate rate at which Golomb entropy encoder's m parameter is updated
     * @param k used to identify channel when storing residuals for histograms
     */
    void encodeRes_intra(cv::Mat &frame, Golomb *golomb, int m_rate, int k);

    //! Use motion compensation to compute residuals, encode them and the corresponding motion vectors using the Golomb
    //! encoder and add them to the bitstream.
    /*!
     * @param p_frame matrix containing single channel values from previous frame.
     * @param curr_frame matrix containing single channel values from current frame.
     * @param golomb Golomb entropy encoder
     * @param m_rate rate at which Golomb entropy encoder's m parameter is updated
     * @param block_size dimension of square frame block block_size*block_size
     * @param search_size number of blocks from center to edges of square search area (square with sides = 2*search_size+1)
     */
    void encodeRes_inter(const cv::Mat &p_frame, const cv::Mat &curr_frame, Golomb *golomb, int m_rate, int block_size, int search_size);

    //! Compute residuals and MSE of two equally sized single channel block
    /*!
     * @param prev previous block matrix
     * @param curr current block matrix
     * @param outRes resulting block matrix from prev - curr (residuals)
     * @param outMSE resulting mean squared error computed using residuals
     */
    void submatsResiduals(const cv::Mat &prev, const cv::Mat &curr, cv::Mat &outRes, double &outMSE);

    //! Use BitStream to write Golomb encoded residuals to file.
    /*!
     * @param filename
     */
    void write(char* filename);

    //! Convert video format to the YUV 4:2:0 format.
    /*!
     * @param frame address in memory of first position pixel
     */
    void convertionTo420(cv::Mat &frame);

    //! Parse header of .y4m file
    /*!
     * @param header string containing header
     */
    void parseHeader(string &header);

    //! Returns residuals from multiple channels computed during encoding to use when producing a histogram.
    /*!
     * @return
     */
    vector<vector<char>> get_res_hists();

    //! Returns samples from multiple channels to use when producing a histogram.
    /*!
     * @return
     */
    vector<vector<char>> get_sample_hists();


};

#endif //VIDEOCODING_VIDEOENCODER_HPP
