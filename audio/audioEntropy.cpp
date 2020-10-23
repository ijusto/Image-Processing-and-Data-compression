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

int main(int argc, char *argv[]) {

    SndfileHandle audio = SndfileHandle(argv[argc-1], SFM_READ);

    std::vector<short> buffer(FRAMES_BUFFER_LEN * audio.channels());

    std::unordered_map<short, int> hist;
    for(sf_count_t nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN); nFrames != 0;
        nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN)){
        if (hist.find(nFrames) != hist.end()){
            hist[nFrames] += 1;
        } else {
            hist[nFrames] = 1;
        }
    }

    /* TODO: resolve underflow */
    long double entropy = 0;
    for(auto iter = hist.begin(); iter != hist.end(); ++iter){
        unsigned long int pi = iter->second / audio.channels();
        printf("%f\n", entropy);
        entropy -= pi * (log(pi)/log(FRAMES_BUFFER_LEN));
    }
    printf("entropy of the audio sample: %f", entropy);
    puts("");


    //plt::hist(vector);
    //plt::title("Histogram of audio samples");
    //plt::show();
}