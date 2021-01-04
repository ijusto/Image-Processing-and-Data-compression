#ifndef VIDEOCODING_VIDEOREADER_HPP
#define VIDEOCODING_VIDEOREADER_HPP

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

//!  Video Reader.
/*!
 *  This class is useful for reading, copying and extract info from video files.
 *  @author Inês Justo
*/
class VideoReader{

private:
    //!
    cv::VideoCapture cap;

    cv::Mat currFrame;

public:
    //! VideoReader constructor.
    /*!
     * @param sourceFileName video file name/path.
    */
    explicit VideoReader(char* sourceFileName);


    //!
    cv::Mat readFrame();

    //!
    cv::Mat getCurrFrame();

};

#endif //VIDEOCODING_VIDEOREADER_HPP