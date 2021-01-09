#include    "VideoReader.hpp"

//! VideoReader constructor.
/*!
 * @param sourceFileName video file name/path.
 * @throws
*/
VideoReader::VideoReader(char* sourceFileName) {

    this->cap = cv::VideoCapture(sourceFileName);

    // check if we succeeded
    if (!this->cap.isOpened()) {
        std::cerr << "ERROR! Unable to open video file\n";
        throw ("ERROR! Unable to open video file.");
    }
}

//!
/*!
 * @returns
 * @throws
*/
cv::Mat VideoReader::readFrame() {
    if (!cap.read(this->currFrame)) {
        std::cerr << "ERROR! blank frame grabbed\n";
        throw("ERROR! blank frame grabbed.");
    }

    return this->currFrame;
}


cv::Mat VideoReader::getCurrFrame() {
    return this->currFrame;
}