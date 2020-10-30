//
// Created by bruno on 20/10/20.
//

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat copyFrame(const Mat& frame){
    Mat copy(frame.rows, frame.cols, CV_8UC3);

    for(int i = 0; i < frame.rows; i++){
        for(int j = 0; j < frame.cols; j++){
            copy.at<Vec3b>(i,j) = frame.at<Vec3b>(i,j);
        }
    }

    return copy;
}

int main(int argc, char *argv[]) {
    if(argc < 3){
        cout << "Usage: " << argv[0] << " src_path dst_path" << endl;
        return 0;
    }

    string src_path = argv[1];  // "../video/akiyo_qcif.y4m";
    string dst_path = argv[2];  // "../video/copy.y4m";

    Mat src, dst;

    VideoCapture cap(src_path);

    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open video file\n";
        return -1;
    }

    // get one frame to know type
    cap >> src;

    // check if we succeeded
    if (src.empty()) {
        cerr << "ERROR! blank frame grabbed\n";
        return -1;
    }

    bool isColor = (src.type() == CV_8UC3);

    VideoWriter writer;

    int codec = VideoWriter::fourcc('M', 'J', 'P', 'G');
    double fps = 25.0;

    writer.open(dst_path, codec, fps, src.size(), isColor);

    // check if we succeeded
    if (!writer.isOpened()) {
        cerr << "Could not open the output video file for write\n";
        return -1;
    }

    cout << "Copying video file: " << src_path << "... "<< endl;

    while (true){
        if (!cap.read(src)) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        dst = copyFrame(src);
        writer.write(dst);

        //imshow("Live", dst);
        //if (waitKey(30) >= 0)
        //    break;
    }

    cout << "Done" << endl;

    return 0;
}