//
// Created by bruno on 11/29/20.
//

#include "../includes/AudioDecoder.hpp"
#include <sndfile.hh>

#include <chrono>
using namespace std::chrono;

using namespace std;

int AudioDecoder::boolvec2int(vector<bool> vec){
    int acc = 0;
    for(int i = vec.size() - 1; i >= 0; i--){
        acc = (acc << 1) | vec.at(i);
    }
    return acc;
}

AudioDecoder::AudioDecoder(char* filename){
    sourceFile = new BitStream(filename, 'r');

    // read 20 byte file header (initial_m, format, channels, samplerate, frames)
    int size = 4*8;
    try {
        initial_m = boolvec2int(sourceFile->readNbits(size));
        format = boolvec2int(sourceFile->readNbits(size));
        channels = boolvec2int(sourceFile->readNbits(size));
        samplerate = boolvec2int(sourceFile->readNbits(size));
        frames = boolvec2int(sourceFile->readNbits(size));
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

    // read all data
    vector<bool> data = sourceFile->readNbits((3847369 - 16)*8); //  for sample01.wav

    unsigned int index = 0;
    int framesRead = 0;
    // decode sample by sample (to update m)
    while(framesRead < frames){
        int nFrames = 20000;
        // to not read more than available
        if (framesRead+ nFrames > frames){
            nFrames = frames - framesRead;
        }

        //auto start = high_resolution_clock::now();
        vector<int> samples = golomb->decode2(data, &index, nFrames*channels);
        //auto stop = high_resolution_clock::now();
        //auto duration = duration_cast<microseconds>(stop - start);
        //cout << "elapsed time: " << duration.count() << "micro seconds" << endl;

        for (int fr = 0; fr < nFrames; fr++) {
            int leftRes = samples.at(fr*channels + 0);
            int rightRes = samples.at(fr*channels + 1);

            int predLeftSample = 3 * leftSample_1 - 3 * leftSample_2 + leftSample_3;
            int predRightSample = 3 * rightSample_1 - 3 * rightSample_2 + rightSample_3;
            leftSample = predLeftSample - leftRes;
            rightSample = predRightSample - rightRes;

            // update
            leftSample_3 = leftSample_2;
            leftSample_2 = leftSample_1;
            leftSample_1 = leftSample;
            rightSample_3 = rightSample_2;
            rightSample_2 = rightSample_1;
            rightSample_1 = rightSample;

            decodedRes.push_back(leftSample);
            decodedRes.push_back(rightSample);
        }

        framesRead += nFrames;
        cout << "frames decoded: " << framesRead << "/" << frames << endl;
    }
}

void AudioDecoder::write(char* filename){
    SndfileHandle destFile  = SndfileHandle(filename, SFM_WRITE, format, channels, samplerate);
    destFile.write(decodedRes.data(), decodedRes.size());
}