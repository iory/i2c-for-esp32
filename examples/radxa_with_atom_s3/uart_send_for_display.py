#!/usr/bin/env python

import argparse
import time

import serial

from i2c_for_esp32 import WirePacker


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Send a string via Serial to a device.')
    parser.add_argument('-s', '--string', type=str,
                        default="hello world.", help='The string to be sent.')
    parser.add_argument('-i', '--input-device', type=str,
                        default='/dev/ttyACM0',
                        help='The device to send the string to.')

    args = parser.parse_args()
    input_string = args.string
    input_device = args.input_device

    ser = serial.Serial(input_device, 1000000,
                        timeout=10,
                        xonxoff=False)

    time.sleep(1.0)

    packer = WirePacker(128)
    for s in input_string:
        packer.write(ord(s))
    packer.end()
    ser.write(packer.buffer[:packer.available()])
    ser.close()
