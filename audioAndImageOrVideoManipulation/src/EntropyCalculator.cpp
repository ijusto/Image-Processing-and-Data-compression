/**
 *  @author Inês Justo
 */

#include    <unordered_map>
#include    "../includes/EntropyCalculator.hpp"
#include    <cmath>

//! An Entropy Calculator constructor.
/*!
 * @param _histMap histogram map with the sample as key and the sample frequency in the audio file as value.
 * @param _sampleCount number of samples in the audio signal.
*/
EntropyCalculator::EntropyCalculator(std::unordered_map<short, int>* _histMap, int _sampleCount) : histMap(_histMap),
                                                                            sampleCount(_sampleCount), histSize(0){}

//! An Entropy Calculator constructor.
/*!
 * @param _histPointer array containing histogram.
 * @param hist_size size of histogram.
 * @param _sampleCount number of samples in the audio signal.
*/
EntropyCalculator::EntropyCalculator(int* _histPointer, int _histSize, int _sampleCount) : histPointer(_histPointer),
                                                                        histSize(_histSize),sampleCount(_sampleCount){}
//! Change the parameters passed in the constructor.
/*!
 * @param _histMap histogram map with the sample as key and the sample frequency in the audio file as value.
 * @param _sampleCount number of samples in the audio signal.
*/
void EntropyCalculator::setParams(std::unordered_map<short, int>* _histMap, int _sampleCount) {
    histMap = _histMap;
    sampleCount = _sampleCount;
    histSize = 0;
}

//! Change the parameters passed in the constructor.
/*!
 * @param _histPointer array containing histogram.
 * @param hist_size size of histogram.
 * @param _sampleCount number of samples in the audio signal.
*/
void EntropyCalculator::setParams(int* _histPointer, int _histSize, int _sampleCount) {
    histPointer = _histPointer;
    histSize = _histSize;
    sampleCount = _sampleCount;
}

//! Gets the array containing histogram.
/*!
 * @return Array containing histogram.
*/
int* EntropyCalculator::getHistPointer() const {
    return histPointer;
}

//! Gets the size of the array containing histogram.
/*!
 * @return Size of histogram.
*/
unsigned int EntropyCalculator::getHistSize() const {
    return histSize;
}

//! Gets the histogram map.
/*!
  @return Histogram map with the sample as key and the sample frequency in the audio file as value.
*/
std::unordered_map<short, int>* EntropyCalculator::getHistMap() const {
    return histMap;
}

//! Gets the number of samples in the audio signal.
/*!
  @return Number of samples.
*/
unsigned int EntropyCalculator::getSampleCount() const {
    return sampleCount;
}

//! Calculates and gets the entropy of the audio signal.
/*!
  @return The entropy.
*/
double EntropyCalculator::getEntropy(){
    double h = 0;
    if(histSize == 0){
        // for each unique sample value
        for(auto & iter : *histMap){
            // probability, using the frequency of the sample value
            double pi = ((double) iter.second) / sampleCount;
            if (pi == 0) continue;
            h -= pi * log2(pi);
        }
    } else {
        // video part (@author Bruno Pereira) (NOT USED)
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

