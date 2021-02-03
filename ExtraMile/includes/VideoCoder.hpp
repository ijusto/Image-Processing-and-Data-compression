//
// Created by irocs on 2/1/21.
//

#ifndef EXTRAMILE_HPP
#define EXTRAMILE_HPP

#include    <sndfile.hh>
#include    <vector>
#include    "Golomb.h"
#include    <opencv2/opencv.hpp>
#include    "LosslessJPEGPredictors.hpp"
#include    <math.h>
#include    <fstream>
#include    <regex>
#include    <algorithm>

class VideoCoder {
private:
    //Audio Encode
    SndfileHandle sourceFileAudio;
    string sourceFileVideo;
    vector<bool> encodedResAudio;
    int initial_m;
    bool lossless;
    unsigned int quantBits;
    bool calcHistogram;
    vector<short> leftResiduals;
    vector<short> rightResiduals;
    vector<short> leftSamples;
    vector<short> rightSamples;

    //Video Encode
    int subsampling, predictor, totalFrames, rows, cols;
    int fps1, fps2;
    bool mode, lossy, calcHist;
    std::vector<bool> encodedResVideo;
    vector<vector<char>> *res_hists;
    vector<vector<char>> *sample_hists;



    /**
     * Converts integer to bool vector representing bits.
     * @param n
     * @return
     */
    std::vector<bool> int2boolvec(int n);


public:

    /**
     * Constructor
     * @param filename
     * @param m, initial m golomb parameter
     * @param ll true is lossless, false is lossy
     * @param qBits number of bits used for quantization
     * @param srcFileName source video file name/path.
     * @param predictor JPEG linear predictors [1,7] or non linear predictor 8
     * @param mode intra (0) or hybrid (1)
     * @param lossy bool true to use quantization false otherwise

     */
    VideoCoder(char* filename, int m, bool ll, unsigned int qBits,char* srcFileName, int pred, int mode, bool lossy);

    /**
     * Computes audio residuals for each channel using predictor.
     */
    void encodeAudio();
    /**
    * Computes residuals for each channel using predictor.
    */
    void encodeVideo();

    /**
     * Use BitStream to writeAudio GolombVideo encoded residuals to file.
     * @param filename
     */
    void writeAudio(char* filename);
    /**
  * Use BitStream to writeVideo GolombVideo encoded residuals to file.
  * @param filename
  */
    void writeVideo(char* filename);

    /**
    * Returns the Residuals of the left channel
    */
    const vector<short> &getLeftResiduals() const;
    /**
    * Returns the Residuals of the Right channel
    */
    const vector<short> &getRightResiduals() const;
    /**
    * Returns the Samples of the Left channel
    */
    const vector<short> &getLeftSamples() const;
    /**
    * Returns the Samples of the Right channel
    */
    const vector<short> &getRightSamples() const;

    //! Use linear and non-linear predictors to compute residuals, encodeAudio them using the GolombVideo encoder and add them to
    //! the bitstream.
    /*!
     * @param frame matrix containing single channel values.
     * @param golomb GolombVideo entropy encoder
     * @param m_rate rate at which GolombVideo entropy encoder's m parameter is updated
     * @param k used to identify channel when storing residuals for histograms
     */
    void encodeRes_intra(cv::Mat &frame, Golomb *golomb, int m_rate, int k);

    //! Use motion compensation to compute residuals, encodeAudio them and the corresponding motion vectors using the GolombVideo
    //! encoder and add them to the bitstream.
    /*!
     * @param p_frame matrix containing single channel values from previous frame.
     * @param curr_frame matrix containing single channel values from current frame.
     * @param golomb GolombVideo entropy encoder
     * @param m_rate rate at which GolombVideo entropy encoder's m parameter is updated
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

    //! Convert video form to the YUV 4:2:0 form.
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


#endif //VIDEOCODING_EXTRAMILE_HPP
