/*!
 *  Copies a wav file, sample by sample. Both file names should be passed as command line arguments to the program.
 *  It is used the libsndfile library to aid in the handling of the wav files.
 *  @author Inês Justo
 */

#include    <sndfile.hh>
#include    <vector>
#include    <iostream>
#include    <cmath>
#include    "AudioReader.cpp"

#define     BIT_DEPTH       16
#define		RESOLUTION		pow(2, BIT_DEPTH)

int main(int argc, char *argv[]) {

    if(argc < 3){
        std::cout << "Usage: " << argv[0] << " [source file path] [destination file path]" << std::endl;
        return 0;
    }
    
    SndfileHandle sourceFile = SndfileHandle(argv[argc-2], SFM_READ);
    int srcFileChannels = sourceFile.channels();

    SndfileHandle destFile  = SndfileHandle(argv[argc-1], SFM_WRITE, sourceFile.format(), srcFileChannels,
                                                sourceFile.samplerate());
    
    /* The file read functions fill the array pointed to by ptr with the requested number of frames.
     * A frame is just a block of samples, one for each channel. Care must be taken to ensure that there is enough space
     * in the array pointed to by ptr, to take (frames * channels) number of items (shorts, ints, floats or doubles).
     * Unless the end of the file was reached during the read, the return value should equal the number of objects
     * requested. Attempts to read beyond the end of the file will not result in an error but will cause the read
     * functions to return less than the number of objects requested or 0 if already at the end of the file.
     */
    std::vector<short> buffer(RESOLUTION*srcFileChannels);
    for(sf_count_t nFrames = sourceFile.readf(buffer.data(), RESOLUTION);
            nFrames != 0; nFrames = sourceFile.readf(buffer.data(), RESOLUTION)) {
        destFile.writef(buffer.data(), nFrames);
    }

    std::vector<short> samplesFromFrame(sourceFile.channels());
    std::vector<std::vector<short>> _channels(sourceFile.frames());

    if (sourceFile.frames() == 0) {
        std::cerr << "Error: File with zero frames." << std::endl;
        exit(EXIT_FAILURE);
    }

    for(sf_count_t frame = sourceFile.readf(samplesFromFrame.data(), 1);
        frame != 0; frame = sourceFile.readf(samplesFromFrame.data(), 1)) {
        _channels[frame] = samplesFromFrame;
    }


    return 0;
}

