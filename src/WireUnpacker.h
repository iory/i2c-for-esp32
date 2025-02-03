/**
 * Modified based on https://github.com/gutierrezps/ESP32_I2C_Slave
 *
 * WireUnpacker is used to unpack the data that was packed
 * with WirePacker and sent to this device, be it
 * master->slave or slave->master.
 *
 * After creating the unpacker object, collect packet bytes
 * with write(). After a complete and valid packet was read,
 * the payload (data) can be read by using available() and
 * read() methods.
 *
 * lastError() will indicate if there was an error while
 * collecting packet bytes, such as invalid length,
 * premature ending or invalid crc.
 *
 * Expected packet format:
 *      [0]: start byte (0x02)
 *      [1]: number of packet length byte L.
 *      [2:2+L]: packet length N
 *      [2+L]: data[0]
 *      [3+L]: data[1]
 *      ...
 *      [N+1+L]: data[N-1]
 *      [N+2+L]: CRC8 of packet length and data
 *      [N+3+L]: end byte (0x04)
 *
 */
#ifndef WireUnpacker_h
#define WireUnpacker_h

#include <Arduino.h>

class WireUnpacker {
public:
    enum Error : char {
        NONE = 0,
        INVALID_CRC,
        INVALID_LENGTH
    };

    WireUnpacker(uint32_t bufferSize = 128);
    ~WireUnpacker();
    /**
     * Collect a packet byte. Returns 0 if the byte was ignored
     * or if there was an error (check with lastError()).
     *
     * The byte will be ignored if a start byte wasn't collected,
     * or if a end byte was read. In the last case, if there wasn't
     * errors, use available() and read() to read the payload.
     *
     * @param data      byte to be collected
     * @return size_t   1 if the byte was collected
     */
    size_t write(uint8_t data);

    /**
     * Collect multiple bytes. Calls write() for every byte.
     *
     * @param data      bytes to be collected
     * @param quantity  number of bytes to collect
     * @return size_t   number of bytes collected
     */
    size_t write(const uint8_t* data, size_t quantity);

    /**
     * Returns number of payload bytes available to be read.
     * Will also return 0 if the packet wasn't processed.
     *
     * @return size_t
     */
    size_t available();

    /**
     * Read the next available payload byte. At each call,
     * the value returned by available() will be decremented.
     *
     * @return int  -1 if there are no bytes to be read
     */
    int read();

    /**
     * Resets the unpacking process.
     */
    void reset();

    bool hasError() const {
        return lastError_ != Error::NONE;
    }

    Error lastError() const {
        return lastError_;
    }

    /**
     * Returns true if a start byte was read and more packet
     * bytes are expected.
     *
     */
    bool isPacketOpen() const {
        return isPacketOpen_;
    }

    uint32_t totalLength() const {
        return totalLength_;
    }

    uint32_t bufferSize() const {
        return bufferSize_;
    }

private:
    const uint8_t frameStart_ = 0x02;
    const uint8_t frameEnd_ = 0x04;

    uint32_t bufferSize_;
    uint8_t* buffer_;
    uint32_t index_;
    uint32_t totalLength_;
    uint32_t payloadLength_;
    bool isPacketOpen_;
    uint32_t expectedLength_;
    uint8_t expectedCrc_;
    uint8_t numBufferLength_;
    uint8_t ignoreLength_;

    Error lastError_;
};

#endif
