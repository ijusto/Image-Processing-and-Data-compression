//!  BaselineJPEG
/*!
 *  Functions useful when computing the sequential mode of JPEG (Baseline)
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

void calculateDCT(cv::Mat frame, int nRows, int nCols);
void dctTransform(int m, int n, int matrix[][8]);
void quantizeDCTCoefficients();
void statisticalCoding();

void calculateDCT(cv::Mat frame, int nRows, int nCols) {

    // The image is partitioned into 8 × 8 blocks of pixels.
    // If the number of rows or columns is not multiple of 8, then they are internally adjusted (using padding).
    int rowsToAdd = (nRows % 8) == 0 ? 0 :  (8 - (nRows % 8));
    if(rowsToAdd != 0){
        cv::Mat rows = cv::Mat::zeros(rowsToAdd, frame.cols, CV_64F);
        frame.push_back(rows);
    }
    int colsToAdd = (nCols % 8) == 0 ? 0 :  (8 - (nCols % 8));
    if(colsToAdd != 0){
        cv::Mat cols = cv::Mat::zeros(frame.rows, colsToAdd, CV_64F);
        cv::Mat temp = frame;
        cv::hconcat(temp, cols, frame);
    }
    std::cout << "nrows: " << nRows << ", rows to add: " << rowsToAdd << std::endl;
    std::cout << "ncols: " << nCols << ", cols to add: " << colsToAdd << std::endl;
    std::cout << "frame nrows: " << frame.rows << std::endl;
    std::cout << "frame ncols: " << frame.cols << std::endl;


    for(int r = 0; r < frame.rows; r += 8) {
        for (int c = 0; c < frame.cols; c += 8) {
            // Subtract 2^(b−1) to each pixel value, where b is the number of bits used to represent the pixels.
            cv::Rect rct(r, c, 8,8);

            cv::Mat block = frame(rct);

            for(int i = 0; i <8; i++){
                for (int j = 0; j < 8; ++j) {

                    block.at<char>(i, j) -= pow(2, 7);
                }
            }

            // Calculate the DCT 2D of each block.
            cv::Mat twoDDCTBlock = dctTransform(8, 8, block);

            quantizeDCTCoefficients()

        }
    }

}

// Function to find discrete cosine transform and print it
// based on this implementation: https://www.geeksforgeeks.org/discrete-cosine-transform-algorithm-program/
float[8][8] dctTransform(cv::Mat matrix) {
    int i, j, k, l;
    int m = 8;
    int n = 8;

    // dct will store the discrete cosine transform
    float dct[m][n];

    float ci, cj, dct1, sum;

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
                    dct1 = matrix.at<char>(k, l) *
                           cos((2 * k + 1) * i * pi / (2 * m)) *
                           cos((2 * l + 1) * j * pi / (2 * n));
                    sum = sum + dct1;
                }
            }
            dct[i][j] = ci * cj * sum;
        }
    }

    return dct;
}

// Quantization of the DCT coefficients, in order to eliminate less relevant information, according to the
// characteristics of the human visual system.
void quantizeDCTCoefficients(){
    // The DCT coefficients are quantized using a quantization matrix, previously scaled by a compression quality factor.

    // Next, the coefficients are organized in a one-dimensional vector according to a zig-zag scan.
}

// Statistical coding (Huffman or arithmetic) of the quantized DCT coefficients.
void statisticalCoding(){
    // The non-zero AC coefficients are encoded using Huffman or arithmetic coding, representing the value of the
    // coefficient, as well as the number of zeros preceding it.

    // The DC coefficient of each block is predicatively encoded in relation to the DC coefficient of the previous block.
}