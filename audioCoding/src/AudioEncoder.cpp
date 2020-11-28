/**
 * Class for reading wav audio files, encode with compression
 * and write result to a new audio file.
 */

#include "../includes/AudioEncoder.hpp"
#include "Golomb.cpp"
#include <iostream>

using namespace std;

AudioEncoder::AudioEncoder(char* filename){
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

    int srcFileChannels = 2;
    int framesRead = 65536;
    std::vector<short> audioSample(srcFileChannels*framesRead);

    short leftSample = 0;
    short rightSample = 0;
    // predictor: equal to last sample
    short predLeftSample = 0;
    short predRightSample = 0;
    // residuals
    short leftRes = 0;
    short rightRes = 0;
    // residual encoded with Golomb
    vector<bool> encodedNumber;
    // Golomb parameter
    int m = 2000; // find good estimate as residuals are computed

    int totalframes = 0;
    for(sf_count_t nFrames = sourceFile.readf(audioSample.data(), framesRead); nFrames != 0; nFrames = sourceFile.readf(audioSample.data(), framesRead)) {
        cout << "totalframes: " << totalframes << endl;
        totalframes += nFrames;

        for (int fr = 0; fr < nFrames; fr++) {
            leftSample = audioSample.at(fr + 0);
            rightSample = audioSample.at(fr + 1);

            leftRes = leftSample - predLeftSample;
            rightRes = rightSample - predRightSample;

            predLeftSample = leftSample;
            predRightSample = rightSample;

            auto *golomb = new Golomb(m);
            // encode left
            encodedNumber = golomb->encode2(leftRes);
            // append
            encodedRes.insert(encodedRes.end(), encodedNumber.begin(), encodedNumber.end());

            // encode right
            encodedNumber = golomb->encode2(rightRes);
            // append
            encodedRes.insert(encodedRes.end(), encodedNumber.begin(), encodedNumber.end());
        }
    }
}

void AudioEncoder::write(char* filename){
    cout << "writing" << endl;
    auto * wbs = new BitStream(filename, 'w');
    int srcFileChannels = 2;

    wbs->writeNbits(encodedRes);
    wbs->endWriteFile();
}