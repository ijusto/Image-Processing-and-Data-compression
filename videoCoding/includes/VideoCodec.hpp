#ifndef VIDEOCODING_VIDEOCODEC_HPP
#define VIDEOCODING_VIDEOCODEC_HPP

//!  VideoCodec
/*!
 *  This class is useful to ...
 *  @author Agostinho Pires
 *  @author Inês Justo
*/
class VideoCodec{

private:
    cv::Mat frame;
    int initial_m;
    cv :: Mat residuals;
    std::vector<bool> encodedRes0;
    std::vector<bool> encodedRes1;
    std::vector<bool> encodedRes2;

    /**
     * Converts integer to bool vector representing bits.
     * @param n
     * @return
     */
    std::vector<bool> int2boolvec(int n);

public:
    //! VideoCodec constructor.
    /*!
     * @param srcFileName source video file name/path.
     * @param destFileName destination video file name/path.
    */
    explicit VideoCodec(char* srcFileName, char* destFileName, std::string predictor);
    /**
     * Use BitStream to write Golomb encoded residuals to file.
     * @param filename
     */
    void write(char* filename);
};

#endif //VIDEOCODING_VIDEOCODEC_HPP
