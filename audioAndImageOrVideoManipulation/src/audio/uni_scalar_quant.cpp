/*!
 * Program to reduce the number of bits used to represent each audio sample (uniform scalar quantization).
 * @author Inês Justo
 * @author
 */

#include    <vector>
#include    "AudioReader.cpp"

int main(int argc, char *argv[]) {

    if(argc < 4){
        std::cout << "Usage: " << argv[0] << " [file path] [dest file path] [bits (1 - 15)]" << std::endl;
        return 0;
    }
    int bits = (int)std::strtol(argv[argc - 1], (char **)nullptr, 10);
    auto * audioReader = new AudioReader(argv[argc - 3]);
    audioReader->uniformScalarQuantization(argv[argc - 2], bits);
}