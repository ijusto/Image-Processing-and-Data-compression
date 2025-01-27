//
// Created by bruno on 11/29/20.
//

#ifndef AUDIOCODING_AUDIODECODER_H
#define AUDIOCODING_AUDIODECODER_H

#include "../../entropyCoding/includes/BitStream.hpp"
#include <sndfile.hh>
#include <vector>

class AudioDecoder {
private:
    vector<short> decodedRes;
    BitStream *sourceFile;
    unsigned int headerSize;
    int initial_m;
    int format;
    int channels;
    int samplerate;
    int frames;
    bool lossless;
    unsigned int quantBits;

    /**
     * Asserts that vector contains bits stored as least significant bit at the biggest address.
     * @param vec
     * @return
     */
    static int boolvec2int(vector<bool> vec);

public:

    /**
     * Constructor
     * @param filename
     */
    AudioDecoder(char* filename);

    /**
     * Decodes residuals for each channel.
     */
    void decode();

    /**
     * Use SndfileHandle to write decoded residuals to wav file.
     * @param filename
     */
    void write(char* filename);


};


#endif //AUDIOCODING_AUDIODECODER_H
