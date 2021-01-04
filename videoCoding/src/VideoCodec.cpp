//
// Created by inesjusto on 10/12/20.
//

#include    "../../audioAndImageOrVideoManipulation/src/EntropyCalculator.cpp"
#include    "VideoCodec.hpp"
#include    "VideoReader.hpp"

//! VideoCodec constructor.
/*!
 * @param srcFileName source video file name/path.
 * @param destFileName destination video file name/path.
*/
VideoCodec::VideoCodec(char* srcFileName, char* destFileName) {
    VideoReader *videoReader;
    try{
        videoReader = new VideoReader(srcFileName);
    } catch (char* msg){
        std::cout << msg << std::endl;
        std::exit(0);
    }

    cv::Mat firstFrame = videoReader->getCurrFrame();

    for(int i = 0; i < firstFrame.rows; i++){
        for(int j = 0; j < firstFrame.cols; j++){
            // TODO: deal with pixel info
            firstFrame.at<cv::Vec3b>(i,j);
        }
    }

    cv::Mat frame;

    while(true){
        try{
            frame = videoReader->readFrame();
        } catch (char* msg){
            std::cout << msg << std::endl;
            break;
        }

        for(int i = 0; i < frame.rows; i++){
            for(int j = 0; j < frame.cols; j++){
                // TODO: deal with pixel info
                frame.at<cv::Vec3b>(i,j);
            }
        }
    }
}