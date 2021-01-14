cmake_minimum_required(VERSION 3.10)
project(videoCoding)


set(CMAKE_CXX_STANDARD  17)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(includes)
include_directories(../audioAndImageOrVideoManipulation/includes)
include_directories(../entropyCoding/includes)

add_executable(codec src/codec.cpp)
#add_executable(testdecoder src/testdecoder.cpp)
add_executable(testvideo src/testvideo.cpp)
target_link_libraries(codec ${OpenCV_LIBS})


