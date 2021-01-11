//
// Created by inesjusto on 10/12/20.
//

#ifndef VIDEOCODING_VIDEOENCODER_HPP
#define VIDEOCODING_VIDEOENCODER_HPP

#include    <opencv2/opencv.hpp>

class VideoEncoder{

private:
    int initial_m, subsampling, mode, predictor, rows, cols;
    std::vector<bool> encodedRes0;
    std::vector<bool> encodedRes1;
    std::vector<bool> encodedRes2;

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
    explicit VideoEncoder(char* srcFileName, int predictor, int mode, int init_m);
    /**
     * Use BitStream to write Golomb encoded residuals to file.
     * @param filename
     */
    void write(char* filename);
};

#endif //VIDEOCODING_VIDEOENCODER_HPP
