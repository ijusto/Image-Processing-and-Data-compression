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



VideoEncoder::VideoEncoder(char* srcFileName, int pred, int mode, int init_m) {
    this->predictor = pred;
    this->mode = mode;
    this->initial_m = init_m;

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

    int frameSize;

    switch(subsampling){
        case 444:
            frameSize = rows * cols * 3;
            break;
        case 422:
            frameSize = rows * cols  + (rows * cols * 2) / 2;
            break;
        case 420:
            frameSize = rows * cols + (rows * cols * 2) / 4;
            break;
    }

    // data buffer of yuv values with subsampling
    unsigned char* frameData = new unsigned char[frameSize];

    // data buffer of yuv values without subsampling
    cv::Mat frame = cv::Mat(rows, cols, CV_8UC3);

    // Golomb encoder
    auto *golomb = new Golomb(this->initial_m);
    // calc m every m_rate frames
    int m_rate = 100;
    // residuals
    cv::Mat residuals = cv::Mat(rows, cols, CV_8UC1);

    while(true){
        // skip word FRAME
        getline(video, header);
        // read data
        video.read((char *) frameData, frameSize);
        if (video.gcount() == 0)
            break;
        // ptr to mat's data buffer (to be filled with pixels in packed mode)
        uchar *buffer = (uchar *) frame.ptr();

        for (int i = 0; i < rows * cols; i++) {
            // get YUV components from data in planar mode
            int y, u, v;

            switch (subsampling) {
                case 444:
                    // 4:4:4
                    y = frameData[i];
                    u = frameData[i + (rows * cols)];
                    v = frameData[i + (rows * cols) * 2];
                    break;
                case 422:
                    // 4:2:2
                    y = frameData[i];
                    u = frameData[i / 2 + (rows * cols)];
                    v = frameData[i / 2 + (rows * cols) + (rows * cols) / 2];
                    break;
                case 420:
                    // 4:2:0 (based on https://en.wikipedia.org/wiki/File:Yuv420.svg)
                    int ci = (i % cols) / 2;   // x of every 2 cols
                    int ri = i / (cols * 2);     // y of every 2 rows
                    int shift = (cols / 2) * ri; // shift amount within U/V planar region
                    y = frameData[i];
                    u = frameData[(ci + shift) + (rows * cols)];
                    v = frameData[(ci + shift) + (rows * cols) + (rows * cols) / 4];
                    break;
            }

            // write to OpenCV buffer in packed mode
            buffer[i*3 ] = y;
            buffer[i*3 + 1] = u;
            buffer[i*3 + 2] = v;
        }

//        cout << "test" << frame.size() << endl;
//        imshow("display", frame);
//        waitKey(0);
        // for every channel
        for(int k = 0; k < 3; k++){
            // used to compute mean of mapped residuals
            float res_sum = 0;
            int numRes = 0;
            int Mapped;

            for(int i = 0; i < this->rows; i++){
                for(int j = 0; j < this->cols; j++){

                    LosslessJPEGPredictors<int> predictors(
                            (j == 0 ? 0 : frame.at<cv::Vec3b>(i,j-1).val[k]),
                            (i == 0 ? 0 : frame.at<cv::Vec3b>(i-1,j).val[k]),
                            ((i == 0 | j == 0) ? 0 : frame.at<cv::Vec3b>(i - 1, j - 1).val[k]));

                    //calculation of residuals for each predictor
                    switch (this->predictor) {
                        case 1:
                            residuals.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor1();
                            break;
                        case 2:
                            residuals.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor2();
                            break;
                        case 3:
                            residuals.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor3();
                            break;
                        case 4:
                            residuals.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor4();
                            break;
                        case 5:
                            residuals.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor5();
                            break;
                        case 6:
                            residuals.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor6();
                            break;
                        case 7:
                            residuals.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictor7();
                            break;
                        case 8:
                            residuals.at<cv::Vec3b>(i,j).val[k] = frame.at<cv::Vec3b>(i,j).val[k] - predictors.usePredictorJLS();
                            break;
                        default:
                            std::cout << "ERROR: Predictor chosen isn't correct!!!" << std::endl;
                            exit(EXIT_FAILURE);
                    }

                    // encode channels
                    vector<bool> encodedResidual = golomb->encode2(residuals.at<cv::Vec3b>(i,j).val[k]);

                    switch (k) {
                        case 0:
                            encodedRes0.insert(encodedRes0.end(), encodedResidual.begin(), encodedResidual.end());
                            break;
                        case 1:
                            encodedRes1.insert(encodedRes1.end(), encodedResidual.begin(), encodedResidual.end());
                            break;
                        case 2:
                            encodedRes2.insert(encodedRes2.end(), encodedResidual.begin(), encodedResidual.end());
                            break;
                        default:
                            std::cout << "ERROR !!!" << std::endl;
                            exit(EXIT_FAILURE);
                    }

                    // compute m
                    Mapped = 2 * residuals.at<cv::Vec3b>(i,j).val[k];
                    if(residuals.at<cv::Vec3b>(i,j).val[k]< 0) Mapped = -Mapped-1;
                    res_sum += Mapped;
                    numRes++;

                    //calculate new m
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
//            cout << "test" << residuals.size() << endl;
//            imshow("display", residuals);
//            waitKey(0);
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

    //data channel 0
    file.insert(file.end(), this->encodedRes0.begin(), this->encodedRes0.end());
    //data channel 1
    file.insert(file.end(), this->encodedRes1.begin(), this->encodedRes1.end());
    //data channel 2
    file.insert(file.end(), this->encodedRes2.begin(), this->encodedRes2.end());

    wbs->writeNbits(file);
    wbs->endWriteFile();
}

//TODO: Test de program