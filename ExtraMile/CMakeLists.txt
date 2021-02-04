cmake_minimum_required(VERSION 3.17)
project(ExtraMile)


set(CMAKE_CXX_STANDARD  14)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(includes)
include_directories(../entropyCoding/includes)
include_directories(../audioAndImageOrVideoManipulation/includes)


set(SNDFILE_LIBRARY "-lsndfile")
set(SNDFILE_DIR "-L/usr/lib/")

add_executable(extra src/Extra.cpp)
target_link_libraries(extra ${OpenCV_LIBS})
target_link_libraries(extra ${SNDFILE_DIR} ${SNDFILE_LIBRARY})