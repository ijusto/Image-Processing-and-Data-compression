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

int main(int argc, char *argv[]) {

    if(argc < 4){
        cout << "usage: " << argv[0] << " <operation> SOURCE DEST <predictor [1,8] used for encoding> [options]" << endl;
        cout << "operations:" << endl;
        cout << "\tencode\tencode SOURCE .y4m video file into DEST compressed file" << endl;
        cout << "\tdecode\tdecode SOURCE compressed file into DEST .y4m video file" << endl;
        cout << "options:" << endl;
        cout << "\t-hist\tcompute histograms and entropy and save to .csv (encode operation only)" << endl;
        return 0;
    }

    string op = argv[1];
    char* src = argv[2];
    char* dst = argv[3];
    int predictor = stoi(argv[4]);
    char* type;
    int mode = 0; // 0 - intra, 1 - hybrid
    int initial_m = 512;
    bool calcHist = false;

    // parse options
    if(argc > 4){
        for(int i = 4; i < 4+(argc-4); i++){
            if(string(argv[i]) == "-hist") {
                calcHist = true;
            }
        }
    }

    if (op == "encode"){
        VideoEncoder* videoEncoder = new VideoEncoder(src, predictor, mode, initial_m, calcHist);
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
    }else if (op == "decode"){
        VideoDecoder* videoDecoder = new VideoDecoder(src,dst,type);
    }else{
        printf("\n ERROR : This option doesn't exist!!");
    }

    return 0;
}