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

    //! 'a' value used to calculate the lossless JPEG predictor.
    T a;

    //! 'b' value used to calculate the lossless JPEG predictor.
    T b;

    //! 'c' value used to calculate the lossless JPEG predictor.
    T c;

public:
    //! LosslessJPEGPredictors constructor.
    /*!
     * @param a 'a' value used to calculate the lossless JPEG predictor.
     * @param b 'b' value used to calculate the lossless JPEG predictor.
     * @param c 'c' value used to calculate the lossless JPEG predictor.
    */
    explicit LosslessJPEGPredictors(T a, T b, T c);

    //! Lossless JPEG Predictor 1.
    /*!
     * @returns the lossless JPEG predictor 1.
    */
    T usePredictor1();

    //! Lossless JPEG Predictor 2.
    /*!
     * @returns the lossless JPEG predictor 2.
    */
    T usePredictor2();

    //! Lossless JPEG Predictor 3.
    /*!
     * @returns the lossless JPEG predictor 3.
    */
    T usePredictor3();

    //! Lossless JPEG Predictor 4.
    /*!
     * @returns the lossless JPEG predictor 4.
    */
    T usePredictor4();

    //! Lossless JPEG Predictor 5.
    /*!
     * @returns the lossless JPEG predictor 5.
    */
    T usePredictor5();

    //! Lossless JPEG Predictor 6.
    /*!
     * @returns the lossless JPEG predictor 6.
    */
    T usePredictor6();

    //! Lossless JPEG Predictor 7.
    /*!
     * @returns the lossless JPEG predictor 7.
    */
    T usePredictor7();

    //! Lossless JPEG Predictor JLS.
    /*!
     * @returns the lossless JPEG predictor JLS.
    */
    T usePredictorJLS();

    //! Gets the 'a' value used to calculate the lossless JPEG predictor.
    /*!
     * @returns 'a' value used to calculate the lossless JPEG predictor.
    */
    T getA() const;

    //! Gets the 'b' value used to calculate the lossless JPEG predictor.
    /*!
     * @returns 'b' value used to calculate the lossless JPEG predictor.
    */
    T getB() const;

    //! Gets the 'c' value used to calculate the lossless JPEG predictor.
    /*!
     * @returns 'c' value used to calculate the lossless JPEG predictor.
    */
    T getC() const;

};

#endif //VIDEOCODING_LOSSLESSJPEGPREDICTORS_HPP
