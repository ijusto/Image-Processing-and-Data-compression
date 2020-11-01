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

    std::vector<short> audioSample(RESOLUTION*srcFileChannels);
    for(sf_count_t nFrames = sourceFile.readf(audioSample.data(), RESOLUTION);
        nFrames != 0; nFrames = sourceFile.readf(audioSample.data(), RESOLUTION)) {
        destFile.writef(audioSample.data(), nFrames);
    }
}

void AudioReader::readChannels(){

    if(sourceFile.channels() != 2){
        std::cerr << "Error: This program is meant to handle stereo audio." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (sourceFile.frames() == 0) {
        std::cerr << "Error: File with zero frames." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::vector<short> samplesFromFrame(sourceFile.channels());

    for(auto frame = sourceFile.readf(samplesFromFrame.data(), 1);
        frame != 0; frame = sourceFile.readf(samplesFromFrame.data(), 1)) {
        leftCh.push_back(samplesFromFrame[0]);
        rightCh.push_back(samplesFromFrame[1]);
        mono.push_back((samplesFromFrame[0]+samplesFromFrame[1])/2);
    }
}

void AudioReader::uniformScalarQuantization(){

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

