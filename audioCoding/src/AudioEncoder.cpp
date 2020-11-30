/**
 * Class for reading wav audio files, encode with compression
 * and write result to a new audio file.
 */

#include "../includes/AudioEncoder.hpp"
#include "Golomb.cpp"
#include <iostream>
#include <math.h>

using namespace std;

vector<bool> AudioEncoder::int2boolvec(int n){
    vector<bool> bool_vec_res;

    unsigned int mask = 0x00000001;
    for(int i = 0; i < sizeof(n)*8; i++){
        bool bit = ((n & (mask << i)) >> i) == 1;
        bool_vec_res.push_back(bit);
    }

    return bool_vec_res;
}

AudioEncoder::AudioEncoder(char* filename, int m){
    initial_m = m;
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

    int framesRead = 65536;
    std::vector<short> audioSample(sourceFile.channels()*framesRead);

    int leftSample = 0;
    int rightSample = 0;

    // predictor: 3*sample_1 - 3*sample_2 + sample_3
    int leftSample_1 = 0;
    int leftSample_2 = 0;
    int leftSample_3 = 0;
    int rightSample_1 = 0;
    int rightSample_2 = 0;
    int rightSample_3 = 0;

    // Golomb parameters
    int left_m = initial_m;
    int right_m = initial_m;
    // Golomb encoder
    auto *golomb = new Golomb(initial_m);

    // residuals
    int leftRes = 0;
    int rightRes = 0;
    // used to compute mean of residuals
    float left_res_sum = 0;
    float right_res_sum = 0;
    int numRes = 0;

    int totalframes = 0;
    for(sf_count_t nFrames = sourceFile.readf(audioSample.data(), framesRead); nFrames != 0; nFrames = sourceFile.readf(audioSample.data(), framesRead)) {
        totalframes += nFrames;
        cout << "encoded frames: " << totalframes << "/" << sourceFile.frames() << endl;

        for (int fr = 0; fr < nFrames; fr++) {
            leftSample = audioSample.at(sourceFile.channels()*fr + 0);
            rightSample = audioSample.at(sourceFile.channels()*fr + 1);

            int predLeftSample = 3*leftSample_1 - 3*leftSample_2 + leftSample_3;
            int predRightSample = 3*rightSample_1 - 3*rightSample_2 + rightSample_3;
            leftRes = leftSample - predLeftSample;
            rightRes = rightSample - predRightSample;

            // update
            leftSample_3 = leftSample_2;
            leftSample_2 = leftSample_1;
            leftSample_1 = leftSample;
            rightSample_3 = rightSample_2;
            rightSample_2 = rightSample_1;
            rightSample_1 = rightSample;

            // compute estimate of m
//            left_res_sum += leftRes;
//            right_res_sum += rightRes;
//            numRes++;
//            if(numRes == 1000){
//                float left_res_mean = left_res_sum/numRes;
//                float right_res_mean = right_res_sum/numRes;
//                float left_alpha = (-1 + sqrt(1+4*left_res_mean))/(2*left_res_mean);
//                float right_alpha = (-1 + sqrt(1+4*right_res_mean))/(2*right_res_mean);
//
//                left_m = ceil(-1/log2(left_alpha));
//                right_m = ceil(-1/log2(right_alpha));
//
//                cout << "left_m " << left_m << endl;
//                cout << "left_res_mean " << left_res_mean << endl;
//
//                left_res_sum = 0;
//                right_res_sum = 0;
//                numRes = 0;
//            }

            // encode left
            golomb->setM(left_m);
            vector<bool> encodedResidual = golomb->encode2(leftRes);
            // append
            encodedRes.insert(encodedRes.end(), encodedResidual.begin(), encodedResidual.end());

            // encode right
            golomb->setM(right_m);
            encodedResidual = golomb->encode2(rightRes);
            // append
            encodedRes.insert(encodedRes.end(), encodedResidual.begin(), encodedResidual.end());
        }
    }
}

void AudioEncoder::write(char* filename){
    cout << "writing..." << endl;
    auto * wbs = new BitStream(filename, 'w');

    vector<bool> file;

    // add 20 byte file header (initial_m, format, channels, samplerate, frames)
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

    // add data
    file.insert(file.end(), encodedRes.begin(), encodedRes.end());

    wbs->writeNbits(file);
    wbs->endWriteFile();
}