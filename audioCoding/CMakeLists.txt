cmake_minimum_required(VERSION 3.10)
project(audioCoding)

include_directories(includes)
file(GLOB SOURCES src/*.cpp)

set(SNDFILE_LIBRARY "-lsndfile")
set(SNDFILE_DIR "-L/usr/lib/")

add_executable(audiocodec src/AudioCodec.cpp)
target_link_libraries(audiocodec ${SNDFILE_DIR} ${SNDFILE_LIBRARY})