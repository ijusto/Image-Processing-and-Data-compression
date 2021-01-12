//
// Created by inesjusto on 04/01/21.
//

#include    "VideoEncoder.cpp"
#include    "VideoDecoder.cpp"

int main(int argc, char *argv[]) {

    if(argc < 5){
        std::cout << "Usage: " << argv[0] << " <src_path> <encod_path> <dst_path> <predictor>" << std::endl;
        return 0;
    }
    char* src = argv[1];
    char* encoded = argv[2];
    char* dst = argv[3];
    char* type;
    int predictor = stoi(argv[4]);
    int mode = 0; // inter
    int initial_m = 512;

    VideoEncoder* videoEncoder = new VideoEncoder(src, predictor, mode, initial_m);
    std::cout << "writing..." << std::endl;
    videoEncoder->write(encoded);
    VideoDecoder* videoDecoder = new VideoDecoder(encoded,dst,type);
    return 0;
}