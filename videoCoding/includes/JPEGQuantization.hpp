#ifndef VIDEOCODING_JPEGQUANTIZATION_HPP
#define VIDEOCODING_JPEGQUANTIZATION_HPP

//!  JPEGQuantization
/*!
 *  This class is useful for quantization purposes using the sequential (baseline) mode of JPEG.
 *  @author Inês Justo
*/

#include    <opencv2/opencv.hpp>
#include    "HuffmanDecoder.hpp"
#include    <vector>

class JPEGQuantization {

private:
    /*!
     *
     */
    double jpeg_matrix_grayscale [8][8] = {{16, 11, 10, 16, 24, 40, 51, 61},
                                           {12, 12, 14, 19, 26, 58, 60, 55},
                                           {14, 13, 16, 24, 40, 57, 69, 56},
                                           {14, 17, 22, 29, 51, 87, 80, 62},
                                           {18, 22, 37, 56, 68, 109, 103, 77},
                                           {24, 35, 55, 64, 81, 104, 113, 92},
                                           {49, 64, 78, 87, 103, 121, 120, 101},
                                           {72, 92, 95, 98, 112, 100, 103, 99}};

    /*!
     *
     */
    double jpeg_matrix_color [8][8] = {{17, 18, 24, 47, 99, 99, 99, 99},
                                       {18, 21, 26, 66, 99, 99, 99, 99},
                                       {24, 26, 56, 99, 99, 99, 99, 99},
                                       {47, 66, 99, 99, 99, 99, 99, 99},
                                       {99, 99, 99, 99, 99, 99, 99, 99},
                                       {99, 99, 99, 99, 99, 99, 99, 99},
                                       {99, 99, 99, 99, 99, 99, 99, 99},
                                       {99, 99, 99, 99, 99, 99, 99, 99}};

    /*!
     *
     */
    const cv::Mat quantMatrixLuminance = cv::Mat(8, 8, CV_64F, &jpeg_matrix_grayscale);

    /*!
     *
     */
    const cv::Mat quantMatrixChrominance = cv::Mat(8, 8, CV_64F, &jpeg_matrix_color);

    /*!
     *  DCT Transformation matrix (T)
     */
    cv::Mat transformationMatrix = cv::Mat::zeros(8, 8, CV_64F);

public:
    //! JPEGQuantization constructor.
    /*!
     *
     */
    explicit JPEGQuantization();

    /*! Divides an image frame into blocks of 8 by 8 and adds padding to the image if their width or the height are not
     * multiple of 8. The image's padding values are copies of the edges of the original frame.
     * @param frame image frame.
     */
    static void divideImageIn8x8Blocks(cv::Mat &frame);

    /*! DCT following the Transformation Matrix Approach
     * @param block 8 by 8 block of image frame that we want to apply the dct transformation.
     */
    void dct(cv::Mat &block);

    /*! Inverse DCT following the Transformation Matrix Approach
     * @param block 8 by 8 block of image frame that we want to inverse the dct transformation.
     */
    void inverseDCT(cv::Mat &block);

    /*! Quantization of coefficients resulted from the DCT transformation.
     * @param block 8 by 8 block of image frame that has the coefficients resulted from the DCT transformation and that we
     *              want to quantize.
     * @param quantMatrix quantization matrix.
     */
    static void quantizeDCTCoeff(cv::Mat &block, cv::Mat quantMatrix);

    /*! Inverse the quantization of coefficients resulted from the DCT transformation.
     * @param block 8 by 8 block of image frame that has the quantized values.
     * @param quantMatrix quantization matrix.
     */
    static void inverseQuantizeDCTCoeff(cv::Mat &block, cv::Mat quantMatrix);

    /*! Operation of applying the DCT transformation and the quantization of an 8 by 8 block of an image frame.
     * @param block 8 by 8 block of an image frame that we want to quantize.
     * @param quantMatrix quantization matrix.
     */
    void quantizeBlock(cv::Mat &block, cv::Mat quantMatrix);

    /*! Operation of reversing the quantization and the DCT transformation of an 8 by 8 block of an image frame.
     * @param block 8 by 8 block of an image frame that we want to reverse the quantization.
     * @param quantMatrix quantization matrix.
     */
    void inverseQuantizeBlock(cv::Mat &block, cv::Mat quantMatrix);

    /*! Apply a zig zag scan to an 8 by 8 block of an image frame.
     * @param block  8 by 8 block of an image frame.
     * @param zigzagVector vector resulted from the zig zag scan.
     */
    static void zigZagScan(cv::Mat &block, std::vector<int> &zigzagVector);

    /*! Run Length Code
     * In this implementation, the code is the pair (number of zeros preceding this value, non zero value).
     * The pair (-1, dc) represents the beginning of a block.
     * @param vec zig zag vector.
     * @param code run length code.
     */
    static void runLengthPairs(std::vector<int> &vec, std::vector<std::pair<int, int>> &code);

    /*! Creates a new tree node.
     * @param data data to be stored in the new tree node.
     * @param leafLeft pointer to the new node's leaft leaf.
     * @param leafRight pointer to the new node's right leaf.
     * @return pointer to new node
     */
    static Node* newNode(int data, Node *leafLeft, Node *leafRight);

    /*! Decode the huffman tree and store it in Node pointers so the decoder can decode faster.
     * @param decodedLeafs golomb decoded huffman tree (leafs).
     * @return huffman tree root node pointer.
     */
    static Node* huffmanTree(std::vector<int> decodedLeafs);

    /*! Construct the huffman tree of the run length code and encode it in order to be sent to the decoder.
     * In this implementation, the non zero values and the values representing the number of zeros can have the same code,
     * because the the non zero values are always on the right leafs and the values representing the number of zeros are
     * always on the left leafs. Moreover, they are always leafs of nodes with data 0 (except for the least probable ones).
     * So we take advantage of this to encode the tree in order to send it to the decoder.
     * The values on the leafs are Golomb encoded from bottom to top in depth. When there are no left leaf values, the -2
     * value is encoded.
     * We golomb encode -3 to represent the end of the tree.
     * The very last two values of the huffman code are -3 to represent the end of the code.
     * @param freqs_listNZero list of (number of zeros of the run length code, frequency of that number) pairs.
     * @param freqs_listValue list of (non zero number of the run length code, frequency of that number) pairs.
     * @param codeZerosMap map with the number of zeros values from the run length code as keys and their huffman code as value.
     * @param codeValueMap map with the non zero number values from the run length code as keys and their huffman code as value.
     * @param golomb pointer to Golomb Object.
     * @return golomb encoded huffman tree.
     */
    static void huffmanTreeEncode(std::vector<bool> &encodedTree, std::list<std::pair<int, double>> freqs_listNZero,
                                        std::list<std::pair<int, double>> freqs_listValue,
                                        std::unordered_map<int, std::vector<bool>> &codeZerosMap,
                                        std::unordered_map<int, std::vector<bool>> &codeValueMap,
                                        Golomb *golomb);

    /*!
     *
     * @param runLengthCode
     * @param code
     * @param encodedTree
     * @param golomb pointer to Golomb Object.
     */
    void huffmanEncode(std::vector<std::pair<int, int>> &runLengthCode, std::vector<bool> &code,
                       std::vector<bool> &encodedTree, Golomb* golomb);

    /*!
     *
     * @param runLengthCode
     * @param code
     * @param golomb pointer to Golomb Object.
     */
    void huffmanEncode(const std::vector<std::pair<int, int>> &runLengthCode, std::vector<bool> &code, Golomb* golomb);

    /*!
     * @param huffmanTreeRoot
     */
    static void printHuffmanTree(Node* huffmanTreeRoot);

    /*!
     * @param code
     * @param encodedTree
     * @param runLengthCode
     * @param golomb pointer to Golomb Object.
     */
    static void huffmanDecode(std::vector<bool> &code, std::vector<bool> &encodedTree,
                       std::vector<std::pair<int, int>> &runLengthCode, Golomb* golomb);

    /*!
     *
     * @param runLengthCode
     * @param frame
     */
    static void getImage(std::vector<std::pair<int, int>> runLengthCode, cv::Mat &frame);

    /*!
     *
     * @param frame
     * @param prevDCs
     * @param golomb
     * @param encodedTree
     * @param code
     */
    void quantizeDctBaselineJPEG(cv::Mat &frame, std::vector<int> &prevDCs, Golomb* golomb, std::vector<bool> &encodedTree,
                                 std::vector<bool> &code, bool luminance);

    /*!
     *
     * @param frame
     * @param prevDCs
     * @param golomb
     * @param code
     * @param luminance
     */
    void quantizeDctBaselineJPEG(cv::Mat &frame, std::vector<int> &prevDCs, Golomb* golomb, std::vector<bool> &code, bool luminance);

    /*!
     *
     * @param frame
     * @param prevDCs
     * @param golomb
     * @param code
     * @param luminance
     */
    void quantize(cv::Mat &frame, std::vector<int> &prevDCs, Golomb* golomb, std::vector<bool> &code, bool luminance);

    //!
    /*!
     *
     * @param prevDCs
     * @param runLengthCode
     * @param frame
     */
    void inverseQuantizeDctBaselineJPEG(std::vector<int> &prevDCs, std::vector<std::pair<int, int>> runLengthCode,
                                        cv::Mat &frame, bool luminance);

    /*!
     *
     * @param f_rows
     * @param f_cols
     * @param prevDCs
     * @param runLengthCode
     * @param outRes
     * @param luminance
     */
    void inverseQuantizeDctBaselineJPEG(int f_rows, int f_cols, std::vector<int> &prevDCs,
                                        std::vector<std::pair<int, int>> &runLengthCode,
                                        vector<int> &outRes, bool luminance);
    /*!
     *
     * @return
     */
    cv::Mat getQuantMatrixLuminance();

    /*!
     *
     * @return
     */
    cv::Mat getQuantMatrixChrominance();

};
#endif //VIDEOCODING_JPEGQUANTIZATION_HPP
