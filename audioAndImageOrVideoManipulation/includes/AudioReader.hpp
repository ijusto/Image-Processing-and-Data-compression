#ifndef IMAGE_PROCESSING_AND_DATA_COMPRESSION_AUDIOREADER_H
#define IMAGE_PROCESSING_AND_DATA_COMPRESSION_AUDIOREADER_H

#include    <sndfile.hh>
#include    <vector>
#include    <iostream>

#define     BIT_DEPTH       16
#define		RESOLUTION		pow(2, BIT_DEPTH)

class AudioReader{

private:
    SndfileHandle sourceFile;
    std::vector<short> leftCh;
    std::vector<short> rightCh;
    std::vector<short> mono;

public:
    explicit AudioReader(char* sourceFileName);

    void readChannels();

    void copySampleBySample(char* destFileName);

    std::vector<short> getLeftCh();

    std::vector<short> getRightCh();

    std::vector<short> getMono();
};

#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_AUDIOREADER_H
