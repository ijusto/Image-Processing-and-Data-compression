//
// Created by irocs on 1/13/21.
//
#include    "../src/VideoDecoder.cpp"

using namespace cv;
using namespace std;


int main(int argc, char *argv[]) {
    char* type;
    VideoDecoder *videoDecoder = new VideoDecoder(argv[1],argv[2], type);
    videoDecoder->decode();

    VideoCapture video(argv[2]);

// check if we succeeded
    if (!video.isOpened()) {
        cerr << "ERROR! Unable to open video file: " << argv[2]<< endl;
        return -1;
    }
// OpenCV buffer
    Mat frame;

// get one frame
    video >> frame;

    while (true) {
        if (!video.read(frame))
            break;

        imshow("RGB", frame);
        double delay = (1 / 25.0) * 1000; // ms
        if (waitKey(delay) >= 0)
            break;
    }
}