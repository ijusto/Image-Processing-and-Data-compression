//
// Created by inesjusto on 10/12/20.
//

#include    "../../audioAndImageOrVideoManipulation/src/EntropyCalculator.cpp"
#include    "VideoCodec.hpp"
#include    "VideoReader.hpp"
#include    "LosslessJPEGPredictors.cpp"
#include    <fstream>
#include "../../entropyCoding/src/Golomb.cpp"


vector<bool> VideoCodec::int2boolvec(int n){
    vector<bool> bool_vec_res;

    unsigned int mask = 0x00000001;
    for(int i = 0; i < sizeof(n)*8; i++){
        bool bit = ((n & (mask << i)) >> i) == 1;
        bool_vec_res.push_back(bit);
    }

    return bool_vec_res;
}


VideoCodec::VideoCodec(char* srcFileName, char* destFileName, std::string predictor, int init_m) {
    /*
    VideoReader *videoReader;
    try{
        videoReader = new VideoReader(srcFileName);
    } catch (char* msg){
        std::cout << msg << std::endl;
        std::exit(0);
    }
    initial_m = init_m;
    cv::Mat firstFrame = videoReader->getCurrFrame();

    for(int i = 0; i < firstFrame.rows; i++){
        for(int j = 0; j < firstFrame.cols; j++){
            // TODO: deal with pixel info
            // TODO: a, b, c Vec3b or compare rgb?
            //   uchar blue = firstFrame.at<cv::Vec3b>(...).val[0];
            //   uchar green = firstFrame.at<cv::Vec3b>(...).val[1];
            //   uchar red = firstFrame.at<cv::Vec3b>(...).val[2];

            LosslessJPEGPredictors<cv::Vec3b> predictors(
                    (j == 0 ? 0 : firstFrame.at<cv::Vec3b>(i - 1 , j)),
                    (i == 0 ? 0 : firstFrame.at<cv::Vec3b>(i, j - 1)),
                    ((i == 0 | j == 0) ? 0 : firstFrame.at<cv::Vec3b>(i - 1, j - 1)));

            cv::Vec3b predicted1 = predictors.usePredictor1();
            cv::Vec3b predicted2 = predictors.usePredictor2();
            cv::Vec3b predicted3 = predictors.usePredictor3();
            cv::Vec3b predicted4 = predictors.usePredictor4();
            cv::Vec3b predicted5 = predictors.usePredictor5();
            cv::Vec3b predicted6 = predictors.usePredictor6();
            cv::Vec3b predicted7 = predictors.usePredictor7();
            cv::Vec3b predictedJLS = predictors.usePredictorJLS();
        }
    }
    */

    initial_m = init_m;

    // open video file
    ifstream video;
    video.open(srcFileName);

    if (!video.is_open()){
        cout << "Error opening file: " << destFileName << endl;
        exit(EXIT_FAILURE);
    }



    // Golomb encoder
    auto *golomb = new Golomb(initial_m);
    // calc m every m_rate frames
    int m_rate = 100;

    // parse header
    string header;
    getline(video, header);
    cout << "header: " << header << endl;

    // get rows, cols
    int rows = stoi(header.substr(header.find(" H") + 2, header.find(" F") - header.find(" H") - 2));
    int cols = stoi(header.substr(header.find(" W") + 2, header.find(" H") - header.find(" W") - 2));

    // OpenCV buffer
    frame = cv::Mat(rows, cols, CV_8UC3);
    residuals = cv::Mat(rows, cols, CV_8UC3);
    // read frame into this buffer
    unsigned char* frameData = new unsigned char[rows*cols*3];

    while(true){

        // skip word FRAME
        getline(video, header);

        video.read((char *) frameData, rows * cols * 3);

        if (video.gcount() == 0)
            break;


        // ptr to mat's data buffer (to be filled with pixels in packed mode)
        uchar *buffer = (uchar *) frame.ptr();

        for (int i = 0; i < rows * cols; i++) {
            // get YUV components from data in planar mode
            int y, u, v;

            // 4:2:0 (based on https://en.wikipedia.org/wiki/File:Yuv420.svg)
            int ci = (i % frame.cols) / 2;   // x of every 2 cols
            int ri = i / (frame.cols * 2);     // y of every 2 rows
            int shift = (frame.cols / 2) * ri; // shift amount within U/V planar region
            y = frameData[i];
            u = frameData[(ci + shift) + (frame.rows * frame.cols)];
            v = frameData[(ci + shift) + (frame.rows * frame.cols) + (frame.rows * frame.cols) / 4];


            // write to OpenCV buffer in packed mode
            buffer[i*3 ] = y;
            buffer[i*3 + 1] = u;
            buffer[i*3 + 2] = v;
        }


        for(int k = 0; k < 3; k++){
            for(int i = 0; i < frame.rows; i++){
                // used to compute mean of mapped residuals
                float res_sum = 0;
                int numRes = 0;
                int Mapped;
                for(int j = 0; j < frame.cols; j++){

                    // TODO: a, b, c Vec3b or compare rgb?
                    //   uchar blue = firstFrame.at<cv::Vec3b>(...).val[0];
                    //   uchar green = firstFrame.at<cv::Vec3b>(...).val[1];
                    //   uchar red = firstFrame.at<cv::Vec3b>(...).val[2];

                    LosslessJPEGPredictors<int> predictors(
                            (j == 0 ? 0 : frame.at<cv::Vec3b>(i,j-1).val[k]),
                            (i == 0 ? 0 : frame.at<cv::Vec3b>(i-1,j).val[k]),
                            ((i == 0 | j == 0) ? 0 : frame.at<cv::Vec3b>(i - 1, j - 1).val[k]));

                    //calculation of residuals for each predictor

                    switch (stoi(predictor)) {
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
                            std::cout << "ERROR: Predictor chosen is not corrected!!!" << std::endl;
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
                    if(residuals.at<cv::Vec3b>(i,j).val[k]<0) Mapped = -Mapped-1;
                    res_sum += Mapped;
                    numRes++;

                    //calculate new m
                    if(numRes == m_rate){
                        // calc mean from last 100 mapped pixels
                        float res_mean = res_sum/numRes;
                        float alpha = res_mean/(1+res_mean);
                        // calc alpha of geometric dist
                        // mu = alpha/(1 - alpha) <=> alpha = mu/(1 + mu)
                        int m = ceil(-1/log(alpha));
                        golomb->setM(m);
                        //reset
                        res_sum = 0;
                        numRes = 0;
                    }
                    std ::cout <<"z : "<< k << "x :" << i << "y: " << j << std::endl;
                }

            }

        }

    }
}

void VideoCodec::write(char *filename) {

    auto * wbs = new BitStream(filename, 'w');

    vector<bool> file;

    // add file header (initial_m, format, channels, frame rows, frame cols)
    // initial_m
    vector<bool> m = int2boolvec(initial_m);
    file.insert(file.cend(), m.begin(), m.end());

    // format
    vector<bool> format = int2boolvec(420);
    file.insert(file.end(), format.begin(), format.end());

    // channels
    vector<bool> channels = int2boolvec(3);
    file.insert(file.end(), channels.begin(), channels.end());

    // rows
    vector<bool> rows = int2boolvec(frame.rows);
    file.insert(file.end(), rows.begin(), rows.end());

    //cols
    vector<bool> cols = int2boolvec(frame.cols);
    file.insert(file.end(), cols.begin(), cols.end());

    //data channel 0
    file.insert(file.end(), encodedRes0.begin(), encodedRes0.end());
    //data channel 1
    file.insert(file.end(), encodedRes1.begin(), encodedRes1.end());
    //data channel 2
    file.insert(file.end(), encodedRes2.begin(), encodedRes2.end());

    wbs->writeNbits(file);
    wbs->endWriteFile();

}

//TODO: Test de program