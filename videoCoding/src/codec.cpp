//
// Created by inesjusto on 04/01/21.
//

#include    "VideoReader.cpp"
#include    "VideoCodec.cpp"

int main(int argc, char *argv[]) {

    if(argc < 5){
        std::cout << "Usage: " << argv[0] << " <src_path> <dst_path> <predictor: [0,8], 8 - JLS> <type: inter or intra> " << std::endl;
        return 0;
    }

    // string src_path = argv[1];  // "../video/akiyo_qcif.y4m";
    // string dst_path = argv[2];  // "../video/copy.y4m";

    auto * videoCodec = new VideoCodec(argv[1], argv[2], argv[3], argv[4]);

    return 0;
}