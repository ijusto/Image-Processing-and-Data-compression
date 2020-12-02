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
    if((argc < 4) || (string(argv[1]) != "encode" && string(argv[1]) != "decode")){
        cout << "usage: " << argv[0] << " <operation> SOURCE DEST" << endl;
        cout << "operations:" << endl;
        cout << "\t encode\t encode SOURCE .wav sound file into DEST compressed file" << endl;
        cout << "\t decode\t decode SOURCE compressed file into DEST .wav sound file" << endl;
        return 0;
    }
    string op = argv[1];
    char* src = argv[2];
    char* dst = argv[3];
    int initial_m = 500;
    bool calcHist = false;

    // init encoder with sound file
    AudioEncoder* encoder = new AudioEncoder(src, initial_m, calcHist);

    if (op == "encode"){

        // encode
        encoder->encode();
        // write compressed file
        cout << "writing..." << endl;
        encoder->write(dst);
    }else if (op == "decode"){
        // init decoder with compressed file
        AudioDecoder* decoder = new AudioDecoder(src);
        // decode
        decoder->decode();
        // write sound file
        cout << "writing..." << endl;
        decoder->write(dst);
    }else if(op == "hist/entropy"){

        // read audio to calculate Histogram of the audio sample (left and right  residuals channels)
        vector<short> leftResidualsChannel = encoder->getLeftResiduals();
        vector<short> rightResidualsChannel = encoder->getRightResiduals();

        unordered_map<short, int> hist_left_residuals_channel = calcHistogram(leftResidualsChannel);
        unordered_map<short, int> hist_right_residuals_channel = calcHistogram(rightResidualsChannel);
        vectorToCsv("./src/audioHistograms/leftResidualsChannel.csv", &leftResidualsChannel);
        vectorToCsv("./src/audio/audioHistograms/rightResidualsChannel.csv", &rightResidualsChannel);

        auto * entropyCalculatorLeftResiduals = new EntropyCalculator(&hist_left_residuals_channel, leftResidualsChannel.size());
        printf("\nentropy of the residuals of left channel: %f", entropyCalculatorLeftResiduals->getEntropy());
        auto * entropyCalculatorRightResiduals = new EntropyCalculator(&hist_right_residuals_channel, rightResidualsChannel.size());
        printf("\nentropy of the residuals of right channel: %f", entropyCalculatorRightResiduals->getEntropy());

        // read audio to calculate Histogram of the audio sample (left and right channels)
        vector<short> leftChannel = encoder->getLeftSamples();
        vector<short> rightChannel = encoder->getRightSamples();

        unordered_map<short, int> hist_left_channel = calcHistogram(leftChannel);
        unordered_map<short, int> hist_right_channel = calcHistogram(rightChannel);

        auto * entropyCalculatorLeft = new EntropyCalculator(&hist_left_residuals_channel, leftResidualsChannel.size());
        printf("\nentropy of the left channel: %f", entropyCalculatorLeft->getEntropy());
        auto * entropyCalculatorRight = new EntropyCalculator(&hist_right_residuals_channel, rightResidualsChannel.size());
        printf("\nentropy of the right channel: %f", entropyCalculatorRight->getEntropy());

    }else{
        printf("\n ERROR : This option doesn't exist!!");
    }

}

