/**
 * Audio encoder/decoder
 */

#include <iostream>
#include "AudioEncoder.cpp"
#include "AudioDecoder.cpp"
#include "EntropyCalculator.cpp"
#include  <unordered_map>
#include  <vector>

using namespace std;

std::unordered_map<short, int> calcHistogram(const std::vector<short>& values){
    unordered_map<short, int> histogram;
    for(short value : values) {
        histogram[value] = (histogram.find(value) != histogram.end()) ? histogram[value] + 1 : 1;
    }
    return histogram;
}

void vectorToCsv(const char* fName, std::vector<short>* hitVec){
    ofstream csv;
    csv.open(fName);
    for(auto & value : *hitVec){
        csv << value << "\n";
    }
    csv.close();
}

int main(int argc, char* argv[]) {
    if(argc < 4){
        cout << "usage: " << argv[0] << " <operation> SOURCE DEST [options]" << endl;
        cout << "operations:" << endl;
        cout << "\tencode\tencode SOURCE .wav sound file into DEST compressed file" << endl;
        cout << "\tdecode\tdecode SOURCE compressed file into DEST .wav sound file" << endl;
        cout << "options:" << endl;
        cout << "\t-hist\tcompute histograms and entropy and save to .csv (encode operation only)" << endl;
        cout << "\t-lossy\tuse lossy compression (encode operation only)" << endl;
        return 0;
    }
    string op = argv[1];
    char* src = argv[2];
    char* dst = argv[3];
    int initial_m = 500;
    bool lossless = true;
    int unsigned quantBits = 10;
    bool calcHist = false;

    // parse options
    if(argc > 4){
        for(int i = 4; i < 4+(argc-4); i++){
            if(string(argv[i]) == "-hist"){
                calcHist = true;
            }
            if(string(argv[i]) == "-lossy"){
                lossless = false;
            }
        }
    }

    if (op == "encode"){
        // init encoder with sound file
        AudioEncoder* encoder = new AudioEncoder(src, initial_m, lossless, quantBits, calcHist);
        // encode
        encoder->encode();
        // write compressed file
        cout << "writing..." << endl;
        encoder->write(dst);
        // histograms and entropy
        if(calcHist){
            // read audio to calculate Histogram of the audio sample (left and right  residuals channels)
            vector<short> leftResidualsChannel = encoder->getLeftResiduals();
            vector<short> rightResidualsChannel = encoder->getRightResiduals();
            unordered_map<short, int> hist_left_residuals_channel = calcHistogram(leftResidualsChannel);
            unordered_map<short, int> hist_right_residuals_channel = calcHistogram(rightResidualsChannel);
            vectorToCsv("leftResidualsChannel.csv", &leftResidualsChannel);
            vectorToCsv("rightResidualsChannel.csv", &rightResidualsChannel);

            auto * entropyCalculatorLeftResiduals = new EntropyCalculator(&hist_left_residuals_channel, leftResidualsChannel.size());
            printf("\nentropy of the residuals of left channel: %f bits", entropyCalculatorLeftResiduals->getEntropy());
            auto * entropyCalculatorRightResiduals = new EntropyCalculator(&hist_right_residuals_channel, rightResidualsChannel.size());
            printf("\nentropy of the residuals of right channel: %f bits", entropyCalculatorRightResiduals->getEntropy());

            // read audio to calculate Histogram of the audio sample (left and right channels)
            vector<short> leftChannel = encoder->getLeftSamples();
            vector<short> rightChannel = encoder->getRightSamples();
            unordered_map<short, int> hist_left_channel = calcHistogram(leftChannel);
            unordered_map<short, int> hist_right_channel = calcHistogram(rightChannel);
            vectorToCsv("leftChannel.csv", &leftChannel);
            vectorToCsv("rightChannel.csv", &rightChannel);

            auto * entropyCalculatorLeft = new EntropyCalculator(&hist_left_channel, leftResidualsChannel.size());
            printf("\nentropy of the left channel: %f bits", entropyCalculatorLeft->getEntropy());
            auto * entropyCalculatorRight = new EntropyCalculator(&hist_right_channel, rightResidualsChannel.size());
            printf("\nentropy of the right channel: %f bits\n", entropyCalculatorRight->getEntropy());
        }
    }else if (op == "decode"){
        // init decoder with compressed file
        AudioDecoder* decoder = new AudioDecoder(src);
        // decode
        decoder->decode();
        // write sound file
        cout << "writing..." << endl;
        decoder->write(dst);
    }else{
        printf("\n ERROR : This option doesn't exist!!");
    }

}

