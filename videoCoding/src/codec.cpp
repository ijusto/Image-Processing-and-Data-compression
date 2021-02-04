//
// Created by inesjusto on 04/01/21.
//

#include    "VideoEncoder.cpp"
#include    "VideoDecoder.cpp"

using namespace std;

void vectorToCsv(const char* fName, vector<char>* hitVec){
    ofstream csv;
    csv.open(fName);
    for(char & value : *hitVec){
        csv << ((int) value) << "\n";
    }
    csv.close();
}

void printUsage(string name){
    cout << "usage: " << name << " <operation>" << endl;
    cout << "operations:\n" << endl;
    cout << "encode\tencode SOURCE .y4m video file into DEST compressed file" << endl;
    cout << "\tusage:" << endl;
    cout << "\t\tencode SOURCE DEST <predictor [1,8]> <mode 0 - intra, 1 - hybrid> [options]" << endl;
    cout << "\toptions:" << endl;
    cout << "\t\t-hist\tcompute histograms and entropy and save to .csv" << endl;
    cout << "\t\t-lossy\tuse lossy compression" << endl;
    cout << "\texample:" << endl;
    cout << "\t\t" << name << " encode video.y4m test 5 0" << endl;
    cout << "decode\tdecode SOURCE compressed file into DEST .y4m video file" << endl;
    cout << "\tusage:" << endl;
    cout << "\t\tdecode SOURCE DEST" << endl;
    cout << "\texample:" << endl;
    cout << "\t\t" << name << " decode test test.y4m" << endl;
}

int main(int argc, char *argv[]) {
    if(argc < 4){
        printUsage(argv[0]);
        return 0;
    }

    string op = argv[1];
    char* src = argv[2];
    char* dst = argv[3];

    if (op == "encode"){
        if(argc < 6){
            printUsage(argv[0]);
            return 0;
        }

        int initial_m = 512;
        int predictor = stoi(argv[4]);
        bool mode = bool(stoi(argv[5]));
        bool calcHist = false;
        bool lossy = false;

        // parse options
        if(argc > 6){
            for(int i = 4; i < 4+(argc-4); i++){
                if(string(argv[i]) == "-hist") {
                    calcHist = true;
                }
                if(string(argv[i]) == "-lossy") {
                    lossy = true;
                }
            }
        }

        cout << "encoding..." << endl;
        VideoEncoder* videoEncoder = new VideoEncoder(src, predictor, initial_m, mode, lossy, calcHist);
        cout << "writing..." << endl;
        videoEncoder->write(dst);

        // histograms and entropy
        if(calcHist){
            cout << "writing histograms..." << endl;
            vector<vector<char>> res = videoEncoder->get_res_hists();
            vector<vector<char>> samples = videoEncoder->get_sample_hists();
            vectorToCsv("y_res.csv", (vector<char> *) &res.at(0));
            vectorToCsv("u_res.csv", (vector<char> *) &res.at(1));
            vectorToCsv("v_res.csv", (vector<char> *) &res.at(2));
            vectorToCsv("y_samples.csv", (vector<char> *) &samples.at(0));
            vectorToCsv("u_samples.csv", (vector<char> *) &samples.at(1));
            vectorToCsv("v_samples.csv", (vector<char> *) &samples.at(2));
        }
        printf("The program ended with success.\n");
    }else if (op == "decode"){
        VideoDecoder* videoDecoder = new VideoDecoder(src);
        cout << "decoding..." << endl;
        videoDecoder->decode();
        cout << "writing..." << endl;
        videoDecoder->write(dst);
        printf("The program ended with success.\n");
    }else{
        printf("ERROR : This option doesn't exist!!\n");
    }

    return 0;
}