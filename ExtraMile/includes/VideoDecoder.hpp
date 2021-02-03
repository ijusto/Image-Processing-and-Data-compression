//
// Created by irocs on 2/3/21.
//

#ifndef EXTRAMILE_VIDEODECODER_HPP
#define EXTRAMILE_VIDEODECODER_HPP

#include    "BitStream.hpp"
#include    <sndfile.hh>
#include    <vector>
#include    "../includes/Golomb.h"
#include    "LosslessJPEGPredictors.hpp"
#include    <opencv2/opencv.hpp>
class VideoDecoder{
private:
    //Audio Decode

    BitStream *file;
    int initial_m;
    vector<short> decodedRes;
    unsigned int headerSize;
    int form;
    int channels;
    int sampleRate;
    int frames;
    bool lossless;
    unsigned int quantBits;

    //Video Decode
    BitStream *sourceFile;
    char *dst;
    int predictor;
    int subsampling;
    int mode;
    int fps1;
    int fps2;
    int totalFrames;
    int rows;
    int cols;
    vector<vector<uchar>> Videoframes;
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
    VideoDecoder(char* filename, char* encodedFileName, char ch);
    /**
    * Decodes audio residuals for each channel.
    */
    void decodeAudio();

    /**
     * Use SndfileHandle to writeVideoFile decoded residuals to wav file.
     * @param filename
     */
    void writeAudioFile(char*filename);
    //! Decodes residuals for each channel.
    void decodeVideo();

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

    void decodeRes_inter(Mat &prev_frame, vector<int> &currFrameResiduals, int grid_h, int grid_w, vector<uchar> &outPlanarValues, int block_size, int search_size);

    //! Write decoded video
    /*!
     * @param destFileName destination decoded video bitstream file name/path.
     */
    void writeVideoFile(char* fileName);
};

#endif //EXTRAMILE_VIDEODECODER_HPP
