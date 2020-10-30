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

    int sample_count = 0;

    // read audio to calculate Histogram of the audio sample (left and right channels)
    std::unordered_map<short, int> hist;
    for(sf_count_t nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN);
        nFrames != 0; nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN)) {

        // read frame's samples
        for(int i = 0; i < nFrames; i++){
            if (hist.find(buffer[i]) != hist.end()) {
                hist[buffer[i]] += 1;
            } else {
                hist[buffer[i]] = 1;
            }
        }

        sample_count += nFrames;
    }
    std::vector<int> hist_vector(hist.size());
    for(auto & iter : hist){
        hist_vector.push_back(iter.second);
    }

    // display histogram
    plt::hist(hist_vector);
    plt::title("Histogram of audio samples");
    plt::show();

    // Calculate the corresponding entropy of the audio sample
    double entropy = calculateEntropy(&hist, sample_count);
    printf("entropy of the audio sample: %f", entropy);

    // Calculate the histogram of the average of the channels (the mono version).
    std::unordered_map<short, int> hist_mono;

    int audioChannels = audio.channels();
    auto audioFrames = audio.frames();
    short audioBuffer[audioFrames*audioChannels];
    audio.read(audioBuffer, audioFrames * audioChannels);
    for(int i = 0; i < audioFrames; i++){
        double iter_mono = 0;
        for(int j = 0; j < audioChannels; j++)
            iter_mono += audioBuffer[i*audioChannels + j];
        double m = iter_mono / audioChannels;
        if (hist_mono.find(m) != hist.end()) {
            hist_mono[m] += 1;
        } else {
            hist_mono[m] = 1;
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
