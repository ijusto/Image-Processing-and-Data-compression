#ifndef IMAGE_PROCESSING_AND_DATA_COMPRESSION_AUDIOREADER_H
#define IMAGE_PROCESSING_AND_DATA_COMPRESSION_AUDIOREADER_H

#include    <sndfile.hh>
#include    <vector>
#include    <iostream>
#include    <cmath>

#define     BIT_DEPTH       16
#define		RESOLUTION		pow(2, BIT_DEPTH)

//!  Entropy Calculator interface.
/*!
 *  @author Inês Justo
 *  This class is useful for reading, copying and extract info from audio files.
*/
class AudioReader{

private:
    SndfileHandle sourceFile;
    std::vector<short> leftCh;
    std::vector<short> rightCh;
    std::vector<short> mono;

public:
    //! A constructor.
    /*!
      A more elaborate description of the constructor.
    */
    explicit AudioReader(char* sourceFileName);

    void readChannels();

    void copySampleBySample(char* destFileName);

    void uniformScalarQuantization(char* destFileName);

    //! ....
    /*!
      @return Vector of info in the left channel.
    */
    std::vector<short> getLeftCh();

    //! ....
    /*!
      @return Vector of info in the right channel.
    */
    std::vector<short> getRightCh();

    //! ....
    /*!
      @return Vector of the average info of the channels.
    */
    std::vector<short> getMono();
};

#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_AUDIOREADER_H
