//
// Created by inesjusto on 18/10/20.
//

#include    <cstdio>
#include    <cstring>
#include    <sndfile.hh>

#define        BUFFER_LEN        100000024

static void create_file(const char *sourcefname, const char *fname, int format) {

    SndfileHandle sourceFile;
    SndfileHandle destFile;

    sourceFile = SndfileHandle(sourcefname);
    const int64_t size = sourceFile.frames();
    short buffer[size];

    SndfileHandle file;
    int channels = sourceFile.channels();
    int srate = sourceFile.samplerate();

    printf("Creating file named '%s'\n", fname);

    file = SndfileHandle(fname, SFM_WRITE, sourceFile.format(), channels, srate);

    memset(buffer, 0, sizeof(buffer));

    file.write(buffer, size);

    puts("");
    /*
    **	The SndfileHandle object will automatically close the file and
    **	release all allocated memory when the object goes out of scope.
    **	This is the Resource Acquisition Is Initailization idom.
    **	See : http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization
    */
} /* create_file */

static void copy_file(const char *sourcefname, const char *destfname) {

    SndfileHandle sourceFile;
    SndfileHandle destFile;

    sourceFile = SndfileHandle(sourcefname);
    destFile = SndfileHandle(destfname);
    const int64_t size = sourceFile.frames();
    short buffer[size];
    printf("Opened file '%s'\n", sourcefname);
    printf("    Sample rate : %d\n", sourceFile.samplerate());
    printf("    Channels    : %d\n", sourceFile.channels());

    sourceFile.read(buffer, sourceFile.frames());
    destFile.write(buffer, sourceFile.frames());

    puts("");

    /* RAII takes care of destroying SndfileHandle object. */
} /* read_file */

int main(void) {
    const char *destfname = "copysample01.wav";
    const char *sourcefpath = "../wav_files/sample01.wav";

    puts("\nSimple example showing usage of the C++ SndfileHandle object.\n");

    create_file(sourcefpath, destfname, SF_FORMAT_WAV | SF_FORMAT_PCM_16);

    copy_file(sourcefpath, destfname);

    puts("Done.\n");
    return 0;
} /* main */

