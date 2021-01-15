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
    cout << "data size " << data.size() << endl;
    unsigned int index = 0;

    // Golomb decoder
    Golomb *golomb = new Golomb(initial_m);
    int m_rate = 100; // must be equal to Encoder's m_rate
    int decodedFrames = 0;

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


    while(decodedFrames < this->totalFrames) {
        decodedFrames++;
        cout << "decoded frames: " << decodedFrames << "/" << this->totalFrames << endl;

        vector<uchar> frame;

        // read y
        vector<int> samples = golomb->decode2(data, &index, Y_frame_rows * Y_frame_cols);
        // decode y component and add to frame vector
        cout << "woops1" << endl;
        this->decodeRes_intra(samples, frame, Y_frame_rows, Y_frame_cols, golomb, m_rate);
        cout << "woops2" << endl;

        // read u
        samples = golomb->decode2(data, &index, U_frame_rows * U_frame_cols);
        // decode u component and add to frame vector
        this->decodeRes_intra(samples, frame, U_frame_rows, U_frame_cols, golomb, m_rate);

        // read v
        samples = golomb->decode2(data, &index, V_frame_rows * V_frame_cols);
        // decode v component and add to frame vector
        this->decodeRes_intra(samples, frame, V_frame_rows, V_frame_cols, golomb, m_rate);

        // add frame to buffer
        this->frames.push_back(frame);
    }
}

void VideoDecoder::decodeRes_intra(vector<int> &residualValues, vector<uchar> &planarValues, int f_rows, int f_cols, Golomb *golomb, int m_rate){
    // used to compute mean of mapped residuals
    float res_sum = 0;
    int numRes = 0;

    for(int i = 0; i < f_rows; i++){
        for(int j = 0; j < f_cols; j++){
            int idx = i * f_rows + j;

            LosslessJPEGPredictors<int> predictors(
                    (j == 0 ? 0 : planarValues[i * f_rows + (j-1)]),                    // (i,j-1)
                    (i == 0 ? 0 : planarValues[(i-1) * f_rows + j]),                    // (i-1,j)
                    ((i == 0 | j == 0) ? 0 : planarValues[(i-1) * f_rows + (j-1)]));    // (i-1,j-1)

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
            planarValues.push_back(value);

            // compute m
            int Mapped = 2 * residualValues.at(idx);
            if(residualValues.at(idx) < 0){
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