#include <M5AtomS3.h>
#include <Wire.h>
#include <WireSlave.h>

/* constexpr int SDA_PIN = 39; */
/* constexpr int SCL_PIN = 38; */
constexpr int SDA_PIN = 2;
constexpr int SCL_PIN = 1;
constexpr int I2C_SLAVE_ADDR = 0x08;
constexpr int MAX_SLAVE_RESPONSE_LENGTH = 32;

void receiveEvent(int howMany);

void setup() {
    /* void begin(bool LCDEnable = true, bool SerialEnable = true, */
    /*            bool I2CEnable = false, bool LEDEnable = false); */
    M5.begin(true, true, true, false);

    // For display
    M5.Lcd.setRotation(2);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Wait for I2C input.");
    char log_msg[50];
    sprintf(log_msg, "I2C address 0x%02x", I2C_SLAVE_ADDR);
    M5.Lcd.println(log_msg);

    bool success = WireSlave.begin(SDA_PIN, SCL_PIN, I2C_SLAVE_ADDR);
    if (!success) {
        M5.Lcd.println("I2C slave init failed");
        while (1) delay(100);
    }

    WireSlave.onReceive(receiveEvent);
}

void loop() {
    WireSlave.update();

    // let I2C and other ESP32 peripherals interrupts work
    delay(1);
}

void receiveEvent(int howMany) {
    // Clear display
    M5.Lcd.fillScreen(M5.Lcd.color565(0, 0, 0));
    M5.Lcd.setCursor(0, 0);
    String str;
    while (0 < WireSlave.available())  // loop through all but the last byte
    {
        char c = WireSlave.read();  // receive byte as a character
        str += c;
    }
    // Draw
    M5.Lcd.println(str);
}
