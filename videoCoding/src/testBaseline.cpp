//
// Created by inesjusto on 14/01/21.
//

#include    "QuantizationJPEG.cpp"
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


int main(int argc, char *argv[]) {

    float	in[15][8]=	{{144,139,149,155,153,155,155,155},
                           {151,151,151,159,156,156,156,158},
                           {151,156,160,162,159,151,151,151},
                           {158,163,161,160,160,160,160,161},
                           {158,160,161,162,160,155,155,156},
                           {161,161,161,161,160,157,157,157},
                           {162,162,161,160,161,157,157,157},
                           {162,162,161,160,163,157,158,154},

                           {144,139,149,155,153,155,155,155},
                           {151,151,151,159,156,156,156,158},
                           {151,156,160,162,159,151,151,151},
                           {158,163,161,160,160,160,160,161},
                           {158,160,161,162,160,155,155,156},
                           {161,161,161,161,160,157,157,157},
                           {162,162,161,160,161,157,157,157}};
    cv::Mat frame = cv::Mat::ones(15, 8, CV_64F);
    for(int r = 0; r < 15; r++){
        for(int c = 0; c < 8; c++){
            frame.at<double>(r,c) = in[r][c];
        }
    }
    quantizeDctBaselineJPEG(frame, true);
    return 0;
}