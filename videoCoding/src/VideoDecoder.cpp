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
}

void VideoDecoder::decode(){
    // read all data
    vector<bool> data = sourceFile->readNbits((sourceFile->size() - headerSize) * 8);
    unsigned int index = 0;

    // Golomb decoder
    auto *golomb = new Golomb(initial_m);
    int framesToDecode = 100; // must be equal to Encoder's m_rate

    int totalframes = 0;
    int frames = cols*rows*channels;
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

        //Read residuals
        int x = 0;
        int y = 0;
        int z = 0;
        for(int i = 0; i<framesToDecode; i++){
            residuals.at<cv::Vec3b>(x,y).val[z]= samples.at(y);
            y++;
            if(x==rows){
                x=0;
                y=0;
                z++;
            }
            if(y==cols){
                y=0;
                x++;
            }
            Mapped = 2 * residuals.at<cv::Vec3b>(x,y).val[z];
            if(residuals.at<cv::Vec3b>(x,y).val[z]< 0) Mapped = -Mapped-1;
            res_sum += Mapped;

            //TODO: calculate real value of frame
            //      write frame when is ready
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

}