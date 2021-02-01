//
// Created by irocs on 1/30/21.
//
#include "extraMile.cpp"


using namespace std;

class files{
public:
    string audioFile = "audio.wav";
    string yuv420File = "yuv420.y4m";

    /*
    virtual ~files() {
        remove("audio.wav");
        remove("yuv420.y4m");
    }*/
};



void extractAudio(char* src, files f){

    string fileName = src;
    string command("ffmpeg -i " + fileName + " -vn -acodec copy " + f.audioFile);
    FILE* name = popen(command.c_str(),"r");
    pclose(name);

}


void fileToYuv420p(char* src, files f){

    string fileName = src;
    string command("ffmpeg -y -i " + fileName + " -pix_fmt yuv420p " + f.yuv420File);
    FILE* name = popen(command.c_str(),"r");
    pclose(name);

}

void reconstructFile( files f, char* src){
    string fileName = src;
    string command("ffmpeg -i " + f.yuv420File + " -c:v libx264 -preset ultrafast -qp 0 -pix_fmt yuv420p -movflags +faststart " + fileName);
    FILE* name = popen(command.c_str(),"r");
    pclose(name);

}

void addAudio( files f, char* src){
    string fileName = src;
    string command("ffmpeg -i " + f.audioFile + " -i " + fileName + " " + fileName);
    FILE* name = popen(command.c_str(),"r");
    pclose(name);
}


int main(int argc, char *argv[]) {

    if(argc < 5){
        cout << "If operation is encoding: " << endl;
        cout << "usage: " << argv[0] << " <operation> SOURCE CODED_VIDEO CODED_AUDIO" << endl;
        cout << "If operation is decoding: " << endl;
        cout << "usage: " << argv[0] << " <operation> CODED_VIDEO CODED_AUDIO RECONSTRUCT_FILE" << endl;
        return 0;
    }

    string op = argv[1];
    char* file1 = argv[2];
    char* file2 = argv[3];
    char* file3 = argv[4];

    files name;

    int initial_m = 512;
    int predictor = 8;
    bool mode = 0;
    bool lossy = false;
    bool calcHist = false;
    bool lossless = true;
    int unsigned quantBits = 10;

    if(op == "encode") {

        extractAudio(file1, name);
        fileToYuv420p(file1, name);

        cout << "\nencoding..." << endl;
        VideoEncoder* videoEncoder = new VideoEncoder(&name.yuv420File[0], predictor, initial_m, mode, lossy, calcHist);

        // init encoder with sound file
        AudioEncoder* encoder = new AudioEncoder(&name.audioFile[0], initial_m, lossless, quantBits, calcHist);
        // encode
        encoder->encode();

        cout << "writing..." << endl;
        videoEncoder->write(file2);

        // write compressed file
        encoder->write(file3);

    }else if("decode"){
        VideoDecoder* videoDecoder = new VideoDecoder(file1);
        cout << "decoding..." << endl;
        videoDecoder->decode();

        // init decoder with compressed file
        AudioDecoder* decoder = new AudioDecoder(file1);
        // decode
        decoder->decode();

        cout << "writing..." << endl;
        videoDecoder->write(&name.yuv420File[0]);

        // write sound file
        decoder->write(file2);

        reconstructFile(name, file3);
        addAudio(name, file3);


    }else{
        printf("\n ERROR : This option doesn't exist!!");
    }

    return 0;
}
