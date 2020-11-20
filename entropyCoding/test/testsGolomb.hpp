//
// Created by inesjusto on 15/11/20.
//

#ifndef ENTROPYCODING_TESTSGOLOMB_HPP
#define ENTROPYCODING_TESTSGOLOMB_HPP

#include    "../src/Golomb.cpp"

int nBitsInDecodedFile;
vector<int> original_array {0, -1, 1, -2, 2, -3, 3, -4, 4, -5, 5, -6, 6, -7, 7, -8};
unsigned int m = 5;
string filename = "../test/encoded_m" + std::to_string(m);
char *encodeFileName = const_cast<char *>(filename.c_str());

string boolVecToString(vector<bool> vec){
    string bits;
    for(int i = 0; i < vec.size(); i++){
        if(vec.at(i)){
            bits += "1";
        } else {
            bits += "0";
        }
        if(((i+1)%8)==0 && i != 0){
            bits += " ";
        }
    }
    return bits;
}

string intVecToString(vector<int> vec){
    string numbers;
    for(int number : vec) {
        numbers += std::to_string(number) + " ";
    }
    return numbers;
}

TEST_CASE("Golomb Encode"){

    INFO("Numbers to encode:\n\t\t[ " + intVecToString(original_array) + "]");
    CAPTURE(m);
    INFO("File where to write the encoded numbers: ../test/encoded");

    /* Encode numbers test */
    auto *golomb = new Golomb(m, encodeFileName, 'e');

    vector<bool> encNumVec, encodedNumber;
    string encNumStr = "Encoded numbers:\n\t\t[";
    for(int number: original_array){
        // encode
        encodedNumber = golomb->encode(number);
        encNumStr += boolVecToString(encodedNumber);
        encNumStr += " ";
        // append
        encNumVec.insert(encNumVec.end(), encodedNumber.begin(), encodedNumber.end());
    }
    golomb->closeEncodeFile();
    INFO("Written all encoded bits in the file.");

    encNumStr += "]";
    INFO(encNumStr);
    INFO("Encoded bytes:\n\t\t[" + boolVecToString(encNumVec)+"]");
    vector<bool> bitsCheck;
    if(m == 5) {
        bitsCheck = {1,0,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,0,0,0,1,0,1,0,1,1,0,0,1,1,1,0,0,1,1,1,1,0,0,1,0,0,0,0,1,0,1,
                     0,0,1,1,0,0,0,1,1,1,0,0,0,1,1,1,1,0,0,0,1,0,0};
    } else if(m==7){
        bitsCheck = {1,0,0,1,0,1,0,1,0,1,1,1,1,0,0,1,1,0,1,1,1,1,0,1,1,1,1,0,1,0,0,0,1,0,1,0,0,1,0,1,1,0,1,1,0,0,0,1,1,
                     0,1,0,1,1,1,0,0,1,1,1,1,0,0,1,0,0,0,0,1,0,1,0};

    }
    CHECK(std::equal(encNumVec.begin(), encNumVec.end(), bitsCheck.begin()));
}

TEST_CASE("Golomb Encode Write in file"){
    vector<bool> bitsCheck;
    if(m == 5) {
        bitsCheck = {1,0,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,0,0,0,1,0,1,0,1,1,0,0,1,1,1,0,0,1,1,1,1,0,0,1,0,0,0,0,1,0,1,
                     0,0,1,1,0,0,0,1,1,1,0,0,0,1,1,1,1,0,0,0,1,0,0};
    } else if(m==7){
        bitsCheck = {1,0,0,1,0,1,0,1,0,1,1,1,1,0,0,1,1,0,1,1,1,1,0,1,1,1,1,0,1,0,0,0,1,0,1,0,0,1,0,1,1,0,1,1,0,0,0,1,1,
                     0,1,0,1,1,1,0,0,1,1,1,1,0,0,1,0,0,0,0,1,0,1,0};

    }

    unsigned int n = bitsCheck.size();

    /* Encode write on file test */
    auto *rbs = new BitStream(encodeFileName, 'r');
    vector<bool> bitsRead;

    CHECK_NOTHROW(bitsRead = rbs->readNbits(n));

    nBitsInDecodedFile = bitsCheck.size();
    INFO("Bits in file:\n\t\t" + boolVecToString(bitsRead));
    CHECK(std::equal(bitsRead.begin(), bitsRead.end(), bitsCheck.begin()));
}

TEST_CASE("Golomb Decode", "[!throws]"){

    INFO("File with the encoded numbers: ../test/encoded");
    auto *rbs = new BitStream(encodeFileName, 'r');
    vector<bool> bitsRead = rbs->readNbits(nBitsInDecodedFile);
    INFO("Bits in file:\n\t\t" + boolVecToString(bitsRead));

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

    INFO("Numbers that were encoded:\n\t\t[ "+intVecToString(original_array)+"]");
    INFO("Decoded numbers:\n\t\t[ "+intVecToString(decoded_numbers)+"]");

    CHECK(std::equal(original_array.begin(), original_array.end(), decoded_numbers.begin()));
}

#endif //ENTROPYCODING_TESTSGOLOMB_HPP
