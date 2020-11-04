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
    void SaveFiles(char* sourceFileName,char* sourceNoisedFileName);


    /*!
      Calculates a SNR
      @return SNR in dB
    */
    int CalculateSNR(void);


};
#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_SNRLIB_HPP
