/*!
 *  ./program file.wav filenoise.wav
 *  test to calculate the value os snr of to audio signal
 *  @author AgostinhO Pires
 *  30/10/2020
 */
#include <iostream>
#include <sndfile.hh>
#include "SNRlib.hpp"

using namespace std;


int main(int argc, char *argv[]) {


    if(argc <3){
        cerr << "Usage : program <wavfile input> <wavfile input> " << endl;
        return 1;
    }

    SNR audio(argv[argc-2],argv[argc-1]);

    cout << "SNR: " << audio.CalculateSNR() << " dB" << endl;
    return 0;
}
