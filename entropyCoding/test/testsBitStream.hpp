//
// Created by inesjusto on 15/11/20.
//

#ifndef ENTROPYCODING_TESTSBITSTREAM_HPP
#define ENTROPYCODING_TESTSBITSTREAM_HPP

#include    "../src/BitStream.cpp"
#include    <string>

TEST_CASE("BitStream readBit") {
    char *fname = strdup("../test/testReadBits.txt");
    auto * bitStream = new BitStream(fname, 'r');
    bool readBit = bitStream->readBit();
    INFO("File: ../test/testReadBits.txt");
    INFO("First bit from the file: 0");
    CAPTURE(readBit);
    INFO("Bit read from the file: " << ((int) readBit));
    CHECK(!readBit); // first bit is 0
}

TEST_CASE("BitStream readNbits") {
    vector<bool> cav_2020 {0,1,0,0,0,0,1,1, // C
                           0,1,0,0,0,0,0,1, // A
                           0,1,0,1,0,1,1,0, // V
                           0,0,1,0,1,1,0,1, // -
                           0,0,1,1,0,0,1,0, // 2
                           0,0,1,1,0,0,0,0, // 0
                           0,0,1,1,0,0,1,0, // 2
                           0,0,1,1,0,0,0,0}; // 0
    string bitsToRead;
    for(int i = 0; i < cav_2020.size(); i++){
        if(cav_2020.at(i)){
            bitsToRead += "1";
        } else {
            bitsToRead += "0";
        }
        if(((i+1)%8)==0 && i != 0){
            bitsToRead += "   ";
        }
    }
    unsigned int n = cav_2020.size();
    char *fname = strdup("../test/testReadBits.txt");
    INFO("  File: ../test/testReadBits.txt");
    INFO("\n\tBits to read from the file:\n\t"+bitsToRead+"\n");
    auto * bitStream = new BitStream(fname, 'r');
    vector<bool> readBits = bitStream->readNbits(n);
    string bitsRead;
    for(int i = 0; i < readBits.size(); i++){
        if(readBits.at(i)){
            bitsRead += "1";
        } else {
            bitsRead += "0";
        }
        if(((i+1)%8)==0 && i != 0){
            bitsRead += "   ";
        }
    }
    INFO("\n\tBits read from the file:\n\t"+bitsRead);
    CHECK(std::equal(cav_2020.begin(), cav_2020.end(), readBits.begin()));
}

TEST_CASE("BitStream writeNbits") {
    vector<bool> cav_2020 {0,1,0,0,0,0,1,1, // C
                           0,1,0,0,0,0,0,1, // A
                           0,1,0,1,0,1,1,0, // V
                           0,0,1,0,1,1,0,1, // -
                           0,0,1,1,0,0,1,0, // 2
                           0,0,1,1,0,0,0,0, // 0
                           0,0,1,1,0,0,1,0, // 2
                           0,0,1,1,0,0,0,0}; // 0
    string bitsToWrite;
    for(int i = 0; i < cav_2020.size(); i++){
        if(cav_2020.at(i)){
            bitsToWrite += "1";
        } else {
            bitsToWrite += "0";
        }
        if(((i+1)%8)==0 && i != 0){
            bitsToWrite += "   ";
        }
    }
    INFO("File: ../test/testWriteNBits");
    INFO("  Bits to write in the file:\n\t"+bitsToWrite+"\n");

    remove( "../test/testWriteNBits");
    INFO("\n\tDeleted file testWriteNBits");

    ofstream file("../test/testWriteNBits");
    file.close();
    INFO("\n\tCreated file testWriteNBits");

    char *fname = strdup("../test/testWriteNBits");
    auto * wbs = new BitStream(fname, 'w');
    wbs->writeNbits(cav_2020);
    wbs->endWriteFile();
    INFO("\n\tWritten bits in the file.");

    unsigned int n = cav_2020.size();
    auto* rbs = new BitStream(fname, 'r');
    vector<bool> readBits = rbs->readNbits(n);
    REQUIRE_THROWS(rbs->readBit());
    string bitsRead;
    for(int i = 0; i < readBits.size(); i++){
        if(readBits.at(i)){
            bitsRead += "1";
        } else {
            bitsRead += "0";
        }
        if(((i+1)%8)==0 && i != 0){
            bitsRead += "   ";
        }
    }
    INFO("\n\tBits read from the file:\n\t"+bitsRead);
    CHECK(std::equal(cav_2020.begin(), cav_2020.end(), readBits.begin()));
}

#endif //ENTROPYCODING_TESTSBITSTREAM_HPP
