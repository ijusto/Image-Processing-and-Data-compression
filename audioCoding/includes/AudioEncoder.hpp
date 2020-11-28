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
    std::vector<short> leftChRes;
    std::vector<short> rightChRes;

public:

    /**
     * Constructor
     * @param filename
     */
    AudioEncoder(char* filename);

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
