#include    <unordered_map>
#include    "../includes/EntropyCalculator.hpp"

//!  Entropy Calculator class.
/*!
 *
  A more elaborate class description.
*/

//! A constructor.
/*!
  A more elaborate description of the constructor.
*/
EntropyCalculator::EntropyCalculator(std::unordered_map<short, int>* _histMap, int _sampleCount) : histMap(_histMap),
                                                                                    sampleCount(_sampleCount){}

//! A constructor.
/*!
  A more elaborate description of the constructor.
*/
EntropyCalculator::EntropyCalculator(int* _histPointer, int _histSize, int _sampleCount) : histPointer(_histPointer),
                                                                        histSize(_histSize),sampleCount(_sampleCount){}

void EntropyCalculator::setParams(std::unordered_map<short, int>* _histMap, int _sampleCount) {
    histMap = _histMap;
    sampleCount = _sampleCount;
    histSize = 0;
}

void EntropyCalculator::setParams(int* _histPointer, int _histSize, int _sampleCount) {
    histPointer = _histPointer;
    histSize = _histSize;
    sampleCount = _sampleCount;
}

//! ....
/*!
  @return
*/
int* EntropyCalculator::getHistPointer() const {
    return histPointer;
}

//! ....
/*!
  @return
*/
unsigned int EntropyCalculator::getHistSize() const {
    return histSize;
}

//! ....
/*!
  @return Histogram map with the sample as key and the sample frequency in the audio file as value.
*/
std::unordered_map<short, int>* EntropyCalculator::getHistMap() const {
    return histMap;
}

//! ....
/*!
  @return Number of samples.
*/
unsigned int EntropyCalculator::getSampleCount() const {
    return sampleCount;
}

//! ....
/*!
  @return The entropy.
*/
double EntropyCalculator::getEntropy(){
    double h = 0;
    if(histSize == 0){
        for(auto & iter : *histMap){
            double pi = ((double) iter.second) / sampleCount;
            if (pi == 0) continue;
            h -= pi * log2(pi);
        }
    } else {
        double P, I;
        for(int i = 0; i < histSize; i++){
            P = ((double) histPointer[i])/sampleCount;
            if (P == 0) continue;   // to avoid inf values
            I = - log2(P);           // when computing log(P)
            h += P*I;
        }
    }
    return h;
}

