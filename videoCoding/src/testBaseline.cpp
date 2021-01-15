//
// Created by inesjusto on 14/01/21.
//

#include    "BaselineJPEG.cpp"
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


int main(int argc, char *argv[]) {
    cv::Mat frame = cv::Mat::ones(15, 8, CV_64F);
    //quantizeDct(frame, true);
    return 0;
}