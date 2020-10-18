//
// Created by inesjusto on 18/10/20.
//

#include    <cstdio>
#include    <cstring>
#include    <sndfile.hh>

#define        BUFFER_LEN        100000024

static void createDestFile(const SndfileHandle *sourceFile, const char *fname) {

    const int64_t size = sourceFile->frames();
    short buffer[size];
    SndfileHandle destFile  = SndfileHandle(fname, SFM_WRITE, sourceFile->format(), sourceFile->channels(),
            sourceFile->samplerate());

    memset(buffer, 0, sizeof(buffer));

    destFile.write(buffer, size);

    puts("");
    /*
    **	The SndfileHandle object will automatically close the file and
    **	release all allocated memory when the object goes out of scope.
    **	This is the Resource Acquisition Is Initailization idom.
    **	See : http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization
    */
} /* create_file */

static void copyFile(const char *sourcefname, const char *destfname) {

    SndfileHandle sourceFile = SndfileHandle(sourcefname, SFM_READ);

    const int64_t size = sourceFile.frames();
    short buffer[size];

    printf("Opened file '%s'\n", sourcefname);
    printf("    Sample rate : %d\n", sourceFile.samplerate());
    printf("    Channels    : %d\n", sourceFile.channels());

    createDestFile(&sourceFile, destfname);

    sourceFile.read(buffer, size);
    SndfileHandle destFile = SndfileHandle(destfname, SFM_WRITE, sourceFile.format(), sourceFile.channels(),
            sourceFile.samplerate());
    destFile.write(buffer, size);

    puts("");

    /* RAII takes care of destroying SndfileHandle object. */
} /* read_file */

int main(void) {
    const char *destfname = "copysample01.wav";
    const char *sourcefpath = "../wav_files/sample01.wav";

    puts("\nSimple example showing usage of the C++ SndfileHandle object.\n");

    copyFile(sourcefpath, destfname);

    puts("Done.\n");
    return 0;
} /* main */

