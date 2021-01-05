//
// Created by inesjusto on 10/12/20.
//

#include    "../../audioAndImageOrVideoManipulation/src/EntropyCalculator.cpp"
#include    "VideoCodec.hpp"
#include    "VideoReader.hpp"
#include    "LosslessJPEGPredictors.cpp"

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
            // TODO: a, b, c Vec3b or compare rgb?
            //   uchar blue = firstFrame.at<cv::Vec3b>(...).val[0];
            //   uchar green = firstFrame.at<cv::Vec3b>(...).val[1];
            //   uchar red = firstFrame.at<cv::Vec3b>(...).val[2];

            LosslessJPEGPredictors<cv::Vec3b> predictors(
                    (j == 0 ? 0 : firstFrame.at<cv::Vec3b>(i - 1 , j)),
                    (i == 0 ? 0 : firstFrame.at<cv::Vec3b>(i, j - 1)),
                    ((i == 0 | j == 0) ? 0 : firstFrame.at<cv::Vec3b>(i - 1, j - 1)));

            cv::Vec3b predicted1 = predictors.usePredictor1();
            cv::Vec3b predicted2 = predictors.usePredictor2();
            cv::Vec3b predicted3 = predictors.usePredictor3();
            cv::Vec3b predicted4 = predictors.usePredictor4();
            cv::Vec3b predicted5 = predictors.usePredictor5();
            cv::Vec3b predicted6 = predictors.usePredictor6();
            cv::Vec3b predicted7 = predictors.usePredictor7();
            cv::Vec3b predictedJLS = predictors.usePredictorJLS();
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

                // TODO: a, b, c Vec3b or compare rgb?
                //   uchar blue = firstFrame.at<cv::Vec3b>(...).val[0];
                //   uchar green = firstFrame.at<cv::Vec3b>(...).val[1];
                //   uchar red = firstFrame.at<cv::Vec3b>(...).val[2];
                
                LosslessJPEGPredictors<cv::Vec3b> predictors(
                        (j == 0 ? 0 : frame.at<cv::Vec3b>(i - 1 , j)),
                        (i == 0 ? 0 : frame.at<cv::Vec3b>(i, j - 1)),
                        ((i == 0 | j == 0) ? 0 : frame.at<cv::Vec3b>(i - 1, j - 1)));

                cv::Vec3b predicted1 = predictors.usePredictor1();
                cv::Vec3b predicted2 = predictors.usePredictor2();
                cv::Vec3b predicted3 = predictors.usePredictor3();
                cv::Vec3b predicted4 = predictors.usePredictor4();
                cv::Vec3b predicted5 = predictors.usePredictor5();
                cv::Vec3b predicted6 = predictors.usePredictor6();
                cv::Vec3b predicted7 = predictors.usePredictor7();
                cv::Vec3b predictedJLS = predictors.usePredictorJLS();
            }
        }
    }
}