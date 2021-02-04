#include    <opencv2/core.hpp>
#include    <opencv2/opencv.hpp>
#include    <cmath>
#include    "Golomb.cpp"
#include    "../src/HuffmanDecoder.cpp"
#include    "../includes/JPEGQuantization.hpp"

JPEGQuantization::JPEGQuantization() {
    for(int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            this->transformationMatrix.at<double>(r, c) = (r == 0) ? 1/(sqrt(8)) : sqrt((2/(double)8))*cos((M_PI*(2*c+1)*r) / (2*(double)8));
        }
    }
}

void JPEGQuantization::divideImageIn8x8Blocks(cv::Mat &frame){
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
}

void JPEGQuantization::dct(cv::Mat &block){
    // DCT of the block: T*block*T'
    cv::Mat mult1 = cv::Mat::zeros(8, 8, CV_64F);
    cv::Mat dct = cv::Mat::zeros(8, 8, CV_64F);
    for(int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            for (int s = 0; s < 8; ++s) {
                // Subtract 2^(b−1) to each pixel value, where b is the number of bits used to represent the pixels.
                mult1.at<double>(r, c) += this->transformationMatrix.at<double>(r, s) * (block.at<double>(s, c) - pow(2, 7));
            }
        }
    }
    for(int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            for (int s = 0; s < 8; ++s) {
                dct.at<double>(r, c) += mult1.at<double>(r, s) * this->transformationMatrix.at<double>(c, s) /* transpose of T */;
            }
        }
    }

    for(int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            block.at<double>(r, c) = (int)(dct.at<double>(r, c) + 0.5 - (dct.at<double>(r, c)<0)); // https://stackoverflow.com/questions/9695329/c-how-to-round-a-double-to-an-int
        }
    }
}

void JPEGQuantization::inverseDCT(cv::Mat &block){
    // inverse of the DCT: T'*dct*T
    cv::Mat mult1 = cv::Mat::zeros(8, 8, CV_64F);
    cv::Mat inverse_dct = cv::Mat::zeros(8, 8, CV_64F);

    for(int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            for (int s = 0; s < 8; ++s) {
                mult1.at<double>(r, c) += this->transformationMatrix.at<double>(s, r) /* transpose of T */ * block.at<double>(s, c);
            }
        }
    }

    for(int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            for (int s = 0; s < 8; ++s) {
                inverse_dct.at<double>(r, c) += mult1.at<double>(r, s) * this->transformationMatrix.at<double>(s, c);
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

void JPEGQuantization::quantizeDCTCoeff(cv::Mat &block, cv::Mat quantMatrix){
    double temp;
    for(int r = 0; r < 8; r++){
        for (int c = 0; c < 8; c++) {
            // ******************************* Quantization of the DCT coefficients ****************************
            // Quantization of the DCT coefficients, in order to eliminate less relevant information, according
            // to the characteristics of the human visual system.
            // The DCT coefficients are quantized using a quantization matrix, previously scaled by a compression
            // quality factor.
            temp = block.at<double>(r, c) / quantMatrix.at<double>(r, c); // ỹ(r,c)=ROUND(y(r,c)/q(r,c))
            block.at<double>(r, c) = (int)(temp + 0.5 - (temp<0)); // https://stackoverflow.com/questions/9695329/c-how-to-round-a-double-to-an-int
        }
    }
}

void JPEGQuantization::inverseQuantizeDCTCoeff(cv::Mat &block, cv::Mat quantMatrix){
    double temp;
    for(int r = 0; r < 8; r++){
        for (int c = 0; c < 8; c++) {
            // ******************************* Quantization of the DCT coefficients ****************************
            // Quantization of the DCT coefficients, in order to eliminate less relevant information, according
            // to the characteristics of the human visual system.
            // The DCT coefficients are quantized using a quantization matrix, previously scaled by a compression
            // quality factor.
            temp = block.at<double>(r, c) * quantMatrix.at<double>(r, c); // ỹ(r,c)=ROUND(y(r,c)/q(r,c))
            block.at<double>(r, c) = (int)(temp + 0.5 - (temp<0)); // https://stackoverflow.com/questions/9695329/c-how-to-round-a-double-to-an-int
        }
    }
}

void JPEGQuantization::quantizeBlock(cv::Mat &block, cv::Mat quantMatrix){
    // DCT of the block: T*block*T'
    cv::Mat mult1 = cv::Mat::zeros(8, 8, CV_64F);
    cv::Mat dct = cv::Mat::zeros(8, 8, CV_64F);
    for(int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            for (int s = 0; s < 8; ++s) {
                // Subtract 2^(b−1) to each pixel value, where b is the number of bits used to represent the pixels.
                mult1.at<double>(r, c) += this->transformationMatrix.at<double>(r, s) * (block.at<double>(s, c) - pow(2, 7));
            }
        }
    }
    for(int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            for (int s = 0; s < 8; ++s) {
                dct.at<double>(r, c) += mult1.at<double>(r, s) * this->transformationMatrix.at<double>(c, s) /* transpose of T */;
            }
        }
    }

    double temp;
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

void JPEGQuantization::inverseQuantizeBlock(cv::Mat &block, cv::Mat quantMatrix){
    // inverse of the DCT: T'*dct*T
    cv::Mat mult1 = cv::Mat::zeros(8, 8, CV_64F);
    cv::Mat inverse_dct = cv::Mat::zeros(8, 8, CV_64F);

    double temp;
    for(int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            // ******************************* Quantization of the DCT coefficients ****************************
            // Quantization of the DCT coefficients, in order to eliminate less relevant information, according
            // to the characteristics of the human visual system.
            // The DCT coefficients are quantized using a quantization matrix, previously scaled by a compression
            // quality factor.
            temp = floor(block.at<double>(r, c) * quantMatrix.at<double>(r, c)); // ỹ(r,c)=ROUND(y(r,c)/q(r,c))
            block.at<double>(r, c) = (int)(temp - 0.5 - (temp<0)); // https://stackoverflow.com/questions/9695329/c-how-to-round-a-double-to-an-int
        }
    }

    for(int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            for (int s = 0; s < 8; ++s) {
                mult1.at<double>(r, c) += this->transformationMatrix.at<double>(s, r) /* transpose of T */ * block.at<double>(s, c);
            }
        }
    }

    for(int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            for (int s = 0; s < 8; ++s) {
                inverse_dct.at<double>(r, c) += mult1.at<double>(r, s) * this->transformationMatrix.at<double>(s, c);
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

void JPEGQuantization::zigZagScan(cv::Mat &block, std::vector<int> &zigzagVector){
    zigzagVector.push_back(block.at<double>(0, 0)); // dc
    int row = 0, col = 0, diagonals = 1, temp_diagonals;
    bool reachedRow8 = false;
    while(true){
        if(!reachedRow8){ col += 1; /* right */ } else { row = row + 1; /* down */ }
        zigzagVector.push_back(block.at<double>(row, col));

        // down diagonal
        temp_diagonals = diagonals;
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

void JPEGQuantization::runLengthPairs(std::vector<int> &vec, std::vector<std::pair<int, int>> &code){
    int nZeros = 0;
    for(int elem : vec){
        if (elem != 0) {
            code.emplace_back(nZeros, elem);
            nZeros = 0;
        } else {
            nZeros += 1;
        }
    }
}

Node* JPEGQuantization::newNode(int data, Node *leafLeft, Node *leafRight) {
    Node* node;
    node = new Node;
    node->data = data;
    node->left = leafLeft;
    node->right = leafRight;
    return node;
}

Node* JPEGQuantization::huffmanTree(std::vector<int> decodedLeafs){
    auto leafIt = decodedLeafs.begin();

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
        father = newNode(1, leftLeaf, rightLeaf);
        leftLeaf = father;
    }
    father->data = -4;

    return father;
}

std::vector<bool> JPEGQuantization::huffmanTreeEncode(std::list<std::pair<int, double>> freqs_listNZero,
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

    auto fNZero = freqs_listNZero.begin(), fValue = freqs_listValue.begin();
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

    for(int leaf : tree){
        golomb->encode2(leaf, encodedTree);
    }
    return encodedTree;
}

void JPEGQuantization::huffmanEncode(std::vector<std::pair<int, int>> &runLengthCode, std::vector<bool> &code,
                                std::vector<bool> &encodedTree, Golomb* golomb){
    std::unordered_map<int, double> freqsMapNZero, freqsMapValue;
    std::unordered_map<int, std::vector<bool>> codeZerosMap, codeValueMap;

    for(std::pair<int, int> coeff: runLengthCode){
        freqsMapNZero[coeff.first]++;
        freqsMapValue[coeff.second]++;
    }
    freqsMapNZero[-4]++;// end of huffman code

    std::list<std::pair<int, double>> freqs_listNZero, freqs_listValue;
    auto fNZero = freqsMapNZero.begin(), fValue = freqsMapValue.begin();
    while(fValue != freqsMapValue.end()){
        freqsMapValue[fValue->first] = fValue->second / runLengthCode.size();
        freqs_listValue.emplace_back(fValue->first, freqsMapValue[fValue->first]);
        fValue++;

        if(fNZero != freqsMapNZero.end()){
            freqsMapNZero[fNZero->first] = fNZero->second / runLengthCode.size();
            freqs_listNZero.emplace_back(fNZero->first, freqsMapNZero[fNZero->first]);
            fNZero++;
        }
    }

    // sort codewords by frequency
    freqs_listNZero.sort([](const auto &a, const auto &b ) { return a.second < b.second; } );
    freqs_listValue.sort([](const auto &a, const auto &b ) { return a.second < b.second; } );

    encodedTree = huffmanTreeEncode(freqs_listNZero, freqs_listValue, codeZerosMap, codeValueMap, golomb);

    for(std::pair<int, int> coeff: runLengthCode){
        code.insert(code.end(), codeZerosMap[coeff.first].begin(), codeZerosMap[coeff.first].end());
        code.insert(code.end(), codeValueMap[coeff.second].begin(), codeValueMap[coeff.second].end());
    }

    code.insert(code.end(), codeZerosMap[-4].begin(), codeZerosMap[-4].end());
}

void JPEGQuantization::huffmanEncode(const std::vector<std::pair<int, int>> &runLengthCode, std::vector<bool> &code, Golomb* golomb){
    std::unordered_map<int, double> freqsMapNZero, freqsMapValue;
    std::unordered_map<int, std::vector<bool>> codeZerosMap, codeValueMap;

    for(std::pair<int, int> coeff: runLengthCode){
        freqsMapNZero[coeff.first]++;
        freqsMapValue[coeff.second]++;
    }
    freqsMapNZero[-4]++;// end of huffman code

    std::list<std::pair<int, double>> freqs_listNZero, freqs_listValue;
    auto fNZero = freqsMapNZero.begin(), fValue = freqsMapValue.begin();
    while(fValue != freqsMapValue.end()){
        freqsMapValue[fValue->first] = fValue->second / runLengthCode.size();
        freqs_listValue.emplace_back(fValue->first, freqsMapValue[fValue->first]);
        fValue++;

        if(fNZero != freqsMapNZero.end()){
            freqsMapNZero[fNZero->first] = fNZero->second / runLengthCode.size();
            freqs_listNZero.emplace_back(fNZero->first, freqsMapNZero[fNZero->first]);
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

    code.insert(code.end(), codeZerosMap[-4].begin(), codeZerosMap[-4].end());
}

void JPEGQuantization::printHuffmanTree(Node* huffmanTreeRoot){
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
    std::string nodeLine, connectLine;
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

void JPEGQuantization::huffmanDecode(std::vector<bool> &code, std::vector<bool> &encodedTree,
                   std::vector<std::pair<int, int>> &runLengthCode, Golomb* golomb){
    std::vector<int> decodedLeafs;
    golomb->decode3(encodedTree, decodedLeafs);
    decodedLeafs.pop_back(); // -3 golomb encoded to represent the end of the huffman tree

    Node* huffmanTreeRoot = huffmanTree(decodedLeafs);
    printHuffmanTree(huffmanTreeRoot);
    Node* node = huffmanTreeRoot;
    int nZeros = -5;
    for(bool bit : code){
        node = bit ? node->left : node->right;
        // if we are decoding the number of zeros
        if(nZeros == -5){
            if(node->left->left == nullptr && node->left->right == nullptr) { // number of zeros are in the left leafs
                if(node->left->data == -3){
                    break;
                }
                nZeros = node->left->data;
                node = huffmanTreeRoot;
            }
        } else { // if we are decoding the value
            if(node->right->left == nullptr && node->right->right == nullptr) { // values are in the right leafs
                runLengthCode.emplace_back(nZeros, node->right->data);
                nZeros = -5;
                node = huffmanTreeRoot;
            }
        }
    }
}

void JPEGQuantization::getImage(std::vector<std::pair<int, int>> runLengthCode, cv::Mat &frame){
    auto rlIt = runLengthCode.begin();
    int diagonals, row = 0, col = 0, temp_diagonals;
    bool reachedRow8;
    std::vector<int> zigzag = std::vector(64, 0);
    auto zigzagIt = zigzag.begin();
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
            temp_diagonals = diagonals;
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

void JPEGQuantization::quantizeDctBaselineJPEG(cv::Mat &frame, std::vector<int> &prevDCs, Golomb* golomb, std::vector<bool> &encodedTree,
                             std::vector<bool> &code, bool luminance) {
    if(frame.rows % 8 != 0 or frame.cols % 8 != 0){
        divideImageIn8x8Blocks(frame);
    }

    // ***************************************** Calculation of the DCT ************************************************
    cv::Mat block;
    std::vector<int> zigzagVector;
    auto prevDC = prevDCs.begin();
    std::vector<std::pair<int, int>> runLength, blockACs;
    double dc;
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
            runLength.emplace_back(-1, (int)(dc - *prevDC));
            frame.at<double>(r,c) = (int)(dc - *prevDC);
            *prevDC = dc;
            prevDC++;

            runLength.insert(runLength.end(), blockACs.begin(), blockACs.end());
        }
    }

    huffmanEncode(runLength, code, encodedTree, golomb);
}

void JPEGQuantization::quantizeDctBaselineJPEG(cv::Mat &frame, std::vector<int> &prevDCs, Golomb* golomb, std::vector<bool> &code, bool luminance) {

    if(frame.rows % 8 != 0 or frame.cols % 8 != 0){
        divideImageIn8x8Blocks(frame);
    }
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
            runLength.emplace_back(-1, (int)(dc - prevDCs.at(prevDCIt)));
            frame.at<double>(r,c) = (int)(dc - prevDCs.at(prevDCIt));
            prevDCs.at(prevDCIt) = dc;
            prevDCIt++;

            runLength.insert(runLength.end(), blockACs.begin(), blockACs.end());
        }
    }

    huffmanEncode(runLength, code, golomb);
}


void JPEGQuantization::quantize(cv::Mat &frame, std::vector<int> &prevDCs, Golomb* golomb, std::vector<bool> &code, bool luminance) {

    if(frame.rows % 8 != 0 or frame.cols % 8 != 0){
        divideImageIn8x8Blocks(frame);
    }
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
            runLength.emplace_back(-1, (int)(dc - prevDCs.at(prevDCIt)));
            prevDCs.at(prevDCIt) = dc;
            prevDCIt++;

            runLength.insert(runLength.end(), blockACs.begin(), blockACs.end());
        }
    }

    huffmanEncode(runLength, code, golomb);
}


void JPEGQuantization::inverseQuantizeDctBaselineJPEG(std::vector<int> &prevDCs, std::vector<std::pair<int, int>> runLengthCode,
                                    cv::Mat &frame, bool luminance){
    auto dcIt = prevDCs.begin();
    auto rlIt = runLengthCode.begin();
    cv::Mat block;
    int diagonals, row = 0, col = 0, temp_diagonals;
    bool reachedRow8;
    std::vector<int> zigzag = std::vector(64, 0);
    auto zigzagIt = zigzag.begin();
    while(rlIt != runLengthCode.end()){
        /* int bob = rlIt->first == -1; // Beginning of block */
        frame.at<double>(row, col) = rlIt->second + *dcIt; // dc
        *dcIt += rlIt->second; // refresh prevDCs
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
            temp_diagonals = diagonals;
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
void JPEGQuantization::inverseQuantizeDctBaselineJPEG(int f_rows, int f_cols, std::vector<int> &prevDCs,
                                                      std::vector<std::pair<int, int>> runLengthCode,
                                                      vector<int> &outRes, bool luminance){
    auto dcIt = prevDCs.begin();
    auto rlIt = runLengthCode.begin();
    cv::Mat frame = cv::Mat::zeros(f_rows, f_cols, CV_64F), block;
    if(f_rows % 8 != 0 or f_cols % 8 != 0){
        divideImageIn8x8Blocks(frame);
    }
    int diagonals, row = 0, col = 0, temp_diagonals;
    bool reachedRow8;
    std::vector<int> zigzag = std::vector(64, 0);
    auto zigzagIt = zigzag.begin();
    while(rlIt != runLengthCode.end()){
        /* int bob = rlIt->first == -1; // Beginning of block */
        frame.at<double>(row, col) = rlIt->second + *dcIt; // dc
        *dcIt += rlIt->second; // refresh prevDCs
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
            temp_diagonals = diagonals;
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


    for(int r = 0; r < f_rows; r += 1) {
        for (int c = 0; c < f_cols; c += 1) {
            outRes.push_back((int) frame.at<double>(r, c));
        }
    }
}

cv::Mat JPEGQuantization::getQuantMatrixLuminance(){
    return this->quantMatrixLuminance;
}

cv::Mat JPEGQuantization::getQuantMatrixChrominance(){
    return this->quantMatrixChrominance;
}