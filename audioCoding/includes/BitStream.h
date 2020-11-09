#ifndef AUDIOCODING_BITSTREAM_H
#define AUDIOCODING_BITSTREAM_H
/*
 * The resulting file should be binary (not text) and take into consideration that the minimum amount of data that you
 * can access in a file is one byte (8 bits). You can implement other methods that you think might be necessary (for
 * example, methods to read and write strings, in binary). This class should be optimized, due to its extensive usage
 * during compression / decompression.
 */

//!  BitStream
/*!
 *  This class is useful to read/write bits from/to a file.
 *  @author ...
*/
class BitStream {

private:

public:
    void writeBit();
    void readBit();
    void writeNBits();
    void readNBits();
}
#endif //AUDIOCODING_BITSTREAM_H
