/**
 * Read src YUV video file encoded with a given subsampling
 * and display it on screen with correct color transformation.
 *
 *  Tested using files (from https://media.xiph.org/video/derf/):
 *  - ducks_take_off_444_720p50.y4m (4:4:4)
 *  - ducks_take_off_422_720p50.y4m (4:2:2)
 *  - ducks_take_off_420_720p50.y4m (4:2:0)
 *
 * @author Bruno Pereira
 */

#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>

using namespace cv;
using namespace std;

int main(int argc, char *argv[]) {
    // args
    if (argc < 3){
        cout << "Usage: " << argv[0] << " src_path subsampling" << endl;
        cout << "src_path:\n\tpath to yuv video file" << endl;
        cout << "subsampling: " << endl;
        cout << "\t1 - 4:4:4,\n"
                "\t2 - 4:2:2,\n"
                "\t3 - 4:2:0\n";
        return 0;
    }
    string src_path = argv[1];
    int subsampling = stoi(argv[2]);
    double fps = 25.0;

    // open video file
    ifstream video;
    video.open(src_path);

    // check if file exists
    if (!video.is_open()){
        cout << "Error opening file: " << src_path << endl;
        return -1;
    }

    // parse header
    string header;
    getline(video, header);
    cout << "header: " << header << endl;

    // get rows, cols
    int rows = stoi(header.substr(header.find(" H") + 2, header.find(" F") - header.find(" H") - 2));
    int cols = stoi(header.substr(header.find(" W") + 2, header.find(" H") - header.find(" W") - 2));

    // OpenCV buffer
    Mat frame = Mat(rows, cols, CV_8UC3);

    // read frame into this buffer
    unsigned char* frameData = new unsigned char[rows*cols*3];

    while (true) {
        // skip word FRAME
        getline(video, header);
        // read frame data
        video.read((char*) frameData, rows*cols*3);
        // check number of bytes read
        if (video.gcount() == 0)
            break;

        // ptr to mat's data buffer (to be filled with pixels in packed mode)
        uchar *buffer = (uchar*) frame.ptr();

        // convert to rgb
        for(int i = 0; i < rows * cols; i++){
            // get YUV components from data in planar mode
            int y,u,v;

            switch (subsampling) {
                case 1:
                    // 4:4:4
                    y = frameData[i];
                    u = frameData[i + (rows * cols)];
                    v = frameData[i + (rows * cols) * 2];
                    break;
                case 2:
                    // 4:2:2
                    y = frameData[i];
                    u = frameData[i/2 + (rows * cols)];
                    v = frameData[i/2 + (rows * cols) + (rows * cols)/2];
                    break;
                case 3:
                    // 4:2:0 (based on https://en.wikipedia.org/wiki/File:Yuv420.svg)
                    int ci = (i % cols)/2;   // x of every 2 cols
                    int ri = i/(cols*2);     // y of every 2 rows
                    int shift = (cols/2)*ri; // shift amount within U/V planar region
                    y = frameData[i];
                    u = frameData[(ci + shift) + (rows * cols)];
                    v = frameData[(ci + shift) + (rows * cols) + (rows * cols)/4];
                    break;
            }

            /* convert to RGB */
            int b = (int)(1.164*(y - 16) + 2.018*(u-128));
            int g = (int)(1.164*(y - 16) - 0.813*(u-128) - 0.391*(v-128));
            int r = (int)(1.164*(y - 16) + 1.596*(v-128));

            /* clipping to [0 ... 255] */
            if(r < 0) r = 0;
            if(g < 0) g = 0;
            if(b < 0) b = 0;
            if(r > 255) r = 255;
            if(g > 255) g = 255;
            if(b > 255) b = 255;

            // write to OpenCV buffer in packed mode
            buffer[i*3 ] = b;
            buffer[i*3 + 1] = g;
            buffer[i*3 + 2] = r;
        }

        imshow("Display " + src_path, frame);

        double delay = (1/fps)*1000; // ms
        if (waitKey(delay) >= 0)
            break;
    }

    return 0;
}