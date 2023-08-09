class WireCrc(object):

    def __init__(self):
        self.seed = 0

    def calc(self, data, length):
        self.seed = 0
        return self.update(data, length)

    def update(self, data, length):
        crc = self.seed
        for i in range(length):
            extract = data[i]
            for _ in range(8):
                sum = (crc ^ extract) & 0x01
                crc >>= 1
                if sum:
                    crc ^= 0x8C
                extract >>= 1
        return crc
