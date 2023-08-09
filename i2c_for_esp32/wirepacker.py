from i2c_for_esp32.wirecrc import WireCrc


class WirePacker(object):

    def __init__(self, buffer_size=5):
        self.buffer_size = buffer_size
        self.frameStart = 0x02
        self.frameEnd = 0x04
        self.buffer = bytearray(self.buffer_size)
        self.index = 0
        self.totalLength = 0
        self.isPacketOpen = False
        self.reset()

    def write(self, data):
        if not self.isPacketOpen:
            return 0
        if self.totalLength >= self.buffer_size - 2:
            return 0
        self.buffer[self.index] = data
        self.index += 1
        self.totalLength = self.index
        return 1

    def end(self):
        self.isPacketOpen = False
        self.index += 1
        self.buffer[self.index] = self.frameEnd
        self.index += 1
        self.totalLength = self.index

        if self.buffer_size < (1 << 8):
            self.buffer[2] = self.totalLength & 0xFF
            offset = 3
        elif self.buffer_size < (1 << 16):
            self.buffer[2] = (self.totalLength >> 8) & 0xFF
            self.buffer[3] = self.totalLength & 0xFF
            offset = 4
        else:
            self.buffer[2] = (self.totalLength >> 16) & 0xFF
            self.buffer[3] = (self.totalLength >> 8) & 0xFF
            self.buffer[4] = self.totalLength & 0xFF
            offset = 5

        payloadLength = self.totalLength - self.ignoreLength

        crc8 = WireCrc()
        crc = crc8.update(self.buffer[offset:], payloadLength)
        self.buffer[self.index - 2] = crc
        self.index = 0

    def available(self):
        if self.isPacketOpen:
            return 0
        return self.totalLength - self.index

    def read(self):
        if self.isPacketOpen or self.index >= self.totalLength:
            return -1
        value = self.buffer[self.index]
        self.index += 1
        return value

    def reset(self):
        self.buffer[0] = self.frameStart
        if self.buffer_size < (1 << 8):
            self.buffer[1] = 1
            self.index = 3
            self.ignoreLength = 5
        elif self.buffer_size < (1 << 16):
            self.buffer[1] = 2
            self.index = 4
            self.ignoreLength = 6
        else:
            self.buffer[1] = 3
            self.index = 5
            self.ignoreLength = 7
        self.totalLength = self.index
        self.isPacketOpen = True
