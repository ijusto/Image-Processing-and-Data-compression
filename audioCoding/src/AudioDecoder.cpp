/**
 * Class for reading audio files in defined compressed format,
 * decode and write result to an audio .wav file.
 */

#include "../includes/AudioDecoder.hpp"
#include <sndfile.hh>
//#include <chrono>
//using namespace std::chrono;

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

    // add 28 byte file header (initial_m, format, channels, samplerate, frames, lossless, qBits)
    headerSize = 28;        // bytes
    int paramsSize = 32;    // bits
    try {
        initial_m = boolvec2int(sourceFile->readNbits(paramsSize));
        format = boolvec2int(sourceFile->readNbits(paramsSize));
        channels = boolvec2int(sourceFile->readNbits(paramsSize));
        samplerate = boolvec2int(sourceFile->readNbits(paramsSize));
        frames = boolvec2int(sourceFile->readNbits(paramsSize));
        lossless = boolvec2int(sourceFile->readNbits(paramsSize));
        quantBits = boolvec2int(sourceFile->readNbits(paramsSize));
    } catch( string mess){
        std::cout << mess << std::endl;
        std::exit(0);
    }

}

void AudioDecoder::decode(){
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

    // Golomb decoder
    auto *golomb = new Golomb(initial_m);
    int framesToDecode = 1000; // must be equal to Encoder's m_rate

    // read all data
    vector<bool> data = sourceFile->readNbits((sourceFile->size() - headerSize) * 8);
    unsigned int index = 0;

    // used to compute mean of mapped residuals
    float left_res_sum = 0;
    float right_res_sum = 0;

    int totalframes = 0;
    while(totalframes < frames){
        // to not read more than available
        if (totalframes + framesToDecode > frames){
            framesToDecode = frames - totalframes;
        }

        totalframes += framesToDecode;
        cout << "decoded frames: " << totalframes << "/" << frames << endl;

        //auto start = high_resolution_clock::now();
        vector<int> samples = golomb->decode2(data, &index, framesToDecode * channels);
        //auto stop = high_resolution_clock::now();
        //auto duration = duration_cast<microseconds>(stop - start);
        //cout << "elapsed time: " << duration.count() << "micro seconds" << endl;

        for (int fr = 0; fr < framesToDecode; fr++) {
            leftRes = samples.at(fr*channels + 0);
            rightRes = samples.at(fr*channels + 1);

            if(lossless){
                // use predictor (best for lossless)
                predLeftSample = 3*leftSample_1 - 3*leftSample_2 + leftSample_3;
                predRightSample = 3*rightSample_1 - 3*rightSample_2 + rightSample_3;
                // re calc samples
                leftSample = predLeftSample + leftRes;
                rightSample = predRightSample + rightRes;
            }else{
                // use predictor (best for lossy)
                predLeftSample = leftSample_1;
                predRightSample = rightSample_1;
                // re calc samples with correctly scaled residuals
                leftSample = predLeftSample + (leftRes << quantBits);
                rightSample = predRightSample + (rightRes << quantBits);
            }

            // update
            leftSample_3 = leftSample_2;
            leftSample_2 = leftSample_1;
            leftSample_1 = leftSample;
            rightSample_3 = rightSample_2;
            rightSample_2 = rightSample_1;
            rightSample_1 = rightSample;

            decodedRes.push_back(leftSample);
            decodedRes.push_back(rightSample);

            // update sum used for mean when computing m
            // first map sin residuals to geometric dist used in golomb encoding
            int leftnMapped = 2 * leftRes;
            if (leftRes < 0){ leftnMapped = -leftnMapped - 1; }
            int rightnMapped = 2 * rightRes;
            if (rightRes < 0){ rightnMapped = -rightnMapped - 1; }
            left_res_sum += leftnMapped;
            right_res_sum += rightnMapped;
        }

        // compute m
        // calc mean from last framesToDecode mapped samples
        float left_res_mean = left_res_sum/framesToDecode;
        float right_res_mean = right_res_sum/framesToDecode;
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
    }
}

void AudioDecoder::write(char* filename){
    SndfileHandle destFile  = SndfileHandle(filename, SFM_WRITE, format, channels, samplerate);
    destFile.write(decodedRes.data(), decodedRes.size());
}