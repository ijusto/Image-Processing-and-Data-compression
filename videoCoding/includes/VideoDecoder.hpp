#ifndef VIDEOCODING_VIDEODECODER_HPP
#define VIDEOCODING_VIDEODECODER_HPP

//!  VideoDecoder
/*!
 *  This class is useful to ...
 *  @author Inês Justo
*/
class VideoDecoder{

private:
    cv::Mat frame;
    int initial_m;
    cv :: Mat residuals;
    std::vector<bool> encodedRes0;
    std::vector<bool> encodedRes1;
    std::vector<bool> encodedRes2;

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
    explicit VideoDecoder(char* encodedFileName, char* destFileName, char* predictor, char* type);

    /**
     * Decodes residuals for each channel.
     */
    void decode();

    /**
     */
    void write();
};

#endif //VIDEOCODING_VIDEODECODER_HPP

