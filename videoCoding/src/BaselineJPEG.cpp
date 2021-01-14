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
void calculateDCT(cv::Mat frame, int nRows, int nCols) {
    int dctMatrix[][8];
    // The image is partitioned into 8 × 8 blocks of pixels.
    // If the number of rows or columns is not multiple of 8, then they are internally adjusted (using padding).
    if(nRows % 8 != 0){

    }
    if(nCols % 8 != 0){

    }

    // Subtract 2 b−1 to each pixel value, where b is the number of bits used to represent the pixels.

    // Calculate the DCT 2D of each block.
    dctTransform();
}

// Function to find discrete cosine transform and print it
// source: https://www.geeksforgeeks.org/discrete-cosine-transform-algorithm-program/
void dctTransform(int matrix[][8]) {
    int i, j, k, l;

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
                    dct1 = matrix[k][l] *
                           cos((2 * k + 1) * i * pi / (2 * m)) *
                           cos((2 * l + 1) * j * pi / (2 * n));
                    sum = sum + dct1;
                }
            }
            dct[i][j] = ci * cj * sum;
        }
    }

    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%f\t", dct[i][j]);
        }
        printf("\n");
    }
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