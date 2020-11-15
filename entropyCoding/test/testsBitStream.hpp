//
// Created by inesjusto on 15/11/20.
//

#ifndef ENTROPYCODING_TESTSBITSTREAM_HPP
#define ENTROPYCODING_TESTSBITSTREAM_HPP

#include    "../src/BitStream.cpp"


TEST_CASE("BitStream readBit"){
    char *fname = strdup("../test/testReadNBits.txt");
    auto * bitStream = new BitStream(fname);
    bool readBit = bitStream->readBit();
    REQUIRE(!readBit); // first bit is 0
}

TEST_CASE("BitStream readNbits"){
    vector<bool> cav_2020 {0,1,0,0,0,0,1,1, // C
                           0,1,0,0,0,0,0,1, // A
                           0,1,0,1,0,1,1,0, // V
                           0,0,1,0,1,1,0,1, // -
                           0,0,1,1,0,0,1,0, // 2
                           0,0,1,1,0,0,0,0, // 0
                           0,0,1,1,0,0,1,0, // 2
                           0,0,1,1,0,0,0,0}; // 0
    unsigned int n = cav_2020.size();
    char *fname = strdup("../test/testReadNBits.txt");
    auto * bitStream = new BitStream(fname);
    vector<bool> readBits = bitStream->readNbits(n);
    for(int i = 0; i < readBits.size(); i++){
        std::cout <<readBits.at(i);
        if(((i+1)%8)==0 && i != 0){
            std::cout << "   ";
        }
    }
    std::cout << std::endl;
    REQUIRE(std::equal(cav_2020.begin(), cav_2020.end(), readBits.begin()));
}

TEST_CASE("BitStream writeNbits"){
    vector<bool> cav_2020 {0,1,0,0,0,0,1,1, // C
                           0,1,0,0,0,0,0,1, // A
                           0,1,0,1,0,1,1,0, // V
                           0,0,1,0,1,1,0,1, // -
                           0,0,1,1,0,0,1,0, // 2
                           0,0,1,1,0,0,0,0, // 0
                           0,0,1,1,0,0,1,0, // 2
                           0,0,1,1,0,0,0,0}; // 0
    remove( "testWriteNBits.txt" );
    char *fname = strdup("../test/testReadNBits.txt");
    auto * bitStream = new BitStream(fname);
    bitStream->writeNbits(cav_2020);
    bitStream->writeOnfile(strdup("testWriteNBits.txt"));

    unsigned int n = cav_2020.size();
    vector<bool> readBits = bitStream->readNbits(n);
    for(int i = 0; i < readBits.size(); i++){
        std::cout <<readBits.at(i);
        if(((i+1)%8)==0 && i != 0){
            std::cout << "   ";
        }
    }
    std::cout << std::endl;
    REQUIRE(std::equal(cav_2020.begin(), cav_2020.end(), readBits.begin()));
}

#endif //ENTROPYCODING_TESTSBITSTREAM_HPP
