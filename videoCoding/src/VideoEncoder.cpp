//
// Created by inesjusto on 10/12/20.
//

#include    "../../audioAndImageOrVideoManipulation/src/EntropyCalculator.cpp"
#include    "../../entropyCoding/src/Golomb.cpp"
#include    "VideoEncoder.hpp"
#include    "LosslessJPEGPredictors.cpp"
#include    <fstream>
#include    <regex>

using namespace cv;
using namespace std;

vector<bool> VideoEncoder::int2boolvec(int n){
    vector<bool> bool_vec_res;

    unsigned int mask = 0x00000001;
    for(int i = 0; i < sizeof(n)*8; i++){
        bool bit = ((n & (mask << i)) >> i) == 1;
        bool_vec_res.push_back(bit);
    }

    return bool_vec_res;
}

VideoEncoder::VideoEncoder(char* srcFileName, int pred, int mode, int init_m, bool calcHist) {
    this->predictor = pred;
    this->mode = mode;
    this->initial_m = init_m;
    this->cHist = calcHist;

    // init histograms
    if(this->cHist){
        this->res_hists = new vector<vector<char>>;
        this->sample_hists = new vector<vector<char>>;
        for(int k = 0; k < 3; k++){
            this->res_hists->push_back(vector<char>());
            this->sample_hists->push_back(vector<char>());
        }
    }

    // open video file
    ifstream video;
    video.open(srcFileName);

    if (!video.is_open()){
        cout << "Error opening file: " << srcFileName << endl;
        exit(EXIT_FAILURE);
    }

    // parse header
    string header;
    getline(video, header);
    cout << "header: " << header << endl;
    smatch match;

    // get rows, cols
    regex rgx_w("W([0-9]+)");
    regex_search(header, match, rgx_w);
    this->cols = stoi(match[1]);

    regex rgx_h("H([0-9]+)");
    regex_search(header, match, rgx_h);
    this->rows = stoi(match[1]);

    // get subsampling mode
    regex rgx_samp("C([0-9]+)");
    regex_search(header, match, rgx_samp);

    if (match.size() == 0){ // 420 by default
        this->subsampling = 420;
    }else{
        this->subsampling = stoi(match[1]);
    }

    // Chroma subsampling dimensions
    int Y_frame_cols, Y_frame_rows;
    int U_frame_cols, U_frame_rows;
    int V_frame_cols, V_frame_rows;

    switch(subsampling){
        case 444:
            Y_frame_rows = U_frame_rows = V_frame_rows = rows;
            Y_frame_cols = U_frame_cols = V_frame_cols = cols;
            break;
        case 422:
            Y_frame_rows = rows;
            Y_frame_cols = cols;
            U_frame_rows = V_frame_rows = rows / 2;
            U_frame_cols = V_frame_cols = cols / 2;
            break;
        case 420:
            Y_frame_rows = rows;
            Y_frame_cols = cols;
            U_frame_rows = V_frame_rows = rows / 4;
            U_frame_cols = V_frame_cols = cols / 4;
            break;
    }

    // data buffer
    Mat frameData;
    // intra coding rate
    const int intra_rate = 10;
    int frameCounter = 0;
    // previous data buffer
    Mat y_prev = Mat(Y_frame_rows, Y_frame_cols, CV_8UC1); // init with 0s
    Mat u_prev = Mat(U_frame_rows, U_frame_cols, CV_8UC1);
    Mat v_prev = Mat(V_frame_rows, V_frame_cols, CV_8UC1);
    // block size
    int block_size = 10;
    // search area size
    int search_size = 4*block_size;

    // Golomb encoder
    auto *golomb = new Golomb(this->initial_m);
    // calc m every m_rate frames
    int m_rate = 100;

    while(true){
        // skip word FRAME
        getline(video, header);

        // read data, compute and encode residuals

        // read y
        frameData = Mat(Y_frame_rows, Y_frame_cols, CV_8UC1);
        video.read((char *) frameData.ptr(), Y_frame_rows * Y_frame_cols);
        if (video.gcount() == 0)
            break;

        // compute residuals for y
        if(this->mode == 0 || frameCounter % intra_rate == 0){
            // intra coding
            // encode residuals
            this->encodeRes_intra(frameData, golomb, m_rate, 0);
        }else if(this->mode == 1){
            // intra + inter coding (hybrid)
            // encode motion vectors and residuals
            encodeRes_inter(y_prev, frameData, golomb, m_rate, block_size, search_size,0);
        }
        // update previous
        y_prev = frameData;

        // read u
        frameData = Mat(U_frame_rows, U_frame_cols, CV_8UC1);
        video.read((char *) frameData.ptr(), U_frame_rows * U_frame_cols);
        // encode residuals
        this->encodeRes_intra(frameData, golomb, m_rate, 1);

        // compute residuals for u
        if(this->mode == 0 || frameCounter % intra_rate == 0){
            // intra coding
            // encode residuals
            this->encodeRes_intra(frameData, golomb, m_rate, 0);
        }else if(this->mode == 1){
            // intra + inter coding (hybrid)
            // encode motion vectors and residuals
            encodeRes_inter(u_prev, frameData, golomb, m_rate, block_size, search_size,0);
        }
        // update previous
        y_prev = frameData;

        // read v
        frameData = Mat(V_frame_rows, V_frame_cols, CV_8UC1);
        video.read((char *) frameData.ptr(), V_frame_rows * V_frame_cols);
        // encode residuals
        this->encodeRes_intra(frameData, golomb, m_rate, 2);

        // compute residuals for v
        if(this->mode == 0 || frameCounter % intra_rate == 0){
            // intra coding
            // encode residuals
            this->encodeRes_intra(frameData, golomb, m_rate, 0);
        }else if(this->mode == 1){
            // intra + inter coding (hybrid)
            // encode motion vectors and residuals
            encodeRes_inter(v_prev, frameData, golomb, m_rate, block_size, search_size,0);
        }
        // update previous
        y_prev = frameData;

        frameCounter++;
    }
}

void VideoEncoder::encodeRes_intra(Mat &frame, Golomb *golomb, int m_rate, int k){
    // residuals
    char residual;
    // used to compute mean of mapped residuals
    float res_sum = 0;
    int numRes = 0;

    for(int i = 0; i < frame.rows; i++){
        for(int j = 0; j < frame.cols; j++){

            LosslessJPEGPredictors<int> predictors(
                    (j == 0 ? 0 : frame.at<uchar>(i,j-1)),
                    (i == 0 ? 0 : frame.at<uchar>(i-1,j)),
                    ((i == 0 | j == 0) ? 0 : frame.at<uchar>(i - 1, j - 1)));

            //calculation of residuals for each predictor
            switch (this->predictor) {
                case 1:
                    residual = frame.at<uchar>(i,j) - predictors.usePredictor1();
                    break;
                case 2:
                    residual = frame.at<uchar>(i,j) - predictors.usePredictor2();
                    break;
                case 3:
                    residual = frame.at<uchar>(i,j) - predictors.usePredictor3();
                    break;
                case 4:
                    residual = frame.at<uchar>(i,j) - predictors.usePredictor4();
                    break;
                case 5:
                    residual = frame.at<uchar>(i,j) - predictors.usePredictor5();
                    break;
                case 6:
                    residual = frame.at<uchar>(i,j) - predictors.usePredictor6();
                    break;
                case 7:
                    residual = frame.at<uchar>(i,j) - predictors.usePredictor7();
                    break;
                case 8:
                    residual = frame.at<uchar>(i,j) - predictors.usePredictorJLS();
                    break;
                default:
                    std::cout << "ERROR: Predictor chosen isn't correct!!!" << std::endl;
                    exit(EXIT_FAILURE);
            }

            if(this->cHist){
                // store residuals
                this->res_hists->at(k).push_back(residual);
                // store samples
                this->sample_hists->at(k).push_back(frame.at<uchar>(i,j));
            }

            // encode residuals
            vector<bool> encodedResidual = golomb->encode2(residual);
            encodedRes.insert(encodedRes.end(), encodedResidual.begin(), encodedResidual.end());

            // compute m
            int Mapped = 2 * residual;
            if(residual < 0){
                Mapped = -Mapped-1;
            }
            res_sum += Mapped;
            numRes++;
            if(numRes == m_rate){
                // calc mean from last 100 mapped pixels
                float res_mean = res_sum/numRes;
                // calc alpha of geometric dist
                // mu = alpha/(1 - alpha) <=> alpha = mu/(1 + mu)
                float alpha = res_mean/(1+res_mean);
                int m = ceil(-1/log(alpha));
                if (m != 0){
                    golomb->setM(m);
                }
                //reset
                res_sum = 0;
                numRes = 0;
            }
        }
    }
}

void VideoEncoder::encodeRes_inter(cv::Mat &prev_frame, cv::Mat &curr_frame, Golomb *golomb, int m_rate, int block_size, int search_size, int k){

    // split current frame into blocks
    // grid of blocks dimensions
    int grid_h = curr_frame.rows / block_size;
    int grid_w = curr_frame.cols / block_size;

    // traverse all block
    for(int i = 0; i < grid_h; i++){
        for(int j = 0; j < grid_w; j++){
            // top left corner of square block
            int x = j*block_size;
            int y = i*block_size;

            // perform 2d log search on previous frame search area
            int best_x, best_y; // motion vector
            int min_mse;

            // compute residuals

            // golomb encode residuals

            // add motion vector to bit stream

            // add encoded residuals to bit stream
        }
    }
}

void VideoEncoder::write(char *filename) {

    auto * wbs = new BitStream(filename, 'w');

    vector<bool> file;

    // add file header (initial_m, predictor, format, mode, channels, frame rows, frame cols)
    // initial_m
    vector<bool> m = int2boolvec(this->initial_m);
    file.insert(file.cend(), m.begin(), m.end());

    //predictor
    vector<bool> pred = int2boolvec(this->predictor);
    file.insert(file.cend(), pred.begin(), pred.end());

    // format
    vector<bool> format = int2boolvec(this->subsampling);
    file.insert(file.end(), format.begin(), format.end());

    // mode
    vector<bool> vecmode = int2boolvec(this->mode);
    file.insert(file.end(), format.begin(), format.end());

    // channels
    vector<bool> channels = int2boolvec(3);
    file.insert(file.end(), channels.begin(), channels.end());

    // rows
    vector<bool> rows = int2boolvec(this->rows);
    file.insert(file.end(), rows.begin(), rows.end());

    //cols
    vector<bool> cols = int2boolvec(this->cols);
    file.insert(file.end(), cols.begin(), cols.end());

    //data
    file.insert(file.end(), this->encodedRes.begin(), this->encodedRes.end());

    wbs->writeNbits(file);
    wbs->endWriteFile();
}

vector<vector<char>> VideoEncoder::get_res_hists(){
    return *(this->res_hists);
}

vector<vector<char>> VideoEncoder::get_sample_hists(){
    return *(this->sample_hists);
}