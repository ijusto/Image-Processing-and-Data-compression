#include    "VideoDecoder.hpp"


using namespace std;

int VideoDecoder::boolvec2int(vector<bool> vec){
    int acc = 0;
    for(int i = vec.size() - 1; i >= 0; i--){
        acc = (acc << 1) | vec.at(i);
    }
    return acc;
}

VideoDecoder::VideoDecoder(char* encodedFileName, char* destFileName, char* type){
    sourceFile = new BitStream(encodedFileName, 'r');
    dst = destFileName;
    headerSize = 28;        // bytes
    int paramsSize = 32;    // bits 4 bytes

    try {
        initial_m = boolvec2int(sourceFile->readNbits(paramsSize));
        predictor =  boolvec2int(sourceFile->readNbits(paramsSize));
        format = boolvec2int(sourceFile->readNbits(paramsSize));
        mode = boolvec2int(sourceFile->readNbits(paramsSize));
        channels = boolvec2int(sourceFile->readNbits(paramsSize));
        rows =  boolvec2int(sourceFile->readNbits(paramsSize));
        cols =  boolvec2int(sourceFile->readNbits(paramsSize));

    } catch( string mess){
        std::cout << mess << std::endl;
        std::exit(0);
    }
    // data buffer of yuv values without subsampling
    frame = cv::Mat(rows, cols, CV_8UC3);
}

void VideoDecoder::decode(){
    // read all data
    vector<bool> data = sourceFile->readNbits((sourceFile->size() - headerSize) * 8);
    unsigned int index = 0;

    // residuals
    cv :: Mat residuals = cv::Mat(rows, cols, CV_8UC3);

    // Golomb decoder
    auto *golomb = new Golomb(initial_m);
    int framesToDecode = 100; // must be equal to Encoder's m_rate

    int totalframes = 0;
    int frames = cols*rows*channels;

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
                    frame.at<cv::Vec3b>(x,y).val[z] = residuals.at<cv::Vec3b>(x,y).val[z] + predictors.usePredictor2();                    break;
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
            if(x == (rows-1)){
                x=0;
                y=0;
                z++;
            }
            if(y == (cols-1)){
                y=0;
                x++;
            }
            if(z == 3 && x == (rows-1)){
                for(int i = 0; i < rows ; i++){
                    for(int j = 0; j < cols; j++){
                        int y,u,v;
                        y = frame.at<cv::Vec3b>(i,j).val[0];
                        u = frame.at<cv::Vec3b>(x,y).val[1];
                        v = frame.at<cv::Vec3b>(x,y).val[2];

                        /* convert to RGB */
                        int b = (int)(1.164*(y - 16) + 2.018*(u-128));
                        int g = (int)(1.164*(y - 16) - 0.813*(u-128) - 0.391*(v-128));
                        int r = (int)(1.164*(y - 16) + 1.596*(v-128));

                        /* clipping to [0 ... 255] */
                        if(r < 0) r = 0;
                        if(g < 0) g = 0;
                        if(b < 0) b = 0;
                        if(r > 255) r = 255;
                        if(g > 255) g = 255;
                        if(b > 255) b = 255;

                        frame.at<cv::Vec3b>(i,j).val[0] = b;
                        frame.at<cv::Vec3b>(x,y).val[1] = g;
                        frame.at<cv::Vec3b>(x,y).val[2] = r;
                    }
                }
                //write de frame
                this->write();

                x=0;
                y=0;
                z=0;
            }

            //TODO: test program
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

void VideoDecoder::write(){

    VideoWriter writer;

    int codec = VideoWriter::fourcc('M', 'J', 'P', 'G');
    double fps = 25.0;

    writer.open(dst, codec, fps, frame.size(), CV_8UC3);

    // check if we succeeded
    if (!writer.isOpened()) {
        cerr << "Could not open the output video file for write\n";
        exit(EXIT_FAILURE);
    }

    writer.write(frame);
}