/**
 * Read 2 source video or image files: original file and
 * transformed original with added noise. Compute SNR between
 * original and noisy files. Print results in dB for each RGB channels.
 *
 * reference: https://en.wikipedia.org/wiki/Signal-to-noise_ratio
 *
 * @author Bruno Pereira
 */

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " original noise" << endl;
        cout << "original:\n\tpath to original video/image file" << endl;
        cout << "noise:\n\tpath to noisy version video/image file" << endl;
        return 0;
    }

    string path1 = argv[1];
    string path2 = argv[2];

    VideoCapture video1(path1); // original
    VideoCapture video2(path2); // noisy

    // check if we succeeded
    if (!video1.isOpened()) {
        cerr << "ERROR! Unable to open video file: " << path1 << endl;
        return -1;
    }
    if (!video2.isOpened()) {
        cerr << "ERROR! Unable to open video file: " << path2 << endl;
        return -1;
    }

    Mat frame1;
    Mat frame2;

    double R_signal = 0;    // sum of R squared signal
    double G_signal = 0;
    double B_signal = 0;
    double R_error = 0;     // sum of R squared errors
    double G_error = 0;
    double B_error = 0;

    int max_abs_error = 0;
    int num_pixels = 0;

    while (true) {
        if (!video1.read(frame1))
            break;
        if (!video2.read(frame2))
            break;

        for(int i = 0; i < frame1.rows; i++){
            for(int j = 0; j < frame1.cols; j++){
                Vec3b frame1_pixel = frame1.at<Vec3b>(i, j);
                Vec3b frame2_pixel = frame2.at<Vec3b>(i, j);

                // square pixel values and add to accumulator
                B_signal += pow(frame1_pixel[0], 2);
                G_signal += pow(frame1_pixel[1], 2);
                R_signal += pow(frame1_pixel[2], 2);

                // calc absolute errors between original frame1 and noisy frame2 pixels
                int B_pixel_abs_error = (frame1_pixel[0] - frame2_pixel[0]);
                int G_pixel_abs_error = (frame1_pixel[1] - frame2_pixel[1]);
                int R_pixel_abs_error = (frame1_pixel[2] - frame2_pixel[2]);

                // update max absolute pixel error
                if (B_pixel_abs_error > max_abs_error) max_abs_error = B_pixel_abs_error;
                if (G_pixel_abs_error > max_abs_error) max_abs_error = G_pixel_abs_error;
                if (R_pixel_abs_error > max_abs_error) max_abs_error = R_pixel_abs_error;

                // square absolute errors and add to accumulator
                B_error += pow(B_pixel_abs_error, 2);
                G_error += pow(G_pixel_abs_error, 2);
                R_error += pow(R_pixel_abs_error, 2);
            }
        }

        //imshow("test", frame2);
        //double delay = (1.0/30)*1000; // ms
        //if (waitKey(delay) >= 0)
        //    break;

        num_pixels += frame1.rows + frame2.cols;
    }

    /*
     * S - signal, random var
     * N - noise, random var
     * snr = E[S^2]/E[N^2]
     *
     * E[S^2] = R_signal/num_pixels
     * E[N^2] = R_error/num_pixels
     *
     * snr = R_signal/R_error
     */
    double R_snr = R_signal/R_error;
    double G_snr = G_signal/G_error;
    double B_snr = B_signal/B_error;

    cout << "R channel SNR: " << 10*log10(R_snr) << " dB" << endl;
    cout << "G channel SNR: " << 10*log10(G_snr) << " dB" << endl;
    cout << "B channel SNR: " << 10*log10(B_snr) << " dB" << endl;

    cout << "Maximum absolute error between pixels: " << max_abs_error << endl;

    return 0;
}