/**
 * Implement a simple program to test the Golomb class.
 * Unitary tests should be considered as well.
 */

//#include "../src/Golomb.cpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <bitset>
#include <../src/Golomb.cpp>

using namespace std;

int main(int argc, char* argv[]) {

    cout << "tests:" << endl;

    // testing encode/decode
    cout << "1. testing encode/decode" << endl;

    // generate random set of ints
    srand (time(0));    // init seed
    int size = 10000;    // interval
    int n = 5;          // number of symbols to generate

    vector<int> original_array;

    for(int i = 0; i < n; i++){
        original_array.push_back(rand() % size ); // - size/2
    }

    // encode set
    vector<bool> encoded_array;

    unsigned int m = 2;
    auto* golomb = new Golomb(m);
    for(int i = 0; i < n; i++){
        // encode
//        std::cout << "encode number: " << original_array.at(i) << std::endl;
        vector<bool> encoded_n = golomb->encode(original_array.at(i));
        // append
        encoded_array.insert(encoded_array.end(), encoded_n.begin(), encoded_n.end());
    }

//    for(bool bit : encoded_array)
//        cout << bit ;
//    cout << endl;

    // decode set
    vector<int> decoded_array = golomb->decode(encoded_array);

    // compare decoded set with original set
    bool equal = true;
    for(int i = 0; i < n; i++){
        if(decoded_array.at(i) != original_array.at(i)){
            equal = false;
            break;
        }
    }

    cout << "   result: " << equal << endl;

}