//
// Created by inesjusto on 15/11/20.
//

#ifndef ENTROPYCODING_TESTSBITSTREAM_HPP
#define ENTROPYCODING_TESTSBITSTREAM_HPP

#include    "../src/BitStream.cpp"
#include    <string>

vector<bool> cav_2020 {0,1,0,0,0,0,1,1, // C
                       0,1,0,0,0,0,0,1, // A
                       0,1,0,1,0,1,1,0, // V
                       0,0,1,0,1,1,0,1, // -
                       0,0,1,1,0,0,1,0, // 2
                       0,0,1,1,0,0,0,0, // 0
                       0,0,1,1,0,0,1,0, // 2
                       0,0,1,1,0,0,0,0}; // 0

string boolVecToString(vector<bool> vec){
    string bits;
    for(int i = 0; i < vec.size(); i++){
        if(vec.at(i)){
            bits += "1";
        } else {
            bits += "0";
        }
        if(((i+1)%8)==0 && i != 0){
            bits += "   ";
        }
    }
    return bits;
}

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

TEST_CASE("BitStream readNbits: CAV-2020") {

    string bitsToRead = boolVecToString(cav_2020);

    unsigned int n = cav_2020.size();
    char *fname = strdup("../test/testReadBits.txt");
    INFO("  File: ../test/testReadBits.txt");
    INFO("\n\tBits to read from the file:\n\t"+bitsToRead+"\n");
    auto * bitStream = new BitStream(fname, 'r');
    vector<bool> readBits = bitStream->readNbits(n);

    string bitsRead = boolVecToString(readBits);
    INFO("\n\tBits read from the file:\n\t"+bitsRead);
    CHECK(std::equal(cav_2020.begin(), cav_2020.end(), readBits.begin()));
}

TEST_CASE("BitStream writeNbits - write last byte: 2020") {

    vector<bool> bits = { 0,0,1,1,0,0,1,0, // 2
                          0,0,1,1,0,0,0,0, // 0
                          0,0,1,1,0,0,1,0, // 2
                          0,0,1,1}; // 0 - Check!


    vector<bool> checkBits = { 0,0,1,1,0,0,1,0, // 2
                               0,0,1,1,0,0,0,0, // 0
                               0,0,1,1,0,0,1,0, // 2
                               0,0,1,1,0,0,0,0}; // 0
    string bitsToWrite = boolVecToString(bits);
    INFO("File: ../test/testWriteNBits2");
    INFO("  Bits to write in the file:\n\t"+bitsToWrite+"\n");

    remove( "../test/testWriteNBits2");
    INFO("\n\tDeleted file testWriteNBits2");

    ofstream file("../test/testWriteNBits2");
    file.close();
    INFO("\n\tCreated file testWriteNBits2");

    char *fname = strdup("../test/testWriteNBits2");
    auto * wbs = new BitStream(fname, 'w');
    wbs->writeNbits(bits);
    wbs->endWriteFile();
    INFO("\n\tWritten bits in the file.");

    unsigned int n = 4*8;
    auto* rbs = new BitStream(fname, 'r');
    vector<bool> readBits = rbs->readNbits(n);
    REQUIRE_THROWS(rbs->readBit());
    string bitsRead = boolVecToString(readBits);
    INFO("\n\tBits read from the file:\n\t"+bitsRead);
    CHECK(std::equal(checkBits.begin(), checkBits.end(), readBits.begin()));
}


#endif //ENTROPYCODING_TESTSBITSTREAM_HPP
