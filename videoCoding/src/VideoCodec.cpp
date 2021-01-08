//
// Created by inesjusto on 10/12/20.
//

#include    "../../audioAndImageOrVideoManipulation/src/EntropyCalculator.cpp"
#include    "VideoCodec.hpp"
#include    "VideoReader.hpp"
#include    "LosslessJPEGPredictors.cpp"
#include    <fstream>


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

    /*
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
    */

    // open video filest::ifstream video;
    std::ifstream video;
    video.open(srcFileName);

    if (!video.is_open()){
        std::cout << "Error opening file: " << srcFileName << std::endl;
        exit(EXIT_FAILURE);
    }

    // parse header
    std::string header;
    getline(video, header);
    std::cout << "header: " << header << std::endl;

    // read frame into this buffer
    unsigned char* frameData = new unsigned char[frame.rows*frame.cols*3];


    while(true){
        try{
            frame = videoReader->readFrame();
        } catch (char* msg){
            std::cout << msg << std::endl;
            break;
        }
        // skip word FRAME
        getline(video, header);
        // read frame data
        video.read((char *) frameData, frame.rows * frame.cols * 3);
        // check number of bytes read
        if (video.gcount() == 0)
            break;

        // ptr to mat's data buffer (to be filled with pixels in packed mode)
        uchar *buffer = (uchar *) frame.ptr();

        for (int i = 0; i < frame.rows * frame.cols; i++) {
            // get YUV components from data in planar mode
            int y, u, v;

            // 4:2:0 (based on https://en.wikipedia.org/wiki/File:Yuv420.svg)
            int ci = (i % frame.cols) / 2;   // x of every 2 cols
            int ri = i / (frame.cols * 2);     // y of every 2 rows
            int shift = (frame.cols / 2) * ri; // shift amount within U/V planar region
            y = frameData[i];
            u = frameData[(ci + shift) + (frame.rows * frame.cols)];
            v = frameData[(ci + shift) + (frame.rows * frame.cols) + (frame.rows * frame.cols) / 4];


            // write to OpenCV buffer in packed mode
            buffer[i*3 ] = y;
            buffer[i*3 + 1] = u;
            buffer[i*3 + 2] = v;
        }

        for(int i = 0; i < frame.rows; i++){
            for(int j = 0; j < frame.cols; j++){

                // TODO: a, b, c Vec3b or compare rgb?
                //   uchar blue = firstFrame.at<cv::Vec3b>(...).val[0];
                //   uchar green = firstFrame.at<cv::Vec3b>(...).val[1];
                //   uchar red = firstFrame.at<cv::Vec3b>(...).val[2];
                for(int k = 0; k < 3; k++){
                    LosslessJPEGPredictors<int> predictors(
                            (j == 0 ? 0 : frame.at<cv::Vec3b>(i,j-1).val[k]),
                            (i == 0 ? 0 : frame.at<cv::Vec3b>(i-1,j).val[k]),
                            ((i == 0 | j == 0) ? 0 : frame.at<cv::Vec3b>(i - 1, j - 1).val[k]));

                    residuals1.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor1();
                    residuals2.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor2();
                    residuals3.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor3();
                    residuals4.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor4();
                    residuals5.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor5();
                    residuals6.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor6();
                    residuals7.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor7();
                    residualsJLS.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictorJLS();
                }

            }

            // TODO: codify in golomb choosing best m
            //       write in a file, define de header
            //       Possible Header m, format, frame_size
        }
    }
}