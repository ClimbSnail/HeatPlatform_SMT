#include "Buzzer.h"


uint16 Buzzer::m_count = 0;
uint16  Buzzer::m_cycle_time = 1;
uint8 Buzzer::m_pin_num = 0;

/***************************************************
 * 初始化
 * buzzer_num为蜂鸣器的引脚
 * cycle_time为蜂鸣器控制的精度 ms
 */
Buzzer::Buzzer(uint8_t buzzer_num, uint16 cycle_time)
{
    m_count = 0;
    m_pin_num = buzzer_num;
    m_cycle_time = cycle_time != 0 ? cycle_time : 1;

    pinMode(m_pin_num, OUTPUT);
    digitalWrite(m_pin_num, HIGH); // 默认关闭蜂鸣器
    // digitalWrite(m_pin_num, LOW);
    // timer1_attachInterrupt(timer_handler);   // arduino原生的定时器接口
}

void Buzzer::timer_handler(void)
{
    if( 0 != m_count)
    {
        --m_count;
    }
    else{
        digitalWrite(m_pin_num, HIGH); // 定时时间到了 关闭蜂鸣器
    }
}

Buzzer::~Buzzer()
{
}

/****************************************
 * 设置长鸣时间
 * time为长鸣时间 ms
 *****************************************/
void Buzzer::set_beep_time(uint16 time)
{
    m_count = time / m_cycle_time;
    if (0 != m_count)
    {
        // timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
        // timer1_write(m_count);
        digitalWrite(m_pin_num, LOW); // 开始定时 开启蜂鸣器
    }
}