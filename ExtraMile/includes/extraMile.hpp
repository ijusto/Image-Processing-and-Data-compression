//
// Created by irocs on 2/1/21.
//

#ifndef VIDEOCODING_EXTRAMILE_HPP
#define VIDEOCODING_EXTRAMILE_HPP

#include    "BitStream.hpp"
#include    <sndfile.hh>
#include    <vector>
#include    "Golomb.h"
#include    <opencv2/opencv.hpp>
#include    "LosslessJPEGPredictors.hpp"
#include    <math.h>
#include    <fstream>
#include    <regex>
#include    <algorithm>

class AudioDecoder {
private:
    vector<short> decodedRes;
    BitStream *sourceFile;
    unsigned int headerSize;
    int initial_m;
    int format;
    int channels;
    int samplerate;
    int frames;
    bool lossless;
    unsigned int quantBits;

    /**
     * Asserts that vector contains bits stored as least significant bit at the biggest address.
     * @param vec
     * @return
     */
    static int boolvec2int(vector<bool> vec);

public:

    /**
     * Constructor
     * @param filename
     */
    AudioDecoder(char* filename);

    /**
     * Decodes residuals for each channel.
     */
    void decode();

    /**
     * Use SndfileHandle to write decoded residuals to wav file.
     * @param filename
     */
    void write(char* filename);


};

using namespace std;

class AudioEncoder {
private:
    SndfileHandle sourceFile;
    vector<bool> encodedRes;
    int initial_m;
    bool lossless;
    unsigned int quantBits;
    bool calcHistogram;
    vector<short> leftResiduals;
    vector<short> rightResiduals;
    vector<short> leftSamples;
    vector<short> rightSamples;

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
     * @param calcHist store samples and residuals
     */
    AudioEncoder(char* filename, int m, bool ll, unsigned int qBits, bool calcHist);

    /**
     * Computes residuals for each channel using predictor.
     */
    void encode();

    /**
     * Use BitStream to write Golomb encoded residuals to file.
     * @param filename
     */
    void write(char* filename);

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
};

class VideoEncoder{

private:
    int initial_m, subsampling, predictor, totalFrames, rows, cols;
    int fps1, fps2;
    bool mode, lossy, calcHist;
    std::vector<bool> encodedRes;
    vector<vector<char>> *res_hists;
    vector<vector<char>> *sample_hists;

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

    void getResAndUpdate(vector<bool> &data, unsigned int *indexPtr, int n_residuals, Golomb *golomb, int m_rate, vector<int> &outRes);

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

    void decodeRes_inter(cv::Mat &prev_frame, vector<int> &currFrameResiduals, int grid_h, int grid_w, vector<uchar> &outPlanarValues, int block_size, int search_size);

    //! Write decoded video
    /*!
     * @param destFileName destination decoded video bitstream file name/path.
     */
    void write(char* fileName);

};
#endif //VIDEOCODING_EXTRAMILE_HPP
