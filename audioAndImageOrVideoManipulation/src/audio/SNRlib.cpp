/*!
 *  @author Agostinho Pires
 */

#include    "SNRlib.hpp"

void SNR::SaveFiles(char *sourceFileName, char *sourceNoisedFileName) {
    sourceFile = SndfileHandle(sourceFileName, SFM_READ);
    sourceFileNoised = SndfileHandle(sourceNoisedFileName, SFM_READ);
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

    std::vector<short> samples(sourceFile.channels());
    std::vector<short> samplesN(FRAMES_BUFFER_SIZE * sourceFileNoised.channels());

    int SNR,SumIn =0;
    int SumInN=0;

    while((sourceFile.readf(samples.data(), 1)) && (sourceFileNoised.readf(samplesN.data(), 1))){
        for(int i = 0; i < samples.size(); i++){
            SumIn = SumIn + pow(samples.at(i),2);
            SumInN = SumInN + pow(samples.at(i)-samplesN.at(i), 2);
        }
    }

    SNR =  10*log10(SumIn/SumInN);
    return SNR;
}