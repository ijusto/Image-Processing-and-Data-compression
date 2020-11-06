#ifndef IMAGE_PROCESSING_AND_DATA_COMPRESSION_AUDIOREADER_H
#define IMAGE_PROCESSING_AND_DATA_COMPRESSION_AUDIOREADER_H

#include    <sndfile.hh>
#include    <vector>
#include    <iostream>
#include    <cmath>

#define     BIT_DEPTH       16
#define		RESOLUTION		pow(2, BIT_DEPTH)

//!  Audio Reader.
/*!
 *  This class is useful for reading, copying and extract info from audio files.
 *  @author Inês Justo
*/
class AudioReader{

private:
    SndfileHandle sourceFile;
    std::vector<short> leftCh;
    std::vector<short> rightCh;
    std::vector<short>  mono;

public:
    //! AudioReader constructor.
    /*!
     * @param sourceFileName audio file name/path.
    */
    explicit AudioReader(char* sourceFileName);

    //! Copies a wav file, sample by sample
    /*!
     * @param destFileName audio file name/path to copy to.
    */
    void copySampleBySample(char* destFileName);

    //! Reads a wav file, sample by sample, the left and right channel one's to the leftCh and rightCh vectors,
    //! respectively and calculates the mono and saves in the vecto mono
    void readChannels();

    //! Reduces the number of bits used to represent each audio sample (uniform scalar quantization).
    /*!
     * @param destFileName audio file name/path to save the changed audio to.
     * @param nBits number of bits of reduction.
    */
    void uniformScalarQuantization(char* destFileName, int nBits);

    //! Gets a vector of the values of the samples of the left channel.
    /*!
      @return Vector of info in the left channel.
    */
    std::vector<short> getLeftCh();

    //! Gets a vector of the values of the samples of the right channel.
    /*!
      @return Vector of info in the right channel.
    */
    std::vector<short> getRightCh();

    //! Gets a vector of values, being the average info of the left and right channels.
    /*
      @return Vector of the average info of the channels.
    */
    std::vector<short> getMono();
};

#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_AUDIOREADER_H
