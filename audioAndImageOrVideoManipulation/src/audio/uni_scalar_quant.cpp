/*!
 * Program to reduce the number of bits used to represent each audio sample (uniform scalar quantization).
 * @author Inês Justo
 */

#include    <vector>
#include    "AudioReader.cpp"

int main(int argc, char *argv[]) {

    if(argc != 4){
        std::cout << "Usage: " << argv[0] << " [src file path] [dest file path] [bits (1 - 15)]" << std::endl;
        return 0;
    }
    int bits = (int)std::strtol(argv[3], (char **)nullptr, 10);
    auto * audioReader = new AudioReader(argv[1]);
    audioReader->uniformScalarQuantization(argv[2], bits);

    return 0;
}