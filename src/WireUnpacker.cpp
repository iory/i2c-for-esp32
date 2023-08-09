/**
 * Modified based on https://github.com/gutierrezps/ESP32_I2C_Slave
 */

#include "WireUnpacker.h"

#include "WireCrc.h"

WireUnpacker::WireUnpacker(uint32_t bufferSize)
    : bufferSize_(bufferSize) {
    buffer_ = (uint8_t *)malloc(sizeof(uint8_t) * bufferSize_);
    if (buffer_ == nullptr) {
        bufferSize_ = 0;  // handle allocation error
    }
    reset();
}

WireUnpacker::~WireUnpacker() {
    free(buffer_);
}

size_t WireUnpacker::write(uint8_t data) {
    if (totalLength_ >= bufferSize_ || hasError()) {
        return 0;
    }

    if (!isPacketOpen_) {
        // enable writing only if buffer is empty
        if (totalLength_ == 0 && data == frameStart_) {
            isPacketOpen_ = true;
            ++totalLength_;
            return 1;
        }
        return 0;
    }

    // first byte after start is packet length
    if (expectedLength_ == 0 || numBufferLength_ > 0) {
        if (numBufferLength_ == 0) {
            numBufferLength_ = data;
            if (numBufferLength_ == 1) {
                ignoreLength_ = 5;
            } else if (numBufferLength_ == 2) {
                ignoreLength_ = 6;
            } else {
                ignoreLength_ = 7;
            }
            ++totalLength_;
            return 1;
        }
        expectedLength_ = (expectedLength_ << 8) + data;
        --numBufferLength_;
        if (numBufferLength_ > 0) {
            ++totalLength_;
            return 1;
        }

        if (expectedLength_ > bufferSize_) {
            isPacketOpen_ = false;
            lastError_ = INVALID_LENGTH;
            return 0;
        }

        ++totalLength_;
        return 1;
    }

    // if end byte index wasn't reached
    if (totalLength_ < (expectedLength_ - 1)) {
        buffer_[index_] = data;
        ++index_;
        ++totalLength_;
        return 1;
    }

    isPacketOpen_ = false;

    // add end byte
    ++totalLength_;

    if (data != frameEnd_) {
        lastError_ = INVALID_LENGTH;
        return 0;
    }

    // ignore start, length, crc and end bytes
    payloadLength_ = totalLength_ - ignoreLength_;

    WireCrc crc8;
    uint8_t crc = crc8.update(buffer_, payloadLength_);

    if (crc != buffer_[index_ - 1]) {
        lastError_ = INVALID_CRC;
        return 0;
    }

    index_ = 0;
    return 1;
}

size_t WireUnpacker::write(const uint8_t *data, size_t quantity) {
    for (size_t i = 0; i < quantity; ++i) {
        if (!write(data[i])) {
            return i;
        }
    }
    return quantity;
}

size_t WireUnpacker::available(void) {
    if (isPacketOpen_) return 0;

    return payloadLength_ - index_;
}

int WireUnpacker::read(void) {
    int value = -1;
    if (!isPacketOpen_ && index_ < payloadLength_) {
        value = buffer_[index_];
        ++index_;
    }
    return value;
}

void WireUnpacker::reset() {
    index_ = 0;
    totalLength_ = 0;
    ignoreLength_ = 0;
    numBufferLength_ = 0;
    payloadLength_ = 0;
    expectedLength_ = 0;
    expectedCrc_ = 0;
    isPacketOpen_ = false;
    lastError_ = WireUnpacker::NONE;
}
