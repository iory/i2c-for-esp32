#!/usr/bin/env python3

import time

from audio_common_msgs.msg import AudioData
import board
import busio
import rospy

from i2c_for_esp32 import WirePacker
from i2c_for_esp32 import WireUnpacker


if __name__ == '__main__':
    i2c = busio.I2C(board.SCL3, board.SDA3, frequency=1000_000)

    while not i2c.try_lock():
        pass

    try:
        while True:
            valid_add = [device_address for device_address in i2c.scan()]
            if len(valid_add) > 0:
                print(
                    "I2C addresses found:",
                    valid_add,
                )
                break
            else:
                print('not found.')
            time.sleep(2)
    finally:  # unlock the i2c bus when ctrl-c'ing out of the loop
        i2c.unlock()

    i2c_addr = valid_add[0]

    while not i2c.try_lock():
        pass

    rospy.init_node('audio_publisher')
    pub = rospy.Publisher('/audio', AudioData, queue_size=1)
    rospy.sleep(3.0)

    unpacker = WireUnpacker(buffer_size=2048)

    while not rospy.is_shutdown():
        packer = WirePacker()
        packer.end()

        if packer.available():
            i2c.writeto(i2c_addr, packer.buffer[:packer.available()],
                        stop=False)
        rospy.sleep(0.01)

        result = bytearray(unpacker.buffer_size)
        i2c.readfrom_into(i2c_addr, result, end=unpacker.buffer_size)

        unpacker.reset()
        for res in result:
            unpacker.write(res)

        rxBuffer = unpacker.buffer[:unpacker.payloadLength]
        pub.publish(AudioData(data=rxBuffer))
