/**
 * Modified based on https://github.com/gutierrezps/ESP32_I2C_Slave
 */

#include "WirePacker.h"

#include "WireCrc.h"

WirePacker::WirePacker(uint32_t bufferSize) : bufferSize_(bufferSize) {
    buffer_ = (uint8_t *)malloc(sizeof(uint8_t) * bufferSize_);
    if (buffer_ == nullptr) {
        bufferSize_ = 0;
        return;
    }
    reset();
}

WirePacker::~WirePacker() {
    free(buffer_);
    buffer_ = nullptr;
}

size_t WirePacker::write(uint8_t data) {
    if (!isPacketOpen_) {
        return 0;
    }

    // leave room for crc and end bytes
    if (totalLength_ >= bufferSize_ - 2) {
        return 0;
    }

    buffer_[index_] = data;
    ++index_;
    totalLength_ = index_;
    return 1;
}

size_t WirePacker::write(const uint8_t *data, size_t quantity) {
    for (size_t i = 0; i < quantity; ++i) {
        if (!write(data[i])) {
            return i;
        }
    }
    return quantity;
}

void WirePacker::end() {
    isPacketOpen_ = false;

    // make room for CRC byte
    ++index_;

    buffer_[index_] = frameEnd_;
    ++index_;
    totalLength_ = index_;

    int offset;
    if (bufferSize_ < (1 << 8)) {
        buffer_[2] = totalLength_ & 0xFF;
        offset = 3;
    } else if (bufferSize_ < (1 << 16)) {
        buffer_[2] = (totalLength_ >> 8) & 0xFF;
        buffer_[3] = totalLength_ & 0xFF;
        offset = 4;
    } else {
        buffer_[2] = (totalLength_ >> 16) & 0xFF;
        buffer_[3] = (totalLength_ >> 8) & 0xFF;
        buffer_[4] = totalLength_ & 0xFF;
        offset = 5;
    }

    uint32_t payloadLength = totalLength_ - ignoreLength_;

    WireCrc crc8;
    uint8_t crc = crc8.update(buffer_ + offset, payloadLength);
    buffer_[index_ - 2] = crc;

    // prepare for reading
    index_ = 0;
}

size_t WirePacker::available() {
    if (isPacketOpen_) {
        return 0;
    }

    return totalLength_ - index_;
}

int WirePacker::read() {
    int value = -1;

    if (!isPacketOpen_ && index_ < totalLength_) {
        value = buffer_[index_];
        ++index_;
    }

    return value;
}

void WirePacker::reset() {
    buffer_[0] = frameStart_;
    // ignore start, length, crc and end bytes
    if (bufferSize_ < (1 << 8)) {
        buffer_[1] = 1;
        index_ = 3;
        ignoreLength_ = 5;
    } else if (bufferSize_ < (1 << 16)) {
        buffer_[1] = 2;
        index_ = 4;
        ignoreLength_ = 6;
    } else {
        buffer_[1] = 3;
        index_ = 5;
        ignoreLength_ = 7;
    }
    totalLength_ = index_;
    isPacketOpen_ = true;
}
