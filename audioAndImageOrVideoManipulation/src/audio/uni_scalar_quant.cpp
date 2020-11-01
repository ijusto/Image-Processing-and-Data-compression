/*!
 * Program to reduce the number of bits used to represent each audio sample (uniform scalar quantization).
 * @author Inês Justo
 * @author
 */

#include    <vector>
#include    "AudioReader.cpp"

int main(int argc, char *argv[]) {

    if(argc < 2){
        std::cout << "Usage: " << argv[0] << " [file path]" << std::endl;
        return 0;
    }

    auto * audioReader = new AudioReader(argv[argc - 1]);
    audioReader->uniformScalarQuantization();
}