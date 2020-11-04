/*!
 *  Display a video in bgr and do some quantization with the numbers of clursters that
 *  we want, exe ./program src_path subsampling clusters
 *  @author AgostinhO Pires
 *  30/10/2020
 */

#include    "KmeansLib.hpp"

using namespace cv;
using namespace std;


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
    auto *  km = new KM;
    while (true) {
        if (!video.read(frame))
            break;

        imshow("RGB", frame);

        int Clusters = stoi(argv[argc-1]);
        Mat Clustered_Image = km->K_Means(frame, Clusters);

        imshow("KMEANS", Clustered_Image);

        double delay = (1/fps)*1000; // ms
        if (waitKey(delay) >= 0)
            break;
    }
    return 0;
}
