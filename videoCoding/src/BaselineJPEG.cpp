#include "BaselineJPEG.hpp"

// Calculation of the DCT.
void BaselineJPEG::calculateDCT() {
    // The image is partitioned into 8 × 8 blocks of pixels.
    // If the number of rows or columns is not multiple of 8, then they are internally adjusted (using padding).

    // Subtract 2 b−1 to each pixel value, where b is the number of bits used to represent the pixels.

    // Calculate the DCT 2D of each block.
}

// Quantization of the DCT coefficients, in order to eliminate less relevant information, according to the
// characteristics of the human visual system.
void BaselineJPEG::quantizeDCTCoefficients(){
    // The DCT coefficients are quantized using a quantization matrix, previously scaled by a compression quality factor.

    // Next, the coefficients are organized in a one-dimensional vector according to a zig-zag scan.
}

// Statistical coding (Huffman or arithmetic) of the quantized DCT coefficients.
void BaselineJPEG::statisticalCoding(){
    // The non-zero AC coefficients are encoded using Huffman or arithmetic coding, representing the value of the
    // coefficient, as well as the number of zeros preceding it.

    // The DC coefficient of each block is predicatively encoded in relation to the DC coefficient of the previous block.
}