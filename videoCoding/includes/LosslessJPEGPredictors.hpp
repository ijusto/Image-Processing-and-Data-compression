#ifndef VIDEOCODING_LOSSLESSJPEGPREDICTORS_HPP
#define VIDEOCODING_LOSSLESSJPEGPREDICTORS_HPP

template <typename T>

//!  LosslessJPEGPredictors
/*!
 *  This class is useful to calculate lossless jpeg predictors.
 *  @author Inês Justo
*/

class LosslessJPEGPredictors{

private:

    T a;

    T b;

    T c;

public:
    //! LosslessJPEGPredictors constructor.
    /*!
     * @param
    */
    explicit LosslessJPEGPredictors(T a, T b, T c);

    T usePredictor1();

    T usePredictor2();

    T usePredictor3();

    T usePredictor4();

    T usePredictor5();

    T usePredictor6();

    T usePredictor7();

    T usePredictorJLS();

    T getA() const;

    T getB() const;

    T getC() const;


};

#endif //VIDEOCODING_LOSSLESSJPEGPREDICTORS_HPP
