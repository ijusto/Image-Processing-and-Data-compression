/**
 * Class for reading wav audio files, encode with compression
 * and write result to a new audio file.
 */

#include "../includes/AudioEncoder.hpp"
#include "../../entropyCoding/src/Golomb.cpp"
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

AudioEncoder::AudioEncoder(char* filename, int m, bool ll, unsigned int qBits, bool calcHist){
    initial_m = m;
    lossless = ll;
    quantBits = qBits;
    calcHistogram = calcHist;

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

    int framesToRead = 65536;
    std::vector<short> audioSample(sourceFile.channels() * framesToRead);

    short leftSample = 0;
    short rightSample = 0;

    // predictor: 3*sample_1 - 3*sample_2 + sample_3
    short leftSample_1 = 0;
    short leftSample_2 = 0;
    short leftSample_3 = 0;
    short rightSample_1 = 0;
    short rightSample_2 = 0;
    short rightSample_3 = 0;

    // predictor values
    short predLeftSample;
    short predRightSample;
    // residuals
    short leftRes;
    short rightRes;

    // Golomb encoder
    auto *golomb = new Golomb(initial_m);
    // calc m every m_rate frames
    int m_rate = 1000;

    // used to compute mean of mapped residuals
    float left_res_sum = 0;
    float right_res_sum = 0;
    int numRes = 0;

    int totalframes = 0;
    for(sf_count_t nFrames = sourceFile.readf(audioSample.data(), framesToRead);
        nFrames != 0;
        nFrames = sourceFile.readf(audioSample.data(), framesToRead)) {

        totalframes += nFrames;
        cout << "encoded frames: " << totalframes << "/" << sourceFile.frames() << endl;

        for (int fr = 0; fr < nFrames; fr++) {
            leftSample = audioSample.at(sourceFile.channels()*fr + 0);
            rightSample = audioSample.at(sourceFile.channels()*fr + 1);

            // original samples
            if (calcHistogram){
                // add to samples list (for histogram)
                leftSamples.push_back(leftSample);
                rightSamples.push_back(rightSample);
            }

            if(lossless){
                // use predictor (best for lossless)
                predLeftSample = 3*leftSample_1 - 3*leftSample_2 + leftSample_3;
                predRightSample = 3*rightSample_1 - 3*rightSample_2 + rightSample_3;
                // calc residuals
                leftRes = leftSample - predLeftSample;
                rightRes = rightSample - predRightSample;
            }else{
                // use predictor (best for lossy)
                predLeftSample = leftSample_1;
                predRightSample = rightSample_1;
                // calc residuals
                leftRes = leftSample - predLeftSample;
                rightRes = rightSample - predRightSample;
                // quantize residuals
                // must not remove gaussian distribution
                leftRes = (leftRes >> quantBits);
                rightRes = (rightRes >> quantBits);
                // re calc samples to sync with decoder
                leftSample = predLeftSample + (leftRes << quantBits);
                rightSample = predRightSample + (rightRes << quantBits);
            }

            if (calcHistogram){
                // add to residuals list (for histogram)
                leftResiduals.push_back(leftRes);
                rightResiduals.push_back(rightRes);
            }

            // update
            leftSample_3 = leftSample_2;
            leftSample_2 = leftSample_1;
            leftSample_1 = leftSample;
            rightSample_3 = rightSample_2;
            rightSample_2 = rightSample_1;
            rightSample_1 = rightSample;

            // encode left
            vector<bool> encodedResidual = golomb->encode2(leftRes);
            // append
            encodedRes.insert(encodedRes.end(), encodedResidual.begin(), encodedResidual.end());

            // encode right
            encodedResidual = golomb->encode2(rightRes);
            // append
            encodedRes.insert(encodedRes.end(), encodedResidual.begin(), encodedResidual.end());

            // compute m
            // first map sin residuals to geometric dist used in golomb encoding
            int leftnMapped = 2 * leftRes;
            if (leftRes < 0){ leftnMapped = -leftnMapped - 1; }
            int rightnMapped = 2 * rightRes;
            if (rightRes < 0){ rightnMapped = -rightnMapped - 1; }
            left_res_sum += leftnMapped;
            right_res_sum += rightnMapped;

//            // display residuals transformed to geometric dist
//            if (calcHistogram){
//                // add to mapped residuals list (for histogram)
//                leftResiduals.push_back(leftnMapped);
//                rightResiduals.push_back(rightnMapped);
//            }

            numRes++;
            if(numRes == m_rate){
                // calc mean from last 100 mapped samples
                float left_res_mean = left_res_sum/numRes;
                float right_res_mean = right_res_sum/numRes;
                // calc alpha of geometric dist
                // mu = alpha/(1 - alpha) <=> alpha = mu/(1 + mu)
                float left_alpha = left_res_mean/(1 + left_res_mean);
                float right_alpha = right_res_mean/(1 + right_res_mean);

                int left_m = ceil(-1/log2(left_alpha));
                int right_m = ceil(-1/log2(right_alpha));
                int new_m = (left_m + right_m)/2 + 2; // off set by 2 if both alphas are 0
                golomb->setM(new_m);

                // reset
                left_res_sum = 0;
                right_res_sum = 0;
                numRes = 0;
            }
        }
    }
}

void AudioEncoder::write(char* filename){
    auto * wbs = new BitStream(filename, 'w');

    vector<bool> file;

    // add 28 byte file header (initial_m, format, channels, samplerate, frames, lossless, qBits)
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
    // lossless mode
    vector<bool> ll = int2boolvec(lossless);
    file.insert(file.end(), ll.begin(), ll.end());
    // quant Bits used in lossy mode
    vector<bool> qBits = int2boolvec(quantBits);
    file.insert(file.end(), qBits.begin(), qBits.end());

    // add data
    file.insert(file.end(), encodedRes.begin(), encodedRes.end());

    wbs->writeNbits(file);
    wbs->endWriteFile();
}

const vector<short> &AudioEncoder::getLeftResiduals() const {
    return leftResiduals;
}

const vector<short> &AudioEncoder::getRightResiduals() const {
    return rightResiduals;
}

const vector<short> &AudioEncoder::getLeftSamples() const {
    return leftSamples;
}

const vector<short> &AudioEncoder::getRightSamples() const {
    return rightSamples;
}

