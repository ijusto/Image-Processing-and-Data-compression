#include    "../includes/VideoDecoder.hpp"

using cv::Mat;
using cv::Scalar;
using cv::BORDER_CONSTANT;
using cv::noArray;
using cv::format;
using namespace std;

int VideoDecoder::boolvec2int(vector<bool> vec){
    int acc = 0;
    for(int i = vec.size() - 1; i >= 0; i--){
        acc = (acc << 1) | vec.at(i);
    }
    return acc;
}

VideoDecoder::VideoDecoder(char* encodedFileName){
    sourceFile = new BitStream(encodedFileName, 'r');

    // 40 byte file header (initial_m, predictor, subsampling, mode, lossy, fps1, fps2, totalFrames, frame rows, frame cols)
    headerSize = 40;        // bytes
    int paramsSize = 32;    // bits 4 bytes

    try {
        initial_m = boolvec2int(sourceFile->readNbits(paramsSize));
        predictor =  boolvec2int(sourceFile->readNbits(paramsSize));
        subsampling = boolvec2int(sourceFile->readNbits(paramsSize));
        mode = boolvec2int(sourceFile->readNbits(paramsSize));
        lossy = boolvec2int(sourceFile->readNbits(paramsSize));
        fps1 = boolvec2int(sourceFile->readNbits(paramsSize));
        fps2 = boolvec2int(sourceFile->readNbits(paramsSize));
        totalFrames = boolvec2int(sourceFile->readNbits(paramsSize));
        rows =  boolvec2int(sourceFile->readNbits(paramsSize));
        cols =  boolvec2int(sourceFile->readNbits(paramsSize));

        if(lossy){
            this->prevDCs = {{}, {}, {}}; // y, u, v
            this->quantization = new JPEGQuantization();
        }

    } catch( string mess){
        std::cout << mess << std::endl;
        std::exit(0);
    }
}

void VideoDecoder::decode(){
    // read all data
    vector<bool> data;
    sourceFile->readToEnd(data);
    // used to index data in golomb.decode2
    unsigned int index = 0;

    // Chroma subsampling dimensions
    int Y_frame_cols, Y_frame_rows;
    int U_frame_cols, U_frame_rows;
    int V_frame_cols, V_frame_rows;

    switch(this->subsampling){
        case 444:
            Y_frame_rows = U_frame_rows = V_frame_rows = rows;
            Y_frame_cols = U_frame_cols = V_frame_cols = cols;
            break;
        case 422:
            Y_frame_rows = rows;
            Y_frame_cols = cols;
            U_frame_rows = V_frame_rows = rows;
            U_frame_cols = V_frame_cols = cols / 2;
            break;
        case 420:
            Y_frame_rows = rows;
            Y_frame_cols = cols;
            U_frame_rows = V_frame_rows = rows / 2;
            U_frame_cols = V_frame_cols = cols / 2;
            break;
    }

    // Golomb decoder
    Golomb *golomb = new Golomb(initial_m);
    int m_rate = 100; // must be equal to Encoder's m_rate

    // intra coding rate
    const int intra_rate = 10;
    int frameCounter = 0;
    // previous data buffer
    Mat y_prev = Mat(Y_frame_rows, Y_frame_cols, CV_8UC1); // init with 0s
    Mat u_prev = Mat(U_frame_rows, U_frame_cols, CV_8UC1);
    Mat v_prev = Mat(V_frame_rows, V_frame_cols, CV_8UC1);
    // block size (in pixels)
    int block_size = 10;
    // search area size (in number of blocks)
    int search_size = 4;

    while(frameCounter < this->totalFrames) {
        cout << "\rdecoded frames: " << frameCounter << "/" << this->totalFrames;
        cout.flush();

        // decoded frame
        vector<uchar> frame;

        // iterate channels
        for(int channel = 0; channel < 3; channel++){
            Mat prev;
            int frame_rows, frame_cols;
            if(channel == 0 ){
                // Y
                frame_rows = Y_frame_rows;
                frame_cols = Y_frame_cols;
                prev = y_prev;
            }else if(channel == 1 ){
                // U
                frame_rows = U_frame_rows;
                frame_cols = U_frame_cols;
                prev = u_prev;
            }else if(channel == 2 ){
                // V
                frame_rows = V_frame_rows;
                frame_cols = V_frame_cols;
                prev = v_prev;
            }

            vector<uchar> component_frame;
            if(this->mode == 0 || frameCounter % intra_rate == 0){
                // intra

                // only residuals
                int size = frame_rows * frame_cols;
                // decode residuals and update m
                vector<int> residuals;
                if(this->lossy) {
                    getResAndUpdate(data, &index, golomb, m_rate, residuals, frame_rows, frame_cols, channel);
                } else {
                    getResAndUpdate(data, &index, size, golomb, m_rate, residuals);
                }
                // decode y component and add to frame vector
                this->decodeRes_intra(residuals, component_frame, frame_rows, frame_cols);
            }else{
                // hybrid

                // split frame into square blocks
                int grid_h = frame_rows / block_size;
                // add block partially outside frame
                if(frame_rows % block_size != 0){
                    grid_h += 1;
                }
                int grid_w = frame_cols / block_size;
                // add block partially outside frame
                if(frame_cols % block_size != 0){
                    grid_w += 1;
                }
                // residuals + motion vectors (2 ints for every block in the grid)
                int size = frame_rows * frame_cols + 2*grid_h*grid_w;
                // decode residuals and update m
                vector<int> residuals;
                getResAndUpdate(data, &index, size, golomb, m_rate, residuals);
                // decode y component residuals + motion vectors
                this->decodeRes_inter(prev, residuals, grid_h, grid_w, component_frame, block_size, search_size);
            }
            if(this->mode == 1){
                // hybrid
                // update prev TODO: ?
                prev.data = component_frame.data();
            }
            // add component to frame
            frame.insert(frame.end(), component_frame.begin(), component_frame.end());
        }

        // add frame to buffer
        this->frames.push_back(frame);

        frameCounter++;
    }

    cout << endl;

}

// TODO: make it fast
void VideoDecoder::update_m(vector<int> residuals, Golomb *golomb, int m_rate){
    // used to compute mean of mapped residuals
    float res_sum = 0;
    int numRes = 0;

    for(int residual:residuals){
        // compute m
        int Mapped = 2 * residual;
        if(residual < 0){
            Mapped = -Mapped-1;
        }
        res_sum += Mapped;
        numRes++;
    }

    if(numRes == m_rate){
        // calc mean from last m_rate mapped pixels
        float res_mean = res_sum/numRes;
        // calc alpha of geometric dist
        // mu = alpha/(1 - alpha) <=> alpha = mu/(1 + mu)
        float alpha = res_mean/(1+res_mean);
        int m = ceil(-1/log(alpha));
        if (m != 0){
            // cout << "NEW M " << m << endl;
            golomb->setM(m);
        }
        //reset
        res_sum = 0;
        numRes = 0;
    }
}

void VideoDecoder::getResAndUpdate(vector<bool> &data, unsigned int *indexPtr, Golomb *golomb, int m_rate, vector<int> &outRes, int f_rows, int f_cols, int channel){
    std::vector<int> decodedLeafs;
    golomb->decode2(data, decodedLeafs, indexPtr, 1);
    // Ler com o golomb numero a numero ate um -3 (inclusivo) -> folhas da huffman tree
    while(decodedLeafs.back() != -3) {
        golomb->decode2(data, decodedLeafs, indexPtr, 1);
    }
    decodedLeafs.pop_back(); // -3 golomb encoded to represent the end of the huffman tree

    auto* huffDec = new HuffmanDecoder(this->quantization->huffmanTree(decodedLeafs));
    std::vector<std::pair<int, int>> rlCode;  // run length code

    bool bit = data.at(*indexPtr);
    // update index
    *indexPtr = *indexPtr + 1;
    while(huffDec->decode(bit, rlCode)){
        bit = data.at(*indexPtr);
        // update index
        *indexPtr = *indexPtr + 1;
    }

    this->quantization->inverseQuantizeDctBaselineJPEG(f_rows, f_cols, this->prevDCs.at(channel), rlCode, outRes,
                                                       channel == 0);
}

void VideoDecoder::getResAndUpdate(vector<bool> &data, unsigned int *indexPtr, int n_residuals, Golomb *golomb, int m_rate, vector<int> &outRes){
    int n_to_decode = m_rate;
    int n_decoded = 0;
    while(n_decoded < n_residuals){
        // read remaining
        if(n_decoded + n_to_decode > n_residuals){
            n_to_decode = n_residuals - n_decoded;
        }
        n_decoded += n_to_decode;
        // read y
        vector<int> tmp;
        golomb->decode2(data, tmp, indexPtr, n_to_decode);
        // update m
        this->update_m(tmp, golomb, m_rate);
        // add to total outRes
        outRes.insert(outRes.end(), tmp.begin(), tmp.end());
    }
}

void VideoDecoder::decodeRes_intra(vector<int> &residualValues, vector<uchar> &outPlanarValues, int f_rows, int f_cols){
    for(int i = 0; i < f_rows; i++){
        for(int j = 0; j < f_cols; j++){
            int idx = i * f_cols + j;

            LosslessJPEGPredictors<int> predictors(
                    (j == 0 ? 0 : outPlanarValues[i * f_cols + (j - 1)]),                    // (i,j-1)
                    (i == 0 ? 0 : outPlanarValues[(i - 1) * f_cols + j]),                    // (i-1,j)
                    ((i == 0 | j == 0) ? 0 : outPlanarValues[(i - 1) * f_cols + (j - 1)]));    // (i-1,j-1)

            uchar value;
            switch (this->predictor) {
                case 1:
                    value = residualValues.at(idx) + predictors.usePredictor1();
                    break;
                case 2:
                    value = residualValues.at(idx) + predictors.usePredictor2();
                    break;
                case 3:
                    value = residualValues.at(idx) +predictors.usePredictor3();
                    break;
                case 4:
                    value = residualValues.at(idx) +predictors.usePredictor4();
                    break;
                case 5:
                    value = residualValues.at(idx) +predictors.usePredictor5();
                    break;
                case 6:
                    value = residualValues.at(idx) +predictors.usePredictor6();
                    break;
                case 7:
                    value = residualValues.at(idx) +predictors.usePredictor7();
                    break;
                case 8:
                    value = residualValues.at(idx) +predictors.usePredictorJLS();
                    break;
                default:
                    std::cout << "ERROR: Predictor chosen isn't correct!!!" << std::endl;
                    exit(EXIT_FAILURE);
            }

            // add value to frame
            outPlanarValues.push_back(value);
        }
    }
}

void VideoDecoder::decodeRes_inter(Mat &prev_frame, vector<int> &currFrameResiduals, int grid_h, int grid_w, vector<uchar> &outPlanarValues, int block_size, int search_size){
    // pad prev_frame with with 0s
    Mat padded_prev_frame;
    int sd = search_size * block_size; // search distance (in pixels)
    int bd = block_size - (prev_frame.rows % block_size);
    int rd = block_size - (prev_frame.cols % block_size);
    copyMakeBorder(prev_frame, padded_prev_frame, sd, sd + bd, sd, sd + rd, BORDER_CONSTANT, Scalar(0));

    int idx = 0;
    // traverse all blocks
    for(int x = 0; x < grid_w; x++){
        for(int y = 0; y < grid_h; y++){
            // read 2 ints (motion vector)
            int motion_x = currFrameResiduals.at(idx);
            int motion_y = currFrameResiduals.at(idx+1);
            // update index
            idx+=2;
            // get rect from prev using motion vect
            Mat prev_block = padded_prev_frame(cv::Rect(motion_x, motion_y, block_size, block_size));
            // compute value using residuals
            for(int j = 0; j < block_size; j++){
                for(int i = 0; i < block_size; i++){
                    // add residuals to prev rect
                    int value = prev_block.at<uchar>(i, j) + currFrameResiduals.at(idx+(i*block_size+j));
                    if(x*block_size+j < prev_frame.cols || y*block_size+i < prev_frame.rows){
                        // add result to outPlanarValues
                        outPlanarValues.push_back(value);
                    }
                }
            }
            // update index
            idx+=block_size*block_size;
        }
    }
}

void VideoDecoder::write(char* fileName){
    // open output video file
    ofstream outvideo;
    outvideo.open (fileName);

    // write HEADER
    string header = format("YUV4MPEG2 W%d H%d F%d:%d C%d\n", this->cols, this->rows, this->fps1, this->fps2, this->subsampling);
    outvideo << header;

    //write frames
    for(vector<uchar> fr: this->frames){
        // write FRAME header
        outvideo << "FRAME\n";
        // write data
        outvideo.write((char *) fr.data(), fr.size());
    }
}