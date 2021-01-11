//
// Created by inesjusto on 04/01/21.
//

#include    "VideoEncoder.cpp"

int main(int argc, char *argv[]) {

    if(argc < 4){
        std::cout << "Usage: " << argv[0] << " <src_path> <dst_path> <predictor>" << std::endl;
        return 0;
    }
    char* src = argv[1];
    char* dst = argv[2];
    int predictor = stoi(argv[3]);
    int mode = 0; // inter
    int initial_m = 512;

    auto * videoCodec = new VideoEncoder(src, predictor, mode, initial_m);
    std::cout << "writing..." << std::endl;
    videoCodec->write(dst);

    return 0;
}