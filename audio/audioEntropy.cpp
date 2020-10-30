/*!
 * Calculates the histogram of an audio sample and corresponding entropy.
 * Calculates the histogram of the left and right channels, as well as the histogram of the average of the channels (the
 * mono version).
 * For visualizing graphically the histograms, it is used the matplotlib library.
 * @author Inês Justo
 */

#include    "../thirdparty/matplotlib-cpp-master/matplotlibcpp.h"
#include    <sndfile.hh>
#include    <vector>
#include    <cstdio>
#include    <cmath>
#include    "../EntropyCalculator.h"

namespace plt = matplotlibcpp;

#define		FRAMES_BUFFER_LEN		65536

int main(int argc, char *argv[]) {

    if(argc < 2){
        std::cout << "Usage: " << argv[0] << " [file path]" << std::endl;
        return 0;
    }

    SndfileHandle audio = SndfileHandle(argv[argc-1], SFM_READ);

    std::vector<short> buffer(FRAMES_BUFFER_LEN * audio.channels());

    int left_channel_sample_count = 0;
    int right_channel_sample_count = 0;
    int mono_sample_count = 0;

    // read audio to calculate Histogram of the audio sample (left and right channels and mono)
    std::unordered_map<short, int> hist_left_channel, hist_right_channel, hist_mono;
    for(sf_count_t nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN);
        nFrames != 0; nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN)) {

        // read frame's samples
        for(int i = 0, j = i + 1; i < nFrames - 1; i++, j++){
            hist_left_channel[buffer[i]] = (hist_left_channel.find(buffer[i]) != hist_left_channel.end()) ? hist_left_channel[buffer[i]] + 1 : 1;
            left_channel_sample_count += 1;
            hist_right_channel[buffer[j]] = (hist_right_channel.find(buffer[j]) != hist_right_channel.end()) ? hist_right_channel[buffer[j]] + 1 : 1;
            right_channel_sample_count += 1;
            auto mean_channels = (hist_left_channel[buffer[i]] * hist_right_channel[buffer[j]])/2;
            hist_mono[mean_channels] = (hist_mono.find(mean_channels) != hist_mono.end()) ? hist_mono[mean_channels] + 1 : 1;
            mono_sample_count +=1;
        }
    }

    std::vector<int> hist_left_channel_vector(hist_left_channel.size());
    for(auto & iter : hist_left_channel){
        hist_left_channel_vector.push_back(iter.second);
    }

    // display histogram
    plt::hist(hist_left_channel_vector);
    plt::title("Histogram of Left channel");
    plt::xlabel("Samples");
    plt::ylabel("Freq");
    plt::show();


    std::vector<int> hist_right_channel_vector(hist_right_channel.size());
    for(auto & iter : hist_right_channel){
        hist_right_channel_vector.push_back(iter.second);
    }

    // display histogram
    plt::hist(hist_right_channel_vector);
    plt::title("Histogram of Right channel");
    plt::xlabel("Samples");
    plt::ylabel("Freq");
    plt::show();


    std::vector<int> hist_mono_vector(hist_mono.size());
    for(auto & iter : hist_mono){
        hist_mono_vector.push_back(iter.second);
    }

    // display histogram
    plt::hist(hist_mono_vector);
    plt::title("Histogram of mono");
    plt::xlabel("Samples");
    plt::ylabel("Freq");
    plt::show();

    // Calculate the corresponding entropy of the audio sample
    auto * entropyCalculator = new EntropyCalculator(&hist_mono, mono_sample_count);
    double entropy = entropyCalculator->getEntropy();
    printf("\nentropy of the mono channel: %f", entropy);

    entropyCalculator->setParams(&hist_left_channel, left_channel_sample_count);
    entropy = entropyCalculator->getEntropy();
    printf("\nentropy of the left channel: %f", entropy);

    entropyCalculator->setParams(&hist_right_channel, right_channel_sample_count);
    entropy = entropyCalculator->getEntropy();
    printf("\nentropy of the right channel: %f", entropy);

    return 0;
}
