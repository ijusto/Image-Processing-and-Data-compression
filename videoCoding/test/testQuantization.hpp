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

int X [8][8] = {{183,160,94,153,194,163,132,165},
                   {183,153,116,176,187,166,130,169},
                   {179,168,171,182,179,170,131,167},
                   {177,177,179,177,179,165,131,167},
                   {178,178,179,176,182,164,130,171},
                   {179,180,180,179,183,169,132,169},
                   {179,179,180,182,183,170,129,173},
                   {180,179,181,179,181,170,130,169}};

int Y [8][8] = {{313,56,-27,18,78,-60,27,-27},
                   {-38,-27,13,44,32,-1,-24,-10},
                   {-20,-17,10,33,21,-6,-16,-9},
                   {-10,-8,9,17,9,-10,-13,1},
                   {-6,1,6,4,-3,-7,-5,5},
                   {2,3,0,-3,-7,-4,0,3},
                   {4,4,-1,-2,-9,0,2,4},
                   {3,1,0,-4,-2,-1,3,1}};

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

    double out[8][8] = {{183,160,94,153,194,163,132,132},
                        {183,153,116,176,187,166,130,130},
                        {179,168,171,182,179,170,131,131},
                        {177,177,179,177,179,165,131,131},
                        {178,178,179,176,182,164,130,130},
                        {179,180,180,179,183,169,132,132},
                        {179,180,180,179,183,169,132,132},
                        {179,180,180,179,183,169,132,132}};

    cv::Mat inFrame = cv::Mat::ones(6, 7, CV_64F);
    cv::Mat outFrame = cv::Mat::ones(8, 8, CV_64F);

    for(int r = 0; r < 6; r++){
        for(int c = 0; c < 7; c++){
            inFrame.at<double>(r,c) = in[r][c];
        }
    }
    for(int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            outFrame.at<double>(r, c) = out[r][c];
        }
    }

    INFO("\ninput frame: \n");
    INFO(inFrame);
    divideImageIn8x8Blocks(inFrame);
    INFO("\noutFrame frame: \n");
    INFO(inFrame);
    cv::Mat diff = outFrame != inFrame;
    CHECK(std::equal(outFrame.begin<double>(), outFrame.end<double>(), inFrame.begin<double>()));
}

TEST_CASE("Applying the DCT to the block") {
    cv::Mat block = cv::Mat::ones(8, 8, CV_64F);
    cv::Mat Y_matrix = cv::Mat::ones(8, 8, CV_64F);
    for(int r = 0; r < 8; r++){
        for(int c = 0; c < 8; c++){
            block.at<double>(r,c) = (double) X[r][c];
            Y_matrix.at<double>(r, c) = Y[r][c];
        }
    }

    cv::Mat dct = applyingDCT2TheBlock(block);

    for(int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            dct.at<double>(r, c) = (int)(dct.at<double>(r, c) + 0.5 - (dct.at<double>(r, c)<0)); // https://stackoverflow.com/questions/9695329/c-how-to-round-a-double-to-an-int
        }
    }

    INFO("\ndct: \n");
    INFO(dct);
    INFO("\nY: \n");
    INFO(Y_matrix);
    CHECK(std::equal(Y_matrix.begin<double>(), Y_matrix.end<double>(), dct.begin<double>()));
}


TEST_CASE("Applying the quantization matrix") {


}
#endif //VIDEOCODING_TESTQUANTIZATION_HPP
