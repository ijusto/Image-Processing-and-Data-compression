//
// Created by inesjusto on 30/10/20.
//

#ifndef IMAGE_PROCESSING_AND_DATA_COMPRESSION_ENTROPYCALCULATOR_H
#define IMAGE_PROCESSING_AND_DATA_COMPRESSION_ENTROPYCALCULATOR_H

#include    <iostream>
#include    <unordered_map>

class EntropyCalculator {

private:
    std::unordered_map<short, int>* hist;
    unsigned int sampleCount;

public:
    EntropyCalculator(std::unordered_map<short, int>* _hist, int _sampleCount) : hist(_hist), sampleCount(_sampleCount){}

    EntropyCalculator(){}

    void setParams(std::unordered_map<short, int>* _hist, int _sampleCount) {
        hist = _hist;
        sampleCount = _sampleCount;
    }

    std::unordered_map<short, int>* getHist() const {
        return hist;
    }

    unsigned int getSampleCount() const {
        return sampleCount;
    }

    double getEntropy(){
        double h = 0;
        for(auto & iter : *hist){
            double pi = ((double) iter.second) / sampleCount;
            if (pi == 0) continue;
            h -= pi * log2(pi);
        }
        return h;
    }
};

#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_ENTROPYCALCULATOR_H
