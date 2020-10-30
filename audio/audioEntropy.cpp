/*!
 * Calculates the histogram of an audio sample and corresponding entropy.
 * Calculates the histogram of the left and right channels, as well as the histogram of the average of the channels (the
 * mono version).
 * For visualizing graphically the histograms, it is used the matplotlib library.
 * @author Inês Justo
 * @author
 */

#include    "../thirdparty/matplotlib-cpp-master/matplotlibcpp.h"
#include    <sndfile.hh>
#include    <vector>
#include    <cstdio>
#include    <cmath>

namespace plt = matplotlibcpp;

#define		FRAMES_BUFFER_LEN		65536

double calculateEntropy(std::unordered_map<short, int>* hist, int sample_count){
    double entropy = 0;
    for(auto & iter : *hist){
        double pi = ((double) iter.second) / sample_count;
        if (pi == 0) continue;
        entropy -= pi * log2(pi);
    }
    return entropy;
}

int main(int argc, char *argv[]) {

    SndfileHandle audio = SndfileHandle(argv[argc-1], SFM_READ);

    std::vector<short> buffer(FRAMES_BUFFER_LEN * audio.channels());

    int sample_count = 0;

    // read audio to calculate Histogram of the audio sample
    std::unordered_map<short, int> hist;
    for(sf_count_t nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN);
        nFrames != 0; nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN)) {
        if (hist.find(nFrames) != hist.end()) {
            hist[nFrames] += 1;
        } else {
            hist[nFrames] = 1;
        }

        sample_count += FRAMES_BUFFER_LEN;
    }
    std::vector<int> hist_vector(hist.size());
    for(auto & iter : hist){
        hist_vector.push_back(iter.second);
    }

    // Calculate the corresponding entropy of the audio sample
    double entropy = calculateEntropy(&hist, sample_count);
    printf("entropy of the audio sample: %f", entropy);

    // Calculate the histogram of the left and right channels

    // Calculate the histogram of the average of the channels (the mono version).

    // display histogram
    plt::hist(hist_vector);
    plt::title("Histogram of audio samples");
    plt::show();

    return 0;
}
