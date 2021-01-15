//
// Created by inesjusto on 10/12/20.
//

#include    "../../audioAndImageOrVideoManipulation/src/EntropyCalculator.cpp"
#include    "../../entropyCoding/src/Golomb.cpp"
#include    "VideoEncoder.hpp"
#include    "LosslessJPEGPredictors.cpp"
#include    <fstream>
#include    <regex>
#include    <algorithm>

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

    // get fps
    regex rgx_f1("F([0-9]+):[0-9]+");
    regex_search(header, match, rgx_f1);
    this->fps1 = stoi(match[1]);
    regex rgx_f2("F[0-9]+:([0-9]+)");
    regex_search(header, match, rgx_f2);
    this->fps2 = stoi(match[1]);

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
        u_prev = frameData;

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
        v_prev = frameData;

        frameCounter++;
    }

    this->totalFrames = frameCounter;
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

            // // quantize residual here before encoding

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
    // split current frame into square blocks
    // grid of blocks dimensions
    int grid_h = curr_frame.rows / block_size;
    // add block partially outside frame
    if(curr_frame.rows % block_size != 0){
        grid_h += 1;
    }
    int grid_w = curr_frame.cols / block_size;
    // add block partially outside frame
    if(curr_frame.cols % block_size != 0){
        grid_w += 1;
    }

    // used to compute mean of mapped residuals
    float res_sum = 0;
    int numRes = 0;

    // traverse all blocks
    for(int x = 0; x < grid_w; x++){
        for(int y = 0; y < grid_h; y++){
            // perform 2d log search on previous frame search area
            // init center block
            Point2i center_block_pt(x * block_size, y * block_size);
            Point2i best_pt(center_block_pt.x, center_block_pt.y);
            int stepSize = search_size/4 + 1;
            vector<Point2i> old_search_blocks;
            while(stepSize != 1){
                // define initial search blocks
                vector<Point2i> search_blocks;
                search_blocks.push_back(Point2i(center_block_pt.x, center_block_pt.y));
                search_blocks.push_back(Point2i(center_block_pt.x - stepSize, center_block_pt.y));
                search_blocks.push_back(Point2i(center_block_pt.x + stepSize, center_block_pt.y));
                search_blocks.push_back(Point2i(center_block_pt.x, center_block_pt.y - stepSize));
                search_blocks.push_back(Point2i(center_block_pt.x, center_block_pt.y + stepSize));

                int min_mse = -1;
                for(Point2i pt : search_blocks){
                    if(find(old_search_blocks.begin(), old_search_blocks.end(), pt) == old_search_blocks.end()) {
                        // pt not in old_search_blocks

                        // calc mse
                        int mse = 0;
                        for(int k = 0; k < block_size; k++){
                            for(int l = 0; l < block_size; l++){
                                int prev_value = 0;
                                // within frame
                                if( (pt.x + k >= 0) && (pt.x + k < prev_frame.cols) &&
                                    (pt.y + l >= 0) && (pt.y + l < prev_frame.rows) ){
                                    prev_value = prev_frame.at<uchar>(pt.x + k, pt.y + l);
                                }
                                int curr_value = curr_frame.at<uchar>(x + k, y + l);
                                int error = prev_value - curr_value;
                                mse += pow(error, 2);
                            }
                        }

                        // update best motion vector
                        if (mse < min_mse || min_mse == -1){
                            min_mse = mse;
                            best_pt = pt;
                        }

                        // add to already seen blocks
                        old_search_blocks.push_back(pt);
                    }
                }

//                cout << stepSize << endl;

                // update step size;
                stepSize = stepSize/2;

//                if(best_pt != center_block_pt){
//                    // update center
//                    center_block_pt = best_pt;
//                }else{
//                    // update step size;
//                    stepSize = stepSize/2;
//                }
            }

            // add motion vector to bit stream
            vector<bool> m_x = golomb->encode2(best_pt.x);
            // add encoded residuals to bit stream
            encodedRes.insert(encodedRes.end(), m_x.begin(), m_x.end());
            vector<bool> m_y = golomb->encode2(best_pt.y);
            // add encoded residuals to bit stream
            encodedRes.insert(encodedRes.end(), m_y.begin(), m_y.end());

            // compute residuals
            for(int k = 0; k < block_size; k++){
                for(int l = 0; l < block_size; l++){
                    int prev_value = 0;
                    // within frame
                    if( (best_pt.x + k >= 0) && (best_pt.x + k < prev_frame.cols) &&
                        (best_pt.y + l >= 0) && (best_pt.y + l < prev_frame.rows) ){
                        prev_value = prev_frame.at<uchar>(best_pt.x + k, best_pt.y + l);
                    }
                    int curr_value = curr_frame.at<uchar>(x + k, y + l);
                    int residual = prev_value - curr_value;
                    // golomb encode residuals
                    vector<bool> encodedResidual = golomb->encode2(residual);
                    // add encoded residuals to bit stream
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
    }
}

void VideoEncoder::write(char *filename) {
    auto * wbs = new BitStream(filename, 'w');

    vector<bool> file;

    // add 36 byte file header (initial_m, predictor, subsampling, mode, fps1, fps2, totalFrames, frame rows, frame cols)

    // initial_m
    vector<bool> m = int2boolvec(this->initial_m);
    file.insert(file.cend(), m.begin(), m.end());
    //predictor
    vector<bool> pred = int2boolvec(this->predictor);
    file.insert(file.cend(), pred.begin(), pred.end());
    // subsampling
    vector<bool> subsamp = int2boolvec(this->subsampling);
    file.insert(file.end(), subsamp.begin(), subsamp.end());
    // mode
    vector<bool> vecmode = int2boolvec(this->mode);
    file.insert(file.end(), vecmode.begin(), vecmode.end());
    // fps part 1
    vector<bool> f1 = int2boolvec(this->fps1);
    file.insert(file.end(), f1.begin(), f1.end());
    // fps part 2
    vector<bool> f2 = int2boolvec(this->fps2);
    file.insert(file.end(), f2.begin(), f2.end());
    // totalFrames
    vector<bool> tFrames = int2boolvec(this->totalFrames);
    file.insert(file.end(), tFrames.begin(), tFrames.end());
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