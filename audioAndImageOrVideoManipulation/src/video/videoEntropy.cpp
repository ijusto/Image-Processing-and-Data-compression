/**
 * Read source video or image file, build histograms for each RGB channel
 * and grayscale, write histograms to .csv files and use histograms to
 * compute entropy.
 *
 * @author Bruno Pereira
 */


#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

/**
 * Write histogram to a .csv file
 *
 * @param filename, name of .csv file to write
 * @param hist, array containing histogram
 * @param hist_size, size of histogram
 */
void hist2csv(string filename, int* hist, int hist_size){
    ofstream outfile;
    outfile.open(filename + ".csv", ios::out | ios::trunc );
    for(int i = 0; i < hist_size; i++)
        outfile << i << "," << hist[i] << endl;
}

/**
 * Calculate entropy using histogram
 *
 * @param hist, array containing histogram
 * @param hist_size, size of histogram
 * @param sample_count, total number of occurrences in histogram
 * @return entropy, result calculated entropy
 */
double calcEntropy(int* hist, int hist_size, int sample_count){
    double H = 0;
    double P, I;
    for(int i = 0; i < hist_size; i++){
        P = ((double) hist[i])/sample_count;
        if (P == 0) continue;   // to avoid inf values
        I = - log2(P);           // when computing log(P)
        H += P*I;
    }
    return H;
}

int main(int argc, char *argv[]) {
    if(argc < 2){
        cout << "Usage: " << argv[0] << " src_path" << endl;
        cout << "src_path:\n\tpath to video/image file" << endl;
        return 0;
    }
    string src_path = argv[1];

    VideoCapture cap(src_path);

    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open video file\n";
        return -1;
    }

    const int hist_size = 256;
    // histograms
    int B_hist[hist_size] = {0};    // init with zeros
    int G_hist[hist_size] = {0};
    int R_hist[hist_size] = {0};
    int Gr_hist[hist_size] = {0};   // grayscale
    int sample_count = 0;           // number of pixels read

    cout << "Reading file and creating histograms..." << endl;

    Mat frame;
    while (true){
        if (!cap.read(frame))
            break;

        // update histograms
        for(int i = 0; i < frame.rows; i++){
            for(int j = 0; j < frame.cols; j++){
                Vec3b pixel = frame.at<Vec3b>(i,j);
                B_hist[pixel[0]]++;
                G_hist[pixel[1]]++;
                R_hist[pixel[2]]++;
                int grayscale_pixel = (pixel[0] + pixel[1] + pixel[2])/3;
                Gr_hist[grayscale_pixel]++;
            }
        }

        sample_count += frame.rows * frame.cols;
    }

    // write histograms to .csv files
    hist2csv("B", B_hist, hist_size);
    hist2csv("G", G_hist, hist_size);
    hist2csv("R", R_hist, hist_size);
    hist2csv("Gr", Gr_hist, hist_size);

    cout << "Calculating entropy..." << endl;

    // calc entropy
    double H_B = calcEntropy(B_hist, hist_size, sample_count);
    double H_G = calcEntropy(G_hist, hist_size, sample_count);
    double H_R = calcEntropy(R_hist, hist_size, sample_count);
    double H_Gr = calcEntropy(Gr_hist, hist_size, sample_count);

    cout << "R channel entropy = " << H_R << " bits" << endl;
    cout << "G channel entropy = " << H_G << " bits" << endl;
    cout << "B channel entropy = " << H_B << " bits" << endl;
    cout << "Grayscale version entropy = " << H_Gr << " bits" << endl;

    return 0;
}
