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
using namespace cv;
using namespace std;

class KM{
private:
    cv::Mat Input;
    int K;
public:
    KM(const cv::Mat &_input, int _k): Input(_input), K(_k) {}

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
    cv::Mat K_Means(void){
        Mat samples(Input.rows * Input.cols, Input.channels(), CV_32F);
        for (int y = 0; y < Input.rows; y++)
            for (int x = 0; x < Input.cols; x++)
                for (int z = 0; z < Input.channels(); z++)
                    if (Input.channels() == 3) {
                        samples.at<float>(y + x * Input.rows, z) = Input.at<Vec3b>(y, x)[z];
                    }
                    else {
                        samples.at<float>(y + x * Input.rows, z) = Input.at<uchar>(y, x);
                    }

        Mat labels;
        int attempts = 5;
        Mat centers;
        kmeans(samples, K, labels, TermCriteria(1 | 2, 10000, 0.0001), attempts, KMEANS_PP_CENTERS, centers);


        Mat new_image(Input.size(), Input.type());
        for (int y = 0; y < Input.rows; y++)
            for (int x = 0; x < Input.cols; x++)
            {
                int cluster_idx = labels.at<int>(y + x * Input.rows, 0);
                if (Input.channels()==3) {
                    for (int i = 0; i < Input.channels(); i++) {
                        new_image.at<Vec3b>(y, x)[i] = centers.at<float>(cluster_idx, i);
                    }
                }
                else {
                    new_image.at<uchar>(y, x) = centers.at<float>(cluster_idx, 0);
                }
            }
        return new_image;
    }
};

#endif //IMAGE_PROCESSING_AND_DATA_COMPRESSION_KMEANSLIB_HPP
