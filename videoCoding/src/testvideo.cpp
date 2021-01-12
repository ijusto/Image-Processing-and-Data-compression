//
// Created by bruno on 12/01/2021.
// Used to create copies of .y4m video with specified number of frames
//

#include    <fstream>
#include    <iostream>
#include    <regex>

using namespace std;

int main(int argc, char *argv[]) {
    if(argc < 4){
        cout << "Usage: " << argv[0] << " <src> <dst> <number of frames>" << std::endl;
        return 0;
    }

    char* srcFileName = argv[1];
    char* dstFileName = argv[2];
    int nframes = stoi(argv[3]);

    // open input video file
    ifstream video;
    video.open(srcFileName);

    // open output video file
    ofstream outvideo;
    outvideo.open (dstFileName);

    // parse header
    string header;
    getline(video, header);
    outvideo << header << "\n"; // copy header
    cout << "header: " << header << endl;
    smatch match;

    // get rows, cols
    regex rgx_w("W([0-9]+)");
    regex_search(header, match, rgx_w);
    int cols = stoi(match[1]);

    regex rgx_h("H([0-9]+)");
    regex_search(header, match, rgx_h);
    int rows = stoi(match[1]);

    // get subsampling mode
    regex rgx_samp("C([0-9]+)");
    regex_search(header, match, rgx_samp);

    int subsampling;
    if (match.size() == 0){ // 420 by default
        subsampling = 420;
    }else{
        subsampling = stoi(match[1]);
    }

    int frameSize;

    switch(subsampling){
        case 444:
            frameSize = rows * cols * 3;
            break;
        case 422:
            frameSize = rows * cols * 3 / 2;
            break;
        case 420:
            frameSize = rows * cols * 3 / 4;
            break;
    }

    // data buffer of yuv values with subsampling
    unsigned char* frameData = new unsigned char[frameSize];

    for(int i = 0; i < nframes; i++){
        // skip word FRAME
        getline(video, header);
        outvideo << header << "\n"; // copy FRAME
        // read data
        video.read((char *) frameData, frameSize);
        // write
        outvideo.write((char *) frameData, frameSize);
    }
}
