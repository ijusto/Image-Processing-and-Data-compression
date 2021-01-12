#ifndef VIDEOCODING_VIDEODECODER_HPP
#define VIDEOCODING_VIDEODECODER_HPP

//!  VideoDecoder
/*!
 *  This class is useful to ...
 *  @author Inês Justo
*/


#include "../../entropyCoding/includes/BitStream.hpp"
class VideoDecoder{

private:
    BitStream *sourceFile;
    unsigned int headerSize;
    int initial_m;
    int predictor;
    int format;
    int mode;
    int channels;
    int rows;
    int cols;

    cv :: Mat frame;
    cv :: Mat residuals;


    /**
     * Asserts that vector contains bits stored as least significant bit at the biggest address.
     * @param vec
     * @return
     */
    static int boolvec2int(vector<bool> vec);

public:
    //! VideoDecoder constructor.
    /*!
     * @param encodedFileName encoded video bitstream file name/path.
     * @param destFileName destination decoded video bitstream file name/path.
     * todo: complete
    */
    explicit VideoDecoder(char* encodedFileName, char* destFileName, char* type);

    /**
     * Decodes residuals for each channel.
     */
    void decode();

    /**
     */
    void write();
};

#endif //VIDEOCODING_VIDEODECODER_HPP

