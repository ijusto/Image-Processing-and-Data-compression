#ifndef IMAGE_PROCESSING_AND_DATA_COMPRESSION_KMEANSLIB_HPP
#define IMAGE_PROCESSING_AND_DATA_COMPRESSION_KMEANSLIB_HPP

#include    <iostream>

#include    <opencv2/opencv.hpp>
#include    <opencv2/core/core.hpp>
#include    <opencv2/highgui/highgui.hpp>

//! Kmeans interfaces
/*!
 *  @author Agostinho Pires.
 *  This class is useful to used the kmeans method to quantizate the image with clusters
 */

class KM{
public:
    /*!
      Aplly k means method, this method is used to quantify the image, reducing the bit
      per pixel, using clusters and the means this method blurred the image according the
      number of clusters used
      @param Input is the frame of the video on that moment, the image that moment
      @param K is the number if clusters to use
      @warning you must have attention on that the max number of clusters is 255 and the min
      is 2
      @return returns the new image after used the k-method with the image blurred because
      the losing bpp
    */
    cv::Mat K_Means(cv::Mat Input, int K);
};

#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_KMEANSLIB_HPP
