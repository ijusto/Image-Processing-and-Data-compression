//
// Created by irocs on 2/1/21.
//

#include "../includes/extraMile.hpp"


using namespace std;
using namespace cv;

vector<bool> AudioEncoder::int2boolvec(int n){
    vector<bool> bool_vec_res;

    unsigned int mask = 0x00000001;
    for(int i = 0; i < sizeof(n)*8; i++){
        bool bit = ((n & (mask << i)) >> i) == 1;
        bool_vec_res.push_back(bit);
    }

    return bool_vec_res;
}

AudioEncoder::AudioEncoder(char* filename, int m, bool ll, unsigned int qBits, bool calcHist){
    initial_m = m;
    lossless = ll;
    quantBits = qBits;
    calcHistogram = calcHist;

    sourceFile = SndfileHandle(filename, SFM_READ);
    if(sourceFile.error()) {
        std::cerr << "Error: invalid input file" << std::endl;
        exit(EXIT_FAILURE);
    }

    if((sourceFile.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        std::cerr << "Error: file is not in WAV format" << std::endl;
        exit(EXIT_FAILURE);
    }

    if((sourceFile.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        std::cerr << "Error: file is not in PCM_16 format" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void AudioEncoder::encode(){
    if (sourceFile.frames() == 0) {
        std::cerr << "Error: File with zero frames." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (sourceFile.channels() != 2){
        std::cerr << "Error: File must have 2 channels." << std::endl;
        exit(EXIT_FAILURE);
    }

    int framesToRead = 65536;
    std::vector<short> audioSample(sourceFile.channels() * framesToRead);

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
    // calc m every m_rate frames
    int m_rate = 1000;

    // used to compute mean of mapped residuals
    float left_res_sum = 0;
    float right_res_sum = 0;
    int numRes = 0;

    int totalframes = 0;
    for(sf_count_t nFrames = sourceFile.readf(audioSample.data(), framesToRead);
        nFrames != 0;
        nFrames = sourceFile.readf(audioSample.data(), framesToRead)) {

        totalframes += nFrames;
        cout << "encoded frames: " << totalframes << "/" << sourceFile.frames() << endl;

        for (int fr = 0; fr < nFrames; fr++) {
            leftSample = audioSample.at(sourceFile.channels()*fr + 0);
            rightSample = audioSample.at(sourceFile.channels()*fr + 1);

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

            // encode left
            vector<bool> encodedResidual = golomb->encode2(leftRes);
            // append
            encodedRes.insert(encodedRes.end(), encodedResidual.begin(), encodedResidual.end());

            // encode right
            encodedResidual = golomb->encode2(rightRes);
            // append
            encodedRes.insert(encodedRes.end(), encodedResidual.begin(), encodedResidual.end());

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

void AudioEncoder::write(char* filename){
    auto * wbs = new BitStream(filename, 'w');

    vector<bool> file;

    // add 28 byte file header (initial_m, format, channels, samplerate, frames, lossless, qBits)
    // initial_m
    vector<bool> m = int2boolvec(initial_m);
    file.insert(file.cend(), m.begin(), m.end());
    // format
    vector<bool> format = int2boolvec(sourceFile.format());
    file.insert(file.end(), format.begin(), format.end());
    // channels
    vector<bool> channels = int2boolvec(sourceFile.channels());
    file.insert(file.end(), channels.begin(), channels.end());
    // samplerate
    vector<bool> samplerate = int2boolvec(sourceFile.samplerate());
    file.insert(file.end(), samplerate.begin(), samplerate.end());
    // frames
    vector<bool> frames = int2boolvec(sourceFile.frames());
    file.insert(file.end(), frames.begin(), frames.end());
    // lossless mode
    vector<bool> ll = int2boolvec(lossless);
    file.insert(file.end(), ll.begin(), ll.end());
    // quant Bits used in lossy mode
    vector<bool> qBits = int2boolvec(quantBits);
    file.insert(file.end(), qBits.begin(), qBits.end());

    // add data
    file.insert(file.end(), encodedRes.begin(), encodedRes.end());

    wbs->writeNbits(file);
    wbs->endWriteFile();
}

const vector<short> &AudioEncoder::getLeftResiduals() const {
    return leftResiduals;
}

const vector<short> &AudioEncoder::getRightResiduals() const {
    return rightResiduals;
}

const vector<short> &AudioEncoder::getLeftSamples() const {
    return leftSamples;
}

const vector<short> &AudioEncoder::getRightSamples() const {
    return rightSamples;
}


using namespace std;

int AudioDecoder::boolvec2int(vector<bool> vec){
    int acc = 0;
    for(int i = vec.size() - 1; i >= 0; i--){
        acc = (acc << 1) | vec.at(i);
    }
    return acc;
}

AudioDecoder::AudioDecoder(char* filename){
    sourceFile = new BitStream(filename, 'r');

    // add 28 byte file header (initial_m, format, channels, samplerate, frames, lossless, qBits)
    headerSize = 28;        // bytes
    int paramsSize = 32;    // bits
    try {
        initial_m = boolvec2int(sourceFile->readNbits(paramsSize));
        format = boolvec2int(sourceFile->readNbits(paramsSize));
        channels = boolvec2int(sourceFile->readNbits(paramsSize));
        samplerate = boolvec2int(sourceFile->readNbits(paramsSize));
        frames = boolvec2int(sourceFile->readNbits(paramsSize));
        lossless = boolvec2int(sourceFile->readNbits(paramsSize));
        quantBits = boolvec2int(sourceFile->readNbits(paramsSize));
    } catch( string mess){
        std::cout << mess << std::endl;
        std::exit(0);
    }

}

void AudioDecoder::decode(){
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

    // GolombVideo decoder
    auto *golomb = new Golomb(initial_m);
    int framesToDecode = 1000; // must be equal to Encoder's m_rate

    // read all data
    vector<bool> data = sourceFile->readNbits((sourceFile->size() - headerSize) * 8);
    unsigned int index = 0;

    // used to compute mean of mapped residuals
    float left_res_sum = 0;
    float right_res_sum = 0;

    int totalframes = 0;
    while(totalframes < frames){
        // to not read more than available
        if (totalframes + framesToDecode > frames){
            framesToDecode = frames - totalframes;
        }

        totalframes += framesToDecode;
        cout << "decoded frames: " << totalframes << "/" << frames << endl;

        //auto start = high_resolution_clock::now();
        vector<int> samples = golomb->decode2(data, &index, framesToDecode * channels);
        //auto stop = high_resolution_clock::now();
        //auto duration = duration_cast<microseconds>(stop - start);
        //cout << "elapsed time: " << duration.count() << "micro seconds" << endl;

        for (int fr = 0; fr < framesToDecode; fr++) {
            leftRes = samples.at(fr*channels + 0);
            rightRes = samples.at(fr*channels + 1);

            if(lossless){
                // use predictor (best for lossless)
                predLeftSample = 3*leftSample_1 - 3*leftSample_2 + leftSample_3;
                predRightSample = 3*rightSample_1 - 3*rightSample_2 + rightSample_3;
                // re calc samples
                leftSample = predLeftSample + leftRes;
                rightSample = predRightSample + rightRes;
            }else{
                // use predictor (best for lossy)
                predLeftSample = leftSample_1;
                predRightSample = rightSample_1;
                // re calc samples with correctly scaled residuals
                leftSample = predLeftSample + (leftRes << quantBits);
                rightSample = predRightSample + (rightRes << quantBits);
            }

            // update
            leftSample_3 = leftSample_2;
            leftSample_2 = leftSample_1;
            leftSample_1 = leftSample;
            rightSample_3 = rightSample_2;
            rightSample_2 = rightSample_1;
            rightSample_1 = rightSample;

            decodedRes.push_back(leftSample);
            decodedRes.push_back(rightSample);

            // update sum used for mean when computing m
            // first map sin residuals to geometric dist used in golomb encoding
            int leftnMapped = 2 * leftRes;
            if (leftRes < 0){ leftnMapped = -leftnMapped - 1; }
            int rightnMapped = 2 * rightRes;
            if (rightRes < 0){ rightnMapped = -rightnMapped - 1; }
            left_res_sum += leftnMapped;
            right_res_sum += rightnMapped;
        }

        // compute m
        // calc mean from last framesToDecode mapped samples
        float left_res_mean = left_res_sum/framesToDecode;
        float right_res_mean = right_res_sum/framesToDecode;
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
    }
}

void AudioDecoder::write(char* filename){
    SndfileHandle destFile  = SndfileHandle(filename, SFM_WRITE, format, channels, samplerate);
    destFile.write(decodedRes.data(), decodedRes.size());
}


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

    // GolombVideo encoder
    auto *golomb = new GolombVideo(this->initial_m);
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

void VideoEncoder::encodeRes_intra(Mat &frame, GolombVideo *golomb, int m_rate, int k){
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

void VideoEncoder::encodeRes_inter(const Mat &prev_frame, const Mat &curr_frame, GolombVideo *golomb, int m_rate, int block_size, int search_size){
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
                    int residual = (int) best_residuals.at<short>(i,j);
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

void VideoEncoder::submatsResiduals(const Mat &prev, const Mat &curr, Mat &outRes, double &outMSE){
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

vector<vector<char>> VideoEncoder::get_sample_hists() {
    return *(this->sample_hists);
}


int VideoDecoder::boolvec2int(vector<bool> vec){
    int acc = 0;
    for(int i = vec.size() - 1; i >= 0; i--){
        acc = (acc << 1) | vec.at(i);
    }
    return acc;
}

VideoDecoder::VideoDecoder(char* encodedFileName){
    sourceFile = new BitStream(encodedFileName, 'r');

    // 36 byte file header (initial_m, predictor, subsampling, mode, fps1, fps2, frame rows, frame cols)
    headerSize = 36;        // bytes
    int paramsSize = 32;    // bits 4 bytes

    try {
        initial_m = boolvec2int(sourceFile->readNbits(paramsSize));
        predictor =  boolvec2int(sourceFile->readNbits(paramsSize));
        subsampling = boolvec2int(sourceFile->readNbits(paramsSize));
        mode = boolvec2int(sourceFile->readNbits(paramsSize));
        fps1 = boolvec2int(sourceFile->readNbits(paramsSize));
        fps2 = boolvec2int(sourceFile->readNbits(paramsSize));
        totalFrames = boolvec2int(sourceFile->readNbits(paramsSize));
        rows =  boolvec2int(sourceFile->readNbits(paramsSize));
        cols =  boolvec2int(sourceFile->readNbits(paramsSize));

    } catch( string mess){
        std::cout << mess << std::endl;
        std::exit(0);
    }
}

void VideoDecoder::decode(){
    // read all data
    vector<bool> data = sourceFile->readNbits((sourceFile->size() - headerSize) * 8);
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

    // GolombVideo decoder
    GolombVideo *golomb = new GolombVideo(initial_m);
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
                getResAndUpdate(data, &index, size, golomb, m_rate, residuals);
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
                // update prev
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
void VideoDecoder::update_m(vector<int> residuals, GolombVideo *golomb, int m_rate){
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
            //cout << "NEW M " << m << endl;
            golomb->setM(m);
        }
        //reset
        res_sum = 0;
        numRes = 0;
    }
}

void VideoDecoder::getResAndUpdate(vector<bool> &data, unsigned int *indexPtr, int n_residuals, GolombVideo *golomb, int m_rate, vector<int> &outRes){
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
    copyMakeBorder(prev_frame, padded_prev_frame, sd, sd, sd, sd, BORDER_CONSTANT, Scalar(0));

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
            for(int i = 0; i < block_size; i++){
                for(int j = 0; j < block_size; j++){
                    // add residuals to prev rect
                    int value = prev_block.at<uchar>(i, j) + currFrameResiduals.at(idx+(i*block_size+j));
                    // add result to outPlanarValues
                    outPlanarValues.push_back(value);
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
