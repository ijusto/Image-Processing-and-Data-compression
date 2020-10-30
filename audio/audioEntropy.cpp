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
        entropy -= pi * (log(pi)/log(2));
    }
    return entropy;
}

int main(int argc, char *argv[]) {

    SndfileHandle audio = SndfileHandle(argv[argc-1], SFM_READ);

    std::vector<short> buffer(FRAMES_BUFFER_LEN * audio.channels());

    // read audio to calculate Histogram
    std::unordered_map<short, int> hist;
    for(sf_count_t nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN);
        nFrames != 0; nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN)){
        if (hist.find(nFrames) != hist.end()){
            hist[nFrames] += 1;
        } else {
            hist[nFrames] = 1;
        }
    }

    std::vector<int> hist_vector(hist.size());
    for(auto & iter : hist){
        hist_vector.push_back(iter.second);
    }

    int sample_size = hist.size(); //?
    // calculate audio samples entropy
    double entropy = calculateEntropy(&hist, sample_size);
    printf("entropy of the audio sample: %f", entropy);

    // display histogram
    plt::hist(hist_vector);
    plt::title("Histogram of audio samples");
    plt::show();
}

