//
// Created by bruno on 11/27/20.
//

#ifndef AUDIOCODING_AUDIOENCODER_H
#define AUDIOCODING_AUDIOENCODER_H

#include <sndfile.hh>
#include <vector>

using namespace std;

class AudioEncoder {
private:
    SndfileHandle sourceFile;
    vector<bool> encodedRes;
    int initial_m;
    bool calcHistogram;
    vector<short> leftResiduals;
    vector<short> rightResiduals;
    vector<short> leftSamples;
    vector<short> rightSamples;

    /**
     * Converts integer to bool vector representing bits.
     * @param n
     * @return
     */
    std::vector<bool> int2boolvec(int n);

public:

    /**
     * Constructor
     * @param filename
     * @param m, initial m golomb parameter
     */
    AudioEncoder(char* filename, int m, bool calcHist);

    /**
     * Computes residuals for each channel using predictor.
     */
    void encode();

    /**
     * Use BitStream to write Golomb encoded residuals to file.
     * @param filename
     */
    void write(char* filename);
};


#endif //AUDIOCODING_AUDIOENCODER_H
