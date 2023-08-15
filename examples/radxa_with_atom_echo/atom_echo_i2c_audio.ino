#include <Arduino.h>
#include <M5Atom.h>
#include <Wire.h>
#include <WireSlave.h>
#include <driver/i2s.h>

constexpr int SDA_PIN = 26;
constexpr int SCL_PIN = 32;
constexpr int I2C_SLAVE_ADDR = 0x41;

constexpr int CONFIG_I2S_BCK_PIN = 19;
constexpr int CONFIG_I2S_LRCK_PIN = 33;
constexpr int CONFIG_I2S_DATA_PIN = 22;
constexpr int CONFIG_I2S_DATA_IN_PIN = 23;

constexpr i2s_port_t SPEAK_I2S_NUMBER = I2S_NUM_0;
constexpr int MODE_MIC = 0;
constexpr int MODE_SPK = 1;
constexpr int I2S_SAMPLE_RATE = 16000;
constexpr int I2S_BUFFER_COUNT = 4;
constexpr int I2S_BUFFER_SIZE = 1000;

uint8_t buffer[I2S_BUFFER_SIZE];
size_t transBytes;

TaskHandle_t i2sTaskHandle = NULL;

class RingBuffer {
public:
    RingBuffer(int size) : size(size), readIdx(0), writeIdx(0) {
        buffer = new uint16_t[size];
    }

    ~RingBuffer() {
        delete[] buffer;
    }

    int available() const {
        int diff = writeIdx - readIdx;
        if (diff >= 0) return diff;
        return size + diff;
    }

    void write(uint16_t value) {
        buffer[writeIdx] = value;
        writeIdx = (writeIdx + 1) % size;
    }

    uint16_t read() {
        uint16_t value = buffer[readIdx];
        readIdx = (readIdx + 1) % size;
        return value;
    }

private:
    uint16_t* buffer;
    int size;
    volatile int readIdx;
    volatile int writeIdx;
};

RingBuffer ringBuffer(I2S_BUFFER_SIZE * 4);

bool InitI2SSpeakOrMic(int mode) {
    esp_err_t err = ESP_OK;

    i2s_driver_uninstall(SPEAK_I2S_NUMBER);
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  // fixed at 12bit, stereo, MSB
        .channel_format = I2S_CHANNEL_FMT_ALL_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = I2S_BUFFER_COUNT,
        .dma_buf_len = I2S_BUFFER_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};
    if (mode == MODE_MIC) {
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
    } else {
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
        i2s_config.tx_desc_auto_clear = true;
    }

    err += i2s_driver_install(SPEAK_I2S_NUMBER, &i2s_config, 0, NULL);
    i2s_pin_config_t tx_pin_config;

#if (ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 3, 0))
    tx_pin_config.mck_io_num = I2S_PIN_NO_CHANGE;
#endif

    tx_pin_config.bck_io_num = CONFIG_I2S_BCK_PIN;
    tx_pin_config.ws_io_num = CONFIG_I2S_LRCK_PIN;
    tx_pin_config.data_out_num = CONFIG_I2S_DATA_PIN;
    tx_pin_config.data_in_num = CONFIG_I2S_DATA_IN_PIN;
    err += i2s_set_pin(SPEAK_I2S_NUMBER, &tx_pin_config);
    err += i2s_set_clk(SPEAK_I2S_NUMBER, I2S_SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT,
                       I2S_CHANNEL_MONO);
    return true;
}

void requestEvent();

void i2sTask(void* parameter) {
    while (1) {
        i2s_read(I2S_NUM_0, (char*)buffer, I2S_BUFFER_SIZE, &transBytes, portMAX_DELAY);
        for (int i = 0; i < transBytes; i += 2) {
            uint16_t* val = (uint16_t*)&buffer[i];
            ringBuffer.write(*val);
        }
    }
}

void setup() {
    M5.begin(true, false, true);
    M5.dis.clear();
    M5.dis.drawpix(0, CRGB(255, 0, 0));

    Serial.begin(115200);

    InitI2SSpeakOrMic(MODE_MIC);

    /* bool begin(int sda, int scl, int address, int rxBufferSize, int txBufferSize); */
    bool res = WireSlave.begin(SDA_PIN, SCL_PIN, I2C_SLAVE_ADDR, 100, 4096);

    if (!res) {
        Serial.println("I2C slave init failed");
        while (1) delay(100);
    }

    WireSlave.onRequest(requestEvent);
    Serial.printf("Slave joined I2C bus with addr #%d\n", I2C_SLAVE_ADDR);

    xTaskCreate(i2sTask, "i2sTask", 2048, NULL, 1, &i2sTaskHandle);

    M5.dis.clear();
    M5.dis.drawpix(0, CRGB(255, 255, 255));
}

void loop() {
    // the slave response time is directly related to how often
    // this update() method is called, so avoid using long delays
    // inside loop(), and be careful with time-consuming tasks
    WireSlave.update();

    // let I2C and other ESP32 peripherals interrupts work
    // delay(1);
}

void requestEvent() {
    while (ringBuffer.available() > 0) {
        uint16_t value = ringBuffer.read();
        WireSlave.write((uint8_t)(value & 0xFF));
        WireSlave.write((uint8_t)((value >> 8) & 0xFF));
    }
}
