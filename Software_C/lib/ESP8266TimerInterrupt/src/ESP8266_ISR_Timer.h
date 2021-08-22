/****************************************************************************************************************************
  ESP8266_ISR_Timer.h
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

#ifndef ISR_TIMER_GENERIC_H
#define ISR_TIMER_GENERIC_H

#if !defined(ESP8266)
  #error This code is designed to run on ESP8266 and ESP8266-based boards! Please check your Tools->Board setting.
#endif

#ifndef ESP8266_TIMER_INTERRUPT_VERSION
  #define ESP8266_TIMER_INTERRUPT_VERSION       "ESP8266TimerInterrupt v1.4.0"
#endif

#include "TimerInterrupt_Generic_Debug.h"

#include <stddef.h>

#ifdef ESP8266
  extern "C"
  {
    #include "ets_sys.h"
    #include "os_type.h"
    #include "mem.h"
  }
#else
  #include <inttypes.h>
#endif

#if defined(ARDUINO)
  #if ARDUINO >= 100
    #include <Arduino.h>
  #else
    #include <WProgram.h>
  #endif
#endif

#define ESP8266_ISR_Timer ISRTimer

typedef void (*timer_callback)();
typedef void (*timer_callback_p)(void *);

class ESP8266_ISR_Timer 
{

  public:

    // maximum number of timers
    #define MAX_NUMBER_TIMERS         16

    // setTimer() constants
    #define TIMER_RUN_FOREVER         0
    #define TIMER_RUN_ONCE            1

    // constructor
    ESP8266_ISR_Timer();

    void IRAM_ATTR init();

    // this function must be called inside loop()
    void IRAM_ATTR run();

    // Timer will call function 'f' every 'd' milliseconds forever
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int IRAM_ATTR setInterval(unsigned long d, timer_callback f);

    // Timer will call function 'f' with parameter 'p' every 'd' milliseconds forever
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int IRAM_ATTR setInterval(unsigned long d, timer_callback_p f, void* p);

    // Timer will call function 'f' after 'd' milliseconds one time
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int IRAM_ATTR setTimeout(unsigned long d, timer_callback f);

    // Timer will call function 'f' with parameter 'p' after 'd' milliseconds one time
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int IRAM_ATTR setTimeout(unsigned long d, timer_callback_p f, void* p);

    // Timer will call function 'f' every 'd' milliseconds 'n' times
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int IRAM_ATTR setTimer(unsigned long d, timer_callback f, unsigned n);

    // Timer will call function 'f' with parameter 'p' every 'd' milliseconds 'n' times
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int IRAM_ATTR setTimer(unsigned long d, timer_callback_p f, void* p, unsigned n);

    // updates interval of the specified timer
    bool IRAM_ATTR changeInterval(unsigned numTimer, unsigned long d);

    // destroy the specified timer
    void IRAM_ATTR deleteTimer(unsigned numTimer);

    // restart the specified timer
    void IRAM_ATTR restartTimer(unsigned numTimer);

    // returns true if the specified timer is enabled
    bool IRAM_ATTR isEnabled(unsigned numTimer);

    // enables the specified timer
    void IRAM_ATTR enable(unsigned numTimer);

    // disables the specified timer
    void IRAM_ATTR disable(unsigned numTimer);

    // enables all timers
    void IRAM_ATTR enableAll();

    // disables all timers
    void IRAM_ATTR disableAll();

    // enables the specified timer if it's currently disabled,
    // and vice-versa
    void IRAM_ATTR toggle(unsigned numTimer);

    // returns the number of used timers
    unsigned IRAM_ATTR getNumTimers();

    // returns the number of available timers
    unsigned IRAM_ATTR getNumAvailableTimers() 
    {
      return MAX_NUMBER_TIMERS - numTimers;
    };

  private:
    // deferred call constants
#define TIMER_DEFCALL_DONTRUN   0       // don't call the callback function
#define TIMER_DEFCALL_RUNONLY   1       // call the callback function but don't delete the timer
#define TIMER_DEFCALL_RUNANDDEL 2       // call the callback function and delete the timer
    // low level function to initialize and enable a new timer
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int IRAM_ATTR setupTimer(unsigned long d, void* f, void* p, bool h, unsigned n);

    // find the first available slot
    int IRAM_ATTR findFirstFreeSlot();

    typedef struct 
    {
      unsigned long prev_millis;        // value returned by the millis() function in the previous run() call
      void* callback;                   // pointer to the callback function
      void* param;                      // function parameter
      bool hasParam;                 // true if callback takes a parameter
      unsigned long delay;              // delay value
      unsigned maxNumRuns;              // number of runs to be executed
      unsigned numRuns;                 // number of executed runs
      bool enabled;                  // true if enabled
      unsigned toBeCalled;              // deferred function call (sort of) - N.B.: only used in run()
    } timer_t;

    volatile timer_t timer[MAX_NUMBER_TIMERS];

    // actual number of timers in use (-1 means uninitialized)
    volatile int numTimers;
};

#include "ESP8266_ISR_Timer-Impl.h"

#endif    // ISR_TIMER_GENERIC_H

