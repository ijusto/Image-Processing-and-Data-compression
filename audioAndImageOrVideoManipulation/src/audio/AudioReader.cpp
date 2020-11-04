/*!
 *  @author Inês Justo
 */

#include    "AudioReader.hpp"

AudioReader::AudioReader(char* sourceFileName){
    sourceFile = SndfileHandle(sourceFileName, SFM_READ);
}

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

std::vector<short> AudioReader::getLeftCh() {
    return leftCh;
}

std::vector<short> AudioReader::getRightCh() {
    return rightCh;
}

std::vector<short> AudioReader::getMono() {
    return mono;
}

