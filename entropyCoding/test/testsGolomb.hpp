//
// Created by inesjusto on 15/11/20.
//

#ifndef ENTROPYCODING_TESTSGOLOMB_HPP
#define ENTROPYCODING_TESTSGOLOMB_HPP

#include    "../src/Golomb.cpp"

int nBitsInDecodedFile;
vector<int> original_array {0, -1, 1, -2, 2, -3, 3, -4, 4, -5, 5, -6, 6, -7, 7, -8};
unsigned int m = 5;
char *encodeFileName = strdup("../test/encoded");

TEST_CASE("Golomb Encode"){

    string numbersToEncode = "Numbers to encode: [ ";
    for(int number : original_array) {
        numbersToEncode += std::to_string(number) + " ";
    }
    numbersToEncode += "]";
    INFO(numbersToEncode);

    CAPTURE(m);

    INFO("File: ../test/encoded");
    auto *golomb = new Golomb(m, encodeFileName, 'e');

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

    auto *rbs = new BitStream(encodeFileName, 'r');
    vector<bool> bitsRead = rbs->readNbits(encoded_array.size());
    int nBitsInFile = encoded_array.size();
    while((nBitsInFile % 8) != 0){
        CHECK_NOTHROW(bitsRead.push_back(rbs->readBit()));
        nBitsInFile++;
    }
    CHECK_THROWS(bitsRead.push_back(rbs->readBit()));

    nBitsInDecodedFile = bitsRead.size();

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


TEST_CASE("Golomb Decode", "[!throws]"){

    INFO("File: ../test/encoded");
    auto *rbs = new BitStream(encodeFileName, 'r');
    vector<bool> bitsRead = rbs->readNbits(nBitsInDecodedFile);
    string bitsReadStr = "Bits (read from the file) to decode: ";
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

    CAPTURE(m);

    /*
    remove( "../test/decoded");
    INFO("\n\tDeleted file decoded");

    ofstream file("../test/decoded");
    file.close();
    INFO("\n\tCreated file decoded");
     */

    auto *golomb = new Golomb(m, encodeFileName, 'd');

    vector<int> decoded_numbers{};
    CHECK_THROWS(golomb->decode(&decoded_numbers));

    string decodedNumbers = "Decoded numbers: [ ";
    for(int number : decoded_numbers) {
        decodedNumbers += std::to_string(number) + " ";
    }
    decodedNumbers += "]";
    INFO(decodedNumbers);

    CHECK(std::equal(original_array.begin(), original_array.end(), decoded_numbers.begin()));
}

#endif //ENTROPYCODING_TESTSGOLOMB_HPP
