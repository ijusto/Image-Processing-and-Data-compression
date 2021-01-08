//
// Created by inesjusto on 10/12/20.
//

#ifndef VIDEOCODING_VIDEOCODEC_HPP
#define VIDEOCODING_VIDEOCODEC_HPP

class VideoCodec{

private:
    cv::Mat frame;
    cv :: Mat residuals1;
    cv :: Mat residuals2;
    cv :: Mat residuals3;
    cv :: Mat residuals4;
    cv :: Mat residuals5;
    cv :: Mat residuals6;
    cv :: Mat residuals7;
    cv :: Mat residualsJLS;

public:
    //! VideoCodec constructor.
    /*!
     * @param srcFileName source video file name/path.
     * @param destFileName destination video file name/path.
    */
    explicit VideoCodec(char* srcFileName, char* destFileName);
};

#endif //VIDEOCODING_VIDEOCODEC_HPP
