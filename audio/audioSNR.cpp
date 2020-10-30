/*!
 *  Display a video in bgr and do some quantization with the numbers of clursters that
 *  we want, exe ./program src_path subsampling clusters
 *  @author AgostinhO Pires
 *  30/10/2020
 */
#include <iostream>
#include <sndfile.hh>
#include <vector>
#include <cmath>
#include <stdio.h>

using namespace std;
constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

int main(int argc, char *argv[]) {


    if(argc <3){
        cerr << "Usage : program <wavfile input> <wavfile input> " << endl;
        return 1;
    }
    //this will put on the class sndfile the wave file
    SndfileHandle sndfileIn {argv[argc-1]};
    SndfileHandle sndfileInN {argv[argc-2]};

    if(sndfileIn.error()){
        cerr << "ERRO : invalid input file, need to be a wave file " << endl;
        return 1;
    }
    if((sndfileIn.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: file is not in WAV format" << endl;
        return 1;
    }

    if((sndfileIn.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: file is not in PCM_16 format" << endl;
        return 1;
    }

    if((sndfileInN.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: file is not in WAV format" << endl;
        return 1;
    }

    if((sndfileInN.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: file is not in PCM_16 format" << endl;
        return 1;
    }



    size_t nFrames;
    vector<short> samples(FRAMES_BUFFER_SIZE * sndfileIn.channels());
    short int SNR,SumIn =0;
    while((nFrames = sndfileIn.readf(samples.data(), FRAMES_BUFFER_SIZE))){

        SumIn = SumIn + pow(*samples.data(),2);
    }

    nFrames = 0;
    vector<short> samplesN(FRAMES_BUFFER_SIZE * sndfileInN.channels());
    short int SumInN=0;
    while((nFrames = sndfileInN.readf(samples.data(), FRAMES_BUFFER_SIZE))){
        SumInN = SumInN + pow(*samples.data()-*samplesN.data(),2);
    }

    SNR = SumIn/SumInN;

    cout << "SNR: " << 10*log10(SNR) << " dB" << endl;
    return 0;
}
