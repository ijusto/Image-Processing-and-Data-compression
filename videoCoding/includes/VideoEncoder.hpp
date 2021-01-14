//
// Created by inesjusto on 10/12/20.
//

#ifndef VIDEOCODING_VIDEOENCODER_HPP
#define VIDEOCODING_VIDEOENCODER_HPP

#include    "../../entropyCoding/includes/Golomb.h"
#include    <opencv2/opencv.hpp>

class VideoEncoder{

private:
    int initial_m, subsampling, mode, predictor, rows, cols;
    int fps1, fps2;
    std::vector<bool> encodedRes;
    bool cHist;
    vector<vector<char>> *res_hists;
    vector<vector<char>> *sample_hists;

    /**
     * Converts integer to bool vector representing bits.
     * @param n
     * @return
     */
    static std::vector<bool> int2boolvec(int n);

public:
    //! VideoEncoder constructor.
    /*!
     * @param srcFileName source video file name/path.
     * @param predictor JPEG linear predictors [1,7] or non linear predictor 8
     * @param mode intra (0) or hybrid (1)
     * @param initial m used in Golomb encoder
    */
    explicit VideoEncoder(char* srcFileName, int predictor, int mode, int init_m, bool calcHist);

    /**
     * Use linear and non-linear predictors to compute residuals, encode them using the
     * Golomb encoder and add them to the bitstream.
     * @param frame matrix containing single channel values.
     * @param golomb Golomb entropy encoder
     * @param m_rate rate at which Golomb entropy encoder's m parameter is updated
     * @param k used to identify channel when storing residuals for histograms
     */
    void encodeRes_intra(cv::Mat &frame, Golomb *golomb, int m_rate, int k);

    /**
     * Use motion compensation to compute residuals, encode them and the
     * corresponding motion vectors using the Golomb encoder and add them
     * to the bitstream.
     * @param prev_frame matrix containing single channel values from previous frame.
     * @param curr_frame matrix containing single channel values from current frame.
     * @param golomb Golomb entropy encoder
     * @param m_rate rate at which Golomb entropy encoder's m parameter is updated
     * @param block_size dimension of square frame block block_size*block_size
     * @param search_size dimension of square search area search_size*search_size
     * @param k used to identify channel when storing residuals for histograms
     */
    void encodeRes_inter(cv::Mat &prev_frame, cv::Mat &curr_frame, Golomb *golomb, int m_rate, int block_size, int search_size,int k);

    /**
     * Use BitStream to write Golomb encoded residuals to file.
     * @param filename
     */
    void write(char* filename);

    /**
     * Returns residuals from multiple channels computed during encoding
     * to use when producing a histogram.
     * @return
     */
    vector<vector<char>> get_res_hists();

    /**
     * Returns samples from multiple channels to use when producing a histogram.
     * @return
     */
    vector<vector<char>> get_sample_hists();
};

#endif //VIDEOCODING_VIDEOENCODER_HPP
