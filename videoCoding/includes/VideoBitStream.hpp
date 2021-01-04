//
// Created by inesjusto on 10/12/20.
//

#ifndef VIDEOCODING_VIDEOBITSTREAM_HPP
#define VIDEOCODING_VIDEOBITSTREAM_HPP

#include "../../entropyCoding/includes/BitStream.hpp"

// All the information required by the decoder should be included in the bit-stream
// (video format, frame size, encoder parameters, etc.).

class VideoBitStream: public BitStream {

private:


public:
    //! VideoBitStream constructor.
    /*!
     * @param
    */
    explicit VideoBitStream();

};

#endif //VIDEOCODING_VIDEOBITSTREAM_HPP
