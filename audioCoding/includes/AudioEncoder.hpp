//
// Created by bruno on 11/27/20.
//

#ifndef AUDIOCODING_AUDIOENCODER_H
#define AUDIOCODING_AUDIOENCODER_H

#include    <sndfile.hh>
#include    <vector>

class AudioEncoder {
private:
    SndfileHandle sourceFile;
    std::vector<bool> encodedRes;
    int initial_m;

public:

    /**
     * Constructor
     * @param filename
     * @param m, initial m golomb parameter
     */
    AudioEncoder(char* filename, int m);

    /**
     * Computes residuals for each channel using predictor.
     */
    void encode();

    /**
     * Converts integer to bool vector representing bits.
     * @param n
     * @return
     */
    std::vector<bool> int2boolvec(int n);

    /**
     * Use BitStream to write Golomb encoded residuals to file.
     * @param filename
     */
    void write(char* filename);
};


#endif //AUDIOCODING_AUDIOENCODER_H
