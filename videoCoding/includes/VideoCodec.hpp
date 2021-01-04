//
// Created by inesjusto on 10/12/20.
//

#ifndef VIDEOCODING_VIDEOCODEC_HPP
#define VIDEOCODING_VIDEOCODEC_HPP

class VideoCodec{

private:


public:
    //! VideoCodec constructor.
    /*!
     * @param srcFileName source video file name/path.
     * @param destFileName destination video file name/path.
    */
    explicit VideoCodec(char* srcFileName, char* destFileName);
};

#endif //VIDEOCODING_VIDEOCODEC_HPP
