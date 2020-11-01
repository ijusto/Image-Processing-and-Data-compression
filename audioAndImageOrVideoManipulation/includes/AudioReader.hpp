#ifndef IMAGE_PROCESSING_AND_DATA_COMPRESSION_AUDIOREADER_H
#define IMAGE_PROCESSING_AND_DATA_COMPRESSION_AUDIOREADER_H

#include    <sndfile.hh>
#include    <vector>
#include    <iostream>

class AudioReader{

private:
    SndfileHandle sourceFile;
    std::vector<short> leftCh;
    std::vector<short> rightCh;
    std::vector<short> mono;

public:
    explicit AudioReader(char* sourceFileName);

    void readSampleBySample();

    std::vector<short> getLeftCh();

    std::vector<short> getRightCh();

    std::vector<short> getMono();
};

#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_AUDIOREADER_H
