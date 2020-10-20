/*!
 *  Copies a wav file, sample by sample. Both file names should be passed as command line arguments to the program.
 *  It is used the libsndfile library to aid in the handling of the wav files.
 *  @author Inês Justo
 *  @author
 */

#include    <cstdio>
#include    <sndfile.hh>

static void copyFile(const char *sourceFName, const char *destFName) {

    SndfileHandle sourceFile = SndfileHandle(sourceFName, SFM_READ);

    int64_t srcFileFrames = sourceFile.frames();
    int srcFileChannels = sourceFile.channels();
    short buffer[srcFileFrames*srcFileChannels];
    sourceFile.read(buffer, srcFileFrames*srcFileChannels);
    puts("");

    /* create dest wav file */
    SndfileHandle destFile  = SndfileHandle(destFName, SFM_WRITE, sourceFile.format(), srcFileChannels,
            sourceFile.samplerate());
    destFile.write(buffer, srcFileFrames*srcFileChannels);

    puts("");

    /* RAII takes care of destroying SndfileHandle object. */
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

