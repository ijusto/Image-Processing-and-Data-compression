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
    int paramsSize = 32;    // bits
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

}

void VideoDecoder::write(){
}