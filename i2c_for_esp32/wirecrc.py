class WireCrc(object):

    def __init__(self):
        self.seed = 0
        self.table = self.generate_crc_table()

    def calc(self, data, length):
        self.seed = 0
        return self.update(data, length)

    def generate_crc_table(self):
        table = []
        for byte in range(256):
            crc = byte
            for _ in range(8):
                if crc & 0x01:
                    crc = (crc >> 1) ^ 0x8C
                else:
                    crc >>= 1
            table.append(crc)
        return table

    def update(self, data, length):
        crc = self.seed
        for i in range(length):
            byte = data[i]
            crc = self.table[crc ^ byte]
        return crc
