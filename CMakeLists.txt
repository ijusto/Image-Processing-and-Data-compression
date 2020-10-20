cmake_minimum_required(VERSION 3.12)
project(Image-Processing-and-Data-compression)

set(CMAKE_CXX_STANDARD  17)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(SNDFILE_LIBRARY "-lsndfile")
set(SNDFILE_DIR "-L/usr/lib/")

add_executable(copywav wavCopy.cpp)
target_link_libraries(copywav ${SNDFILE_DIR} ${SNDFILE_LIBRARY})

add_executable(video video/copy.cpp)
target_link_libraries(video ${OpenCV_LIBS})
add_executable(audioEnt audio/audioEntropy.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#target_link_libraries(audioEnt "-I/usr/local/include/python3.7m -I/usr/local/lib/python3.7/site-packages/numpy/core/include -lpython3.7m -lpthread -lutil -ldl -Xlinker -export-dynamic")

# Matplotlib
find_package(Python2 COMPONENTS Development NumPy)
target_include_directories(audioEnt PRIVATE ${Python2_INCLUDE_DIRS} ${Python2_NumPy_INCLUDE_DIRS})
target_link_libraries(audioEnt Python2::Python Python2::NumPy ${SNDFILE_DIR} ${SNDFILE_LIBRARY})

