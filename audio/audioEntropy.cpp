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
        printf("pi: %f\n", pi);
        entropy -= pi * log2(pi);
    }
    return entropy;
}

int main(int argc, char *argv[]) {

    SndfileHandle audio = SndfileHandle(argv[argc-1], SFM_READ);

    std::vector<short> buffer(FRAMES_BUFFER_LEN * audio.channels());

    // read audio to calculate Histogram of the audio sample (left and right channels)
    std::unordered_map<short, int> hist;
    for(sf_count_t nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN);
        nFrames != 0; nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN)) {
        if (hist.find(nFrames) != hist.end()) {
            hist[nFrames] += 1;
        } else {
            hist[nFrames] = 1;
        }
    }
    std::vector<int> hist_vector(hist.size());
    for(auto & iter : hist){
        hist_vector.push_back(iter.second);
    }

    // display histogram
    plt::hist(hist_vector);
    plt::title("Histogram of audio samples");
    plt::show();

    int sample_size = hist.size(); //?
    // Calculate the corresponding entropy of the audio sample
    double entropy = calculateEntropy(&hist, sample_size);
    printf("entropy of the audio sample: %f", entropy);

    std::vector<short> channels_buffer(FRAMES_BUFFER_LEN*audio.channels());
    // Calculate the histogram of the average of the channels (the mono version).
    std::unordered_map<short, int> hist_mono;
    for(sf_count_t prev = audio.readf(buffer.data(), FRAMES_BUFFER_LEN),
            current = audio.readf(buffer.data(), FRAMES_BUFFER_LEN);
            current != 0; prev = audio.readf(buffer.data(), FRAMES_BUFFER_LEN),
            current = audio.readf(buffer.data(), FRAMES_BUFFER_LEN)) {
        sf_count_t mean = (prev + current)/2;
        if (hist_mono.find(mean) != hist_mono.end()) {
            hist_mono[mean] += 1;
        } else {
            hist_mono[mean] = 1;
        }
    }
    std::vector<int> hist_vector_mono(hist_mono.size());
    for(auto & iter : hist_mono){
        hist_vector_mono.push_back(iter.second);
    }

    // display histogram
    plt::hist(hist_vector_mono);
    plt::title("Histogram of the average of the channels");
    plt::show();

    return 0;
}
