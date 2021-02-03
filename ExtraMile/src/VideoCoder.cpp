//
// Created by irocs on 2/1/21.
//

#include "../includes/VideoCoder.hpp"
#include "Golomb.cpp"
#include "LosslessJPEGPredictors.cpp"


using namespace std;
using namespace cv;

vector<bool> VideoCoder::int2boolvec(int n){
    vector<bool> bool_vec_res;

    unsigned int mask = 0x00000001;
    for(int i = 0; i < sizeof(n)*8; i++){
        bool bit = ((n & (mask << i)) >> i) == 1;
        bool_vec_res.push_back(bit);
    }

    return bool_vec_res;
}

VideoCoder::VideoCoder(char* filename, int m, bool ll, unsigned int qBits,char* srcFileName, int pred, int mode, bool lossy){
    //Audio
    initial_m = m;
    lossless = ll;
    quantBits = qBits;
    calcHistogram = calcHist;
    sourceFileAudio = SndfileHandle(filename, SFM_READ);
    if(sourceFileAudio.error()) {
        std::cerr << "Error: invalid input file" << std::endl;
        exit(EXIT_FAILURE);
    }

    if((sourceFileAudio.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        std::cerr << "Error: file is not in WAV form" << std::endl;
        exit(EXIT_FAILURE);
    }

    if((sourceFileAudio.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        std::cerr << "Error: file is not in PCM_16 form" << std::endl;
        exit(EXIT_FAILURE);
    }

    //Video
    this->predictor = pred;
    this->mode = mode;
    this->lossy = lossy;
    this->sourceFileVideo =srcFileName;

}

void VideoCoder::encodeAudio(){
    if (sourceFileAudio.frames() == 0) {
        std::cerr << "Error: File with zero frames." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (sourceFileAudio.channels() != 2){
        std::cerr << "Error: File must have 2 channels." << std::endl;
        exit(EXIT_FAILURE);
    }

    int framesToRead = 65536;
    std::vector<short> audioSample(sourceFileAudio.channels() * framesToRead);

    short leftSample = 0;
    short rightSample = 0;

    // predictor: 3*sample_1 - 3*sample_2 + sample_3
    short leftSample_1 = 0;
    short leftSample_2 = 0;
    short leftSample_3 = 0;
    short rightSample_1 = 0;
    short rightSample_2 = 0;
    short rightSample_3 = 0;

    // predictor values
    short predLeftSample;
    short predRightSample;
    // residuals
    short leftRes;
    short rightRes;

    // GolombVideo encoder
    auto *golomb = new Golomb(initial_m);
    // calc m every m_rate Videoframes
    int m_rate = 1000;

    // used to compute mean of mapped residuals
    float left_res_sum = 0;
    float right_res_sum = 0;
    int numRes = 0;

    int totalframes = 0;
    for(sf_count_t nFrames = sourceFileAudio.readf(audioSample.data(), framesToRead);
        nFrames != 0;
        nFrames = sourceFileAudio.readf(audioSample.data(), framesToRead)) {

        totalframes += nFrames;
        cout << "encoded frames: " << totalframes << "/" << sourceFileAudio.frames() << endl;

        for (int fr = 0; fr < nFrames; fr++) {
            leftSample = audioSample.at(sourceFileAudio.channels() * fr + 0);
            rightSample = audioSample.at(sourceFileAudio.channels() * fr + 1);

            // original samples
            if (calcHistogram){
                // add to samples list (for histogram)
                leftSamples.push_back(leftSample);
                rightSamples.push_back(rightSample);
            }

            if(lossless){
                // use predictor (best for lossless)
                predLeftSample = 3*leftSample_1 - 3*leftSample_2 + leftSample_3;
                predRightSample = 3*rightSample_1 - 3*rightSample_2 + rightSample_3;
                // calc residuals
                leftRes = leftSample - predLeftSample;
                rightRes = rightSample - predRightSample;
            }else{
                // use predictor (best for lossy)
                predLeftSample = leftSample_1;
                predRightSample = rightSample_1;
                // calc residuals
                leftRes = leftSample - predLeftSample;
                rightRes = rightSample - predRightSample;
                // quantize residuals
                // must not remove gaussian distribution
                leftRes = (leftRes >> quantBits);
                rightRes = (rightRes >> quantBits);
                // re calc samples to sync with decoder
                leftSample = predLeftSample + (leftRes << quantBits);
                rightSample = predRightSample + (rightRes << quantBits);
            }

            if (calcHistogram){
                // add to residuals list (for histogram)
                leftResiduals.push_back(leftRes);
                rightResiduals.push_back(rightRes);
            }

            // update
            leftSample_3 = leftSample_2;
            leftSample_2 = leftSample_1;
            leftSample_1 = leftSample;
            rightSample_3 = rightSample_2;
            rightSample_2 = rightSample_1;
            rightSample_1 = rightSample;

            // encodeAudio left
            vector<bool> encodedResidual = golomb->encode2(leftRes);
            // append
            encodedResAudio.insert(encodedResAudio.end(), encodedResidual.begin(), encodedResidual.end());

            // encodeAudio right
            encodedResidual = golomb->encode2(rightRes);
            // append
            encodedResAudio.insert(encodedResAudio.end(), encodedResidual.begin(), encodedResidual.end());

            // compute m
            // first map sin residuals to geometric dist used in golomb encoding
            int leftnMapped = 2 * leftRes;
            if (leftRes < 0){ leftnMapped = -leftnMapped - 1; }
            int rightnMapped = 2 * rightRes;
            if (rightRes < 0){ rightnMapped = -rightnMapped - 1; }
            left_res_sum += leftnMapped;
            right_res_sum += rightnMapped;

//            // display residuals transformed to geometric dist
//            if (calcHistogram){
//                // add to mapped residuals list (for histogram)
//                leftResiduals.push_back(leftnMapped);
//                rightResiduals.push_back(rightnMapped);
//            }

            numRes++;
            if(numRes == m_rate){
                // calc mean from last 100 mapped samples
                float left_res_mean = left_res_sum/numRes;
                float right_res_mean = right_res_sum/numRes;
                // calc alpha of geometric dist
                // mu = alpha/(1 - alpha) <=> alpha = mu/(1 + mu)
                float left_alpha = left_res_mean/(1 + left_res_mean);
                float right_alpha = right_res_mean/(1 + right_res_mean);

                int left_m = ceil(-1/log2(left_alpha));
                int right_m = ceil(-1/log2(right_alpha));
                int new_m = (left_m + right_m)/2 + 2; // off set by 2 if both alphas are 0
                golomb->setM(new_m);

                // reset
                left_res_sum = 0;
                right_res_sum = 0;
                numRes = 0;
            }
        }
    }
}

void VideoCoder::encodeVideo() {
    //data buffer
    Mat frameData; // open video file
    ifstream video;
    video.open(sourceFileVideo);

    if (!video.is_open()){
        cout << "Error opening file: " << sourceFileVideo << endl;
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


    // GolombVideo encoder
    auto *golomb = new Golomb(this->initial_m);
    // calc m every m_rate Videoframes
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
        cout << "\rencoded Videoframes: " << frameCounter;
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
        // update previous
        y_prev = frameData;

        // read u
        frameData = Mat(U_frame_rows, U_frame_cols, CV_8UC1);
        video.read((char *) frameData.ptr(), U_frame_rows * U_frame_cols);

        // compute residuals for u
        if(this->mode == 0 || frameCounter % intra_rate == 0){
            // intra coding
            // encode residuals
            this->encodeRes_intra(frameData, golomb, m_rate, 0);
            //cout << "sample size u" << frameData.size() << endl;
        }else if(this->mode == 1){
            // intra + inter coding (hybrid)
            // encode motion vectors and residuals
            encodeRes_inter(u_prev, frameData, golomb, m_rate, block_size, search_size);
        }
        // update previous
        u_prev = frameData;

        // read v
        frameData = Mat(V_frame_rows, V_frame_cols, CV_8UC1);
        video.read((char *) frameData.ptr(), V_frame_rows * V_frame_cols);

        // compute residuals for v
        if(this->mode == 0 || frameCounter % intra_rate == 0){
            // intra coding
            // encode residuals
            this->encodeRes_intra(frameData, golomb, m_rate, 0);
            //cout << "sample size v" << frameData.size() << endl;
        }else if(this->mode == 1){
            // intra + inter coding (hybrid)
            // encode motion vectors and residuals
            encodeRes_inter(v_prev, frameData, golomb, m_rate, block_size, search_size);
        }
        // update previous
        v_prev = frameData;

        frameCounter++;
    }
    cout << endl;

    this->totalFrames = frameCounter;
}

void VideoCoder::writeAudio(char* filename){
    auto * wbs = new BitStream(filename, 'w');

    vector<bool> file;

    // add 28 byte file header (initial_m, form, channels, sampleRate, Videoframes, lossless, qBits)
    // initial_m
    vector<bool> m = int2boolvec(initial_m);
    file.insert(file.cend(), m.begin(), m.end());
    // form
    vector<bool> format = int2boolvec(sourceFileAudio.format());
    file.insert(file.end(), format.begin(), format.end());
    // channels
    vector<bool> channels = int2boolvec(sourceFileAudio.channels());
    file.insert(file.end(), channels.begin(), channels.end());
    // sampleRate
    vector<bool> samplerate = int2boolvec(sourceFileAudio.samplerate());
    file.insert(file.end(), samplerate.begin(), samplerate.end());
    // Videoframes
    vector<bool> frames = int2boolvec(sourceFileAudio.frames());
    file.insert(file.end(), frames.begin(), frames.end());
    // lossless mode
    vector<bool> ll = int2boolvec(lossless);
    file.insert(file.end(), ll.begin(), ll.end());
    // quant Bits used in lossy mode
    vector<bool> qBits = int2boolvec(quantBits);
    file.insert(file.end(), qBits.begin(), qBits.end());

    // add data
    file.insert(file.end(), encodedResAudio.begin(), encodedResAudio.end());

    wbs->writeNbits(file);
    wbs->endWriteFile();
}

const vector<short> &VideoCoder::getLeftResiduals() const {
    return leftResiduals;
}

const vector<short> &VideoCoder::getRightResiduals() const {
    return rightResiduals;
}

const vector<short> &VideoCoder::getLeftSamples() const {
    return leftSamples;
}

const vector<short> &VideoCoder::getRightSamples() const {
    return rightSamples;
}


void VideoCoder::parseHeader(string &header){
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

void VideoCoder::encodeRes_intra(Mat &frame, Golomb *golomb, int m_rate, int k){
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

            if(this->calcHist){
                // store residuals
                this->res_hists->at(k).push_back(residual);
                // store samples
                this->sample_hists->at(k).push_back(frame.at<uchar>(i,j));
            }

            // quantize residual here before encoding

            // encode residuals
            vector<bool> encodedResidual;
            golomb->encode3(residual, encodedResidual);
            encodedResVideo.insert(encodedResVideo.end(), encodedResidual.begin(), encodedResidual.end());

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

void VideoCoder::encodeRes_inter(const Mat &prev_frame, const Mat &curr_frame, Golomb *golomb, int m_rate, int block_size, int search_size){
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

    // pad prev_frame with with 0s
    Mat padded_prev_frame;
    int sd = search_size * block_size; // search distance (in pixels)
    copyMakeBorder(prev_frame, padded_prev_frame, sd, sd, sd, sd, BORDER_CONSTANT, Scalar(0));

    // used to compute mean of mapped residuals
    float res_sum = 0;
    int numRes = 0;

    // traverse all blocks, for each block find motion vector with smallest MSE
    for(int x = 0; x < grid_w; x++){
        for(int y = 0; y < grid_h; y++){

            int best_block_x = 0;
            int best_block_y = 0;

            Mat curr_block = curr_frame(cv::Rect(x,y, block_size, block_size));
            Mat best_residuals(block_size, block_size, CV_16SC1);
            double min_mse = -1;

            // perform exhaustive search on previous frame search area
            for(int search_x = x; search_x < (2*search_size + 1)*block_size; search_x+=block_size){
                for(int search_y = y; search_y < (2*search_size + 1)*block_size; search_y+=block_size){
                    Mat prev_block = padded_prev_frame(cv::Rect(search_x, search_y, block_size, block_size));

                    // calc mse
                    Mat residuals(block_size, block_size, CV_16SC1);
                    double mse = 0;
                    submatsResiduals(prev_block, curr_block, residuals, mse);

                    // update best motion vector
                    if (min_mse == -1 || mse < min_mse){
                        best_residuals = residuals;
                        min_mse = mse;
                        best_block_x = search_x;
                        best_block_y = search_y;
                    }
                }
            }

            // add motion vector to bit stream
            vector<bool> m_x;
            golomb->encode3(best_block_x, m_x);
            // add encoded residuals to bit stream
            encodedResVideo.insert(encodedResVideo.end(), m_x.begin(), m_x.end());
            vector<bool> m_y;
            golomb->encode3(best_block_y, m_y);
            // add encoded residuals to bit stream
            encodedResVideo.insert(encodedResVideo.end(), m_y.begin(), m_y.end());

            // writeVideoFile residuals
            for(int j = 0; j < best_residuals.cols; j++){
                for(int i = 0; i < best_residuals.rows; i++){
                    int residual = (int) best_residuals.at<short>(i,j);
                    // golomb encodeAudio residuals
                    vector<bool> encodedResidual;
                    golomb->encode3(residual, encodedResidual);
                    // add encoded residuals to bit stream
                    encodedResVideo.insert(encodedResVideo.end(), encodedResidual.begin(), encodedResidual.end());

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

void VideoCoder::submatsResiduals(const Mat &prev, const Mat &curr, Mat &outRes, double &outMSE){
    // calc residuals
    subtract(prev, curr, outRes);
    // calc MSE
    Mat newRes;
    outRes.copyTo(newRes);
    newRes.convertTo(newRes, CV_32F);
    newRes = newRes.mul(newRes);
    Scalar s = sum(newRes);
    outMSE = s.val[0] / (double) (newRes.total());
}

void VideoCoder::writeVideo(char *filename) {
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

    // write header
    wbs->writeNbits(file);

    // write data
    wbs->writeNbits(this->encodedResVideo);

    wbs->endWriteFile();
}

void VideoCoder::convertionTo420(Mat &FrameData){
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

vector<vector<char>> VideoCoder::get_res_hists(){
    return *(this->res_hists);
}

vector<vector<char>> VideoCoder::get_sample_hists() {
    return *(this->sample_hists);
}

