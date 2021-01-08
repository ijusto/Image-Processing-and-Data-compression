//
// Created by inesjusto on 04/01/21.
//

#ifndef VIDEOCODING_LOSSLESSJPEGPREDICTORS_HPP
#define VIDEOCODING_LOSSLESSJPEGPREDICTORS_HPP

template <typename T>

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


};

#endif //VIDEOCODING_LOSSLESSJPEGPREDICTORS_HPP
