/*!
 * Calculates the histogram of an audio sample and corresponding entropy.
 * Calculates the histogram of the left and right channels, as well as the histogram of the average of the channels (the
 * mono version).
 * For visualizing graphically the histograms, it is used the matplotlib library.
 * @author Inês Justo
 */

#include    "../thirdparty/matplotlib-cpp-master/matplotlibcpp.h"
#include    <vector>
#include    <cstdio>
#include    <unordered_map>
#include    "../EntropyCalculator.cpp"
#include    "AudioReader.cpp"

namespace plt = matplotlibcpp;

void plotHistogram(const std::vector<short>& histVector, const char* title, const char* savePath, size_t bins) {
    plt::hist(histVector, 500);
    plt::title(title);
    plt::xlabel("Samples");
    plt::ylabel("Freq");
    plt::save(savePath);
}

std::unordered_map<short, int> calcHistogram(const std::vector<short>& values){
    std::unordered_map<short, int> histogram;
    for(short value : values) {
        histogram[value] = (histogram.find(value) != histogram.end()) ? histogram[value] + 1 : 1;
    }
    return histogram;
}

int main(int argc, char *argv[]) {

    if(argc < 2){
        std::cout << "Usage: " << argv[0] << " [file path]" << std::endl;
        return 0;
    }

    auto * audioReader = new AudioReader(argv[argc - 1]);
    audioReader->readChannels();
    std::vector<short> leftChannel = audioReader->getLeftCh();
    std::vector<short> rightChannel = audioReader->getRightCh();
    std::vector<short> mono = audioReader->getMono();

    // read audio to calculate Histogram of the audio sample (left and right channels and mono)
    std::unordered_map<short, int> hist_left_channel = calcHistogram(leftChannel);
    std::unordered_map<short, int> hist_right_channel = calcHistogram(rightChannel);
    std::unordered_map<short, int> hist_mono = calcHistogram(mono);

    plotHistogram(leftChannel, "Histogram of Left channel", "../src/audio/audioHistograms/Left_channel.png", leftChannel.size());
    plotHistogram(rightChannel, "Histogram of Right channel", "../src/audio/audioHistograms/Right_channel.png", rightChannel.size());
    plotHistogram(mono, "Histogram of mono", "../src/audio/audioHistograms/Mono.png", mono.size());

    // Calculate the corresponding entropy of the audio sample
    auto * entropyCalculator = new EntropyCalculator(&hist_mono, mono.size());
    printf("\nentropy of the mono channel: %f", entropyCalculator->getEntropy());

    entropyCalculator->setParams(&hist_left_channel, leftChannel.size());
    printf("\nentropy of the left channel: %f", entropyCalculator->getEntropy());

    entropyCalculator->setParams(&hist_right_channel, rightChannel.size());
    printf("\nentropy of the right channel: %f", entropyCalculator->getEntropy());

    return 0;
}