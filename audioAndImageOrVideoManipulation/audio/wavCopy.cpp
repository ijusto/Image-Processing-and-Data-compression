/*!
 *  Copies a wav file, sample by sample. Both file names should be passed as command line arguments to the program.
 *  It is used the libsndfile library to aid in the handling of the wav files.
 *  @author Inês Justo
 */

#include    <sndfile.hh>
#include    <vector>
#include    <iostream>

#define		FRAMES_BUFFER_LEN		65536

int main(int argc, char *argv[]) {

    if(argc < 3){
        std::cout << "Usage: " << argv[0] << " [source file path] [destination file path]" << std::endl;
        return 0;
    }

    SndfileHandle sourceFile = SndfileHandle(argv[argc-2], SFM_READ);
    int srcFileChannels = sourceFile.channels();

    SndfileHandle destFile  = SndfileHandle(argv[argc-1], SFM_WRITE, sourceFile.format(), srcFileChannels,
                                                sourceFile.samplerate());

    std::vector<short> buffer(FRAMES_BUFFER_LEN * srcFileChannels);
    for(sf_count_t nFrames = sourceFile.readf(buffer.data(), FRAMES_BUFFER_LEN);
            nFrames != 0; nFrames = sourceFile.readf(buffer.data(), FRAMES_BUFFER_LEN)){
        destFile.writef(buffer.data(), nFrames);
    }

    return 0;
}

