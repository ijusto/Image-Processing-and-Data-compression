#ifndef IMAGE_PROCESSING_AND_DATA_COMPRESSION_F_H
#define IMAGE_PROCESSING_AND_DATA_COMPRESSION_F_H

#include    <iostream>
#include    <unordered_map>

//!  Entropy Calculator interface.
/*!
 *  @author Inês Justo
*/
class EntropyCalculator {

private:
    std::unordered_map<short, int> *histMap;
    int *histPointer;
    int histSize;
    unsigned int sampleCount;

public:
    //! An Entropy Calculator constructor.
    /*!
     * @param _histMap, histogram map with the sample as key and the sample frequency in the audio file as value.
     * @param _sampleCount, number of samples in the audio signal.
    */
    EntropyCalculator(std::unordered_map<short, int> *_histMap, int _sampleCount);

    //! An Entropy Calculator constructor.
    /*!
     * @param _histPointer, array containing histogram.
     * @param hist_size, size of histogram.
     * @param _sampleCount, number of samples in the audio signal.
    */
    EntropyCalculator(int *_histPointer, int _histSize, int _sampleCount);

    //! Change the parameters passed in the constructor.
    /*!
     * @param _histMap, histogram map with the sample as key and the sample frequency in the audio file as value.
     * @param _sampleCount, number of samples in the audio signal.
    */
    void setParams(std::unordered_map<short, int> *_histMap, int _sampleCount);

    //! Change the parameters passed in the constructor.
    /*!
     * @param _histPointer, array containing histogram.
     * @param hist_size, size of histogram.
     * @param _sampleCount, number of samples in the audio signal.
    */
    void setParams(int *_histPointer, int _histSize, int _sampleCount);

    //! Gets the array containing histogram.
    /*!
     * @return Array containing histogram.
    */
    int *getHistPointer() const;

    //! Gets the size of the array containing histogram.
    /*!
     * @return Size of histogram.
    */
    unsigned int getHistSize() const;

    //! Gets the histogram map.
    /*!
      @return Histogram map with the sample as key and the sample frequency in the audio file as value.
    */
    std::unordered_map<short, int> *getHistMap() const;

    //! Gets the number of samples in the audio signal.
    /*!
      @return Number of samples.
    */
    unsigned int getSampleCount() const;

    //! Calculates and gets the entropy of the audio signal.
    /*!
      @return The entropy.
    */
    double getEntropy();
};

#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_F_H