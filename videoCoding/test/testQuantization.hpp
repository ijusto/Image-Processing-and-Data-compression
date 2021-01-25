//
// Created by inesjusto on 25/01/21.
//

#ifndef VIDEOCODING_TESTQUANTIZATION_HPP
#define VIDEOCODING_TESTQUANTIZATION_HPP

#include    "../src/QuantizationJPEG.cpp"
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

double X [8][8] = {{183,160,94,153,194,163,132,165},
                   {183,153,116,176,187,166,130,169},
                   {179,168,171,182,179,170,131,167},
                   {177,177,179,177,179,165,131,167},
                   {178,178,179,176,182,164,130,171},
                   {179,180,180,179,183,169,132,169},
                   {179,179,180,182,183,170,129,173},
                   {180,179,181,179,181,170,130,169}};

double Y [8][8] = {{313,56,-27,18,78,-60,27,-27},
                   {-38,-27,13,44,32,-1,-24,-10},
                   {-20,-17,10,33,21,-6,-16,-9},
                   {-10,-8,9,17,9,-10,-13,1},
                   {-6,1,6,4,-3,-7,-5,5},
                   {2,3,0,-3,-7,-4,0,3},
                   {4,4,-1,-2,-9,0,2,4},
                   {3,1,0,4,-2,-1,3,1}};

double final_Y [8][8] = {{20,5,-3,1,3,-2,1,0},
                         {-3,-2,1,2,1,0,0,0},
                         {-1,-1,1,1,1,0,0,0},
                         {-1,0,0,1,0,0,0,0},
                         {0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0}};

TEST_CASE("Quantization divideImageIn8x8Blocks") {

    double in[6][7] = {{183,160,94,153,194,163,132},
                       {183,153,116,176,187,166,130},
                       {179,168,171,182,179,170,131},
                       {177,177,179,177,179,165,131},
                       {178,178,179,176,182,164,130},
                       {179,180,180,179,183,169,132}};
    cv::Mat inFrame = cv::Mat::ones(6, 7, CV_64F);
    for(int r = 0; r < 6; r++){
        for(int c = 0; c < 7; c++){
            inFrame.at<double>(r,c) = in[r][c];
        }
    }
    divideImageIn8x8Blocks(inFrame);
    INFO(inFrame);


    double out[8][8] = {{183,160,94,153,194,163,132,132},
                        {183,153,116,176,187,166,130,130},
                        {179,168,171,182,179,170,131,131},
                        {177,177,179,177,179,165,131,131},
                        {178,178,179,176,182,164,130,130},
                        {179,180,180,179,183,169,132,132},
                        {179,180,180,179,183,169,132,132},
                        {179,180,180,179,183,169,132,132}};
    cv::Mat outFrame = cv::Mat::ones(8, 8, CV_64F);
    for(int r = 0; r < 8; r++){
        for(int c = 0; c < 8; c++){
            outFrame.at<double>(r,c) = out[r][c];
        }
    }
    cv::Mat diff = outFrame != inFrame;
    CHECK(std::equal(outFrame.begin<double>(), outFrame.end<double>(), inFrame.begin<double>()));
}

TEST_CASE("Applying the DCT to the block") {


}


TEST_CASE("Applying the quantization matrix") {


}
#endif //VIDEOCODING_TESTQUANTIZATION_HPP
