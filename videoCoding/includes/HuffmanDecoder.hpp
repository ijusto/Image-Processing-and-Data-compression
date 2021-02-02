#ifndef VIDEOCODING_HUFFMANDECODER_HPP
#define VIDEOCODING_HUFFMANDECODER_HPP
//!  HuffmanDecoder
/*!
 *  This class is useful to decode an huffman code.
 *  @author Inês Justo
*/

#include    <opencv2/opencv.hpp>
#include    <vector>

//! Data structure to store a tree node.
struct Node {
    int data;
    Node *left, *right;
};

class HuffmanDecoder {

private:
    //!
    Node* huffmanTree;

    //!
    Node* node;

    //!
    int nZeros;

public:
    //! HuffmanDecoder constructor.
    /*!
     * @param huffmanTree Huffman Tree.
     */
    explicit HuffmanDecoder(Node *huffmanTree);

    //!
    /*!
     *
     * @param bit
     * @param runLengthCode
     * @return
     */
    bool decode(bool bit, std::vector<std::pair<int, int>> &runLengthCode);

};
#endif //VIDEOCODING_HUFFMANDECODER_HPP
