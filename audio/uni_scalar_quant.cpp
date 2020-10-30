/*!
 * Program to reduce the number of bits used to represent each audio sample (uniform scalar quantization).
 * @author Inês Justo
 * @author
 */

int main(int argc, char *argv[]) {


    SndfileHandle audio = SndfileHandle(argv[argc-1], SFM_READ);

    std::vector<short> buffer(FRAMES_BUFFER_LEN * audio.channels());

    // read audio to calculate Histogram
    std::unordered_map<short, int> hist;
    for(sf_count_t nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN);
        nFrames != 0; nFrames = audio.readf(buffer.data(), FRAMES_BUFFER_LEN)){

    }

}