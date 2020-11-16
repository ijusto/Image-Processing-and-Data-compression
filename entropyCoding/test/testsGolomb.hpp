//
// Created by inesjusto on 15/11/20.
//

#ifndef ENTROPYCODING_TESTSGOLOMB_HPP
#define ENTROPYCODING_TESTSGOLOMB_HPP

#include    "../src/Golomb.cpp"

TEST_CASE("Golomb Encode"){

    vector<int> original_array { 1, -1, 0, -2, 4, 3};

    string numbersToEncode = "Numbers to encode: [ ";
    for(int number : original_array) {
        numbersToEncode += std::to_string(number) + " ";
    }
    numbersToEncode += "]";
    INFO(numbersToEncode);

    unsigned int m = 2;
    CAPTURE(m);

    INFO("File: ../test/encoded");
    char *fname = strdup("../test/encoded");
    auto *golomb = new Golomb(m, fname, 'e');

    vector<bool> encoded_array;
    vector<bool> encoded_number;
    string encodedNumbers = "Encoded numbers:\t";
    for(int i = 0; i < original_array.size(); i++){
        // encode
        encoded_number = golomb->encode(original_array.at(i));
        for(bool bit : encoded_number){
            if(bit){
                encodedNumbers += "1";
            } else {
                encodedNumbers += "0";
            }
        }
        encodedNumbers += "\t";
        // append
        if(i == 0){
            encoded_array = encoded_number;
        } else {
            encoded_array.insert(encoded_array.end(), encoded_number.begin(), encoded_number.end());
        }
    }
    golomb->closeEncodeFile();
    INFO(encodedNumbers);


    string encodedArray = "Encoded array:\t";
    for(int i = 0; i < encoded_array.size(); i++){
        if(encoded_array.at(i)){
            encodedArray += "1";
        } else {
            encodedArray += "0";
        }
        if(((i + 1) % 8)==0) {
            encodedArray += " ";
        }
    }
    INFO(encodedArray);

    auto *rbs = new BitStream(fname, 'r');
    vector<bool> bitsRead = rbs->readNbits(encoded_array.size());
    int nBitsInFile = encoded_array.size();
    std::cout << ((nBitsInFile % 8) != 0) << std::endl;
    while((nBitsInFile % 8) != 0){
        CHECK_NOTHROW(bitsRead.push_back(rbs->readBit()));
        nBitsInFile++;
    }
    REQUIRE_THROWS(bitsRead.push_back(rbs->readBit()));

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

    REQUIRE(1); /* TODO: change*/
}


#endif //ENTROPYCODING_TESTSGOLOMB_HPP
