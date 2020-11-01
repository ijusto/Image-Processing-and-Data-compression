/*!
 *  Copies a wav file, sample by sample. Both file names should be passed as command line arguments to the program.
 *  It is used the libsndfile library to aid in the handling of the wav files.
 *  @author Inês Justo
 */

#include    <vector>
#include    <iostream>
#include    "AudioReader.cpp"

int main(int argc, char *argv[]) {

    if(argc < 3){
        std::cout << "Usage: " << argv[0] << " [source file path] [destination file path]" << std::endl;
        return 0;
    }

    auto * audioReader = new AudioReader(argv[argc-2]);
    audioReader->copySampleBySample(argv[argc-1]);

    return 0;
}

