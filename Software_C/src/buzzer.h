#ifndef __BUZZER_H
#define __BUZZER_H

#include <Arduino.h>

/**********************************
 * 蜂鸣器控制器
 */
class Buzzer
{
public:
    static uint16 m_count;
    static uint16 m_cycle_time;
    static uint8_t m_pin_num;

public:
    Buzzer(uint8_t buzzer_num, uint16 cycle_time = 10);
    ~Buzzer();
    IRAM_ATTR void static timer_handler(void);
    void set_beep_time(uint16 time);
};

#endif