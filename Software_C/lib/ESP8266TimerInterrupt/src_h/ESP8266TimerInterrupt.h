/****************************************************************************************************************************
  ESP8266TimerInterrupt.h
  For ESP8266 boards
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/ESP8266TimerInterrupt
  Licensed under MIT license

  The ESP8266 timers are badly designed, using only 23-bit counter along with maximum 256 prescaler. They're only better than UNO / Mega.
  The ESP8266 has two hardware timers, but timer0 has been used for WiFi and it's not advisable to use. Only timer1 is available.
  The timer1's 23-bit counter terribly can count only up to 8,388,607. So the timer1 maximum interval is very short.
  Using 256 prescaler, maximum timer1 interval is only 26.843542 seconds !!!

  Now with these new 16 ISR-based timers, the maximum interval is practically unlimited (limited only by unsigned long miliseconds)
  The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
  Therefore, their executions are not blocked by bad-behaving functions / tasks.
  This important feature is absolutely necessary for mission-critical tasks.

  Based on SimpleTimer - A timer library for Arduino.
  Author: mromani@ottotecnica.com
  Copyright (c) 2010 OTTOTECNICA Italy

  Based on BlynkTimer.h
  Author: Volodymyr Shymanskyy

  Version: 1.4.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K Hoang      23/11/2019 Initial coding
  1.0.1   K Hoang      25/11/2019 New release fixing compiler error
  1.0.2   K.Hoang      26/11/2019 Permit up to 16 super-long-time, super-accurate ISR-based timers to avoid being blocked
  1.0.3   K.Hoang      17/05/2020 Restructure code. Fix example. Enhance README.
  1.1.0   K.Hoang      27/10/2020 Restore cpp code besides Impl.h code to use if Multiple-Definition linker error.
  1.1.1   K.Hoang      06/12/2020 Add Version String and Change_Interval example to show how to change TimerInterval
  1.2.0   K.Hoang      08/01/2021 Add better debug feature. Optimize code and examples to reduce RAM usage
  1.3.0   K.Hoang      18/05/2021 Update to match new ESP8266 core v3.0.0
  1.4.0   K.Hoang      01/06/2021 Add complex examples. Fix compiler errors due to conflict to some libraries.
*****************************************************************************************************************************/

#pragma once

#ifndef ESP8266TIMERINTERRUPT_H
#define ESP8266TIMERINTERRUPT_H

#if !defined(ESP8266)
  #error This code is designed to run on ESP8266 and ESP8266-based boards! Please check your Tools->Board setting.
#endif

#ifndef ESP8266_TIMER_INTERRUPT_VERSION
  #define ESP8266_TIMER_INTERRUPT_VERSION       "ESP8266TimerInterrupt v1.4.0"
#endif

#ifndef TIMER_INTERRUPT_DEBUG
  #define TIMER_INTERRUPT_DEBUG      0
#endif

#include "TimerInterrupt_Generic_Debug.h"

/* From /arduino-1.8.10/hardware/esp8266com/esp8266/cores/esp8266/esp8266_peri.h

  #define ESP8266_REG(addr) *((volatile uint32_t *)(0x60000000+(addr)))
  #define ESP8266_DREG(addr) *((volatile uint32_t *)(0x3FF00000+(addr)))
  #define ESP8266_CLOCK 80000000UL

  //CPU Register
  #define CPU2X     ESP8266_DREG(0x14) //when bit 0 is set, F_CPU = 160MHz
*/

/* From /arduino-1.8.10/hardware/esp8266com/esp8266/cores/esp8266/Arduino.h

  //timer dividers
  enum TIM_DIV_ENUM {
  TIM_DIV1 = 0,   // 80 / 160 MHz (80 / 160 ticks/us - 104857.588 us max)
  TIM_DIV16 = 1,  // 5  / 10  MHz (5  /  10 ticks/us - 1677721.4 us max)
  TIM_DIV256 = 3  // 312.5 / 625 Khz (1 tick = 3.2 / 1.6 us - 26843542.4 us max)
  };

  //timer int_types
  #define TIM_EDGE	0
  #define TIM_LEVEL	1
  //timer reload values
  #define TIM_SINGLE	0 //on interrupt routine you need to write a new value to start the timer again
  #define TIM_LOOP	1 //on interrupt the counter will start with the same value again

*/

// ESP8266 only has one usable timer1, max count is only 8,388,607. So to get longer time, we use max available 256 divider
class ESP8266TimerInterrupt;

typedef ESP8266TimerInterrupt ESP8266Timer;

#define MAX_ESP8266_NUM_TIMERS      1
#define MAX_ESP8266_COUNT           8388607

typedef void (*timer_callback)  ();


class ESP8266TimerInterrupt
{
  private:
    timer_callback  _callback;        // pointer to the callback function
    float           _frequency;       // Timer frequency
    uint32_t        _timerCount;      // count to activate timer

  public:

    ESP8266TimerInterrupt()
    {
      _frequency  = 0;
      _timerCount = 0;
      _callback   = NULL;
    };

    // frequency (in hertz) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    // No params and duration now. To be addes in the future by adding similar functions here or to esp32-hal-timer.c
    bool setFrequency(float frequency, timer_callback callback)
    {
      bool isOKFlag = true;

      // ESP8266 only has one usable timer1, max count is only 8,388,607. So to get longer time, we use max available 256 divider
      // Will use later if very low frequency is needed.
      _frequency  = 80000000 / 256;
      _timerCount = (uint32_t) _frequency / frequency;
      _callback   = callback;

      if ( _timerCount > MAX_ESP8266_COUNT)
      {
        _timerCount = MAX_ESP8266_COUNT;
        // Flag error
        isOKFlag = false;
      }

      // count up
      TISR_LOGWARN3(F("ESP8266TimerInterrupt: _fre ="), _frequency, F(", _count ="), _timerCount);

      // Clock to timer (prescaler) is always 80MHz, even F_CPU is 160 MHz

      timer1_attachInterrupt(callback);

      timer1_write(_timerCount);

      // Interrupt on EGDE, autoloop
      timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);

      return isOKFlag;
    }

    // interval (in microseconds) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    // No params and duration now. To be addes in the future by adding similar functions here or to esp32-hal-timer.c
    bool setInterval(unsigned long interval, timer_callback callback)
    {
      return setFrequency((float) (1000000.0f / interval), callback);
    }

    bool attachInterrupt(float frequency, timer_callback callback)
    {
      return setFrequency(frequency, callback);
    }

    // interval (in microseconds) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    // No params and duration now. To be addes in the future by adding similar functions here or to esp32-hal-timer.c
    bool attachInterruptInterval(unsigned long interval, timer_callback callback)
    {
      return setFrequency( (float) ( 1000000.0f / interval), callback);
    }

    void detachInterrupt()
    {
      timer1_disable();
    }

    void disableTimer()
    {
      timer1_disable();
    }

    // Duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    void reattachInterrupt()
    {
      if ( (_frequency != 0) && (_timerCount != 0) && (_callback != NULL) )
        setFrequency(_frequency, _callback);
    }

    // Duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    void enableTimer()
    {
      reattachInterrupt();
    }

    // Just stop clock source, clear the count
    void stopTimer()
    {
      timer1_disable();
    }

    // Just reconnect clock source, start current count from 0
    void restartTimer()
    {
      enableTimer();
    }
}; // class ESP8266TimerInterrupt

#endif    // ESP8266TIMERINTERRUPT_H
