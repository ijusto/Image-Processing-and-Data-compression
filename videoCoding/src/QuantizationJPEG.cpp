//!  QuantizationJPEG
/*!
 *  Quantization using the sequential (baseline) or the progressive mode of JPEG.
 *  @author Inês Justo
*/

#include    <opencv2/core.hpp>
#include    <opencv2/opencv.hpp>
#include    <cmath>
#include    "Golomb.cpp"
#include    "../src/HuffmanDecoder.cpp"

double jpeg_matrix_grayscale [8][8] = {{16, 11, 10, 16, 24, 40, 51, 61},
                                       {12, 12, 14, 19, 26, 58, 60, 55},
                                       {14, 13, 16, 24, 40, 57, 69, 56},
                                       {14, 17, 22, 29, 51, 87, 80, 62},
                                       {18, 22, 37, 56, 68, 109, 103, 77},
                                       {24, 35, 55, 64, 81, 104, 113, 92},
                                       {49, 64, 78, 87, 103, 121, 120, 101},
                                       {72, 92, 95, 98, 112, 100, 103, 99}};

double jpeg_matrix_color [8][8] = {{17, 18, 24, 47, 99, 99, 99, 99},
                                   {18, 21, 26, 66, 99, 99, 99, 99},
                                   {24, 26, 56, 99, 99, 99, 99, 99},
                                   {47, 66, 99, 99, 99, 99, 99, 99},
                                   {99, 99, 99, 99, 99, 99, 99, 99},
                                   {99, 99, 99, 99, 99, 99, 99, 99},
                                   {99, 99, 99, 99, 99, 99, 99, 99},
                                   {99, 99, 99, 99, 99, 99, 99, 99}};

const cv::Mat quantMatrixLuminance = cv::Mat(8, 8, CV_64F, &jpeg_matrix_grayscale);
const cv::Mat quantMatrixChrominance = cv::Mat(8, 8, CV_64F, &jpeg_matrix_color);

/*! Calculates the DCT Transformation matrix (T).
 * @return DCT Transformation matrix.
 */
cv::Mat transformationMatrix(){
    int m = 8;
    cv::Mat t = cv::Mat::zeros(m, m, CV_64F);
    for(int r = 0; r <m; r++) {
        for (int c = 0; c < m; c++) {
            if(r == 0){
                t.at<double>(r, c) = 1/(sqrt(m));
            } else {
                t.at<double>(r, c) = sqrt((2/(double)m))*cos((M_PI*(2*c+1)*r) / (2*(double)m));
            }
        }
    }

    return t;
}

const cv::Mat t = transformationMatrix();

/*! Divides an image frame into blocks of 8 by 8 and adds padding to the image if their width or the height are not
 * multiple of 8. The image's padding values are copies of the edges of the original frame.
 * @param frame image frame.
 */
void divideImageIn8x8Blocks(cv::Mat &frame){

    //std::cout << "nrows: " << frame.rows << ", rows to add: " << ((frame.rows % 8) == 0 ? 0 :  (8 - (frame.rows % 8))) << std::endl;
    //std::cout << "ncols: " << frame.cols << ", cols to add: " << ((frame.cols % 8) == 0 ? 0 :  (8 - (frame.cols % 8))) << std::endl;

    // The image is partitioned into 8 × 8 blocks of pixels.
    // If the number of rows or columns is not multiple of 8, then they are internally adjusted (using padding).
    int nRowsToAdd = (frame.rows % 8) == 0 ? 0 :  (8 - (frame.rows % 8));
    if(nRowsToAdd != 0){
        cv::Mat rowsToAdd = cv::Mat::zeros(nRowsToAdd, frame.cols, CV_64F);
        // padding is equal to last row of original image
        for(int r = 0; r < rowsToAdd.rows; r += 1) {
            for (int c = 0; c < frame.cols; c += 1) {
                rowsToAdd.at<double>(r, c) = frame.at<double>(frame.rows - 1, c);
            }
        }
        frame.push_back(rowsToAdd);
    }

    int nColsToAdd = (frame.cols % 8) == 0 ? 0 :  (8 - (frame.cols % 8));
    if(nColsToAdd != 0){
        cv::Mat colsToAdd = cv::Mat::zeros(frame.rows, nColsToAdd, CV_64F);
        // padding is equal to last row of the original image (along with the padded rows)
        for(int r = 0; r < frame.rows; r += 1) {
            for (int c = 0; c < colsToAdd.cols; c += 1) {
                colsToAdd.at<double>(r, c) = frame.at<double>(r, frame.cols - 1);
            }
        }
        cv::Mat temp = frame;
        cv::hconcat(temp, colsToAdd, frame);
    }

    //std::cout << "frame nrows: " << frame.rows << std::endl;
    //std::cout << "frame ncols: " << frame.cols << std::endl;
}

/*! DCT following the Transformation Matrix Approach
 * @param block 8 by 8 block of image frame that we want to apply the dct transformation.
 */
void dct(cv::Mat &block){
    int m = 8;

    // DCT of the block: T*block*T'
    cv::Mat mult1 = cv::Mat::zeros(m, m, CV_64F);
    cv::Mat dct = cv::Mat::zeros(m, m, CV_64F);
    for(int r = 0; r < m; ++r) {
        for (int c = 0; c < m; ++c) {
            for (int s = 0; s < m; ++s) {
                // Subtract 2^(b−1) to each pixel value, where b is the number of bits used to represent the pixels.
                mult1.at<double>(r, c) += t.at<double>(r, s) * (block.at<double>(s, c) - pow(2, 7));
            }
        }
    }
    for(int r = 0; r < m; ++r) {
        for (int c = 0; c < m; ++c) {
            for (int s = 0; s < m; ++s) {
                dct.at<double>(r, c) += mult1.at<double>(r, s) * t.at<double>(c, s) /* transpose of T */;
            }
        }
    }

    for(int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            block.at<double>(r, c) = (int)(dct.at<double>(r, c) + 0.5 - (dct.at<double>(r, c)<0)); // https://stackoverflow.com/questions/9695329/c-how-to-round-a-double-to-an-int
        }
    }
}

/*! Inverse DCT following the Transformation Matrix Approach
 * @param block 8 by 8 block of image frame that we want to inverse the dct transformation.
 */
void inverseDCT(cv::Mat &block){
    int m = 8;

    // inverse of the DCT: T'*dct*T
    cv::Mat mult1 = cv::Mat::zeros(m, m, CV_64F);
    cv::Mat inverse_dct = cv::Mat::zeros(m, m, CV_64F);

    for(int r = 0; r < m; ++r) {
        for (int c = 0; c < m; ++c) {
            for (int s = 0; s < m; ++s) {
                mult1.at<double>(r, c) += t.at<double>(s, r) /* transpose of T */ * block.at<double>(s, c);
            }
        }
    }

    for(int r = 0; r < m; ++r) {
        for (int c = 0; c < m; ++c) {
            for (int s = 0; s < m; ++s) {
                inverse_dct.at<double>(r, c) += mult1.at<double>(r, s) * t.at<double>(s, c);
            }
        }
    }

    for(int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            // Add 2^(b−1) to each pixel value, where b is the number of bits used to represent the pixels.
            block.at<double>(r, c) = (int)(inverse_dct.at<double>(r, c) + 0.5 - (inverse_dct.at<double>(r, c)<0)  + pow(2, 7)); // https://stackoverflow.com/questions/9695329/c-how-to-round-a-double-to-an-int
        }
    }
}

/*! Quantization of coefficients resulted from the DCT transformation.
 * @param block 8 by 8 block of image frame that has the coefficients resulted from the DCT transformation and that we
 *              want to quantize.
 * @param quantMatrix quantization matrix.
 */
void quantizeDCTCoeff(cv::Mat &block, cv::Mat quantMatrix){
    for(int r = 0; r < 8; r++){
        for (int c = 0; c < 8; c++) {
            // ******************************* Quantization of the DCT coefficients ****************************
            // Quantization of the DCT coefficients, in order to eliminate less relevant information, according
            // to the characteristics of the human visual system.
            // The DCT coefficients are quantized using a quantization matrix, previously scaled by a compression
            // quality factor.
            float temp = block.at<double>(r, c) / quantMatrix.at<double>(r, c); // ỹ(r,c)=ROUND(y(r,c)/q(r,c))
            block.at<double>(r, c) = (int)(temp + 0.5 - (temp<0)); // https://stackoverflow.com/questions/9695329/c-how-to-round-a-double-to-an-int
        }
    }
}

/*! Inverse the quantization of coefficients resulted from the DCT transformation.
 * @param block 8 by 8 block of image frame that has the quantized values.
 * @param quantMatrix quantization matrix.
 */
void inverseQuantizeDCTCoeff(cv::Mat &block, cv::Mat quantMatrix){
    for(int r = 0; r < 8; r++){
        for (int c = 0; c < 8; c++) {
            // ******************************* Quantization of the DCT coefficients ****************************
            // Quantization of the DCT coefficients, in order to eliminate less relevant information, according
            // to the characteristics of the human visual system.
            // The DCT coefficients are quantized using a quantization matrix, previously scaled by a compression
            // quality factor.
            float temp = block.at<double>(r, c) * quantMatrix.at<double>(r, c); // ỹ(r,c)=ROUND(y(r,c)/q(r,c))
            block.at<double>(r, c) = (int)(temp + 0.5 - (temp<0)); // https://stackoverflow.com/questions/9695329/c-how-to-round-a-double-to-an-int
        }
    }
}

/*! Operation of applying the DCT transformation and the quantization of an 8 by 8 block of an image frame.
 * @param block 8 by 8 block of an image frame that we want to quantize.
 * @param quantMatrix quantization matrix.
 */
void quantizeBlock(cv::Mat &block, cv::Mat quantMatrix){
    int m = 8;

    // DCT of the block: T*block*T'
    cv::Mat mult1 = cv::Mat::zeros(m, m, CV_64F);
    cv::Mat dct = cv::Mat::zeros(m, m, CV_64F);
    for(int r = 0; r < m; ++r) {
        for (int c = 0; c < m; ++c) {
            for (int s = 0; s < m; ++s) {
                // Subtract 2^(b−1) to each pixel value, where b is the number of bits used to represent the pixels.
                mult1.at<double>(r, c) += t.at<double>(r, s) * (block.at<double>(s, c) - pow(2, 7));
            }
        }
    }
    for(int r = 0; r < m; ++r) {
        for (int c = 0; c < m; ++c) {
            for (int s = 0; s < m; ++s) {
                dct.at<double>(r, c) += mult1.at<double>(r, s) * t.at<double>(c, s) /* transpose of T */;
            }
        }
    }

    float temp;
    for(int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            temp = (int)(dct.at<double>(r, c) + 0.5 - (dct.at<double>(r, c)<0)); // https://stackoverflow.com/questions/9695329/c-how-to-round-a-double-to-an-int

            // ******************************* Quantization of the DCT coefficients ****************************
            // Quantization of the DCT coefficients, in order to eliminate less relevant information, according
            // to the characteristics of the human visual system.
            // The DCT coefficients are quantized using a quantization matrix, previously scaled by a compression
            // quality factor.
            temp = temp / quantMatrix.at<double>(r, c); // ỹ(r,c)=ROUND(y(r,c)/q(r,c))
            block.at<double>(r, c) = (int)(temp + 0.5 - (temp<0)); // https://stackoverflow.com/questions/9695329/c-how-to-round-a-double-to-an-int
        }
    }
}

/*! Operation of reversing the quantization and the DCT transformation of an 8 by 8 block of an image frame.
 * @param block 8 by 8 block of an image frame that we want to reverse the quantization.
 * @param quantMatrix quantization matrix.
 */
void inverseQuantizeBlock(cv::Mat &block, cv::Mat quantMatrix){
    int m = 8;

    // inverse of the DCT: T'*dct*T
    cv::Mat mult1 = cv::Mat::zeros(m, m, CV_64F);
    cv::Mat inverse_dct = cv::Mat::zeros(m, m, CV_64F);

    for(int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            // ******************************* Quantization of the DCT coefficients ****************************
            // Quantization of the DCT coefficients, in order to eliminate less relevant information, according
            // to the characteristics of the human visual system.
            // The DCT coefficients are quantized using a quantization matrix, previously scaled by a compression
            // quality factor.
            float temp = floor(block.at<double>(r, c) * quantMatrix.at<double>(r, c)); // ỹ(r,c)=ROUND(y(r,c)/q(r,c))
            block.at<double>(r, c) = (int)(temp - 0.5 - (temp<0)); // https://stackoverflow.com/questions/9695329/c-how-to-round-a-double-to-an-int
        }
    }

    for(int r = 0; r < m; ++r) {
        for (int c = 0; c < m; ++c) {
            for (int s = 0; s < m; ++s) {
                mult1.at<double>(r, c) += t.at<double>(s, r) /* transpose of T */ * block.at<double>(s, c);
            }
        }
    }

    for(int r = 0; r < m; ++r) {
        for (int c = 0; c < m; ++c) {
            for (int s = 0; s < m; ++s) {
                inverse_dct.at<double>(r, c) += mult1.at<double>(r, s) * t.at<double>(s, c);
            }
        }
    }

    for(int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            // Add 2^(b−1) to each pixel value, where b is the number of bits used to represent the pixels.
            block.at<double>(r, c) = (int)(floor(inverse_dct.at<double>(r, c)) + pow(2, 7));
        }
    }

}

/*! Apply a zig zag scan to an 8 by 8 block of an image frame.
 * @param block  8 by 8 block of an image frame.
 * @param zigzagVector vector resulted from the zig zag scan.
 */
void zigZagScan(cv::Mat &block, std::vector<int> &zigzagVector){
    zigzagVector.push_back(block.at<double>(0, 0)); // dc
    int row = 0, col = 0, diagonals = 1;
    bool reachedRow8 = false;
    while(true){
        if(!reachedRow8){ col += 1; /* right */ } else { row = row + 1; /* down */ }
        zigzagVector.push_back(block.at<double>(row, col));

        // down diagonal
        int temp_diagonals = diagonals;
        while(temp_diagonals != 0){
            row += 1; col -= 1;
            zigzagVector.push_back(block.at<double>(row, col));
            temp_diagonals -= 1;
        }
        if(row == 7){ reachedRow8 = true; }
        if(!reachedRow8){ diagonals += 1; } else { diagonals -= 1; }

        if(!reachedRow8){ row = row + 1; /* down */ } else { col += 1; /* right */ }
        zigzagVector.push_back(block.at<double>(row, col));

        if(diagonals == 0){ break; }

        // up diagonal
        temp_diagonals = diagonals;
        while(temp_diagonals != 0){
            row -= 1; col += 1;
            zigzagVector.push_back(block.at<double>(row, col));
            temp_diagonals -= 1;
        }
        if(!reachedRow8){ diagonals += 1; } else { diagonals -= 1; }
    }
}

/*! Run Length Code
 * In this implementation, the code is the pair (number of zeros preceding this value, non zero value).
 * The pair (-1, dc) represents the beginning of a block.
 * @param vec zig zag vector.
 * @param code run length code.
 */
void runLengthPairs(std::vector<int> vec, std::vector<std::pair<int, int>> &code){
    int nZeros = 0;
    for(int elem : vec){
        if (elem != 0) {
            code.push_back(std::pair<int, int>(nZeros, elem));
            nZeros = 0;
        } else {
            nZeros += 1;
        }
    }
}

/*! Run Length Code
 * In this implementation, the code has the same symbols of the original, except if there is a sequence of one or more
 * zeros. In that case, that sequence is represented by two symbols: 0 and the number of zeros.
 * The symbols 0 0 represent the end of a block.
 * @param vec zig zag vector.
 * @param code run length code.
 */
void runLength(std::vector<int> vec, std::vector<int> &code){
    int nZeros = 0;
    for(int elem : vec){
        if (elem != 0) {
            if(nZeros > 0) {
                code.push_back(0);
                code.push_back(nZeros);
            }
            code.push_back(elem);
            nZeros = 0;
        } else {
            nZeros += 1;
        }
    }
    code.pop_back();
    code.push_back(0);
}

/*! Creates a new tree node.
 * @param data data to be stored in the new tree node.
 * @param leafLeft pointer to the new node's leaft leaf.
 * @param leafRight pointer to the new node's right leaf.
 * @return pointer to new node
 */
Node* newNode(int data, Node *leafLeft, Node *leafRight) {
    Node* node;
    node = new Node;
    node->data = data;
    node->left = leafLeft;
    node->right = leafRight;
    return node;
}

/*! Decode the huffman tree and store it in Node pointers so the decoder can decode faster.
 * @param decodedLeafs golomb decoded huffman tree (leafs).
 * @return huffman tree root node pointer.
 */
Node* huffmanTree(std::vector<int> decodedLeafs){
    std::vector<int>::iterator leafIt = decodedLeafs.begin();

    Node* father = nullptr;
    Node* leftLeaf = newNode(1,
                             (decodedLeafs.at(0) == -2) ? nullptr : newNode(decodedLeafs.at(0),
                                                                                     nullptr, nullptr),
                             newNode(decodedLeafs.at(1), nullptr, nullptr));
    Node* rightLeaf = nullptr;
    leafIt += 2;
    int leftLeafData;
    while(leafIt != decodedLeafs.end()){
        leftLeafData = *leafIt;
        leafIt++;
        rightLeaf = newNode(0,
                          (leftLeafData == -2) ? nullptr : newNode(leftLeafData, nullptr, nullptr),
                         newNode(*leafIt, nullptr, nullptr));
        leafIt++;

        father =  newNode(1, leftLeaf, rightLeaf);
        leftLeaf = father;
    }
    father->data = -4;

    return father;
}


/*! Decode the huffman tree and store it in Node pointers so the decoder can decode faster.
 * @param decodedLeafs golomb decoded huffman tree (leafs).
 * @return huffman tree root node pointer.
 */
Node* huffmanTreev2(std::vector<int> decodedLeafs){

    //TODO: REVISE AND TEST
}


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
std::vector<bool> huffmanTreeEncode(std::list<std::pair<int, double>> freqs_listNZero,
                                    std::list<std::pair<int, double>> freqs_listValue,
                                    std::unordered_map<int, std::vector<bool>> &codeZerosMap,
                                    std::unordered_map<int, std::vector<bool>> &codeValueMap,
                                    Golomb *golomb){
    std::vector<int> tree;
    std::vector<bool> encodedTree;
    int diffSizeLists = freqs_listValue.size() - freqs_listNZero.size();
    if (diffSizeLists == 0){
        codeZerosMap[freqs_listNZero.front().first].push_back(true);
        tree.push_back(freqs_listNZero.front().first);
        freqs_listNZero.erase(freqs_listNZero.begin());
    } else {
        tree.push_back(-2);
    }

    codeValueMap[freqs_listValue.front().first] = {};
    tree.push_back(freqs_listValue.front().first);
    freqs_listValue.erase(freqs_listValue.begin());
    diffSizeLists--;

    std::list<std::pair<int, double>>::iterator fNZero = freqs_listNZero.begin(), fValue = freqs_listValue.begin();
    while(fNZero != freqs_listNZero.end() && fValue != freqs_listValue.end()){
        if (diffSizeLists < 1){
            for(const auto &cw : codeZerosMap){ codeZerosMap[cw.first].insert(codeZerosMap[cw.first].begin(), true); }
            codeZerosMap[fNZero->first].insert(codeZerosMap[fNZero->first].begin(), false);
            tree.push_back(fNZero->first);
            fNZero++;
        } else {
            tree.push_back(-2);
        }
        for(const auto &cw : codeValueMap){ codeValueMap[cw.first].insert(codeValueMap[cw.first].begin(), true); }
        codeValueMap[fValue->first].insert(codeValueMap[fValue->first].begin(), false);
        tree.push_back(fValue->first);
        fValue++;
        diffSizeLists--;
    }
    tree.push_back(-3);  // end of tree

    //int count = 0;
    for(int leaf : tree){
        //std::cout << leaf << std::endl;
        golomb->encode2(leaf, encodedTree);
        /*
        int tmp = 0;
        for(bool bit : encodedTree) {
            tmp += 1;
            if (tmp > count){
                std::cout << bit;
            }
        }
        count = tmp;
        std::cout << std::endl;
         */
    }

    return encodedTree;
}

/*! Construct the huffman tree of the run length code and encode it in order to be sent to the decoder.
 * In this implementation all the values are in the right leafs (except for the least probable one).
 * The values on the leafs are Golomb encoded from top to bottom in depth.
 * We golomb encode -3 to represent the end of the tree.
 * @param freqs_list list of value of the run length code, frequency pairs.
 * @param codeMap map with the value from the run length code as keys and their frequency as value.
 * @param golomb pointer to Golomb Object.
 * @return golomb encoded huffman tree.
 */
std::vector<bool> huffmanTreeEncodev2(std::list<std::pair<int, double>> freqs_list,
                                      std::unordered_map<int, std::vector<bool>> &codeMap,
                                      Golomb *golomb){

    //TODO: REVISE AND TEST
    std::vector<int> tree;
    std::vector<bool> encodedTree;

    codeMap[freqs_list.front().first] = {};
    tree.push_back(freqs_list.front().first);
    freqs_list.erase(freqs_list.begin());

    for(std::pair<int, double> freq : freqs_list){
        for(const auto &cw : codeMap){ codeMap[cw.first].insert(codeMap[cw.first].begin(), true); }
        codeMap[freq.first].insert(codeMap[freq.first].begin(), false);
        tree.push_back(freq.first);
    }
    tree.push_back(-3);  // end of tree

    //int count = 0;
    for(int leaf : tree){
        //std::cout << leaf << std::endl;
        golomb->encode2(leaf, encodedTree);
        /*
        int tmp = 0;
        for(bool bit : encodedTree) {
            tmp += 1;
            if (tmp > count){
                std::cout << bit;
            }
        }
        count = tmp;
        std::cout << std::endl;
         */
    }

    return encodedTree;
}


/*!
 *
 * @param runLengthCode
 * @param code
 * @param encodedTree
 * @param golomb pointer to Golomb Object.
 */
void huffmanEncode(std::vector<std::pair<int, int>> runLengthCode, std::vector<bool> &code,
                                std::vector<bool> &encodedTree, Golomb* golomb){
    std::unordered_map<int, double> freqsMapNZero, freqsMapValue;
    std::unordered_map<int, std::vector<bool>> codeZerosMap, codeValueMap;

    for(std::pair<int, int> coeff: runLengthCode){
        freqsMapNZero[coeff.first]++;
        freqsMapValue[coeff.second]++;
    }
    freqsMapNZero[-3]++;// end of huffman code

    std::list<std::pair<int, double>> freqs_listNZero, freqs_listValue;
    std::unordered_map<int, double>::iterator fNZero = freqsMapNZero.begin(), fValue = freqsMapValue.begin();
    while(fValue != freqsMapValue.end()){
        freqsMapValue[fValue->first] = fValue->second / runLengthCode.size();
        freqs_listValue.push_back(std::pair<int, double>(fValue->first, freqsMapValue[fValue->first]));
        fValue++;

        if(fNZero != freqsMapNZero.end()){
            freqsMapNZero[fNZero->first] = fNZero->second / runLengthCode.size();
            freqs_listNZero.push_back(std::pair<int, double>(fNZero->first, freqsMapNZero[fNZero->first]));
            fNZero++;
        }
    }

    // sort codewords by frequency
    freqs_listNZero.sort([](const auto &a, const auto &b ) { return a.second < b.second; } );
    freqs_listValue.sort([](const auto &a, const auto &b ) { return a.second < b.second; } );

    //TODO: retest this function
    encodedTree = huffmanTreeEncode(freqs_listNZero, freqs_listValue, codeZerosMap, codeValueMap, golomb);

    for(std::pair<int, int> coeff: runLengthCode){
        code.insert(code.end(), codeZerosMap[coeff.first].begin(), codeZerosMap[coeff.first].end());
        code.insert(code.end(), codeValueMap[coeff.second].begin(), codeValueMap[coeff.second].end());
    }

    code.insert(code.end(), codeZerosMap[-3].begin(), codeZerosMap[-3].end());
}


/*!
 *
 * @param runLengthCode
 * @param code
 * @param golomb pointer to Golomb Object.
 */
void huffmanEncode(std::vector<std::pair<int, int>> runLengthCode, std::vector<bool> &code, Golomb* golomb){
    std::unordered_map<int, double> freqsMapNZero, freqsMapValue;
    std::unordered_map<int, std::vector<bool>> codeZerosMap, codeValueMap;

    for(std::pair<int, int> coeff: runLengthCode){
        freqsMapNZero[coeff.first]++;
        freqsMapValue[coeff.second]++;
    }
    freqsMapNZero[-3]++;// end of huffman code

    std::list<std::pair<int, double>> freqs_listNZero, freqs_listValue;
    std::unordered_map<int, double>::iterator fNZero = freqsMapNZero.begin(), fValue = freqsMapValue.begin();
    while(fValue != freqsMapValue.end()){
        freqsMapValue[fValue->first] = fValue->second / runLengthCode.size();
        freqs_listValue.push_back(std::pair<int, double>(fValue->first, freqsMapValue[fValue->first]));
        fValue++;

        if(fNZero != freqsMapNZero.end()){
            freqsMapNZero[fNZero->first] = fNZero->second / runLengthCode.size();
            freqs_listNZero.push_back(std::pair<int, double>(fNZero->first, freqsMapNZero[fNZero->first]));
            fNZero++;
        }
    }

    // sort codewords by frequency
    freqs_listNZero.sort([](const auto &a, const auto &b ) { return a.second < b.second; } );
    freqs_listValue.sort([](const auto &a, const auto &b ) { return a.second < b.second; } );

    // Golomb encoded tree
    std::vector<bool> encodedTree = huffmanTreeEncode(freqs_listNZero, freqs_listValue, codeZerosMap, codeValueMap, golomb);
    code.insert(code.end(), encodedTree.begin(), encodedTree.end());

    // Huffman code
    for(std::pair<int, int> coeff: runLengthCode){
        code.insert(code.end(), codeZerosMap[coeff.first].begin(), codeZerosMap[coeff.first].end());
        code.insert(code.end(), codeValueMap[coeff.second].begin(), codeValueMap[coeff.second].end());
    }

    code.insert(code.end(), codeZerosMap[-3].begin(), codeZerosMap[-3].end());

}


/*!
 *
 * @param runLengthCode
 * @param code
 * @param golomb pointer to Golomb Object.
 */
void huffmanEncodev2(std::vector<int> runLengthCode, std::vector<bool> &code, Golomb* golomb){

    //TODO: REVISE AND TEST
    std::unordered_map<int, double> freqsMap;
    std::unordered_map<int, std::vector<bool>> codeMap;

    for(int coeff: runLengthCode){
        freqsMap[coeff]++;
    }

    std::list<std::pair<int, double>> freqs_list;
    for(std::pair<int, double> freq : freqs_list){
        freqsMap[freq.first] = freq.second / runLengthCode.size();
        freqs_list.push_back(std::pair<int, double>(freq.first, freqsMap[freq.first]));
    }

    // sort codewords by frequency
    freqs_list.sort([](const auto &a, const auto &b ) { return a.second < b.second; } );

    // Golomb encoded tree
    std::vector<bool> encodedTree = huffmanTreeEncodev2(freqs_list, codeMap, golomb);
    code.insert(code.end(), encodedTree.begin(), encodedTree.end());

    // Huffman code
    for(int coeff: runLengthCode){
        code.insert(code.end(), codeMap[coeff].begin(), codeMap[coeff].end());
    }
}


/*!
 * @param huffmanTreeRoot
 */
void printHuffmanTree(Node* huffmanTreeRoot){

    int numberOfLeftLeafs = 0;
    int space = 0;
    Node* node = huffmanTreeRoot;
    while(node != nullptr){
        node = node->left;
        numberOfLeftLeafs += 1;
    }
    space = (int)((numberOfLeftLeafs/2 + 1)*4);
    numberOfLeftLeafs = (int)((numberOfLeftLeafs/2 + 1)*4*4);
    std::cout<<std::string(numberOfLeftLeafs, '\t')<<"*"<<std::endl;
    std::string nodeLine = "";
    std::string connectLine = "";
    std::vector<Node*> prevLineFathers = {huffmanTreeRoot};
    std::vector<Node*> currLineNodes;
    bool leafs;
    while(prevLineFathers.at(0)->right != nullptr){
        currLineNodes.clear();
        nodeLine = std::string(numberOfLeftLeafs+space, ' ');
        connectLine = std::string(numberOfLeftLeafs+space, ' ');
        leafs = false;
        for(Node* father : prevLineFathers){
            if(father->left != nullptr){
                currLineNodes.push_back(father->left);
                nodeLine += (father->data == 1) ? "\033[32m |" : " \033[31m|";
                nodeLine += "\033[39m";
                nodeLine += (father->left->right == nullptr) ? "\033[36m" : "\033[32m";
                nodeLine += std::to_string(father->left->data) + "\033[39m";
                nodeLine += (father->data == 1) ? "\033[32m|" : "\033[31m|";
                nodeLine += "\033[39m";
                connectLine += (father->data == 1) ? "   \033[32m/" : "  \033[31m/";
                connectLine += "\033[39m";
            } else {
                nodeLine += " ";
                connectLine += " ";
            }

            if(!leafs){
                nodeLine += "  ";
                connectLine += ' ';
            }
            space += 1;
            leafs = true;
            if(father->right != nullptr){
                currLineNodes.push_back(father->right);
                nodeLine += (father->data == 1) ? "\033[32m |" : " \033[31m|";
                nodeLine += "\033[39m";
                nodeLine += (father->right->right == nullptr) ? "\033[33m" : "\033[31m";
                nodeLine += std::to_string(father->right->data) + "\033[39m";
                nodeLine += (father->data == 1) ? "\033[32m|" : "\033[31m|";
                nodeLine += "\033[39m";
                connectLine += (father->data == 1) ? "   \033[32m\\" : "  \033[31m\\";
                connectLine += "\033[39m";
            } else {
                nodeLine += " ";
                connectLine += " ";
            }
        }
        prevLineFathers = currLineNodes;

        std::cout<<connectLine<<std::endl;
        std::cout<<nodeLine<<std::endl;
        numberOfLeftLeafs -= 7;
    }

    std::cout<<"Legend:\n\t\033[36mNumber of preceding zeros\033[31m\n\t\033[33mValue\033[31m"<<std::endl;
}

/*!
 * @param code
 * @param encodedTree
 * @param runLengthCode
 * @param golomb pointer to Golomb Object.
 */
void huffmanDecode(std::vector<bool> &code, std::vector<bool> &encodedTree,
                   std::vector<std::pair<int, int>> &runLengthCode, Golomb* golomb){
    std::vector<int> decodedLeafs;
    golomb->decode3(encodedTree, decodedLeafs);
    decodedLeafs.pop_back(); // -3 golomb encoded to represent the end of the huffman tree

    Node* huffmanTreeRoot = huffmanTree(decodedLeafs);
    printHuffmanTree(huffmanTreeRoot);
    Node* node = huffmanTreeRoot;
    int nZeros = -4;
    for(bool bit : code){
        node = bit ? node->left : node->right;

        // if we are decoding the number of zeros
        if(nZeros == -4){
            if(node->left->left == nullptr && node->left->right == nullptr) { // number of zeros are in the left leafs
                if(node->left->data == -3){
                    break;
                }
                nZeros = node->left->data;
                node = huffmanTreeRoot;
            }
        } else { // if we are decoding the value
            if(node->right->left == nullptr && node->right->right == nullptr) { // values are in the right leafs
                runLengthCode.push_back(std::pair<int, int>(nZeros, node->right->data));
                nZeros = -4;
                node = huffmanTreeRoot;
            }
        }
    }
}

/*!
 * @param code
 * @param encodedTree
 * @param runLengthCode
 * @param golomb pointer to Golomb Object.
 */
void huffmanDecodev2(std::vector<bool> &code, std::vector<bool> &encodedTree,
                   std::vector<int> &runLengthCode, Golomb* golomb){
    //TODO: REVISE AND TEST

    std::vector<int> decodedLeafs;
    golomb->decode3(encodedTree, decodedLeafs);
    decodedLeafs.pop_back(); // -3 golomb encoded to represent the end of the huffman tree

    Node* huffmanTreeRoot = huffmanTree(decodedLeafs);
    //printHuffmanTree(huffmanTreeRoot);
    Node* node = huffmanTreeRoot;
    for(bool bit : code){
        if(bit){
            node = node->left;
            if(node->right == nullptr){
                runLengthCode.push_back(node->data);
                node = huffmanTreeRoot;
            }
        } else {
            runLengthCode.push_back(node->right->data);
            node = huffmanTreeRoot;
        }
    }
}


/*!
 *
 * @param runLengthCode
 * @param frame
 */
void getImage(std::vector<std::pair<int, int>> runLengthCode, cv::Mat &frame){
    std::vector<std::pair<int, int>>::iterator rlIt = runLengthCode.begin();
    int diagonals, row = 0, col = 0;
    bool reachedRow8;
    std::vector<int> zigzag = std::vector(64, 0);
    std::vector<int>::iterator zigzagIt = zigzag.begin();
    while(rlIt != runLengthCode.end()){
        /* int bob = rlIt->first == -1; // Beginning of block */
        frame.at<double>(row, col) = rlIt->second; // dc
        if(col == frame.cols - 1){
            col = 0;
            row += 1;
        } else if(rlIt != runLengthCode.begin()){
            col += 1;
            row -= 8;
        }
        rlIt++;
        diagonals = 1;
        reachedRow8 = false;

        while(rlIt->first != -1 && rlIt != runLengthCode.end()){
            if(rlIt->first != 0){
                zigzagIt += rlIt->first;
            }
            *zigzagIt = rlIt->second;
            zigzagIt++;
            rlIt++;
        }
        zigzagIt = zigzag.begin();
        while(zigzagIt != zigzag.end()){
            if(!reachedRow8){ col += 1; /* right */ } else { row = row + 1; /* down */ }
            frame.at<double>(row, col) = *zigzagIt;
            zigzagIt++;

            // down diagonal
            int temp_diagonals = diagonals;
            while(temp_diagonals != 0){
                row += 1; col -= 1;
                frame.at<double>(row, col) = *zigzagIt;
                zigzagIt++;
                temp_diagonals -= 1;
            }
            if(row == 7){ reachedRow8 = true; }
            if(!reachedRow8){ diagonals += 1; } else { diagonals -= 1; }

            if(!reachedRow8){ row = row + 1; /* down */ } else { col += 1; /* right */ }
            frame.at<double>(row, col) = *zigzagIt;
            zigzagIt++;

            if(diagonals == 0){ break; }

            // up diagonal
            temp_diagonals = diagonals;
            while(temp_diagonals != 0){
                row -= 1; col += 1;
                frame.at<double>(row, col) = *zigzagIt;
                zigzagIt++;
                temp_diagonals -= 1;
            }
            if(!reachedRow8){ diagonals += 1; } else { diagonals -= 1; }
        }
        zigzag.clear();
    }
}


/*!
 *
 * @param frame
 * @param prevDCs
 * @param golomb
 * @param encodedTree
 * @param code
 */
void quantizeDctBaselineJPEG(cv::Mat &frame, std::vector<int> &prevDCs, Golomb* golomb, std::vector<bool> &encodedTree,
                             std::vector<bool> &code) {

    divideImageIn8x8Blocks(frame);

    // ***************************************** Calculation of the DCT ************************************************
    cv::Mat block;
    std::vector<int> zigzagVector;
    std::vector<int>::iterator prevDC = prevDCs.begin();
    std::vector<std::pair<int, int>> runLength, blockACs;
    double dc;
    for(int r = 0; r < frame.rows; r += 8) {
        for(int c = 0; c < frame.cols; c += 8) {
            frame(cv::Rect(c, r,8,8)).copyTo(block);

            // TODO see what jpeg matrix to use
            quantizeBlock(block, quantMatrixLuminance);
            block.copyTo(frame(cv::Rect(c, r,8,8)));

            // Next, the coefficients are organized in a one-dimensional vector according to a zig-zag scan.
            zigzagVector.clear();
            zigZagScan(block, zigzagVector);

            dc = zigzagVector.at(0);
            zigzagVector.erase(zigzagVector.begin()); // remove dc from zigzag array

            // ********************* Statistical coding (Huffman) of the quantized DCT coefficients ********************

            // The non-zero AC coefficients are encoded using Huffman or arithmetic coding, representing the value of
            // the coefficient, as well as the number of zeros preceding it.
            // In this case  the symbol to represent the end of the block doesn't exist. We use -1 in the value
            // representing the number of zeros, to represent the beginning of a block, followed by the dc.
            blockACs.clear();
            runLengthPairs(zigzagVector, blockACs);

            // The DC coefficient of each block is predicatively encoded in relation to the DC coefficient of the
            // previous block.
            runLength.push_back(std::pair(-1, (int)(dc - *prevDC)));
            frame.at<double>(r,c) = (int)(dc - *prevDC);
            *prevDC = dc;
            prevDC++;

            runLength.insert(runLength.end(), blockACs.begin(), blockACs.end());
        }
    }

    huffmanEncode(runLength, code, encodedTree, golomb);
}



/*!
 *
 * @param frame
 * @param prevDCs
 * @param golomb
 * @param code
 * @param luminance
 */
void quantizeDctBaselineJPEG(cv::Mat &frame, std::vector<int> &prevDCs, Golomb* golomb, std::vector<bool> &code, bool luminance) {

    divideImageIn8x8Blocks(frame);

    // ***************************************** Calculation of the DCT ************************************************
    cv::Mat block;
    std::vector<int> zigzagVector;
    int prevDCIt = 0;
    std::vector<std::pair<int, int>> runLength, blockACs;
    double dc;
    bool prevDcsEmpty = prevDCs.empty();
    for(int r = 0; r < frame.rows; r += 8) {
        for(int c = 0; c < frame.cols; c += 8) {
            frame(cv::Rect(c, r,8,8)).copyTo(block);
            quantizeBlock(block, luminance ? quantMatrixLuminance : quantMatrixChrominance);
            block.copyTo(frame(cv::Rect(c, r,8,8)));

            // Next, the coefficients are organized in a one-dimensional vector according to a zig-zag scan.
            zigzagVector.clear();
            zigZagScan(block, zigzagVector);

            dc = zigzagVector.at(0);
            zigzagVector.erase(zigzagVector.begin()); // remove dc from zigzag array

            // ********************* Statistical coding (Huffman) of the quantized DCT coefficients ********************

            // The non-zero AC coefficients are encoded using Huffman or arithmetic coding, representing the value of
            // the coefficient, as well as the number of zeros preceding it.
            // In this case  the symbol to represent the end of the block doesn't exist. We use -1 in the value
            // representing the number of zeros, to represent the beginning of a block, followed by the dc.
            blockACs.clear();
            runLengthPairs(zigzagVector, blockACs);

            // The DC coefficient of each block is predicatively encoded in relation to the DC coefficient of the
            // previous block.
            if(prevDcsEmpty){
                prevDCs.push_back(0);
            }
            runLength.push_back(std::pair(-1, (int)(dc - prevDCs.at(prevDCIt))));
            frame.at<double>(r,c) = (int)(dc - prevDCs.at(prevDCIt));
            prevDCs.at(prevDCIt) = dc;
            prevDCIt++;

            runLength.insert(runLength.end(), blockACs.begin(), blockACs.end());
        }
    }

    huffmanEncode(runLength, code, golomb);
}


//!
/*!
 *
 * @param prevDCs
 * @param runLengthCode
 * @param frame
 */
void inverseQuantizeDctBaselineJPEG(std::vector<int> &prevDCs, std::vector<std::pair<int, int>> runLengthCode,
                                    cv::Mat &frame, bool luminance){
    std::vector<int>::iterator dcIt = prevDCs.begin();
    std::vector<std::pair<int, int>>::iterator rlIt = runLengthCode.begin();
    cv::Mat block;
    int diagonals, row = 0, col = 0;
    bool reachedRow8;
    std::vector<int> zigzag = std::vector(64, 0);
    std::vector<int>::iterator zigzagIt = zigzag.begin();
    while(rlIt != runLengthCode.end()){
        /* int bob = rlIt->first == -1; // Beginning of block */
        *dcIt += rlIt->second; // refresh prevDCs
        frame.at<double>(row, col) = rlIt->second + *dcIt; // dc
        dcIt++;
        if(col == frame.cols - 1){
            col = 0;
            row += 1;
        } else if(rlIt != runLengthCode.begin()){
            col += 1;
            row -= 8;
        }
        rlIt++;
        diagonals = 1;
        reachedRow8 = false;

        while(rlIt->first != -1 && rlIt != runLengthCode.end()){
            if(rlIt->first != 0){
                zigzagIt += rlIt->first;
            }
            *zigzagIt = rlIt->second;
            zigzagIt++;
            rlIt++;
        }
        zigzagIt = zigzag.begin();
        while(zigzagIt != zigzag.end()){
            if(!reachedRow8){ col += 1; /* right */ } else { row = row + 1; /* down */ }
            frame.at<double>(row, col) = *zigzagIt;
            zigzagIt++;

            // down diagonal
            int temp_diagonals = diagonals;
            while(temp_diagonals != 0){
                row += 1; col -= 1;
                frame.at<double>(row, col) = *zigzagIt;
                zigzagIt++;
                temp_diagonals -= 1;
            }
            if(row == 7){ reachedRow8 = true; }
            if(!reachedRow8){ diagonals += 1; } else { diagonals -= 1; }

            if(!reachedRow8){ row = row + 1; /* down */ } else { col += 1; /* right */ }
            frame.at<double>(row, col) = *zigzagIt;
            zigzagIt++;

            if(diagonals == 0){ break; }

            // up diagonal
            temp_diagonals = diagonals;
            while(temp_diagonals != 0){
                row -= 1; col += 1;
                frame.at<double>(row, col) = *zigzagIt;
                zigzagIt++;
                temp_diagonals -= 1;
            }
            if(!reachedRow8){ diagonals += 1; } else { diagonals -= 1; }
        }
        zigzag.clear();

        frame(cv::Rect(col - 7, row - 7, 8, 8)).copyTo(block);
        inverseQuantizeBlock(block, luminance ? quantMatrixLuminance : quantMatrixChrominance);
        block.copyTo(frame(cv::Rect(col - 7, row - 7, 8, 8)));
    }
}
