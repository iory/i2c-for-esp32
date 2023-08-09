# i2c-for-esp32

## Extended Arduino Library for I2C Slave on ESP32

This repository contains modifications to the original Arduino library found at https://github.com/gutierrezps/ESP32_I2C_Slave .
The key enhancement enables the sending of longer packets.

Packet Format
The expected packet format is detailed below:

```
[0]: start byte (0x02)
[1]: number of packet length bytes (L)
[2:2+L]: packet length (N)
[2+L]: data[0]
[3+L]: data[1]
...
[N+1+L]: data[N-1]
[N+2+L]: CRC8 of packet length and data
[N+3+L]: end byte (0x04)
```

Feel free to expand on this by adding any installation, usage instructions or any other relevant details as required.
