#include    "VideoDecoder.hpp"


using namespace std;

int VideoDecoder::boolvec2int(vector<bool> vec){
    int acc = 0;
    for(int i = vec.size() - 1; i >= 0; i--){
        acc = (acc << 1) | vec.at(i);
    }
    return acc;
}

VideoDecoder::VideoDecoder(char* encodedFileName, char* destFileName, char* predictor, char* type){
}

void VideoDecoder::decode(){
}

void VideoDecoder::write(){
}