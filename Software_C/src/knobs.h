#ifndef __KNOBS_H
#define __KNOBS_H

#include <Arduino.h>

#define uint8_t unsigned char
#define uint16_t unsigned int

struct KeyInfo
{
    int16_t pulse_count;
    uint8_t switch_status;
};

class Knobs
{
public:
    static KeyInfo m_key_info;
    static uint8_t m_pinA_num;
    static uint8_t m_pinB_num;
    static uint8_t m_pinSw_num;
    static uint8_t m_pinA_Status;
    static uint8_t m_pinB_Status;
    static uint8_t flag;

public:
    Knobs(uint8_t pinA_num, uint8_t pinB_num, uint8_t pinSw_num);
    ~Knobs();
    KeyInfo get_data(void);
    ICACHE_RAM_ATTR void static interruter_funcA(void);
    ICACHE_RAM_ATTR void static interruter_funcSW(void);
};

#endif