/**
 *  @author Inês Justo
 */

#include    "AudioReader.hpp"

//! AudioReader constructor.
/*!
 * @param sourceFileName, audio file name/path.
*/
AudioReader::AudioReader(char* sourceFileName){
    sourceFile = SndfileHandle(sourceFileName, SFM_READ);
}

//! Copies a wav file, sample by sample
/*!
 * @param destFileName, audio file name/path to copy to.
*/
void AudioReader::copySampleBySample(char* destFileName){

    if (sourceFile.frames() == 0) {
        std::cerr << "Error: File with zero frames." << std::endl;
        exit(EXIT_FAILURE);
    }

    auto srcFileChannels = sourceFile.channels();
    SndfileHandle destFile  = SndfileHandle(destFileName, SFM_WRITE, sourceFile.format(),
                                            srcFileChannels,sourceFile.samplerate());

    std::vector<short> audioSample(srcFileChannels);
    for(sf_count_t nFrames = sourceFile.readf(audioSample.data(), 1);
        nFrames != 0; nFrames = sourceFile.readf(audioSample.data(), 1)) {
        destFile.writef(audioSample.data(), nFrames);
    }
}

//! Reads a wav file, sample by sample, the left and right channel one's to the leftCh and rightCh vectors,
//! respectively and calculates the mono and saves in the vecto mono
void AudioReader::readChannels(){

    auto srcFileChannels = sourceFile.channels();

    if(srcFileChannels != 2){
        std::cerr << "Error: This program is meant to handle stereo audio." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (sourceFile.frames() == 0) {
        std::cerr << "Error: File with zero frames." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::vector<short> samplesFromFrame(srcFileChannels);

    for(auto frame = sourceFile.readf(samplesFromFrame.data(), 1);
        frame != 0; frame = sourceFile.readf(samplesFromFrame.data(), 1)) {
        leftCh.push_back(samplesFromFrame[0]);
        rightCh.push_back(samplesFromFrame[1]);
        mono.push_back((samplesFromFrame[0]+samplesFromFrame[1])/2);
    }
}

//! Reduces the number of bits used to represent each audio sample (uniform scalar quantization).
/*!
 * @param destFileName audio file name/path to save the changed audio to.
 * @param nBits number of bits of reduction.
*/
void AudioReader::uniformScalarQuantization(char* destFileName, int nBits){

    if (sourceFile.frames() == 0) {
        std::cerr << "Error: File with zero frames." << std::endl;
        exit(EXIT_FAILURE);
    }

    auto srcFileChannels = sourceFile.channels();
    SndfileHandle destFile  = SndfileHandle(destFileName, SFM_WRITE, sourceFile.format(),
                                            srcFileChannels,sourceFile.samplerate());

    std::vector<short> samplesFromFrame(srcFileChannels);
    std::vector<short> samplesShifted(srcFileChannels);

    for(auto nFrames = sourceFile.readf(samplesFromFrame.data(), 1);
        nFrames != 0; nFrames = sourceFile.readf(samplesFromFrame.data(), 1)) {

        // for each channel's sample, set first nBits to 0
        for(int i = 0; i < samplesShifted.size(); i++){
            short quantizedSample = (samplesFromFrame.at(i) >> nBits) << nBits;
            samplesShifted.at(i) = quantizedSample;
        }

        destFile.writef(samplesShifted.data(), nFrames);
    }
}

//! Gets a vector of the values of the samples of the left channel.
/*!
  @return Vector of info in the left channel.
*/
std::vector<short> AudioReader::getLeftCh() {
    return leftCh;
}

//! Gets a vector of the values of the samples of the right channel.
/*!
  @return Vector of info in the right channel.
*/
std::vector<short> AudioReader::getRightCh() {
    return rightCh;
}

//! Gets a vector of values, being the average info of the left and right channels.
/*
  @return Vector of the average info of the channels.
*/
std::vector<short> AudioReader::getMono() {
    return mono;
}

