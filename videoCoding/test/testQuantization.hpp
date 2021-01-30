//
// Created by inesjusto on 25/01/21.
//

#ifndef VIDEOCODING_TESTQUANTIZATION_HPP
#define VIDEOCODING_TESTQUANTIZATION_HPP

#include    "../src/QuantizationJPEG.cpp"
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>

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
                   {3,1,0,-4,-2,-1,3,1}};

double final_Y [8][8] = {{20,5,-3,1,3,-2,1,0},
                         {-3,-2,1,2,1,0,0,0},
                         {-1,-1,1,1,1,0,0,0},
                         {-1,0,0,1,0,0,0,0},
                         {0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0}};

std::vector<int> zigzag_array = {5,-3,-1,-2,-3,1,1,-1,-1,0,0,1,2,3,-2,1,1,0,0,0,0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//std::vector<bool> huffmanCode = {0,1,1,1,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,0,0,1,0,0,1,0,0,1,1,0,0,1,1,0,1,
//                                 1,1,1,1,0,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,0,0,1,0,1,1,1,1,1,1,1,1,
//                                 1,1,0,0,1,0,1,0,1,0};

std::vector<bool> huffmanCode = {0,1,1,1,1,1,0,0,1,1,1,0,0,1,0,0,1,1,0,0,1,1,1,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,1,1,1,1,0,
                                 0,1,1,1,1,1,1,0,1,1,0,0,0,0,0,1,1,0,0,0,0,1,1,1,0,0};

std::vector<std::pair<int, int>> acs = {{0,5}, {0, -3}, {0, -1}, {0, -2}, {0, -3},
                                        {0,1}, {0, 1}, {0, -1}, {0, -1}, {2, 1},
                                        {0, 2}, {0, 3}, {0, -2}, {0, 1}, {0, 1},
                                        {6,1}, {0, 1}, {1, 1}};

Node *huffmanTreeRoot;

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
    cv::Mat block = cv::Mat(8, 8, CV_64F, &X);
    cv::Mat Y_matrix = cv::Mat(8, 8, CV_64F, &Y);

    INFO("\nX: \n");
    INFO(block);
    INFO("\nY: \n");
    INFO(Y_matrix);

    dct(block);

    INFO("\ndct: \n");
    INFO(block);
    CHECK(std::equal(Y_matrix.begin<double>(), Y_matrix.end<double>(), block.begin<double>()));
}

TEST_CASE("Applying the inverse of the DCT") {
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
                       {3,1,0,-4,-2,-1,3,1}};

    cv::Mat X_matrix = cv::Mat(8, 8, CV_64F, &X);
    cv::Mat block = cv::Mat(8, 8, CV_64F, &Y);
    cv::Mat Y_matrix = cv::Mat(8, 8, CV_64F, &Y);

    INFO("\ndct: \n");
    INFO(Y_matrix);
    INFO("\nblock: \n");
    INFO(block);

    inverseDCT(block);

    INFO("\ninverseDCT: \n");
    INFO(block);

    INFO("\nX: \n");
    INFO(X_matrix);

    INFO("\nDifferences between X and inverse of DCT: \n");
    INFO(X_matrix-block);

    bool is_small_diff = cv::checkRange(X_matrix-block, true, nullptr, -2, 2);
    CHECK(is_small_diff);
}

TEST_CASE("Applying the quantization matrix") {
    double Y [8][8] = {{313,56,-27,18,78,-60,27,-27},
                       {-38,-27,13,44,32,-1,-24,-10},
                       {-20,-17,10,33,21,-6,-16,-9},
                       {-10,-8,9,17,9,-10,-13,1},
                       {-6,1,6,4,-3,-7,-5,5},
                       {2,3,0,-3,-7,-4,0,3},
                       {4,4,-1,-2,-9,0,2,4},
                       {3,1,0,-4,-2,-1,3,1}};
    cv::Mat block = cv::Mat(8, 8, CV_64F, &Y);
    cv::Mat finalY = cv::Mat(8, 8, CV_64F, &final_Y);

    INFO("\nY: \n");
    INFO(block);
    quantizeDCTCoeff(block, quantMatrixLuminance);
    INFO("\nQ: \n");
    INFO(quantMatrixLuminance);
    INFO("\nỸ: \n");
    INFO(finalY);
    INFO("\nresult: \n");
    INFO(block);
    CHECK(std::equal(block.begin<double>(), block.end<double>(), finalY.begin<double>()));
}

TEST_CASE("Removing the effect of the quantization matrix") {

    double Y [8][8] = {{313,56,-27,18,78,-60,27,-27},
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
    cv::Mat initialY = cv::Mat(8, 8, CV_64F, &Y);
    cv::Mat block = cv::Mat(8, 8, CV_64F, &final_Y);

    INFO("\nQ: \n");
    INFO(quantMatrixLuminance);
    INFO("\nỸ: \n");
    INFO(block);
    inverseQuantizeDCTCoeff(block, quantMatrixLuminance);
    INFO("\nY: \n");
    INFO(initialY);
    INFO("\nresult: \n");
    INFO(block);

    INFO("\nDifferences between Y and inverse of the quantization: \n");
    INFO(initialY-block);

    bool is_small_diff = cv::checkRange(initialY-block, true, nullptr, -30, 30);
    CHECK(is_small_diff);
}

TEST_CASE("DCT Quantization From Start To Finish"){

    double X [8][8] = {{183,160,94,153,194,163,132,165},
                       {183,153,116,176,187,166,130,169},
                       {179,168,171,182,179,170,131,167},
                       {177,177,179,177,179,165,131,167},
                       {178,178,179,176,182,164,130,171},
                       {179,180,180,179,183,169,132,169},
                       {179,179,180,182,183,170,129,173},
                       {180,179,181,179,181,170,130,169}};

    cv::Mat block = cv::Mat(8, 8, CV_64F, &X);
    cv::Mat finalY = cv::Mat(8, 8, CV_64F, &final_Y);

    INFO("\nX: \n");
    INFO(block);
    quantizeBlock(block, quantMatrixLuminance);
    INFO("\nQ: \n");
    INFO(quantMatrixLuminance);
    INFO("\nỸ: \n");
    INFO(finalY);
    INFO("\nresult: \n");
    INFO(block);
    CHECK(std::equal(block.begin<double>(), block.end<double>(), finalY.begin<double>()));
}

TEST_CASE("DCT Inverse Quantization From Start To Finish"){

    double X [8][8] = {{183,160,94,153,194,163,132,165},
                       {183,153,116,176,187,166,130,169},
                       {179,168,171,182,179,170,131,167},
                       {177,177,179,177,179,165,131,167},
                       {178,178,179,176,182,164,130,171},
                       {179,180,180,179,183,169,132,169},
                       {179,179,180,182,183,170,129,173},
                       {180,179,181,179,181,170,130,169}};

    double final_Y [8][8] = {{20,5,-3,1,3,-2,1,0},
                             {-3,-2,1,2,1,0,0,0},
                             {-1,-1,1,1,1,0,0,0},
                             {-1,0,0,1,0,0,0,0},
                             {0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0}};
    cv::Mat block = cv::Mat(8, 8, CV_64F, &final_Y);
    cv::Mat initialX = cv::Mat(8, 8, CV_64F, &X);

    INFO("\nỸ: \n");
    INFO(block);
    inverseQuantizeBlock(block, quantMatrixLuminance);
    INFO("\nQ: \n");
    INFO(quantMatrixLuminance);
    INFO("\nX: \n");
    INFO(initialX);
    INFO("\nresult: \n");
    INFO(block);
    INFO("\nDifferences between X and inverse of the quantization: \n");
    INFO(initialX-block);
    bool is_small_diff = cv::checkRange(initialX-block, true, nullptr, -30, 30);
    CHECK(is_small_diff);
}

TEST_CASE("Zig Zag San"){
    double final_Y [8][8] = {{20,5,-3,1,3,-2,1,0},
                             {-3,-2,1,2,1,0,0,0},
                             {-1,-1,1,1,1,0,0,0},
                             {-1,0,0,1,0,0,0,0},
                             {0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0}};
    cv::Mat block = cv::Mat(8, 8, CV_64F, &final_Y);
    INFO("\nY: \n");
    INFO(block);
    std::vector<int> result = zigZagScan(block);
    std::string info = "\ncoefficients (except dc) in zig zag order: \n";
    for(double elem: result){
        info += std::to_string((int) elem);
        info += ", ";
    }
    info.erase(info.end() - 2, info.end());
    INFO(info);
    CHECK(std::equal(zigzag_array.begin(), zigzag_array.end(), result.begin()));
}

TEST_CASE("Run Length Code"){
    std::vector<std::pair<int, int>> result = runLengthCode(zigzag_array);
    std::string info = "\ncodewords run length (acs): \n";
    for(std::pair<int, int> ac: result){
        info += "(" + std::to_string(ac.first) + "," + std::to_string(ac.second) + ")";
        info += ", ";
    }
    info.erase(info.end() - 2, info.end());
    INFO(info);
    CHECK(std::equal(result.begin(), result.end(), acs.begin()));
}

TEST_CASE("Huffman Encode") {
    std::vector<bool> encode = huffmanEncode(acs, huffmanTreeRoot);

    int numberOfLeftLeafs = 0;
    Node* node = huffmanTreeRoot;
    while(node != nullptr){
        node = node->left;
        numberOfLeftLeafs += 1;
    }
    numberOfLeftLeafs *= 2;
    std::cout<<std::string(numberOfLeftLeafs + 1, '\t')<<"*"<<std::endl;
    std::string nodeLine = "";
    std::string connectLine = "";
    std::vector<Node*> prevLineFathers = {huffmanTreeRoot};
    std::vector<Node*> currLineNodes;
    int n = 5;
    while(prevLineFathers.at(0)->right != nullptr){
        currLineNodes.clear();
        nodeLine = std::string(numberOfLeftLeafs, '\t');
        connectLine = std::string(numberOfLeftLeafs, '\t');
        for(Node* father : prevLineFathers){
            if(father->left != nullptr){
                currLineNodes.push_back(father->left);
                nodeLine += (father->left->right == nullptr) ? "\033[36m |" : " \033[32m|";
                nodeLine += std::to_string(father->left->data) + "|\033[39m ";
                connectLine += (father->data == 1) ? "  \033[32m/" : "  \033[31m/";
                connectLine += "\033[39m  ";
            } else {
                nodeLine += "     ";
                connectLine += "     ";
            }
            nodeLine += std::string((int)(numberOfLeftLeafs/n)*2, '\t');
            connectLine += std::string((int)(numberOfLeftLeafs/n)*2, '\t');
            if(father->right != nullptr){
                currLineNodes.push_back(father->right);
                nodeLine += (father->right->right == nullptr) ? "\033[33m |" : " \033[31m|";
                nodeLine += std::to_string(father->right->data) + "|\033[39m ";
                connectLine += (father->data == 1) ? "  \033[32m\\" : "  \033[31m\\";
                connectLine += "\033[39m  ";
            } else {
                nodeLine += "     ";
                connectLine += "     ";
            }
            nodeLine += std::string((int)(numberOfLeftLeafs/n), '\t');
            connectLine += std::string((int)(numberOfLeftLeafs/n), '\t');
        }
        prevLineFathers = currLineNodes;

        n *= 2;
        std::cout<<connectLine<<std::endl;
        std::cout<<nodeLine<<std::endl;
        numberOfLeftLeafs -= 1;
    }

    std::cout<<"Legend:\n\t\033[36mNumber of preceding zeros\033[31m\n\t\033[33mValue\033[31m"<<std::endl;

    CHECK(std::equal(encode.begin(), encode.end(), huffmanCode.begin()));
}

TEST_CASE("Huffman Decode"){
    std::vector<std::pair<int, int>> decode = huffmanDecode(huffmanCode, huffmanTreeRoot);
    CHECK(std::equal(decode.begin(), decode.end(), acs.begin()));
}

#endif //VIDEOCODING_TESTQUANTIZATION_HPP
