#include    "../includes/VideoDecoder.hpp"

using namespace cv;

int VideoDecoder::boolvec2int(vector<bool> vec){
    int acc = 0;
    for(int i = vec.size() - 1; i >= 0; i--){
        acc = (acc << 1) | vec.at(i);
    }
    return acc;
}

VideoDecoder::VideoDecoder(char* encodedFileName){
    sourceFile = new BitStream(encodedFileName, 'r');

    // 32 byte file header (initial_m, predictor, subsampling, mode, fps1, fps2, frame rows, frame cols)
    headerSize = 32;        // bytes
    int paramsSize = 32;    // bits 4 bytes

    try {
        initial_m = boolvec2int(sourceFile->readNbits(paramsSize));
        predictor =  boolvec2int(sourceFile->readNbits(paramsSize));
        subsampling = boolvec2int(sourceFile->readNbits(paramsSize));
        mode = boolvec2int(sourceFile->readNbits(paramsSize));
        fps1 = boolvec2int(sourceFile->readNbits(paramsSize));
        fps2 = boolvec2int(sourceFile->readNbits(paramsSize));
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
    cout << data.size() << endl;
    unsigned int index = 0;

    // residuals
    cv :: Mat residuals = cv::Mat(rows, cols, CV_8UC3);

    // Golomb decoder
    Golomb *golomb = new Golomb(initial_m);
    int framesToDecode = 100; // must be equal to Encoder's m_rate

    int totalframes = 0;
    int frames = data.size();

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

    //vars to reconstruct de frame
    int x = 0;
    int y = 0;
    int z = 0;
    while(totalframes < frames) {
        // to not read more than available
        if (totalframes + framesToDecode > frames) {
            framesToDecode = frames - totalframes;
        }

        totalframes += framesToDecode;
        cout << "decoded frames: " << totalframes << "/" << frames << endl;

        vector<int> samples = golomb->decode2(data, &index, framesToDecode);
        cout << index << endl;

        // used to compute mean of mapped residuals
        float res_sum = 0;
        int numRes = 0;
        int Mapped;

        for(int i = 0; i<framesToDecode; i++){
            residuals.at<cv::Vec3b>(x,y).val[z]= samples.at(i);

            LosslessJPEGPredictors<int> predictors(
                    (y == 0 ? 0 : frame.at<cv::Vec3b>(x,y-1).val[z]),
                    (x == 0 ? 0 : frame.at<cv::Vec3b>(x-1,y).val[z]),
                    ((x == 0 | y == 0) ? 0 : frame.at<cv::Vec3b>(x - 1, y - 1).val[z]));

            //calculation of residuals for each predictor
            switch (this->predictor) {
                case 1:
                    frame.at<cv::Vec3b>(x,y).val[z] = residuals.at<cv::Vec3b>(x,y).val[z] + predictors.usePredictor1();
                    break;
                case 2:
                    frame.at<cv::Vec3b>(x,y).val[z] = residuals.at<cv::Vec3b>(x,y).val[z] + predictors.usePredictor2();
                    break;
                case 3:
                    frame.at<cv::Vec3b>(x,y).val[z] = residuals.at<cv::Vec3b>(x,y).val[z] + predictors.usePredictor3();
                    break;
                case 4:
                    frame.at<cv::Vec3b>(x,y).val[z] = residuals.at<cv::Vec3b>(x,y).val[z] + predictors.usePredictor4();
                    break;
                case 5:
                    frame.at<cv::Vec3b>(x,y).val[z] = residuals.at<cv::Vec3b>(x,y).val[z] + predictors.usePredictor5();
                    break;
                case 6:
                    frame.at<cv::Vec3b>(x,y).val[z] = residuals.at<cv::Vec3b>(x,y).val[z] + predictors.usePredictor6();
                    break;
                case 7:
                    frame.at<cv::Vec3b>(x,y).val[z] = residuals.at<cv::Vec3b>(x,y).val[z] + predictors.usePredictor7();
                    break;
                case 8:
                    frame.at<cv::Vec3b>(x,y).val[z] = residuals.at<cv::Vec3b>(x,y).val[z] + predictors.usePredictorJLS();
                    break;
                default:
                    std::cout << "ERROR: Predictor chosen isn't correct!!!" << std::endl;
                    exit(EXIT_FAILURE);
            }

            Mapped = 2 * residuals.at<cv::Vec3b>(x,y).val[z];
            if(residuals.at<cv::Vec3b>(x,y).val[z]< 0) Mapped = -Mapped-1;
            res_sum += Mapped;

            y++;
            if(z==0) {
                if (x == (Y_frame_rows - 1) && y == (Y_frame_cols - 1)) {
                    x = 0;
                    y = 0;
                    z++;
                }
                if (y == (Y_frame_cols - 1)) {
                    y = 0;
                    x++;
                }
            }
            else if(z==1){
                if (x == (U_frame_rows - 1) && y == (Y_frame_cols - 1)) {
                    x = 0;
                    y = 0;
                    z++;
                }
                if (y == (U_frame_cols - 1)) {
                    y = 0;
                    x++;
                }
            }
            else{
                if (x == (V_frame_rows - 1) && y == (Y_frame_cols - 1)) {
                    x = 0;
                    y = 0;
                    z++;
                }
//                if(z==channels)
//                {
//                    //write de frame
//                    this->write();
//                    x=0;
//                    y=0;
//                    z=0;
//                }
                if (y == (V_frame_cols - 1)) {
                    y = 0;
                    x++;
                }

            }

        }

        // calc mean from last 100 mapped pixels
        float res_mean = res_sum/numRes;
        // calc alpha of geometric dist
        // mu = alpha/(1 - alpha) <=> alpha = mu/(1 + mu)
        float alpha = res_mean/(1+res_mean);
        int m = ceil(-1/log(alpha));
        if (m != 0){
            golomb->setM(m);
        }
    }

}

void VideoDecoder::write(char* fileName){
    // open output video file
    ofstream outvideo;
    outvideo.open (fileName);

    // write HEADER
    string header = format("YUV4MPEG2 W%d H%d F%d:%d\n", this->cols, this->rows, this->fps1, this->fps2);
    outvideo << header;

    //write frames
    for(vector<uchar> fr: this->frames){
        // write FRAME header
        outvideo << "FRAME\n";
        // write data
        outvideo.write((char *) fr.data(), fr.size());
    }
}