#include <Arduino.h>

#include "surface.h"

Surface::Surface(uint8 sda, uint8 scl)
{
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    Wire.begin(/*SDA*/ sda, /*SCL*/ scl);
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS, false, false))
    {
        // Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
}

void Surface::init(void)
{
    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.

    display->display();
    delay(1000); // Pause for 2 seconds

    // Clear the buffer
    display->clearDisplay();

    display->display();
    delay(100); // Pause for 2 seconds

    display->setTextSize(2); // Draw 2X-scale text
    display->setTextColor(SSD1306_WHITE);

    // // Draw a single pixel in white
    // display->drawPixel(10, 10, SSD1306_WHITE);

    // Show the display buffer on the screen. You MUST call display() after
    // drawing commands to make them visible on screen!
}

void Surface::set_now_temperature(int temp)
{
    display->setCursor(10, 20);
    display->print("Now: ");
    if (4095 == temp)
    {
        display->println("xxx");
    }
    else
        display->println(temp);
}

void Surface::set_now_temperature(const char *temp)
{
    display->setCursor(10, 20);
    display->print("Now: ");
    display->println(temp);
}

void Surface::set_target_temperature(int temp)
{
    display->setCursor(10, 0);
    display->print("Set: ");
    display->println(temp);
}

void Surface::set_target_temperature(const char *temp)
{
    display->setCursor(10, 0);
    display->print("Set: ");
    display->println(temp);
}

void Surface::set_pwm(int pwm)
{
    display->setCursor(10, 40);
    display->print("Pwm: ");
    display->println(pwm);
}

void Surface::set_temperature(const char *target, const char *now)
{
    // Clear the buffer
    display->clearDisplay();
    set_target_temperature(target);
    set_now_temperature(now);
    display->display();
}

void Surface::set_temperature(int target, const char *now)
{
    // Clear the buffer
    display->clearDisplay();
    set_target_temperature(target);
    set_now_temperature(now);
    display->display();
}

void Surface::set_temperature(const char *target, int now)
{
    // Clear the buffer
    display->clearDisplay();
    set_target_temperature(target);
    set_now_temperature(now);
    display->display();
}

void Surface::set_temperature(int target, int now, int pwm)
{
    // Clear the buffer
    display->clearDisplay();
    set_target_temperature(target);
    set_now_temperature(now);
    set_pwm(pwm);
    display->display();
}
