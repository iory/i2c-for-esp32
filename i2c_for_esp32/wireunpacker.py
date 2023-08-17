from i2c_for_esp32.wirecrc import WireCrc


class WireUnpacker(object):

    NONE = 0
    INVALID_CRC = 1
    INVALID_LENGTH = 2

    def __init__(self, buffer_size):
        self.buffer_size = buffer_size

        self.frameStart = 0x02
        self.frameEnd = 0x04
        self.buffer = [0] * self.buffer_size
        self.index = 0
        self.totalLength = 0
        self.payloadLength = 0
        self.isPacketOpen = False
        self.expectedLength = 0
        self.expectedCrc = 0
        self.lastError = self.NONE
        self.reset()

    def hasError(self):
        return self.lastError != self.NONE

    def write(self, data):
        if self.totalLength >= self.buffer_size or self.hasError():
            return 0

        if not self.isPacketOpen:
            if self.totalLength == 0 and data == self.frameStart:
                self.isPacketOpen = True
                self.totalLength += 1
                return 1
            return 0

        if self.expectedLength == 0 or self.numBufferLength > 0:
            if self.numBufferLength == 0:
                self.numBufferLength = data
                if self.numBufferLength == 1:
                    self.ignoreLength = 5
                elif self.numBufferLength == 2:
                    self.ignoreLength = 6
                else:
                    self.ignoreLength = 7
                self.totalLength += 1
                return 1

            self.expectedLength = (self.expectedLength << 8) + data
            self.numBufferLength -= 1
            if self.numBufferLength > 0:
                self.totalLength += 1
                return 1
            if self.expectedLength > self.buffer_size:
                self.isPacketOpen = False
                self.lastError = self.INVALID_LENGTH
                return 0

            self.totalLength += 1
            return 1

        if self.totalLength < (self.expectedLength - 1):
            self.buffer[self.index] = data
            self.index += 1
            self.totalLength += 1
            return 1

        self.isPacketOpen = False
        self.totalLength += 1

        if data != self.frameEnd:
            self.lastError = self.INVALID_LENGTH
            return 0

        self.payloadLength = self.totalLength - self.ignoreLength

        crc8 = WireCrc()
        crc = crc8.update(self.buffer, self.payloadLength)

        if crc != self.buffer[self.index - 1]:
            self.lastError = self.INVALID_CRC
            return 0

        self.index = 0
        return 1

    def write_data_list(self, byte_data_list):
        if byte_data_list[0] != self.frameStart \
           or self.hasError() or self.totalLength >= self.buffer_size:
            return 0

        self.isPacketOpen = True
        self.totalLength += 1

        numBufferLength = byte_data_list[1]
        if numBufferLength == 1:
            self.ignoreLength = 5
        elif numBufferLength == 2:
            self.ignoreLength = 6
        else:
            self.ignoreLength = 7
        self.totalLength += 1

        self.expectedLength = 0
        for i in range(numBufferLength):
            self.expectedLength = (self.expectedLength << 8) \
                + byte_data_list[2 + i]
            self.totalLength += 1

        if self.expectedLength > self.buffer_size:
            self.isPacketOpen = False
            self.lastError = self.INVALID_LENGTH
            return 0

        self.buffer[:self.expectedLength] = byte_data_list[
            2 + numBufferLength:self.expectedLength + 2 + numBufferLength]
        self.totalLength += self.expectedLength

        if byte_data_list[self.expectedLength - 1] != self.frameEnd:
            self.lastError = self.INVALID_LENGTH
            return 0
        self.totalLength += 1

        self.payloadLength = self.totalLength - self.ignoreLength
        print(2 + numBufferLength,
              self.expectedLength + 2 + numBufferLength,
              self.expectedLength, self.payloadLength)
        crc8 = WireCrc()
        crc = crc8.update(self.buffer, self.payloadLength)
        if crc != self.buffer[self.index - 1]:
            self.lastError = self.INVALID_CRC
            return 0

        self.index = 0
        return self.totalLength

    def write_array(self, data):
        return sum(self.write(byte) for byte in data)

    def available(self):
        return 0 if self.isPacketOpen else self.payloadLength - self.index

    def read(self):
        if self.isPacketOpen or self.index >= self.payloadLength:
            return -1
        value = self.buffer[self.index]
        self.index += 1
        return value

    def reset(self):
        self.index = 0
        self.totalLength = 0
        self.expectedLength = 0
        self.payloadLength = 0
        self.isPacketOpen = False
        self.numBufferLength = 0
        self.lastError = self.NONE
        self.ignoreLength = 0
