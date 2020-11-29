//
// Created by bruno on 11/29/20.
//

#include "../includes/AudioDecoder.hpp"
#include <sndfile.hh>
#include <numeric>

using namespace std;

/**
 * Asserts that vector contains bits stored as least significant bit at the biggest address.
 * @param vec
 * @return
 */
int boolvec2int(vector<bool> vec){
    int acc = 0;
    for(int i = vec.size() - 1; i >= 0; i--){
        acc = (acc << 1) | vec.at(i);
    }
    return acc;
}

AudioDecoder::AudioDecoder(char* filename){
    sourceFile = new BitStream(filename, 'r');

    // read 16 byte file header (initial_m, format, channels, samplerate)
    int size = 4*8;
    try {
        initial_m = boolvec2int(sourceFile->readNbits(size));
        format = boolvec2int(sourceFile->readNbits(size));
        channels = boolvec2int(sourceFile->readNbits(size));
        samplerate = boolvec2int(sourceFile->readNbits(size));
    } catch( string mess){
        std::cout << mess << std::endl;
        std::exit(0);
    }
}

void AudioDecoder::decode(){
    int left_m = initial_m;
    int right_m = initial_m;
    // Golomb decoder
    auto *golomb = new Golomb(initial_m);

    int leftSample = 0;
    int rightSample = 0;

    // predictor: 3*sample_1 - 3*sample_2 + sample_3
    int leftSample_1 = 0;
    int leftSample_2 = 0;
    int leftSample_3 = 0;
    int rightSample_1 = 0;
    int rightSample_2 = 0;
    int rightSample_3 = 0;

    // residuals
    int leftRes = 0;
    int rightRes = 0;
    // used to compute mean of residuals
    float left_res_sum = 0;
    float right_res_sum = 0;
    int numRes = 0;

    // decode sample by sample (to update m)
}

void AudioDecoder::write(char* filename){
    SndfileHandle destFile  = SndfileHandle(filename, SFM_WRITE, format, channels, samplerate);
    int frames = decodedRes.size()/channels;
    destFile.write(decodedRes.data(), frames);
}