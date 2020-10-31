#ifndef IMAGE_PROCESSING_AND_DATA_COMPRESSION_ENTROPYCALCULATOR_H
#define IMAGE_PROCESSING_AND_DATA_COMPRESSION_ENTROPYCALCULATOR_H

#include    <iostream>
#include    <unordered_map>

//!  Entropy Calculator class.
/*!
  A more elaborate class description.
*/
class EntropyCalculator {

private:
    std::unordered_map<short, int>*  histMap;
    int* histPointer;
    int histSize;
    unsigned int sampleCount;

public:
    //! A constructor.
    /*!
      A more elaborate description of the constructor.
    */
    EntropyCalculator(std::unordered_map<short, int>* _histMap, int _sampleCount) : histMap(_histMap),
                        sampleCount(_sampleCount){}

    EntropyCalculator(int* _histPointer, int _histSize, int _sampleCount) : histPointer(_histPointer),
                        histSize(_histSize),sampleCount(_sampleCount){}

    void setParams(std::unordered_map<short, int>* _histMap, int _sampleCount) {
        histMap = _histMap;
        sampleCount = _sampleCount;
    }

    void setParams(int* _histPointer, int _histSize, int _sampleCount) {
        histPointer = _histPointer;
        histSize = _histSize;
        sampleCount = _sampleCount;
    }

    int* getHistPointer() const {
        return histPointer;
    }

    unsigned int getHistSize() const {
        return histSize;
    }

    std::unordered_map<short, int>* getHistMap() const {
        return histMap;
    }

    unsigned int getSampleCount() const {
        return sampleCount;
    }

    //! ....
    /*!
      \return The entropy
    */
    double getEntropy(){
        double h = 0;
        if(histSize != 0){
            double h = 0;
            double P, I;
            for(int i = 0; i < histSize; i++){
                P = ((double) histPointer[i])/sampleCount;
                if (P == 0) continue;   // to avoid inf values
                I = - log2(P);           // when computing log(P)
                h += P*I;
            }
        } else {
            for(auto & iter : *histMap){
                double pi = ((double) iter.second) / sampleCount;
                if (pi == 0) continue;
                h -= pi * log2(pi);
            }
        }
        return h;
    }
};

#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_ENTROPYCALCULATOR_H
