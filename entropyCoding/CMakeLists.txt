cmake_minimum_required(VERSION 3.10)
project(entropyCoding)

set(CMAKE_CXX_STANDARD  17)
include_directories(includes)

add_executable(testsGolomb ${PROJECT_SOURCE_DIR}/test/testsGolombMain.cpp)
add_executable(testsBitStream ${PROJECT_SOURCE_DIR}/test/testsBitStreamMain.cpp)
