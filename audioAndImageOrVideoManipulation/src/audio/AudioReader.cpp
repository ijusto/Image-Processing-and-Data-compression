#include    "AudioReader.hpp"

AudioReader::AudioReader(char* sourceFileName){
    sourceFile = SndfileHandle(sourceFileName, SFM_READ);
    readSampleBySample();
}

void AudioReader::readSampleBySample(){

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

std::vector<short> AudioReader::getLeftCh() {
    return leftCh;
}

std::vector<short> AudioReader::getRightCh() {
    return rightCh;
}

std::vector<short> AudioReader::getMono() {
    return mono;
}

