//!  BaselineJPEG
/*!
 *  Quantization using the sequential mode of JPEG (Baseline)
 *  @author Inês Justo
*/

//! Calculates the DCT.
/*!
 * @param frame image frame of the video.
 * @param nRows number of rows.
 * @param nCols number of columns.
 * @returns
*/
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include <math.h>      /* pow */

#define pi 3.142857

void dct(cv::Mat frame, bool isColor) {


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

    for(int r = 0; r < frame.rows; r += 8) {
        for (int c = 0; c < frame.cols; c += 8) {
            // Subtract 2^(b−1) to each pixel value, where b is the number of bits used to represent the pixels.
            cv::Mat block = cv::Mat::zeros(8, 8, CV_64F);

            for(int i = 0; i <8; i++){
                for (int j = 0; j < 8; ++j) {
                    //block.at<char>(i, j) = frame.at<uchar>(r + i, c + j) - pow(2, 7);
                    std::cout << "pos "<< r+i << "," << c+j << " " << frame.at<double>(r + i, c + j) - pow(2, 7) <<  " ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
            std::cout << std::endl;
            std::cout << std::endl;


            // Calculate the DCT 2D of each block.
            double twoDDCTBlock[8][8];

            // Find discrete cosine transform
            // based on this implementation: https://www.geeksforgeeks.org/discrete-cosine-transform-algorithm-program/
            int i, j, k, l;
            int m = 8;
            int n = 8;

            double ci, cj, dct1, sum;

            for (i = 0; i < m; i++) {
                for (j = 0; j < n; j++) {

                    // ci and cj depends on frequency as well as
                    // number of row and columns of specified matrix
                    if (i == 0)
                        ci = 1 / sqrt(m);
                    else
                        ci = sqrt(2) / sqrt(m);
                    if (j == 0)
                        cj = 1 / sqrt(n);
                    else
                        cj = sqrt(2) / sqrt(n);

                    // sum will temporarily store the sum of
                    // cosine signals
                    sum = 0;
                    for (k = 0; k < m; k++) {
                        for (l = 0; l < n; l++) {
                            dct1 = block.at<double>(k, l) *
                                   cos((2 * k + 1) * i * pi / (2 * m)) *
                                   cos((2 * l + 1) * j * pi / (2 * n));
                            sum = sum + dct1;
                        }
                    }
                    twoDDCTBlock[i][j] = ci * cj * sum;
                }
            }

            // ******************************* Quantization of the DCT coefficients ************************************
            // Quantization of the DCT coefficients, in order to eliminate less relevant information, according to the
            // characteristics of the human visual system.

            // The DCT coefficients are quantized using a quantization matrix, previously scaled by a compression
            // quality factor.

            // base quantization matrix of JPEG (luminance)
            float jpeg_matrix[8][8];
            memcpy(jpeg_matrix,
                    (isColor) ? jpeg_matrix_color : jpeg_matrix_grayscale,
                    sizeof((isColor) ? jpeg_matrix_color : jpeg_matrix_grayscale));
            
            /*
            Q = 80; // Define Q factor

            // Determine S
            if (Q < 50)
                S = 5000/Q;
            else
                S = 200 - 2*Q;
            end

            Ts = floor((S*Tb + 50) / 100);
            Ts(Ts == 0) = 1; % // Prevent divide by 0 error
            */

            // Next, the coefficients are organized in a one-dimensional vector according to a zig-zag scan.

            // ********************* Statistical coding (Huffman) of the quantized DCT coefficients ********************

            // The non-zero AC coefficients are encoded using Huffman or arithmetic coding, representing the value of the
            // coefficient, as well as the number of zeros preceding it.

            // The DC coefficient of each block is predicatively encoded in relation to the DC coefficient of the previous block.


        }
    }

}