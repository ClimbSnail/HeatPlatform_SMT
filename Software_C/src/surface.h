#ifndef _SURFACE_H_
#define _SURFACE_H_

#include <Adafruit_I2CDevice.h>
// #include <Adafruit_SPIDevice.h>
// #include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

class Surface
{
public:
    Adafruit_SSD1306 *display;

public:
    Surface(uint8 sda, uint8 scl);
    ~Surface(){};
    void init(void);
    void set_now_temperature(int temp);
    void set_now_temperature(const char* temp);
    void set_target_temperature(int temp);
    void set_target_temperature(const char* temp);
    void set_pwm(int pwm);
    void set_temperature(const char *target, const char *now);
    void set_temperature(int target, const char *now);
    void set_temperature(const char *target, int now);
    void set_temperature(int target, int now, int pwm);
};

#endif