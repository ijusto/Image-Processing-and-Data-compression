//
// Created by irocs on 1/30/21.
//
#include "VideoCoder.cpp"
#include "VideoDecoder.cpp"

using namespace std;

class files{
public:
    string audioMP3 = "audio.mp3";
    string audioWAV = "audio.wav";
    string yuv420File = "yuv420.y4m";
    string DecodeWAV = "decodeWAV.wav";

    /*
    virtual ~files() {
        remove("audio.mp3");
        remove("audio.wav");
        remove("yuv420.y4m");
        remove("decodeWAV.wav");
    }*/
};



void extractAudio(char* src, files f){

    string fileName = src;
    string command("ffmpeg -i " + fileName + " -vn -acodec copy " + f.audioMP3);
    FILE* name = popen(command.c_str(),"r");
    pclose(name);

}

void audioToWav(files f){
    string command("ffmpeg -i " + f.audioMP3 + " " + f.audioWAV);
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
    string command("ffmpeg -i " + f.DecodeWAV + " -i " + fileName + " " + fileName);
    FILE* name = popen(command.c_str(),"r");
    pclose(name);
}



int main(int argc, char *argv[]) {

    if(argc < 5){
        cout << "If operation is encoding: " << endl;
        cout << "usage: " << argv[0] << " <operation> SOURCE CODED_AUDIO CODED_VIDEO" << endl;
        cout << "If operation is decoding: " << endl;
        cout << "usage: " << argv[0] << " <operation> CODED_AUDIO CODED_VIDEO RECONSTRUCT_FILE" << endl;
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

    VideoCoder * videoCoder;
    VideoDecoder * videoDecoder;
    if(op == "encode") {

        //extract from the video audi file
        extractAudio(file1, name);

        //transform on .wav format
        audioToWav(name);

        fileToYuv420p(file1, name);

        videoCoder = new VideoCoder(&name.audioWAV[0], initial_m, lossless, quantBits, &name.yuv420File[0], predictor, mode, lossy);
        cout << "\nencoding audio..." << endl;
        videoCoder->encodeAudio();
        cout << "writing audio codes..." << endl;
        videoCoder->writeAudio(file2);

        cout << "\nencoding video..." << endl;
        videoCoder->encodeVideo();
        cout << "writing video codes..." << endl;
        videoCoder->writeVideo(file3);
        cout << "Encoding DONE!!" << endl;

    }else if("decodeVideo"){

        videoDecoder = new VideoDecoder(file1,file2,'a');
        cout << "\ndencoding audio codes..." << endl;
        videoDecoder->decodeAudio();
        cout << "writing audio file..." << endl;
        videoDecoder->writeAudioFile(&name.DecodeWAV[0]);
        cout << "Decoding Audio DONE!!" << endl;

        videoDecoder = new VideoDecoder(file1,file2,'v');
        cout << "\ndencoding video codes..." << endl;
        videoDecoder->decodeVideo();
        cout << "writing video file..." << endl;
        videoDecoder->writeVideoFile(&name.yuv420File[0]);
        cout << "Decoding Video DONE!!" << endl;

        //reconstruct .y4m on .avi
        reconstructFile( name, file3);

        addAudio(name,file3);

    }else
        cout<<"\n ERROR : This option doesn't exist!!"<< endl;

    return 0;
}
