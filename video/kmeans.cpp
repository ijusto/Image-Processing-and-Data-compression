/*!
 *  Display a video in bgr and do some quantization with the numbers of clursters that
 *  we want, exe ./program src_path subsampling clusters
 *  @author AgostinhO Pires
 *  30/10/2020
 */

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <fstream>

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

int main(int argc, char *argv[]) {
    // args
    if (argc < 3){
        cout << "Usage: " << argv[0] << " src_path cluster" << endl;
        cout << "src_path:\n\tpath to video/image file" << endl;
        cout << "cluster:\n\tcluster size" << endl;
        return 0;
    }
    string src_path = argv[1];
    double fps = 25.0;

    // open video file
    VideoCapture video(src_path);

    // check if we succeeded
    if (!video.isOpened()) {
        cerr << "ERROR! Unable to open video file: " << src_path << endl;
        return -1;
    }

    // OpenCV buffer
    Mat frame;

    while (true) {
        if (!video.read(frame))
            break;

        imshow("RGB", frame);

        int Clusters = stoi(argv[argc-1]);
        Mat Clustered_Image = K_Means(frame, Clusters);

        imshow("KMEANS", Clustered_Image);

        double delay = (1/fps)*1000; // ms
        if (waitKey(delay) >= 0)
            break;
    }
    return 0;
}
