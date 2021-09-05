#include <Arduino.h>

#include "surface.h"
#define OLED_ABLE 1

Surface::Surface(uint8 sda, uint8 scl)
{
#if OLED_ABLE
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    Wire.begin(/*SDA*/ sda, /*SCL*/ scl);
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS, false, false))
    {
        // Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    m_temperature_target = 0;
    m_temperature_now = 0;
    m_pwm = 0;
#else
    Serial.begin(74880);
    Serial.print(F("Starting OK."));
#endif
}

void Surface::init(void)
{
#if OLED_ABLE
    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.

    // Clear the buffer
    // display->display();
    // delay(1000); // Pause for 2 seconds

    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);

    display->setTextSize(1); // Draw 1X-scale text
    display->setCursor(0, 5);
    display->print("HeatPlatform_SMT");
    display->setCursor(0, 20);
    display->print("QQ group: 148563337");
    display->display();
    delay(2000); // Pause for 2 seconds

    // Clear the buffer
    display->clearDisplay();
    display->setTextSize(2); // Draw 2X-scale text
    display->display();
    delay(100); // Pause for 2 seconds

// // Draw a single pixel in white
// display->drawPixel(10, 10, SSD1306_WHITE);

// Show the display buffer on the screen. You MUST call display() after
// drawing commands to make them visible on screen!
#endif
}

void Surface::set_now_temperature(int temp)
{
#if OLED_ABLE
    int index_display = 0;
    display->setTextSize(4); // Draw 1X-scale text
    if (temp < 10)
    {
        index_display = 56;
    }
    else if (temp < 100)
    {
        index_display = 46;
    }
    else
    {
        index_display = 30;
    }
    display->setCursor(index_display, 10);
    if (4095 == temp)
    {
        display->println("xxx");
    }
    else
        display->println(temp);
#endif
}

void Surface::set_now_temperature(const char *temp)
{
#if OLED_ABLE
    display->setTextSize(4); // Draw 1X-scale text
    display->setCursor(35, 10);
    display->println(temp);
#endif
}

void Surface::set_target_temperature(int temp)
{
#if OLED_ABLE
    display->setTextSize(1); // Draw 1X-scale text
    display->setCursor(0, 50);
    display->print("Set: ");
    display->println(temp);
#endif
}

void Surface::set_target_temperature(const char *temp)
{
#if OLED_ABLE
    display->setTextSize(1); // Draw 1X-scale text
    display->setCursor(0, 50);
    display->print("Set: ");
    display->println(temp);
#endif
}

void Surface::set_pwm(int pwm)
{
#if OLED_ABLE
    display->setTextSize(1); // Draw 1X-scale text
    display->setCursor(80, 50);
    display->print("Pwm: ");
    display->println(pwm);
#endif
}

void Surface::set_temperature(int target, int now, int pwm)
{
    m_temperature_target = target;
    m_temperature_now = now;
    m_pwm = pwm;
#if OLED_ABLE
    // Clear the buffer
    display->clearDisplay();
    set_target_temperature(target);
    set_now_temperature(now);
    set_pwm(pwm);
    display->display();
#else
    Serial.print("\n\ntarget->");
    Serial.print(m_temperature_target);
    Serial.print("\nnow->");
    Serial.print(m_temperature_now);
    Serial.print("\npwm->");
    Serial.print(m_pwm);
#endif
}

void Surface::display_main(void)
{
#if OLED_ABLE
    // Clear the buffer
    display->clearDisplay();
    set_target_temperature(m_temperature_target);
    set_now_temperature(m_temperature_now);
    set_pwm(m_pwm);
    display->display();
#else
    Serial.print("\n\ntarget->");
    Serial.print(m_temperature_target);
    Serial.print("\nnow->");
    Serial.print(m_temperature_now);
    Serial.print("\npwm->");
    Serial.print(m_pwm);
#endif
}

void Surface::set_setting(int num, double *data, int chooseNum, boolean isChoose)
{
    static char *setting_info[10] = {"ST:", "MD:", "DC:", "ET:", "Kp:", "Ki:", "Kd:", "Kt:", "DF", "SR"};
    static uint8 pos_x[10] = {0, 45, 90, 0, 45, 90, 0, 45, 90, 110};
    static uint8 pos_y[10] = {0, 0, 0, 16, 16, 16, 32, 32, 32, 32};
    static uint8 pos_d[8] = {0, 0, 0, 0, 1, 1, 1, 1};
    static uint8 choose_x[10] = {20, 65, 110, 20, 65, 110, 20, 65, 95, 115};
    static uint8 choose_y[10] = {9, 9, 9, 25, 25, 25, 40, 40, 40, 40};

#if OLED_ABLE
    // Clear the buffer
    display->clearDisplay();
    display->setTextSize(1); // Draw 1X-scale text
    for (int pos = 0; pos < num; ++pos)
    {
        display->setCursor(pos_x[pos], pos_y[pos]);
        display->print(setting_info[pos]);
        if (1 == pos_d[pos])
        {
            display->print(data[pos], 1);
        }
        else
            display->print((int)data[pos]);
    }
    // "参数恢复默认值"
    display->setCursor(pos_x[8], pos_y[8]);
    display->print(setting_info[8]);
    // 最后一个 "保存设置并重启"
    display->setCursor(pos_x[9], pos_y[9]);
    display->print(setting_info[9]);
    if (0 == isChoose)
    {
        display->drawTriangle(choose_x[chooseNum], choose_y[chooseNum],
                              choose_x[chooseNum] - 3, choose_y[chooseNum] + 3,
                              choose_x[chooseNum] + 3, choose_y[chooseNum] + 3, 1);
    }
    else
    {
        display->fillTriangle(choose_x[chooseNum], choose_y[chooseNum],
                              choose_x[chooseNum] - 3, choose_y[chooseNum] + 3,
                              choose_x[chooseNum] + 3, choose_y[chooseNum] + 3, 1);
    }
    display->display();
#else
    Serial.print("\n\nstatic_cast<int>(data[pos])->");
    Serial.print(static_cast<int>(data[4]));
#endif
}
