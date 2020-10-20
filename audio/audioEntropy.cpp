/*!
 * Calculates the histogram of an audio sample and corresponding entropy.
 * Calculates the histogram of the left and right channels, as well as the histogram of the average of the channels (the
 * mono version).
 * For visualizing graphically the histograms, it is used the matplotlib library.
 * @author Inês Justo
 * @author
 */

// TODO: Everything xd
#include    "../thirdparty/matplotlib-cpp-master/matplotlibcpp.h"
#include    <sndfile.hh>
#include    <vector>
#include    <cstdio>
#include    <cmath>

namespace plt = matplotlibcpp;

int main(int argc, char *argv[]) {
    SndfileHandle audio = SndfileHandle(argv[argc-1], SFM_READ);
    auto numSamples = audio.frames() * audio.channels();
    short buffer[numSamples];
    audio.read(buffer, numSamples);

    std::vector<short> vector;
    vector.assign(buffer, buffer + numSamples);

    // 16 bits (short) for each sample
    auto numSymbols = pow(2, numSamples);

    std::unordered_map<short, int> hist;
    for(int i = 0; i < numSamples; i++){
        if (hist.find(buffer[i]) != hist.end()){
            hist[buffer[i]] += 1;
        } else {
            hist[buffer[i]] = 1;
        }
    }

    /* TODO: resolve underflow */
    long double entropy = 0;
    for(auto iter = hist.begin(); iter != hist.end(); ++iter){
        unsigned long int pi = iter->second / numSamples;
        printf("%f\n", entropy);
        entropy -= pi * (log(pi)/log(numSymbols));
    }
    printf("entropy of the audio sample: %f", entropy);
    puts("");
    
    plt::hist(vector);
    plt::title("Histogram of audio samples");
    plt::show();
}