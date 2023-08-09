#!/usr/bin/env python3

import time

import board
import busio

from i2c_for_esp32 import WirePacker


if __name__ == '__main__':
    i2c = busio.I2C(board.SCL3, board.SDA3, frequency=400_000)

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

    while True:
        i2c.unlock()
        while not i2c.try_lock():
            pass

        input_string = input("input_string: ")
        if len(input_string) == 0:
            break
        packer = WirePacker(buffer_size=len(input_string) + 8)
        for s in input_string:
            packer.write(ord(s))
        packer.end()
        if packer.available():
            i2c.writeto(i2c_addr, packer.buffer[:packer.available()],
                        stop=False)
        time.sleep(0.01)
