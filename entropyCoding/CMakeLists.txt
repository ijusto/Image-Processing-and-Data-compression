cmake_minimum_required(VERSION 3.10)
project(entropyCoding)

set(CMAKE_CXX_STANDARD  17)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(includes)
file(GLOB SOURCES src/*.cpp)

set(SNDFILE_LIBRARY "-lsndfile")
set(SNDFILE_DIR "-L/usr/lib/")

add_executable(testBitStream test/testBitStream.cpp)
add_executable(testGolomb test/testGolomb.cpp)

add_executable(testsGolomb test/testsGolombMain.cpp)
add_executable(testsBitStream test/testsBitStreamMain.cpp )
