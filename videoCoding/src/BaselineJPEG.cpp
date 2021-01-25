//!  BaselineJPEG
/*!
 *  Quantization using the sequential mode of JPEG (Baseline).
 *  @author Inês Justo
*/

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <naive-dct.hpp>

void quantizeDct(cv::Mat frame, /*float previous_dc[frame.rows][frame.cols],*/ bool isColor) {

    cv::Mat quantizeDct = cv::Mat::zeros(frame.rows, frame.cols, CV_64F);

    float jpeg_matrix_grayscale [8][8] = {{16, 11, 10, 16, 24, 40, 51, 61},
                                          {12, 12, 14, 19, 26, 58, 60, 55},
                                          {14, 13, 16, 24, 40, 57, 69, 56},
                                          {14, 17, 22, 29, 51, 87, 80, 62},
                                          {18, 22, 37, 56, 68, 109, 103, 77},
                                          {24, 35, 55, 64, 81, 104, 113, 92},
                                          {49, 64, 78, 87, 103, 121, 120, 101},
                                          {72, 92, 95, 98, 112, 100, 103, 99}};

    float jpeg_matrix_color [8][8] = {{17, 18, 24, 47, 99, 99, 99, 99},
                                      {18, 21, 26, 66, 99, 99, 99, 99},
                                      {24, 26, 56, 99, 99, 99, 99, 99},
                                      {47, 66, 99, 99, 99, 99, 80, 99},
                                      {99, 99, 99, 99, 99, 99, 99, 99},
                                      {99, 99, 99, 99, 99, 99, 99, 99},
                                      {99, 99, 99, 99, 99, 99, 99, 99},
                                      {99, 99, 99, 99, 99, 99, 99, 99}};

    // ***************************************** Calculation of the DCT ************************************************
    std::cout << "nrows: " << frame.rows << ", rows to add: " << ((frame.rows % 8) == 0 ? 0 :  (8 - (frame.rows % 8))) << std::endl;
    std::cout << "ncols: " << frame.cols << ", cols to add: " << ((frame.cols % 8) == 0 ? 0 :  (8 - (frame.cols % 8))) << std::endl;

    // The image is partitioned into 8 × 8 blocks of pixels.
    // If the number of rows or columns is not multiple of 8, then they are internally adjusted (using padding).
    int rowsToAdd = (frame.rows % 8) == 0 ? 0 :  (8 - (frame.rows % 8));
    if(rowsToAdd != 0){
        cv::Mat rows = cv::Mat::zeros(rowsToAdd, frame.cols, CV_64F);
        frame.push_back(rows);
    }
    int colsToAdd = (frame.cols % 8) == 0 ? 0 :  (8 - (frame.cols % 8));
    if(colsToAdd != 0){
        cv::Mat cols = cv::Mat::zeros(frame.rows, colsToAdd, CV_64F);
        cv::Mat temp = frame;
        cv::hconcat(temp, cols, frame);
    }
    std::cout << "frame nrows: " << frame.rows << std::endl;
    std::cout << "frame ncols: " << frame.cols << std::endl;

    float current_dc[frame.rows][frame.cols];
    for(int r = 0; r < frame.rows; r += 8) {
        for (int c = 0; c < frame.cols; c += 8) {

            // Subtract 2^(b−1) to each pixel value, where b is the number of bits used to represent the pixels.
            cv::Mat block = cv::Mat::zeros(8, 8, CV_64F);

            for(int i = 0; i <8; i++){
                for (int j = 0; j < 8; ++j) {
                    block.at<char>(i, j) = frame.at<double>(r + i, c + j) - pow(2, 7);
                    //std::cout << "pos "<< r+i << "," << c+j << " " << frame.at<double>(r + i, c + j) - pow(2, 7) <<  " ";
                }
                //std::cout << std::endl;
            }
            //std::cout << std::endl;

            // Calculate the DCT 2D of each block.
            double twoDDCTBlock[8][8];

            // base quantization matrix of JPEG (luminance)
            float jpeg_matrix[8][8];
            memcpy(jpeg_matrix,
                   (isColor) ? jpeg_matrix_color : jpeg_matrix_grayscale,
                   sizeof((isColor) ? jpeg_matrix_color : jpeg_matrix_grayscale));

            // Find discrete cosine transform
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    double vector[1] = {block.at<double>(i,j)};
                    twoDDCTBlock[i][j] = *NaiveDct_transform(vector, 1);

                    // ******************************* Quantization of the DCT coefficients ****************************
                    // Quantization of the DCT coefficients, in order to eliminate less relevant information, according
                    // to the characteristics of the human visual system.

                    // The DCT coefficients are quantized using a quantization matrix, previously scaled by a
                    // compression quality factor.
                    twoDDCTBlock[i][j] = floor(twoDDCTBlock[i][j] / jpeg_matrix[i][j]); // ỹ(r,c)=ROUND(y(r,c)/q(r,c))
                }
            }

            // Next, the coefficients are organized in a one-dimensional vector according to a zig-zag scan.
            std::vector<double> zigzag_array;
            double zig_zag_dc = twoDDCTBlock[0][0];
            int row = 0;
            int col = 0;
            int diagonals = 1;
            bool reachedRow8 = false;
            while(true){
                if(!reachedRow8){ col += 1; /* right */ } else { row = row + 1; /* down */ }
                zigzag_array.push_back(twoDDCTBlock[row][col]);

                // down diagonal
                int temp_diagonals = diagonals;
                while(temp_diagonals != 0){
                    row += 1; col -= 1;
                    zigzag_array.push_back(twoDDCTBlock[row][col]);
                    temp_diagonals -= 1;
                }
                if(row == 7){ reachedRow8 = true; }
                if(!reachedRow8){ diagonals += 1; } else { diagonals -= 1; }

                if(!reachedRow8){ row = row + 1; /* down */ } else { col += 1; /* right */ }
                zigzag_array.push_back(twoDDCTBlock[row][col]);

                if(diagonals == 0){ break; }

                // up diagonal
                temp_diagonals = diagonals;
                while(temp_diagonals != 0){
                    row -= 1; col += 1;
                    zigzag_array.push_back(twoDDCTBlock[row][col]);
                    temp_diagonals -= 1;
                }
                if(!reachedRow8){ diagonals += 1; } else { diagonals -= 1; }
            }

            // ********************* Statistical coding (Golomb) of the quantized DCT coefficients ********************

            // The non-zero AC coefficients are encoded using Huffman or arithmetic coding, representing the value of
            // the coefficient, as well as the number of zeros preceding it.
            // In this case, we use the Golomb code
            int nZeros = 0;
            std::vector<std::pair<int, double>> ac;

            for(float coef : zigzag_array){
                if (coef != 0) {
                    ac.push_back(std::pair<int, double>(nZeros, coef));
                    nZeros = 0;
                } else {
                    nZeros += 1;
                }

            }

            // The DC coefficient of each block is predicatively encoded in relation to the DC coefficient of the
            // previous block.
            // float dc = zig_zag_dc - previous_dc[r][c];
            // current_dc[r][c] = zigzag_array.at(0);
            // TODO get previous_dc

        }
    }

}

void inverseQuantizeDct(cv::Mat frame, /*float previous_dc[frame.rows][frame.cols],*/ bool isColor){

    cv::Mat return_frame  = cv::Mat::zeros(frame.rows, frame.cols, CV_64F);

    float jpeg_matrix_grayscale [8][8] = {{16, 11, 10, 16, 24, 40, 51, 61},
                                          {12, 12, 14, 19, 26, 58, 60, 55},
                                          {14, 13, 16, 24, 40, 57, 69, 56},
                                          {14, 17, 22, 29, 51, 87, 80, 62},
                                          {18, 22, 37, 56, 68, 109, 103, 77},
                                          {24, 35, 55, 64, 81, 104, 113, 92},
                                          {49, 64, 78, 87, 103, 121, 120, 101},
                                          {72, 92, 95, 98, 112, 100, 103, 99}};

    float jpeg_matrix_color [8][8] = {{17, 18, 24, 47, 99, 99, 99, 99},
                                      {18, 21, 26, 66, 99, 99, 99, 99},
                                      {24, 26, 56, 99, 99, 99, 99, 99},
                                      {47, 66, 99, 99, 99, 99, 80, 99},
                                      {99, 99, 99, 99, 99, 99, 99, 99},
                                      {99, 99, 99, 99, 99, 99, 99, 99},
                                      {99, 99, 99, 99, 99, 99, 99, 99},
                                      {99, 99, 99, 99, 99, 99, 99, 99}};

    float current_dc[frame.rows][frame.cols];

    // *********************** Statistical decoding (Golomb) of the quantized DCT coefficients *************************
    // TODO

    cv::Mat block = cv::Mat::zeros(8, 8, CV_64F);
    for(int r = 0; r < frame.rows; r += 8) {
        for (int c = 0; c < frame.cols; c += 8) {

            // Calculate the inverse DCT 2D of each block.
            double twoIDDCTBlock[8][8];

            // base quantization matrix of JPEG (luminance)
            float jpeg_matrix[8][8];
            memcpy(jpeg_matrix,
                   (isColor) ? jpeg_matrix_color : jpeg_matrix_grayscale,
                   sizeof((isColor) ? jpeg_matrix_color : jpeg_matrix_grayscale));

            for(int i = 0; i < 8; i++) {
                for(int j = 0; j < 8; j++) {
                    // ******************************* Dequantization of the DCT coefficients **************************
                    twoIDDCTBlock[i][j] = block.at<double>(i, j) * jpeg_matrix[i][j];

                    // ************************************** Inverse DCT **********************************************
                    double vector[1] = {twoIDDCTBlock[i][j]};
                    twoIDDCTBlock[i][j] = *NaiveDct_inverseTransform(vector, 1);

                    // Add 2^(b−1) to each pixel value, where b is the number of bits used to represent the pixels.
                    return_frame.at<double>(r + i, c + j) = twoIDDCTBlock[i][j] + pow(2, 7);
                }
            }

        }
    }
}

