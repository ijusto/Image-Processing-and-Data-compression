#ifndef IMAGE_PROCESSING_AND_DATA_COMPRESSION_SNRLIB_HPP
#define IMAGE_PROCESSING_AND_DATA_COMPRESSION_SNRLIB_HPP



#include    <sndfile.hh>
#include    <vector>
#include    <cmath>
#include    <iostream>
#include    <stdio.h>

constexpr size_t FRAMES_BUFFER_SIZE = 65536;

//! SNR interfaces
/*!
 *  @author Agostinho Pires.
 *  This class is useful to calculate de SNR between the original signal and the noised signal
 */
class SNR {

private:
    SndfileHandle sourceFile;
    SndfileHandle sourceFileNoised;

public:

    /*!
      Saves on memory the wav file of original signal
      @param sourceFileName is the pointer of the path where the wav file is located
      @param sourceNoisedFileName is the pointer of the path where the wav file is located
      @warning you must put the wav files signal in the right position to calculate the SNR
      @see CalculateSNR(void)
    */
    SNR(const SndfileHandle &_sourceFile, const SndfileHandle &_sourceFileNoised){
        sourceFile = SndfileHandle(_sourceFile, SFM_READ);
        sourceFileNoised = SndfileHandle(_sourceFileNoised, SFM_READ);
    }

    /*!
      Calculates a SNR
      @return SNR in dB
    */
    int CalculateSNR(void){
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


};
#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_SNRLIB_HPP
