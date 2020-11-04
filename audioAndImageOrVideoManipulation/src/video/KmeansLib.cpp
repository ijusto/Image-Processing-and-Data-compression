/*!
 *  @author Agostinho Pires
 */

#include    "KmeansLib.hpp"

using namespace cv;
using namespace std;


Mat K_Means(Mat Input, int K) {
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

