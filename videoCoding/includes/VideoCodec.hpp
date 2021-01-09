//
// Created by inesjusto on 10/12/20.
//

#ifndef VIDEOCODING_VIDEOCODEC_HPP
#define VIDEOCODING_VIDEOCODEC_HPP

class VideoCodec{

private:
    cv::Mat frame;
    cv :: Mat residuals;
    int initial_m;
    std::vector<bool> encodedRes0;
    std::vector<bool> encodedRes1;
    std::vector<bool> encodedRes2;

public:
    //! VideoCodec constructor.
    /*!
     * @param srcFileName source video file name/path.
     * @param destFileName destination video file name/path.
    */
    explicit VideoCodec(char* srcFileName, char* destFileName, std::string predictor);
};

#endif //VIDEOCODING_VIDEOCODEC_HPP
