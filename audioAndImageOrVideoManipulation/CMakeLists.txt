cmake_minimum_required(VERSION 3.10)
project(Image-Processing-and-Data-compression)

set(CMAKE_CXX_STANDARD  17)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(includes)
file(GLOB SOURCES src/*.cpp)

set(SNDFILE_LIBRARY "-lsndfile")
set(SNDFILE_DIR "-L/usr/lib/")

add_executable(copywav src/audio/wavCopy.cpp)
target_link_libraries(copywav ${SNDFILE_DIR} ${SNDFILE_LIBRARY})

add_executable(AudioSNR src/audio/audioSNR.cpp)
target_link_libraries(AudioSNR ${SNDFILE_DIR} ${SNDFILE_LIBRARY})

add_executable(copyVideo src/video/copy.cpp)
target_link_libraries(copyVideo ${OpenCV_LIBS})

add_executable(dispVideo src/video/display.cpp)
target_link_libraries(dispVideo ${OpenCV_LIBS})

add_executable(videoEnt src/video/videoEntropy.cpp)
target_link_libraries(videoEnt ${OpenCV_LIBS})

add_executable(videoSNR src/video/videoSNR.cpp)
target_link_libraries(videoSNR ${OpenCV_LIBS})

add_executable(kmeans src/video/kmeans.cpp )
target_link_libraries(kmeans ${OpenCV_LIBS} )

add_executable(uniscalarquant src/audio/uni_scalar_quant.cpp)
target_link_libraries(uniscalarquant ${SNDFILE_DIR} ${SNDFILE_LIBRARY})

add_executable(audioEnt src/audio/audioEntropy.cpp)
target_link_libraries(audioEnt ${SNDFILE_DIR} ${SNDFILE_LIBRARY})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#target_link_libraries(audioEnt "-I/usr/local/include/python3.7m -I/usr/local/lib/python3.7/site-packages/numpy/core/include -lpython3.7m -lpthread -lutil -ldl -Xlinker -export-dynamic")

# Matplotlib
find_package(PythonLibs 2.7)
#target_include_directories(audioEnt PRIVATE ${PYTHON_INCLUDE_DIRS})
#target_link_libraries(audioEnt ${PYTHON_LIBRARIES} ${SNDFILE_DIR} ${SNDFILE_LIBRARY})

