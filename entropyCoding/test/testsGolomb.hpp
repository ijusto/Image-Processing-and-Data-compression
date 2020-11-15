//
// Created by inesjusto on 15/11/20.
//

#ifndef ENTROPYCODING_TESTSGOLOMB_HPP
#define ENTROPYCODING_TESTSGOLOMB_HPP

#include    "../src/Golomb.cpp"

TEST_CASE("Golomb Encode"){

    // generate random set of ints
    srand (time(0));    // init seed
    int size = 10000;    // interval
    int n = 5;          // number of symbols to generate

    vector<int> original_array;

    for(int i = 0; i < n; i++){
        original_array.push_back(rand() % size ); // - size/2
    }

    string numbersToEncode = "Numbers to encode: [ ";
    for(int number : original_array) {
        numbersToEncode += std::to_string(number) + " ";
    }
    numbersToEncode += "]";
    INFO(numbersToEncode);

    unsigned int m = 2;
    CAPTURE(m);

    INFO("File: ../test/encoded.txt");
    char *fname = strdup("../test/encoded.txt");
    auto *bitStream = new BitStream(fname);
    auto *golomb = new Golomb(m, *bitStream);

    vector<bool> encoded_array;
    for(int i = 0; i < n; i++){
        // encode
        vector<bool> encoded_number = golomb->encode(original_array.at(i));
        // append
        encoded_array.insert(encoded_array.end(), encoded_number.begin(), encoded_number.end());
    }

    string encodedNumbers = "Encoded numbers:";
    for(int i = 0; i < encoded_array.size(); i++){
        if(encoded_array.at(i)){
            encodedNumbers += "1";
        } else {
            encodedNumbers += "0";
        }
        if(((i+1)%8)==0 && i != 0){
            encodedNumbers += "   ";
        }
    }
    INFO(encodedNumbers);

    auto *bs = new BitStream(fname);
    vector<bool> bitsRead = bs->readNbits(encodedNumbers.size());
    string bitsReadStr = "Bits read from the file: ";
    for(int i = 0; i < bitsRead.size(); i++){
        if(bitsRead.at(i)){
            bitsReadStr += "1";
        } else {
            bitsReadStr += "0";
        }
        if(((i+1)%8)==0 && i != 0){
            bitsReadStr += "   ";
        }
    }
    INFO(bitsReadStr);

    CHECK(1); // first bit is 0
}


#endif //ENTROPYCODING_TESTSGOLOMB_HPP
