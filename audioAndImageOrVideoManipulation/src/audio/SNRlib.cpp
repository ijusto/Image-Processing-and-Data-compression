/*!
 *  @author Agostinho Pires
 */

#include    "SNRlib.hpp"

void SNR::SaveFiles(char *sourceFileName, char *sourceNoisedFileName) {
    sourceFile = SndfileHandle(sourceFileName, SFM_READ);
    sourceFileNoised = SndfileHandle(sourceFileName, SFM_READ);
}


int SNR::CalculateSNR(void){
    if (sourceFile.frames() == 0) {
        std::cerr << "Error: File with zero frames." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (sourceFileNoised.frames() == 0) {
        std::cerr << "Error: File with zero frames." << std::endl;
        exit(EXIT_FAILURE);
    }

    size_t nFrames;
    std::vector<short> samples(FRAMES_BUFFER_SIZE * sourceFile.channels());
    int SNR,SumIn =0;
    while((nFrames = sourceFile.readf(samples.data(), FRAMES_BUFFER_SIZE))){

        SumIn = SumIn + pow(*samples.data(),2);
    }

    nFrames = 0;
    std::vector<short> samplesN(FRAMES_BUFFER_SIZE * sourceFileNoised.channels());
    int SumInN=0;
    while((nFrames = sourceFileNoised.readf(samples.data(), FRAMES_BUFFER_SIZE))){
        SumInN = SumInN + pow(*samples.data()-*samplesN.data(),2);
    }

    SNR =  10*log10(SumIn/SumInN);
    return SNR;
}