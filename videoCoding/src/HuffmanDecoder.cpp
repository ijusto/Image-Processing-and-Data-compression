#include    "HuffmanDecoder.hpp"
#include    <opencv2/core.hpp>
#include    <opencv2/opencv.hpp>
#include    <cmath>

HuffmanDecoder::HuffmanDecoder(Node* huffmanTree) : huffmanTree(huffmanTree), node(huffmanTree){ this->nZeros = -4; }

bool HuffmanDecoder::decode(bool bit, std::vector<std::pair<int, int>> &runLengthCode){
    this->node = bit ? this->node->left : this->node->right;

    // if we are decoding the number of zeros
    if(this->nZeros == -4){
        if(this->node->left->left == nullptr && this->node->left->right == nullptr) { // number of zeros are in the left leafs
            if(this->node->left->data == -3){ // end of huffman code
                return false;
            }
            this->nZeros = this->node->left->data;
            this->node = huffmanTree;
        }
    } else { // if we are decoding the value
        if(this->node->right->left == nullptr && this->node->right->right == nullptr) { // values are in the right leafs
            runLengthCode.push_back(std::pair<int, int>(this->nZeros, this->node->right->data));
            this->nZeros = -4;
            this->node = huffmanTree;
        }
    }
    return true;
}