#ifndef IMAGE_PROCESSING_AND_DATA_COMPRESSION_F_H
#define IMAGE_PROCESSING_AND_DATA_COMPRESSION_F_H

#include    <iostream>
#include    <unordered_map>

//!  Entropy Calculator interface.
/*!
 *
  A more elaborate class description.
*/
class EntropyCalculator {

private:
    std::unordered_map<short, int> *histMap;
    int *histPointer;
    int histSize;
    unsigned int sampleCount;

public:
    //! A constructor.
    /*!
      A more elaborate description of the constructor.
    */
    EntropyCalculator(std::unordered_map<short, int> *_histMap, int _sampleCount);

    //! A constructor.
    /*!
      A more elaborate description of the constructor.
    */
    EntropyCalculator(int *_histPointer, int _histSize, int _sampleCount);

    void setParams(std::unordered_map<short, int> *_histMap, int _sampleCount);

    void setParams(int *_histPointer, int _histSize, int _sampleCount);

    //! ....
    /*!
      @return
    */
    int *getHistPointer() const;

    //! ....
    /*!
      @return
    */
    unsigned int getHistSize() const;

    //! ....
    /*!
      @return Histogram map with the sample as key and the sample frequency in the audio file as value.
    */
    std::unordered_map<short, int> *getHistMap() const;

    //! ....
    /*!
      @return Number of samples.
    */
    unsigned int getSampleCount() const;

    //! ....
    /*!
      @return The entropy.
    */
    double getEntropy();
};

#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_F_H