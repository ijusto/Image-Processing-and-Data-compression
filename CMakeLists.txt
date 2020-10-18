cmake_minimum_required(VERSION 3.12)
project(Image-Processing-and-Data-compression)

set(CMAKE_CXX_STANDARD  17)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(SNDFILE_LIBRARY "-lsndfile")
set(SNDFILE_DIR "-L/usr/lib/")

add_executable(copywav wav_copy.cpp sndfile.hh)
target_link_libraries(copywav ${SNDFILE_DIR} ${SNDFILE_LIBRARY})