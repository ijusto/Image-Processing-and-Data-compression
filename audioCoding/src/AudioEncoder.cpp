/**
 * Class for reading wav audio files, encode with compression
 * and write result to a new audio file.
 */

#include "../includes/AudioEncoder.hpp"
#include "../includes/BitStream.hpp"
#include "../includes/Golomb.h"
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

    int srcFileChannels = 2; ;
    int framesRead = 65536;
    std::vector<short> audioSample(srcFileChannels*framesRead);

    short leftSample = 0;
    short rightSample = 0;
    // predictor: equal to last sample
    short predLeftSample = 0;
    short predRightSample = 0;

    for(sf_count_t nFrames = sourceFile.readf(audioSample.data(), framesRead); nFrames != 0; nFrames = sourceFile.readf(audioSample.data(), framesRead)) {
        for (int fr = 0; fr < nFrames; fr++) {
            leftSample = audioSample.at(fr + 0);
            rightSample = audioSample.at(fr + 0);

            leftChRes.push_back(leftSample - predLeftSample);
            rightChRes.push_back(rightSample - predRightSample);

            predLeftSample = leftSample;
            predRightSample = rightSample;
        }
    }

    cout << leftChRes.size() << endl;
    cout << rightChRes.size() << endl;
}

void AudioEncoder::write(char* filename){
    auto * wbs = new BitStream(filename, 'w');

    vector<bool> golombEncodedBits;

    /* Encode numbers test */
    auto *golomb = new Golomb(m, encodeFileName, 'e');

    vector<bool> encNumVec, encodedNumber;
    for(short number: original_array){
        // encode
        encodedNumber = golomb->encode(number);
        // append
        encNumVec.insert(encNumVec.end(), encodedNumber.begin(), encodedNumber.end());
    }

    wbs->writeNbits(golombEncodedBits);
    wbs->endWriteFile();
}