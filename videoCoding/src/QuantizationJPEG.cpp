//!  QuantizationJPEG
/*!
 *  Quantization using the sequential (baseline) or the progressive mode of JPEG.
 *  @author Inês Justo
*/

#include    <opencv2/core.hpp>
#include    <opencv2/opencv.hpp>
#include    <cmath>
#include    "../../entropyCoding/src/Golomb.cpp"

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
                                   {47, 66, 99, 99, 99, 99, 80, 99},
                                   {99, 99, 99, 99, 99, 99, 99, 99},
                                   {99, 99, 99, 99, 99, 99, 99, 99},
                                   {99, 99, 99, 99, 99, 99, 99, 99},
                                   {99, 99, 99, 99, 99, 99, 99, 99}};

cv::Mat quantMatrixGrayscale = cv::Mat(8, 8, CV_64F, &jpeg_matrix_grayscale);
cv::Mat quantMatrixColor = cv::Mat(8, 8, CV_64F, &jpeg_matrix_color);

//! Transformation matrix (T)
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

cv::Mat t = transformationMatrix();

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

//! DCT following the Transformation Matrix Approach
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

void quantDCTCoeff(cv::Mat &block, cv::Mat quantMatrix){
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

void wholeDCTQuant(cv::Mat &block, cv::Mat quantMatrix){
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

std::vector<int> zigZagScan(cv::Mat &block){
    std::vector<int> zigzag_array;
    double dc = block.at<double>(0, 0);
    int row = 0, col = 0, diagonals = 1;
    bool reachedRow8 = false;
    while(true){
        if(!reachedRow8){ col += 1; /* right */ } else { row = row + 1; /* down */ }
        zigzag_array.push_back(block.at<double>(row, col));

        // down diagonal
        int temp_diagonals = diagonals;
        while(temp_diagonals != 0){
            row += 1; col -= 1;
            zigzag_array.push_back(block.at<double>(row, col));
            temp_diagonals -= 1;
        }
        if(row == 7){ reachedRow8 = true; }
        if(!reachedRow8){ diagonals += 1; } else { diagonals -= 1; }

        if(!reachedRow8){ row = row + 1; /* down */ } else { col += 1; /* right */ }
        zigzag_array.push_back(block.at<double>(row, col));

        if(diagonals == 0){ break; }

        // up diagonal
        temp_diagonals = diagonals;
        while(temp_diagonals != 0){
            row -= 1; col += 1;
            zigzag_array.push_back(block.at<double>(row, col));
            temp_diagonals -= 1;
        }
        if(!reachedRow8){ diagonals += 1; } else { diagonals -= 1; }
    }

    return zigzag_array;
}

//! Run Length Code
/*!
 * In this implementation, the code is the pair (number of zeros preceding this value, non zero value)
 * @param arr
 * @return
 */
std::vector<std::pair<int, int>> runLengthCode(std::vector<int> arr){
    std::vector<std::pair<int, int>> code;
    int nZeros = 0;
    for(int elem : arr){
        if (elem != 0) {
            code.push_back(std::pair<int, int>(nZeros, elem));
            nZeros = 0;
        } else {
            nZeros += 1;
        }
    }
    return code;
}

// Data structure to store a tree node
struct Node {
    int data;
    Node *left, *right;
};

// Function to create a new tree node
Node* newNode(int data, Node *leafLeft, Node *leafRight) {
    Node* node = new Node;
    node->data = data;
    node->left = leafLeft;
    node->right = leafRight;

    return node;
}

Node* huffmanTree(std::list<std::pair<int, double>> freqs_listNZero, std::list<std::pair<int, double>> freqs_listValue,
                  std::unordered_map<int, std::vector<bool>> &codeZerosMap,
                  std::unordered_map<int, std::vector<bool>> &codeValueMap){
    int diffSizeLists = freqs_listValue.size() - freqs_listNZero.size();
    Node* leftLeafLeftLeaf = nullptr;
    if (diffSizeLists == 0){
        leftLeafLeftLeaf = newNode(freqs_listNZero.front().first, nullptr, nullptr);
        codeZerosMap[freqs_listNZero.front().first].push_back(true);
        freqs_listNZero.erase(freqs_listNZero.begin());
    }

    Node* father = nullptr;
    Node* leftLeaf = newNode(1,
                                   leftLeafLeftLeaf,
                          newNode(freqs_listValue.front().first, nullptr, nullptr));

    codeValueMap[freqs_listValue.front().first] = {};
    freqs_listValue.erase(freqs_listValue.begin());
    diffSizeLists--;

    std::list<std::pair<int, double>>::iterator fNZero = freqs_listNZero.begin();
    std::list<std::pair<int, double>>::iterator fValue = freqs_listValue.begin();
    while(fNZero != freqs_listNZero.end() && fValue != freqs_listValue.end()){
        Node* rightLeafLeftLeaf = nullptr;
        if (diffSizeLists < 1){
            rightLeafLeftLeaf = newNode(fNZero->first, nullptr, nullptr);

            for(const auto &cw : codeZerosMap){ codeZerosMap[cw.first].insert(codeZerosMap[cw.first].begin(), true); }
            codeZerosMap[fNZero->first].insert(codeZerosMap[fNZero->first].begin(), false);
            fNZero++;
        }

        Node* rightLeaf = newNode(0, rightLeafLeftLeaf, newNode(fValue->first, nullptr, nullptr));
        father =  newNode(1, leftLeaf, rightLeaf);
        leftLeaf = father;

        for(const auto &cw : codeValueMap){ codeValueMap[cw.first].insert(codeValueMap[cw.first].begin(), true); }
        codeValueMap[fValue->first].insert(codeValueMap[fValue->first].begin(), false);
        fValue++;

        diffSizeLists--;
    }
    father->data = -1;

    return father;
}

std::vector<bool> huffmanEncode(std::vector<std::pair<int, int>> runLengthCode, Node* &huffmanTreeRoot){
    std::unordered_map<int, double> freqsMapNZero; // word, freq
    std::unordered_map<int, double> freqsMapValue; // word, freq

    std::unordered_map<int, std::vector<bool>> codeZerosMap; // Nzeros, codeword
    std::unordered_map<int, std::vector<bool>> codeValueMap; // Value, codeword

    for(std::pair<int, int> ac: runLengthCode){
        freqsMapNZero[ac.first]++;
        freqsMapValue[ac.second]++;
    }

    std::list<std::pair<int, double>> freqs_listNZero;
    std::list<std::pair<int, double>> freqs_listValue;

    std::unordered_map<int, double>::iterator fNZero = freqsMapNZero.begin();
    std::unordered_map<int, double>::iterator fValue = freqsMapValue.begin();
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

    //std::cout << "freqs_listNZero sorted list: " << std::endl;
    //for(const auto &cw : freqs_listNZero){
    //    std::cout << cw.first << ", freq: " << cw.second << std::endl;
    //}
    //std::cout<<std::endl;

    //std::cout << "freqs_listValue sorted list: " << std::endl;
    //for(const auto &cw : freqs_listValue){
    //    std::cout << cw.first << ", freq: " << cw.second << std::endl;
    //}
    //std::cout<<std::endl;


    huffmanTreeRoot = huffmanTree(freqs_listNZero, freqs_listValue, codeZerosMap, codeValueMap);

    //std::cout<<std::endl;

    std::vector<bool> code;
    for(std::pair<int, int> ac: runLengthCode){
        //std::cout << ac.first << ", codeword: ";
        //for(const auto &bit : codeZerosMap[ac.first]){
        //    std::cout << bit;
        //}
        //std::cout << std::endl;

        //std::cout << ac.second << ", codeword: ";
        //for(const auto &bit : codeValueMap[ac.second]){
        //    std::cout << bit;
        //}
        //std::cout << std::endl;
        code.insert( code.end(), codeZerosMap[ac.first].begin(), codeZerosMap[ac.first].end());
        code.insert( code.end(), codeValueMap[ac.second].begin(), codeValueMap[ac.second].end());
    }

    //for(const auto &bit : code){
    //    std::cout<<bit;
    //}
    //std::cout<<std::endl;

    return code;
}

std::vector<std::pair<int, int>> huffmanDecode(std::vector<bool> code, Node* huffmanTreeRoot){
    std::vector<std::pair<int, int>> runLengthCode; //nZeros, elem
    Node* node = huffmanTreeRoot;
    int nZeros = -2;
    for(bool bit : code){
        //std::cout<< "bit: " << bit << std::endl;
        if(bit){
            node = node->left;
        } else {
            node = node->right;
        }

        // if we are decoding the number of zeros
        if(nZeros == -2){
            if(node->left->left == nullptr && node->left->right == nullptr) { // number of zeros are in the left leafs
                nZeros = node->left->data;
                node = huffmanTreeRoot;
            }
        } else { // if we are decoding the value
            if(node->right->left == nullptr && node->right->right == nullptr) { // values are in the right leafs
                //std::cout << "runLengthCode node->data: (" << nZeros << ", " << node->right->data << ")" << std::endl;
                runLengthCode.push_back(std::pair<int, int>(nZeros, node->right->data));
                nZeros = -2;
                node = huffmanTreeRoot;
            }
        }
    }

    return runLengthCode;
}

void quantizeDctBaselineJPEG(cv::Mat frame,  /*cv::Mat prev_frame,*/ bool isColor) {

    cv::Mat quantizeDct = cv::Mat::zeros(frame.rows, frame.cols, CV_64F);

    divideImageIn8x8Blocks(frame);

    // ***************************************** Calculation of the DCT ************************************************
    cv::Mat block;
    double current_dc[frame.rows][frame.cols];
    for(int r = 0; r < frame.rows; r += 8) {
        for(int c = 0; c < frame.cols; c += 8) {
            frame(cv::Rect(r,c,8,8)).copyTo(block);

            // TODO see what jpeg matrix to use
            wholeDCTQuant(block, quantMatrixGrayscale);

            // Next, the coefficients are organized in a one-dimensional vector according to a zig-zag scan.
            std::vector<int> zigzag_array = zigZagScan(block);

            // ********************* Statistical coding (Huffman) of the quantized DCT coefficients ********************

            // The non-zero AC coefficients are encoded using Huffman or arithmetic coding, representing the value of
            // the coefficient, as well as the number of zeros preceding it.
            // In this case it is used golomb encoding and the symbol to represent the end of the block is -1 because
            // the number of zeros can't be negative. (TODO)
            std::vector<std::pair<int, int>> acs = runLengthCode(zigzag_array);

            // The DC coefficient of each block is predicatively encoded in relation to the DC coefficient of the
            // previous block.
            // double dc = zig_zag_dc - previous_dc[r][c];
            // current_dc[r][c] = zigzag_array.at(0);
            // TODO get previous_dc

        }
    }

}

void inverseQuantizeDctBaselineJPEG(cv::Mat frame, cv::Mat prev_frame, bool isColor){

    cv::Mat return_frame  = cv::Mat::zeros(frame.rows, frame.cols, CV_64F);

    double current_dc[frame.rows][frame.cols];

    // *********************** Statistical decoding (Golomb) of the quantized DCT coefficients *************************
    // TODO

    cv::Mat block = cv::Mat::zeros(8, 8, CV_64F);
    for(int r = 0; r < frame.rows; r += 8) {
        for (int c = 0; c < frame.cols; c += 8) {

            // Calculate the inverse DCT 2D of each block.
            double twoIDDCTBlock[8][8];

            // base quantization matrix of JPEG (luminance)
            double jpeg_matrix[8][8];
            memcpy(jpeg_matrix,
                   (isColor) ? jpeg_matrix_color : jpeg_matrix_grayscale,
                   sizeof((isColor) ? jpeg_matrix_color : jpeg_matrix_grayscale));

            for(int i = 0; i < 8; i++) {
                for(int j = 0; j < 8; j++) {
                    // ******************************* Dequantization of the DCT coefficients **************************
                    twoIDDCTBlock[i][j] = block.at<double>(i, j) * jpeg_matrix[i][j];

                    // ************************************** Inverse DCT **********************************************
                    double vector[1] = {twoIDDCTBlock[i][j]};
                    //twoIDDCTBlock[i][j] = *NaiveDct_inverseTransform(vector, 1);

                    // Add 2^(b−1) to each pixel value, where b is the number of bits used to represent the pixels.
                    return_frame.at<double>(r + i, c + j) = twoIDDCTBlock[i][j] + pow(2, 7);
                }
            }

        }
    }
}

/*! This mode relies on encoding the DCT coefficients using several passes, such that in each pass only part of the
 * information associated to those coefficients is transmitted.
 *  The coefficients are organized in spectral bands, and those corresponding to the lower frequencies are transmitted
 * first.
 */
void quantizeDctProgressiveSpectralJPEG(cv::Mat frame, bool isColor){

}


/*! This mode relies on encoding the DCT coefficients using several passes, such that in each pass only part of the
 * information associated to those coefficients is transmitted.
 *  All coefficients are first transmitted using a limited precision. Afterwards, additional detail is sent using more
 * passes through the coefficients.
 */
void inverseQuantizeDctProgressiveApproxJPEG(cv::Mat frame, bool isColor){

}