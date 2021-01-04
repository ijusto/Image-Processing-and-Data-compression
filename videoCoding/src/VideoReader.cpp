/**
 * Reads a video file pixel by pixel. (Class adapted from the script copy in the video folder from the 1st milestone) .
 * @author Inês Justo
 */

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

    // get one frame to know type
    this->cap >> this->currFrame;

    // check if we succeeded
    if (this->currFrame.empty()) {
        std::cerr << "ERROR! blank frame grabbed\n";
        throw ("ERROR! Blank frame grabbed.");
    }

    bool isColor = (this->currFrame.type() == CV_8UC3);
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