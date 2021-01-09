#include    "LosslessJPEGPredictors.hpp"

template <typename T>
LosslessJPEGPredictors<T>::LosslessJPEGPredictors(T a, T b, T c){
    this->a = a;
    this->b = b;
    this->c = c;
}

template <typename T>
T LosslessJPEGPredictors<T>::usePredictor1(){
    return this->a;
}

template <typename T>
T LosslessJPEGPredictors<T>::usePredictor2(){
    return this->b;
}

template <typename T>
T LosslessJPEGPredictors<T>::usePredictor3(){
    return this->c;
}

template <typename T>
T LosslessJPEGPredictors<T>::usePredictor4(){
    return this->a + this->b - this->c;
}

template <typename T>
T LosslessJPEGPredictors<T>::usePredictor5(){
    return this->a + (this->b - this->c)/2;
}

template <typename T>
T LosslessJPEGPredictors<T>::usePredictor6(){
    return this->b + (this->a - this->c)/2;
}

template <typename T>
T LosslessJPEGPredictors<T>::usePredictor7(){
    return (this->a + this->b)/2;
}

template <typename T>
T LosslessJPEGPredictors<T>::usePredictorJLS(){
    if (this->c >= std::max(this->a, this->b)){
        return std::min(this->a, this->b);
    } else if (this->c <= std::min(this->a, this->b)){
        return std::max(this->a, this->b);
    } else {
        return this->a + this->b - this->c;
    }
}