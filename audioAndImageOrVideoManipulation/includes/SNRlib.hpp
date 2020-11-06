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
    double max_abs_error = 0;

public:

    double getMax_abs_error(){
        return max_abs_error;
    }

    /*!
      Saves on memory the wav file of original signal
      @param sourceFileName is the pointer of the path where the wav file is located
      @param sourceNoisedFileName is the pointer of the path where the wav file is located
      @warning you must put the wav files signal in the right position to calculate the SNR
      @see CalculateSNR(void)
    */
    void SaveFiles(char *sourceFileName, char *sourceNoisedFileName) {
        sourceFile = SndfileHandle(sourceFileName, SFM_READ);
        sourceFileNoised = SndfileHandle(sourceNoisedFileName, SFM_READ);
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

        std::vector<short> samples(sourceFile.channels());
        std::vector<short> samplesN(sourceFileNoised.channels());

        double SNR;
        double SumIn = 0;
        double SumInN = 0;

        while((sourceFile.readf(samples.data(), 1)) && (sourceFileNoised.readf(samplesN.data(), 1))){
            for(int i = 0; i < sourceFile.channels(); i++){
                SumIn = SumIn + pow(samples.at(i),2);

                double abs_error = samples.at(i) - samplesN.at(i);
                if (abs_error > max_abs_error) max_abs_error = abs_error;

                SumInN = SumInN + pow(abs_error, 2);
            }
        }

        SNR =  10*log10(SumIn/SumInN);
        return SNR;
    }

};
#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_SNRLIB_HPP
