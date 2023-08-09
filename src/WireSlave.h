/**
 * Modified based on https://github.com/gutierrezps/ESP32_I2C_Slave
 *
 * Adapted to ESP32 as a workaround to temporarily provide
 * I2C slave functionality. Uses i2c_slave_read_buffer and
 * i2c_slave_write_buffer. As the reading/writing process
 * through those functions is not interrupt-based, these
 * operations may happen while the master is sending or
 * requesting data, consequently breaking the data. To
 * mitigate this, WirePacker and WireUnpacker are used on
 * WireSlave, and must also be used by master. WirePacker
 * is used to pack the data, and WireUnpacker is used to
 * collect incoming data, check if it's not broken, and
 * then pass it forward to the user callback.
 *
 */

#ifndef TwoWireSlave_h
#define TwoWireSlave_h
#ifdef ARDUINO_ARCH_ESP32

#include <Stream.h>
#include <WirePacker.h>
#include <WireUnpacker.h>
#include <driver/i2c.h>
#include <stdint.h>

class TwoWireSlave : public Stream {
public:
    TwoWireSlave(uint8_t bus_num);
    ~TwoWireSlave();

    bool begin(int sda, int scl, int address);
    bool begin(int sda, int scl, int address, int rxBufferSize, int txBufferSize);
    void update();

    size_t write(uint8_t);
    size_t write(const uint8_t*, size_t);
    int available(void);
    int read(void);
    int peek(void);
    void flush(void);

    inline size_t write(const char* s) {
        return write((uint8_t*)s, strlen(s));
    }
    inline size_t write(unsigned long n) {
        return write((uint8_t)n);
    }
    inline size_t write(long n) {
        return write((uint8_t)n);
    }
    inline size_t write(unsigned int n) {
        return write((uint8_t)n);
    }
    inline size_t write(int n) {
        return write((uint8_t)n);
    }

    void onReceive(void (*)(int));
    void onRequest(void (*)());

private:
    uint8_t num;
    i2c_port_t portNum;
    int8_t sda;
    int8_t scl;

    uint8_t* rxBuffer;
    uint32_t rxIndex;
    uint32_t rxLength;

    uint8_t* txBuffer;
    uint32_t txIndex;
    uint32_t txLength;

    void (*user_onRequest)(void);
    void (*user_onReceive)(int);

    WirePacker* packer_;
    WireUnpacker* unpacker_;
};

extern TwoWireSlave WireSlave;
extern TwoWireSlave WireSlave1;

#endif  // ifdef ARDUINO_ARCH_ESP32
#endif  // ifndef TwoWireSlave_h
