#include    "JPEGQuantization.cpp"
#include    "VideoEncoder.hpp"
#include    "LosslessJPEGPredictors.cpp"
#include    <fstream>
#include    <regex>
#include    <algorithm>

using cv::Mat;
using cv::Scalar;
using cv::BORDER_CONSTANT;
using cv::noArray;
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

void VideoEncoder::parseHeader(string &header){
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
}

VideoEncoder::VideoEncoder(char* srcFileName, int pred, int init_m, int mode, bool lossy, bool calcHist) {
    this->predictor = pred;
    this->initial_m = init_m;
    this->mode = mode;
    this->lossy = lossy;
    if(this->lossy){
        this->prevDCs = {{}, {}, {}}; // y, u, v
        this->quantization = new JPEGQuantization();
    }
    this->calcHist = calcHist;

    // init histograms
    if(this->calcHist){
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
    this->parseHeader(header);
    // cout << header << endl;

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

    // data buffer
    Mat frameData;

    // Golomb encoder
    auto *golomb = new Golomb(this->initial_m);
    // calc m every m_rate frames
    int m_rate = 100;

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

    while(true){
        cout << "\rencoded frames: " << frameCounter;
        cout.flush();

        // skip word FRAME
        getline(video, header);

        // read data, compute and encode residuals

        // read y
        frameData = Mat(Y_frame_rows, Y_frame_cols, CV_8UC1);
        video.read((char *) frameData.ptr(), Y_frame_rows * Y_frame_cols);

        // check if stream is empty
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
            encodeRes_inter(y_prev, frameData, golomb, m_rate, block_size, search_size);
        }

        if(this->lossy){
            // this->last_res contains last residuals (Y)
            // encode residuals using huffman/golomb
            // append encoded residuals to this->encodedRes
            quantization->quantize(this->last_res, this->prevDCs.at(0), golomb, this->encodedRes, true);
        }

        if(this->mode == 1){
            // update previous
            y_prev = frameData;
        }

        // read u
        frameData = Mat(U_frame_rows, U_frame_cols, CV_8UC1);
        video.read((char *) frameData.ptr(), U_frame_rows * U_frame_cols);

        // compute residuals for u
        if(this->mode == 0 || frameCounter % intra_rate == 0){
            // intra coding
            // encode residuals
            this->encodeRes_intra(frameData, golomb, m_rate, 1);
            //cout << "sample size u" << frameData.size() << endl;
        }else if(this->mode == 1){
            // intra + inter coding (hybrid)
            // encode motion vectors and residuals
            encodeRes_inter(u_prev, frameData, golomb, m_rate, block_size, search_size);
        }

        if(this->lossy){
            // this->last_res contains last residuals (U)
            // encode residuals using huffman/golomb
            // append encoded residuals to this->encodedRes
            quantization->quantize(this->last_res, this->prevDCs.at(1), golomb, this->encodedRes, false);
        }

        if(this->mode == 1){
            // update previous
            u_prev = frameData;
        }

        // read v
        frameData = Mat(V_frame_rows, V_frame_cols, CV_8UC1);
        video.read((char *) frameData.ptr(), V_frame_rows * V_frame_cols);

        // compute residuals for v
        if(this->mode == 0 || frameCounter % intra_rate == 0){
            // intra coding
            // encode residuals
            this->encodeRes_intra(frameData, golomb, m_rate, 2);
            //cout << "sample size v" << frameData.size() << endl;
        }else if(this->mode == 1){
            // intra + inter coding (hybrid)
            // encode motion vectors and residuals
            encodeRes_inter(v_prev, frameData, golomb, m_rate, block_size, search_size);
        }
        if(this->lossy){
            // this->last_res contains last residuals (V)
            // encode residuals using huffman/golomb
            // append encoded residuals to this->encodedRes
            quantization->quantize(this->last_res, this->prevDCs.at(2), golomb, this->encodedRes, false);
        }

        if(this->mode == 1){
            // update previous
            v_prev = frameData;
        }

        frameCounter++;
    }
    cout << endl;

    this->totalFrames = frameCounter;

}

void VideoEncoder::encodeRes_intra(Mat &frame, Golomb *golomb, int m_rate, int k){
    // residuals
    char residual;
    // used to compute mean of mapped residuals
    float res_sum = 0;
    int numRes = 0;

    this->last_res = Mat::zeros(frame.rows, frame.cols, CV_64F);

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

            if(this->calcHist){
                // store residuals
                this->res_hists->at(k).push_back(residual);
                // store samples
                this->sample_hists->at(k).push_back(frame.at<uchar>(i,j));
            }

            // quantize residual here before encoding

            if (this->lossy){
                // save residual
                this->last_res.at<double>(i, j) = residual;
            }else{
                // encode residuals
                vector<bool> encodedResidual;
                golomb->encode2(residual, encodedResidual);
                encodedRes.insert(encodedRes.end(), encodedResidual.begin(), encodedResidual.end());

                // compute m
                int Mapped = 2 * residual;
                if(residual < 0){
                    Mapped = -Mapped-1;
                }
                res_sum += Mapped;
                numRes++;
                if(numRes == m_rate){
                    // calc mean from last m_rate mapped pixels
                    float res_mean = res_sum/numRes;
                    // calc alpha of geometric dist
                    // mu = alpha/(1 - alpha) <=> alpha = mu/(1 + mu)
                    float alpha = res_mean/(1+res_mean);
                    int m = ceil(-1/log(alpha));
                    if (m != 0){
                        //cout << "NEW M " << m << endl;
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

void VideoEncoder::encodeRes_inter(const Mat &prev_frame, const Mat &curr_frame, Golomb *golomb, int m_rate, int block_size, int search_size){
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

    // pad curr_frame with 0s on bottom and right sides
    Mat padded_curr_frame;
    int bd = block_size - (curr_frame.rows % block_size);
    int rd = block_size - (curr_frame.cols % block_size);
    copyMakeBorder(prev_frame, padded_curr_frame, 0, bd, 0, rd, BORDER_CONSTANT, Scalar(0));

    // pad prev_frame with 0s on all sides
    Mat padded_prev_frame;
    int sd = search_size * block_size; // search distance (in pixels)
    copyMakeBorder(prev_frame, padded_prev_frame, sd, sd + bd, sd, sd + rd, BORDER_CONSTANT, Scalar(0));

    // used to compute mean of mapped residuals
    float res_sum = 0;
    int numRes = 0;

    // traverse all blocks, for each block find motion vector with smallest MSE
    for(int grid_x = 0; grid_x < grid_w; grid_x++){
        for(int grid_y = 0; grid_y < grid_h; grid_y++){
            int x = grid_x * block_size;
            int y = grid_y * block_size;

            int best_block_x = 0;
            int best_block_y = 0;

            Mat curr_block = padded_curr_frame(cv::Rect(x, y, block_size, block_size));
            Mat best_residuals(block_size, block_size, CV_16SC1);
            double min_mse = -1;

            // perform exhaustive search on previous frame search area
            for(int search_x = x; search_x - x < (2*search_size + 1)*block_size && min_mse != 0; search_x+=block_size){
                for(int search_y = y; search_y - y < (2*search_size + 1)*block_size && min_mse != 0; search_y+=block_size){
                    Mat prev_block = padded_prev_frame(cv::Rect(search_x, search_y, block_size, block_size));

                    // calc residuals
                    Mat residuals(block_size, block_size, CV_16SC1);
                    subtract(curr_block, prev_block, residuals, noArray(), CV_16SC1);
                    // calc MSE
                    Mat newRes;
                    residuals.copyTo(newRes);
                    newRes.convertTo(newRes, CV_32F);
                    newRes = newRes.mul(newRes);
                    Scalar s = sum(newRes);
                    double mse = s.val[0] / (double) (newRes.total());

                    // update best motion vector
                    if (min_mse == -1 || mse < min_mse){
                        best_residuals = residuals;
                        best_block_x = search_x;
                        best_block_y = search_y;
                        min_mse = mse;
                    }
                }
            }

            // add motion vector to bit stream
            vector<bool> m_x;
            golomb->encode2(best_block_x, m_x);
            // add encoded residuals to bit stream
            encodedRes.insert(encodedRes.end(), m_x.begin(), m_x.end());
            vector<bool> m_y;
            golomb->encode2(best_block_y, m_y);
            // add encoded residuals to bit stream
            encodedRes.insert(encodedRes.end(), m_y.begin(), m_y.end());

            // write residuals
            for(int j = 0; j < best_residuals.cols; j++){
                for(int i = 0; i < best_residuals.rows; i++){
                    int residual = best_residuals.at<short>(i,j);
                    // golomb encode residuals
                    vector<bool> encodedResidual;
                    golomb->encode2(residual, encodedResidual);
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
                        // calc mean from last m_rate mapped pixels
                        float res_mean = res_sum/numRes;
                        // calc alpha of geometric dist
                        // mu = alpha/(1 - alpha) <=> alpha = mu/(1 + mu)
                        float alpha = res_mean/(1+res_mean);
                        int m = ceil(-1/log(alpha));
                        if (m != 0){
                            //cout << "NEW M " << m << "hybrid" << endl;
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

    // add 40 byte file header (initial_m, predictor, subsampling, mode, lossy, fps1, fps2, totalFrames, frame rows, frame cols)

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
    // lossy
    vector<bool> veclossy = int2boolvec(this->lossy);
    file.insert(file.end(), veclossy.begin(), veclossy.end());
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

    // write header
    wbs->writeNbits(file);

    // write data
    wbs->writeNbits(this->encodedRes);

    wbs->endWriteFile();
}

void VideoEncoder::convertionTo420(Mat &FrameData){
    if(subsampling!=420) {
        Mat NewFrameData;
        int rows, cols, k;
        if(subsampling==444){
            k = 2;
            rows = FrameData.rows/4;
            cols = FrameData.cols/4;
            NewFrameData = Mat(rows,cols,CV_8UC1);
        }
        else if(subsampling==422){
            k=1;
            rows = FrameData.rows/2;
            cols = FrameData.cols/2;
            NewFrameData = Mat(rows,cols,CV_8UC1);
        }
        int x = 0, y = 0;
        for (int i = 0; i < FrameData.rows; i += 2*k) {
            for (int j = 0; j < FrameData.cols; j += 2*k) {
                NewFrameData.at<uchar>(x,y) = FrameData.at<uchar>(i, j);
                y++;
            }
            x++;
        }
        FrameData.reshape(rows,cols) = NewFrameData;
    }
}

vector<vector<char>> VideoEncoder::get_res_hists(){
    return *(this->res_hists);
}

vector<vector<char>> VideoEncoder::get_sample_hists(){
    return *(this->sample_hists);
}