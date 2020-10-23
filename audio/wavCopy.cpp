/*!
 *  Copies a wav file, sample by sample. Both file names should be passed as command line arguments to the program.
 *  It is used the libsndfile library to aid in the handling of the wav files.
 *  @author Inês Justo
 *  @author
 */

#include    <cstdio>
#include    <sndfile.hh>
#include    <vector>

#define		FRAMES_BUFFER_LEN		65536

static void copyFile(const char *sourceFName, const char *destFName) {

    SndfileHandle sourceFile = SndfileHandle(sourceFName, SFM_READ);
    int srcFileChannels = sourceFile.channels();

    SndfileHandle destFile  = SndfileHandle(destFName, SFM_WRITE, sourceFile.format(), srcFileChannels,
                                            sourceFile.samplerate());

    std::vector<short> buffer(FRAMES_BUFFER_LEN * srcFileChannels);
    for(sf_count_t nFrames = sourceFile.readf(buffer.data(), FRAMES_BUFFER_LEN); nFrames != 0;
            nFrames = sourceFile.readf(buffer.data(), FRAMES_BUFFER_LEN)){
        destFile.writef(buffer.data(), nFrames);
    }
}

int main(int argc, char *argv[]) {
    if(argc < 3){
        puts("Not enough program arguments.\nArguments: [source file path] [destination file path]");
    } else {
        const char *sourceFPath = argv[argc-2];  // "../wav_files/sample01.wav"
        const char *destFName = argv[argc-1];  // "copysample01.wav";

        puts("\nSimple example showing usage of the C++ SndfileHandle object.\n");

        copyFile(sourceFPath, destFName);

        puts("Done.\n");
    }
    return 0;
}

