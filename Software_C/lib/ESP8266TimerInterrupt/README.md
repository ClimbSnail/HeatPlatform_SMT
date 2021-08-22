# ESP8266TimerInterrupt Library

[![arduino-library-badge](https://www.ardu-badge.com/badge/ESP8266TimerInterrupt.svg?)](https://www.ardu-badge.com/ESP8266TimerInterrupt)
[![GitHub release](https://img.shields.io/github/release/khoih-prog/ESP8266TimerInterrupt.svg)](https://github.com/khoih-prog/ESP8266TimerInterrupt/releases)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/khoih-prog/ESP8266TimerInterrupt/blob/master/LICENSE)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/khoih-prog/ESP8266TimerInterrupt.svg)](http://github.com/khoih-prog/ESP8266TimerInterrupt/issues)

---
---

## Table of Contents

* [Why do we need this ESP8266TimerInterrupt library](#why-do-we-need-this-esp8266timerinterrupt-library)
  * [Features](#features)
  * [Why using ISR-based Hardware Timer Interrupt is better](#why-using-isr-based-hardware-timer-interrupt-is-better)
  * [Currently supported Boards](#currently-supported-boards)
  * [Important Notes about ISR](#important-notes-about-isr)
* [Changelog](#changelog)
  * [Releases v1.4.0](#releases-v140)
  * [Releases v1.3.0](#releases-v130)
  * [Releases v1.2.0](#releases-v120)
  * [Releases v1.1.1](#releases-v111)
  * [Releases v1.1.0](#releases-v110)
  * [Releases v1.0.3](#releases-v103)
  * [Releases v1.0.2](#releases-v102)
* [Prerequisites](#prerequisites)
* [Installation](#installation)
  * [Use Arduino Library Manager](#use-arduino-library-manager)
  * [Manual Install](#manual-install)
  * [VS Code & PlatformIO](#vs-code--platformio)
* [HOWTO Fix `Multiple Definitions` Linker Error](#howto-fix-multiple-definitions-linker-error)
* [HOWTO Use PWM analogWrite() with ESP8266 running Timer1 Interrupt](#howto-use-pwm-analogwrite-with-esp8266-running-timer1-interrupt)
  * [1. ESP8266 has only 2 hardware timers, named Timer0 and Timer1](#1-esp8266-has-only-2-hardware-timers-named-timer0-and-timer1)
  * [2. ESP8266 hardware timers' functions](#2-esp8266-hardware-timers-functions)
  * [3. How to use PWM analogWrite() functions while using this library](#3-how-to-use-pwm-analogwrite-functions-while-using-this-library)
* [More useful Information](#more-useful-information)
* [Examples](#examples)
  * [  1. Argument_None](examples/Argument_None)
  * [  2. **Change_Interval**](examples/Change_Interval). New.
  * [  3. ISR_RPM_Measure](examples/ISR_RPM_Measure)
  * [  4. ISR_Switch](examples/ISR_Switch)
  * [  5. ISR_Timer_Complex](examples/ISR_Timer_Complex)
  * [  6. ISR_Timer_Switch](examples/ISR_Timer_Switch)
  * [  7. ISR_Timer_4_Switches](examples/ISR_Timer_4_Switches)
  * [  8. ISR_Timer_Switches](examples/ISR_Timer_Switches)
  * [  9. RPM_Measure](examples/RPM_Measure)
  * [ 10. SwitchDebounce](examples/SwitchDebounce)
  * [ 11. TimerInterruptTest](examples/TimerInterruptTest)
  * [ 12. ISR_16_Timers_Array](examples/ISR_16_Timers_Array) **New**
  * [ 13. ISR_16_Timers_Array_Complex](examples/ISR_16_Timers_Array_Complex) **New**
* [Example ISR_Timer_Complex](#example-isr_timer_complex)
* [Debug Terminal Output Samples](#debug-terminal-output-samples)
  * [1. ISR_Timer_Complex on ESP8266_NODEMCU_ESP12E](#1-isr_timer_complex-on-esp8266_nodemcu_esp12e)
  * [2. TimerInterruptTest on ESP8266_NODEMCU_ESP12E](#2-timerinterrupttest-on-esp8266_nodemcu_esp12e)
  * [3. Change_Interval on ESP8266_NODEMCU_ESP12E](#3-change_interval-on-esp8266_nodemcu_esp12e)
  * [4. ISR_16_Timers_Array on ESP8266_NODEMCU_ESP12E](#4-isr_16_timers_array-on-esp8266_nodemcu_esp12e)
  * [5. ISR_16_Timers_Array_Complex on ESP8266_NODEMCU_ESP12E](#5-isr_16_timers_array_complex-on-esp8266_nodemcu_esp12e)
* [Debug](#debug)
* [Troubleshooting](#troubleshooting)
* [Releases](#releases)
* [Issues](#issues)
* [TO DO](#to-do)
* [DONE](#done)
* [Contributions and Thanks](#contributions-and-thanks)
* [Contributing](#contributing)
* [License](#license)
* [Copyright](#copyright)


---
---

### Why do we need this [ESP8266TimerInterrupt library](https://github.com/khoih-prog/ESP8266TimerInterrupt)

### Features

This library enables you to use Interrupt from Hardware Timers on an ESP8266-based board.

As **Hardware Timers are rare, and very precious assets** of any board, this library now enables you to use up to **16 ISR-based Timers, while consuming only 1 Hardware Timer**. Timers' interval is very long (**ulong millisecs**).

Now with these new **16 ISR-based timers**, the maximum interval is **practically unlimited** (limited only by unsigned long miliseconds) while **the accuracy is nearly perfect** compared to software timers. 

The most important feature is they're ISR-based timers. Therefore, their executions are **not blocked by bad-behaving functions / tasks**. This important feature is absolutely necessary for mission-critical tasks. 

The [**ISR_Timer_Complex**](examples/ISR_Timer_Complex) example will demonstrate the nearly perfect accuracy compared to software timers by printing the actual elapsed millisecs of each type of timers.

Being ISR-based timers, their executions are not blocked by bad-behaving functions / tasks, such as connecting to WiFi, Internet and Blynk services. You can also have many `(up to 16)` timers to use.

This non-being-blocked important feature is absolutely necessary for mission-critical tasks.

You'll see blynkTimer Software is blocked while system is connecting to WiFi / Internet / Blynk, as well as by blocking task 
in loop(), using delay() function as an example. The elapsed time then is very unaccurate

#### Why using ISR-based Hardware Timer Interrupt is better

Imagine you have a system with a **mission-critical** function, measuring water level and control the sump pump or doing something much more important. You normally use a software timer to poll, or even place the function in loop(). But what if another function is **blocking** the loop() or setup().

So your function **might not be executed, and the result would be disastrous.**

You'd prefer to have your function called, no matter what happening with other functions (busy loop, bug, etc.).

The correct choice is to use a Hardware Timer with **Interrupt** to call your function.

These hardware timers, using interrupt, still work even if other functions are blocking. Moreover, they are much more **precise** (certainly depending on clock frequency accuracy) than other software timers using millis() or micros(). That's necessary if you need to measure some data requiring better accuracy.

Functions using normal software timers, relying on loop() and calling millis(), won't work if the loop() or setup() is blocked by certain operation. For example, certain function is blocking while it's connecting to WiFi or some services.

The catch is **your function is now part of an ISR (Interrupt Service Routine), and must be lean / mean, and follow certain rules.** More to read on:

[**HOWTO Attach Interrupt**](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/)

---

### Currently supported Boards

1. ESP8266-based boards

---

### Important Notes about ISR

1. Inside the attached function, **delay() won’t work and the value returned by millis() will not increment.** Serial data received while in the function may be lost. You should declare as **volatile any variables that you modify within the attached function.**

2. Typically global variables are used to pass data between an ISR and the main program. To make sure variables shared between an ISR and the main program are updated correctly, declare them as volatile.

---
---

## Changelog

### Releases v1.4.0

1. Fix compiler errors due to conflict to some libraries.
2. Add complex examples.

### Releases v1.3.0

1. Update to match new ESP8266 core v3.0.0

### Releases v1.2.0

1. Add better debug feature.
2. Optimize code and examples to reduce RAM usage

### Releases v1.1.1

1. Add [**Change_Interval**](examples/Change_Interval) example to show how to change TimerInterval on-the-fly
2. Add Version String

### Releases v1.1.0

1. Restore cpp code besides Impl.h code to use if Multiple-Definition linker error.
2. Update examples.
3. Enhance README.

### Releases v1.0.3

1. Restructure code.
2. Fix example.
3. Enhance README.

### Releases v1.0.2

1. Basic hardware timers for ESP8266.
2. Fix compatibility issue causing compiler error while using Arduino IDEs before 1.8.10 and ESP8266 cores 2.5.2 and before
3. More hardware-initiated software-enabled timers
4. Longer time interval

---
---

## Prerequisites

1. [`Arduino IDE 1.8.15+`](https://www.arduino.cc/en/Main/Software)
2. [`ESP8266 Core 3.0.0+`](https://github.com/esp8266/Arduino) for ESP8266-based boards. [![Latest release](https://img.shields.io/github/release/esp8266/Arduino.svg)](https://github.com/esp8266/Arduino/releases/latest/). To use ESP8266 core 2.7.1+ for LittleFS. 
3. [`Blynk_WM library v1.6.0+`](https://github.com/khoih-prog/Blynk_WM) to use with some examples. To install. check [![arduino-library-badge](https://www.ardu-badge.com/badge/Blynk_WiFiManager.svg?)](https://www.ardu-badge.com/Blynk_WiFiManager)
4. [`SimpleTimer library`](https://github.com/jfturcot/SimpleTimer) to use with some examples.


---
---

## Installation

### Use Arduino Library Manager

The best and easiest way is to use `Arduino Library Manager`. Search for [**ESP8266TimerInterrupt**](https://github.com/khoih-prog/ESP8266TimerInterrupt), then select / install the latest version.
You can also use this link [![arduino-library-badge](https://www.ardu-badge.com/badge/ESP8266TimerInterrupt.svg?)](https://www.ardu-badge.com/ESP8266TimerInterrupt) for more detailed instructions.

### Manual Install

Another way to install is to:

1. Navigate to [**ESP8266TimerInterrupt**](https://github.com/khoih-prog/ESP8266TimerInterrupt) page.
2. Download the latest release `ESP8266TimerInterrupt-master.zip`.
3. Extract the zip file to `ESP8266TimerInterrupt-master` directory 
4. Copy whole `ESP8266TimerInterrupt-master` folder to Arduino libraries' directory such as `~/Arduino/libraries/`.

### VS Code & PlatformIO

1. Install [VS Code](https://code.visualstudio.com/)
2. Install [PlatformIO](https://platformio.org/platformio-ide)
3. Install [**ESP8266TimerInterrupt** library](https://platformio.org/lib/show/11385/ESP8266TimerInterrupt) by using [Library Manager](https://platformio.org/lib/show/11385/ESP8266TimerInterrupt/installation). Search for **ESP8266TimerInterrupt** in [Platform.io Author's Libraries](https://platformio.org/lib/search?query=author:%22Khoi%20Hoang%22)
4. Use included [platformio.ini](platformio/platformio.ini) file from examples to ensure that all dependent libraries will installed automatically. Please visit documentation for the other options and examples at [Project Configuration File](https://docs.platformio.org/page/projectconf.html)

---
---

### HOWTO Fix `Multiple Definitions` Linker Error

The current library implementation, using **xyz-Impl.h instead of standard xyz.cpp**, possibly creates certain `Multiple Definitions` Linker error in certain use cases. Although it's simple to just modify several lines of code, either in the library or in the application, the library is adding 2 more source directories

1. **scr_h** for new h-only files
2. **src_cpp** for standard h/cpp files

besides the standard **src** directory.

To use the **old standard cpp** way, locate this library' directory, then just 

1. **Delete the all the files in src directory.**
2. **Copy all the files in src_cpp directory into src.**
3. Close then reopen the application code in Arduino IDE, etc. to recompile from scratch.

To re-use the **new h-only** way, just 

1. **Delete the all the files in src directory.**
2. **Copy the files in src_h directory into src.**
3. Close then reopen the application code in Arduino IDE, etc. to recompile from scratch.

---
---

### HOWTO Use PWM analogWrite() with ESP8266 running Timer1 Interrupt

Please have a look at [ESP8266TimerInterrupt Issue 8: **ESP8266Timer and PWM --> wdt reset**](https://github.com/khoih-prog/ESP8266TimerInterrupt/issues/8) to have more detailed description and solution of the issue.

#### 1. ESP8266 has only 2 hardware timers, named Timer0 and Timer1

#### 2. ESP8266 hardware timers' functions

- Timer0 has been used for WiFi and it's not advisable to use while using WiFi (if not using WiFi, why select ESP8266 ??)
- Timer1 is used by this [**ESP8266TimerInterrupt Library**](https://github.com/khoih-prog/ESP8266TimerInterrupt)

#### 3. How to use PWM analogWrite() functions while using this library

  1. If possible, use software timer instead of [**ESP8266TimerInterrupt Hardware Timer1**](https://github.com/khoih-prog/ESP8266TimerInterrupt) 
  2. If using [**ESP8266TimerInterrupt Hardware Timer1**](https://github.com/khoih-prog/ESP8266TimerInterrupt) is a must, you can either
  - use external DAC such as AD5662, AD5667, AD5696.
  - use software PWM such as mentioned in [ESP8266 PWM REVISITED (AND REIMPLEMENTED)](https://lurchi.wordpress.com/2016/06/29/esp8266-pwm-revisited-and-reimplemented/)
  

---
---

## More useful Information

The ESP8266 timers are **badly designed**, using only 23-bit counter along with maximum 256 prescaler. They're only better than UNO / Mega.
The ESP8266 has two hardware timers, but timer0 has been used for WiFi and it's not advisable to use. Only timer1 is available.
The timer1's 23-bit counter terribly can count only up to 8,388,607. So the timer1 maximum interval is very short.
Using 256 prescaler, maximum timer1 interval is only **26.843542 seconds !!!**

The timer1 counters can be configured to support automatic reload.

---

Now with these new **`16 ISR-based timers`**, the maximum interval is **practically unlimited** (limited only by unsigned long miliseconds).

The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers. Therefore, their executions are not blocked by bad-behaving functions / tasks.

This important feature is absolutely necessary for mission-critical tasks. 

The [ISR_Timer_Complex example](examples/ISR_Timer_Complex) will demonstrate the nearly perfect accuracy compared to software timers by printing the actual elapsed millisecs of each type of timers.

Being ISR-based timers, their executions are not blocked by bad-behaving functions / tasks, such as connecting to WiFi, Internet and Blynk services. You can also have many `(up to 16)` timers to use. This non-being-blocked important feature is absolutely necessary for mission-critical tasks. 

You'll see blynkTimer Software is blocked while system is connecting to WiFi / Internet / Blynk, as well as by blocking task in loop(), using delay() function as an example. The elapsed time then is very unaccurate

---
---


### Examples: 

 1. [Argument_None](examples/Argument_None)
 2. [ISR_RPM_Measure](examples/ISR_RPM_Measure)
 3. [ISR_Switch](examples/ISR_Switch) 
 4. [ISR_Timer_4_Switches](examples/ISR_Timer_4_Switches) 
 5. [ISR_Timer_Complex](examples/ISR_Timer_Complex)
 6. [ISR_Timer_Switch](examples/ISR_Timer_Switch)
 7. [ISR_Timer_Switches](examples/ISR_Timer_Switches) 
 8. [RPM_Measure](examples/RPM_Measure)
 9. [SwitchDebounce](examples/SwitchDebounce)
10. [TimerInterruptTest](examples/TimerInterruptTest)
11. [Change_Interval](examples/Change_Interval).
12. [**ISR_16_Timers_Array**](examples/ISR_16_Timers_Array) **New**
13. [**ISR_16_Timers_Array_Complex**](examples/ISR_16_Timers_Array_Complex) **New**

---
---

### Example [ISR_Timer_Complex](examples/ISR_Timer_Complex)

```cpp
#if !defined(ESP8266)
  #error This code is designed to run on ESP8266 and ESP8266-based boards! Please check your Tools->Board setting.
#endif

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG true

#include <ESP8266WiFi.h>

#define USE_SSL     false

#if USE_SSL
  #include <BlynkSimpleEsp8266_SSL.h>
  #define BLYNK_HARDWARE_PORT     9443
#else
  #include <BlynkSimpleEsp8266.h>
  #define BLYNK_HARDWARE_PORT     8080
#endif

#define USE_LOCAL_SERVER    true

// If local server
#if USE_LOCAL_SERVER
  char blynk_server[]   = "account.duckdns.org";
  //char blynk_server[]   = "192.168.2.110";
#else
  char blynk_server[]   = "";
#endif

char auth[]     = "****";
char ssid[]     = "****";
char pass[]     = "****";

// These define's must be placed at the beginning before #include "ESP8266TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

#include "ESP8266TimerInterrupt.h"
#include "ESP8266_ISR_Timer.h"

#ifndef LED_BUILTIN
  #define LED_BUILTIN       2         // Pin D4 mapped to pin GPIO2/TXD1 of ESP8266, NodeMCU and WeMoS, control on-board LED
#endif

#define HW_TIMER_INTERVAL_MS        50

#define WIFI_TIMEOUT      20000L

volatile uint32_t lastMillis = 0;

// Init ESP8266 timer 0
ESP8266Timer ITimer;

// Init BlynkTimer
ESP8266_ISR_Timer ISR_Timer;

BlynkTimer blynkTimer;

void IRAM_ATTR TimerHandler()
{
  static bool toggle = false;
  static bool started = false;

  static int timeRun      = 0;

  ISR_Timer.run();

  // Toggle LED every 50 x 100 = 5000ms = 5s
  if (++timeRun == 100)
  { 
    timeRun = 0;

    if (!started)
    {
      started = true;
      pinMode(LED_BUILTIN, OUTPUT);
    }
  
#if (TIMER_INTERRUPT_DEBUG > 0)
    Serial.print("Delta ms = "); Serial.println(millis() - lastMillis);
    lastMillis = millis();
#endif
    
    //timer interrupt toggles pin LED_BUILTIN
    digitalWrite(LED_BUILTIN, toggle);
    toggle = !toggle;
  }
}

void IRAM_ATTR doingSomething2s()
{
#if (TIMER_INTERRUPT_DEBUG > 0)  
  static unsigned long previousMillis = lastMillis;

  Serial.print("doingSomething2s: Delta ms = "); Serial.println(millis() - previousMillis);

  previousMillis = millis();
#endif
}

void IRAM_ATTR doingSomething5s()
{
#if (TIMER_INTERRUPT_DEBUG > 0)  
  static unsigned long previousMillis = lastMillis;

  Serial.print("doingSomething5s: Delta ms = "); Serial.println(millis() - previousMillis);

  previousMillis = millis();
#endif
}

void IRAM_ATTR doingSomething10s()
{
#if (TIMER_INTERRUPT_DEBUG > 0)  
  static unsigned long previousMillis = lastMillis;

  Serial.print("doingSomething10s: Delta ms = "); Serial.println(millis() - previousMillis);

  previousMillis = millis();
#endif
}

void IRAM_ATTR doingSomething50s()
{
#if (TIMER_INTERRUPT_DEBUG > 0)  
  static unsigned long previousMillis = lastMillis;

  Serial.print("doingSomething50s: Delta ms = "); Serial.println(millis() - previousMillis);

  previousMillis = millis();
#endif
}

#define BLYNK_TIMER_MS        2000L
  
void blynkDoingSomething2s()
{
  static unsigned long previousMillis = lastMillis;
  
  Serial.print(F("blynkDoingSomething2s: Delta programmed ms = ")); Serial.print(BLYNK_TIMER_MS);
  Serial.print(F(", actual = ")); Serial.println(millis() - previousMillis);
  
  previousMillis = millis();
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  
  delay(200);

  Serial.print(F("\nStarting ISR_Timer_Complex on ")); Serial.println(ARDUINO_BOARD);
  Serial.println(ESP8266_TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));
  
  // Interval in microsecs
  if (ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS * 1000, TimerHandler))
  {
    lastMillis = millis();
    Serial.print(F("Starting  ITimer OK, millis() = ")); Serial.println(lastMillis);
  }
  else
    Serial.println(F("Can't set ITimer correctly. Select another freq. or interval"));

  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  ISR_Timer.setInterval(2000L, doingSomething2s);  
  ISR_Timer.setInterval(5000L, doingSomething5s);  
  ISR_Timer.setInterval(10000L, doingSomething10s);  
  ISR_Timer.setInterval(50000L, doingSomething50s);

  // You need this timer for non-critical tasks. Avoid abusing ISR if not absolutely necessary.
  blynkTimer.setInterval(BLYNK_TIMER_MS, blynkDoingSomething2s);  

  unsigned long startWiFi = millis();
  
  WiFi.begin(ssid, pass);
  
  do
  {
    delay(200);
    
    if ( (WiFi.status() == WL_CONNECTED) || (millis() > startWiFi + WIFI_TIMEOUT) )
      break;
  } while (WiFi.status() != WL_CONNECTED);
  
  Blynk.config(auth, blynk_server, BLYNK_HARDWARE_PORT);
  Blynk.connect();

  if (Blynk.connected())
    Serial.println(F("Blynk connected"));
  else
    Serial.println(F("Blynk not connected yet"));
}

#define BLOCKING_TIME_MS      3000L

void loop()
{ 
  Blynk.run();

  // This unadvised blocking task is used to demonstrate the blocking effects onto the execution and accuracy to Software timer
  // You see the time elapse of ISR_Timer still accurate, whereas very unaccurate for Software Timer
  // The time elapse for 2000ms software timer now becomes 3000ms (BLOCKING_TIME_MS)
  // While that of ISR_Timer is still prefect.
  delay(BLOCKING_TIME_MS);
  
  // You need this Software timer for non-critical tasks. Avoid abusing ISR if not absolutely necessary
  // You don't need to and never call ISR_Timer.run() here in the loop(). It's already handled by ISR timer.
  blynkTimer.run();
}
```

---
---

### Debug Terminal Output Samples

### 1. ISR_Timer_Complex on ESP8266_NODEMCU_ESP12E

The following is the sample terminal output when running example [ISR_Timer_Complex](examples/ISR_Timer_Complex) on **ESP8266_NODEMCU_ESP12E** to demonstrate the accuracy of ISR Hardware Timer, **especially when system is very busy**.  The ISR timer is **programmed for 2s, is activated exactly after 2.000s !!!**

While software timer, **programmed for 2s, is activated after 4.258s !!!**

```
Starting ISR_Timer_Complex on ESP8266_NODEMCU_ESP12E
ESP8266TimerInterrupt v1.4.0
CPU Frequency = 160 MHz
ESP8266TimerInterrupt: _fre = 312500.00, _count = 15625
Starting  ITimer OK, millis() = 64
doingSomething2s: Delta ms = 2000   // ISR_Timer, 2000ms interval, still working even if blocked by WiFi/Blynk connecting tasks
doingSomething2s: Delta ms = 2000
doingSomething5s: Delta ms = 5000
Delta ms = 5000                     //ISR_Timer, 5000ms interval, blynking an LED
doingSomething2s: Delta ms = 2000
[6159] 
    ___  __          __
   / _ )/ /_ _____  / /__
  / _  / / // / _ \/  '_/
 /____/_/\_, /_//_/_/\_\
        /___/ v0.6.1 on NodeMCU

[6161] Protocol connect: timeout =9000
[6164] Connecting to ****.duckdns.org:8080
[6255] Ready (ping: 5ms).
Blynk connected
doingSomething2s: Delta ms = 2000  <=== ISR Timer still very accurate while system busy
blynkDoingSomething2s: Delta programmed ms = 2000, actual = 4258  //Software Timer, 2000ms interval, blocked by WiFi/Blynk connecting tasks
doingSomething2s: Delta ms = 2000
doingSomething5s: Delta ms = 5000
doingSomething10s: Delta ms = 5000
Delta ms = 5000
doingSomething2s: Delta ms = 2000
blynkDoingSomething2s: Delta programmed ms = 2000, actual = 3000  //Software Timer, 2000ms interval, blocked by delay() in loop() to demonstrate bad task.
doingSomething2s: Delta ms = 2000
doingSomething5s: Delta ms = 5000
Delta ms = 5000
blynkDoingSomething2s: Delta programmed ms = 2000, actual = 3001 <=== Software Timer not accurate when system busy
doingSomething2s: Delta ms = 2000
doingSomething2s: Delta ms = 2000
blynkDoingSomething2s: Delta programmed ms = 2000, actual = 3000 <=== Software Timer not accurate when system busy
doingSomething2s: Delta ms = 2000
doingSomething5s: Delta ms = 5000
doingSomething10s: Delta ms = 10000
```

---

### 2. TimerInterruptTest on ESP8266_NODEMCU_ESP12E

The following is the sample terminal output when running example [TimerInterruptTest](examples/TimerInterruptTest) on **ESP8266_NODEMCU_ESP12E** to demonstrate the accuracy of Hardware Timers.

```
Starting TimerInterruptTest on ESP8266_NODEMCU_ESP12E
ESP8266TimerInterrupt v1.4.0
CPU Frequency = 160 MHz
ESP8266TimerInterrupt: _fre = 312500.00, _count = 312500
Starting  ITimer OK, millis() = 262
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000
Delta ms = 1000

```

---

### 3. Change_Interval on ESP8266_NODEMCU_ESP12E

The following is the sample terminal output when running example [Change_Interval](examples/Change_Interval) on **ESP8266_NODEMCU_ESP12E** to demonstrate how to change Timer Interval on-the-fly

```
Starting Change_Interval on ESP8266_NODEMCU_ESP12E
ESP8266TimerInterrupt v1.4.0
CPU Frequency = 160 MHz
Starting  ITimer OK, millis() = 162
Time = 10001, TimerCount = 19
Time = 20002, TimerCount = 39
Changing Interval, Timer = 1000
Time = 30003, TimerCount = 49
Time = 40004, TimerCount = 59
Changing Interval, Timer = 500
Time = 50005, TimerCount = 79
Time = 60006, TimerCount = 99
Changing Interval, Timer = 1000
Time = 70007, TimerCount = 109
Time = 80008, TimerCount = 119
Changing Interval, Timer = 500
Time = 90009, TimerCount = 139
Time = 100010, TimerCount = 159
Changing Interval, Timer = 1000
Time = 110011, TimerCount = 169
Time = 120012, TimerCount = 179
Changing Interval, Timer = 500
Time = 130013, TimerCount = 199
Time = 140014, TimerCount = 219
Changing Interval, Timer = 1000
Time = 150015, TimerCount = 229
Time = 160016, TimerCount = 239
Changing Interval, Timer = 500
Time = 170017, TimerCount = 259
Time = 180018, TimerCount = 279
```

---

### 4. ISR_16_Timers_Array on ESP8266_NODEMCU_ESP12E

The following is the sample terminal output when running example [ISR_16_Timers_Array](examples/ISR_16_Timers_Array) on **ESP8266_NODEMCU_ESP12E** to demonstrate of ISR Hardware Timer, especially when system is very busy or blocked. The 16 independent ISR timers are programmed to be activated repetitively after certain intervals, is activated exactly after that programmed interval !!!

```
Starting ISR_16_Timers_Array on ESP8266_NODEMCU_ESP12E
ESP8266TimerInterrupt v1.4.0
CPU Frequency = 160 MHz
Starting ITimer OK, millis() = 175
1s: Delta ms = 1003, ms = 1178
1s: Delta ms = 999, ms = 2177
2s: Delta ms = 2002, ms = 2177
1s: Delta ms = 1000, ms = 3177
3s: Delta ms = 3002, ms = 3177
1s: Delta ms = 1000, ms = 4177
2s: Delta ms = 2000, ms = 4177
4s: Delta ms = 4002, ms = 4177
1s: Delta ms = 1000, ms = 5177
5s: Delta ms = 5002, ms = 5177
1s: Delta ms = 1000, ms = 6177
2s: Delta ms = 2001, ms = 6178
3s: Delta ms = 3001, ms = 6178
6s: Delta ms = 6003, ms = 6178
1s: Delta ms = 1000, ms = 7177
7s: Delta ms = 7002, ms = 7177
1s: Delta ms = 1000, ms = 8177
2s: Delta ms = 1999, ms = 8177
4s: Delta ms = 4000, ms = 8177
8s: Delta ms = 8002, ms = 8177
1s: Delta ms = 1000, ms = 9177
3s: Delta ms = 2999, ms = 9177
9s: Delta ms = 9002, ms = 9177
simpleTimerDoingSomething2s: Delta programmed ms = 2000, actual = 10002
1s: Delta ms = 1000, ms = 10177
2s: Delta ms = 2000, ms = 10177
5s: Delta ms = 5001, ms = 10178
10s: Delta ms = 10006, ms = 10181
1s: Delta ms = 1000, ms = 11177
11s: Delta ms = 11003, ms = 11178
1s: Delta ms = 1000, ms = 12177
2s: Delta ms = 2000, ms = 12177
3s: Delta ms = 3000, ms = 12177
4s: Delta ms = 4000, ms = 12177
6s: Delta ms = 5999, ms = 12177
12s: Delta ms = 12005, ms = 12180
1s: Delta ms = 1000, ms = 13177
13s: Delta ms = 13002, ms = 13177
1s: Delta ms = 1000, ms = 14177
2s: Delta ms = 2000, ms = 14177
7s: Delta ms = 7000, ms = 14177
14s: Delta ms = 14002, ms = 14177
1s: Delta ms = 1000, ms = 15177
3s: Delta ms = 3000, ms = 15177
5s: Delta ms = 4999, ms = 15177
15s: Delta ms = 15002, ms = 15177
1s: Delta ms = 1000, ms = 16177
2s: Delta ms = 2000, ms = 16177
4s: Delta ms = 4001, ms = 16178
8s: Delta ms = 8001, ms = 16178
16s: Delta ms = 16003, ms = 16178
1s: Delta ms = 1000, ms = 17177
1s: Delta ms = 1000, ms = 18177
2s: Delta ms = 2000, ms = 18177
3s: Delta ms = 3000, ms = 18177
6s: Delta ms = 6000, ms = 18177
9s: Delta ms = 9001, ms = 18178
1s: Delta ms = 1000, ms = 19177
simpleTimerDoingSomething2s: Delta programmed ms = 2000, actual = 10000
```

---

### 5. ISR_16_Timers_Array_Complex on ESP8266_NODEMCU_ESP12E

The following is the sample terminal output when running example [ISR_16_Timers_Array_Complex](examples/ISR_16_Timers_Array_Complex) on **ESP8266_NODEMCU_ESP12E** to demonstrate of ISR Hardware Timer, especially when system is very busy or blocked. The 16 independent ISR timers are programmed to be activated repetitively after certain intervals, is activated exactly after that programmed interval !!!


```
Starting ISR_16_Timers_Array_Complex on ESP8266_NODEMCU_ESP12E
ESP8266TimerInterrupt v1.4.0
CPU Frequency = 160 MHz
Starting ITimer OK, millis() = 177
SimpleTimer : 2, ms : 10179, Dms : 10000
Timer : 0, programmed : 5000, actual : 5008
Timer : 1, programmed : 10000, actual : 0
Timer : 2, programmed : 15000, actual : 0
Timer : 3, programmed : 20000, actual : 0
Timer : 4, programmed : 25000, actual : 0
Timer : 5, programmed : 30000, actual : 0
Timer : 6, programmed : 35000, actual : 0
Timer : 7, programmed : 40000, actual : 0
Timer : 8, programmed : 45000, actual : 0
Timer : 9, programmed : 50000, actual : 0
Timer : 10, programmed : 55000, actual : 0
Timer : 11, programmed : 60000, actual : 0
Timer : 12, programmed : 65000, actual : 0
Timer : 13, programmed : 70000, actual : 0
Timer : 14, programmed : 75000, actual : 0
Timer : 15, programmed : 80000, actual : 0
SimpleTimer : 2, ms : 20232, Dms : 10053
Timer : 0, programmed : 5000, actual : 5000
Timer : 1, programmed : 10000, actual : 10000
Timer : 2, programmed : 15000, actual : 15008
Timer : 3, programmed : 20000, actual : 20008
Timer : 4, programmed : 25000, actual : 0
Timer : 5, programmed : 30000, actual : 0
Timer : 6, programmed : 35000, actual : 0
Timer : 7, programmed : 40000, actual : 0
Timer : 8, programmed : 45000, actual : 0
Timer : 9, programmed : 50000, actual : 0
Timer : 10, programmed : 55000, actual : 0
Timer : 11, programmed : 60000, actual : 0
Timer : 12, programmed : 65000, actual : 0
Timer : 13, programmed : 70000, actual : 0
Timer : 14, programmed : 75000, actual : 0
Timer : 15, programmed : 80000, actual : 0
SimpleTimer : 2, ms : 30286, Dms : 10054
Timer : 0, programmed : 5000, actual : 5000
Timer : 1, programmed : 10000, actual : 10000
Timer : 2, programmed : 15000, actual : 15000
Timer : 3, programmed : 20000, actual : 20008
Timer : 4, programmed : 25000, actual : 25008
Timer : 5, programmed : 30000, actual : 30008
Timer : 6, programmed : 35000, actual : 0
Timer : 7, programmed : 40000, actual : 0
Timer : 8, programmed : 45000, actual : 0
Timer : 9, programmed : 50000, actual : 0
Timer : 10, programmed : 55000, actual : 0
Timer : 11, programmed : 60000, actual : 0
Timer : 12, programmed : 65000, actual : 0
Timer : 13, programmed : 70000, actual : 0
Timer : 14, programmed : 75000, actual : 0
Timer : 15, programmed : 80000, actual : 0
SimpleTimer : 2, ms : 40341, Dms : 10055
Timer : 0, programmed : 5000, actual : 5000
Timer : 1, programmed : 10000, actual : 10000
Timer : 2, programmed : 15000, actual : 15000
Timer : 3, programmed : 20000, actual : 20000
Timer : 4, programmed : 25000, actual : 25008
Timer : 5, programmed : 30000, actual : 30008
Timer : 6, programmed : 35000, actual : 35008
Timer : 7, programmed : 40000, actual : 40008
Timer : 8, programmed : 45000, actual : 0
Timer : 9, programmed : 50000, actual : 0
Timer : 10, programmed : 55000, actual : 0
Timer : 11, programmed : 60000, actual : 0
Timer : 12, programmed : 65000, actual : 0
Timer : 13, programmed : 70000, actual : 0
Timer : 14, programmed : 75000, actual : 0
Timer : 15, programmed : 80000, actual : 0
SimpleTimer : 2, ms : 50396, Dms : 10055
Timer : 0, programmed : 5000, actual : 5000
Timer : 1, programmed : 10000, actual : 10000
Timer : 2, programmed : 15000, actual : 15000
Timer : 3, programmed : 20000, actual : 20000
Timer : 4, programmed : 25000, actual : 25000
Timer : 5, programmed : 30000, actual : 30008
Timer : 6, programmed : 35000, actual : 35008
Timer : 7, programmed : 40000, actual : 40008
Timer : 8, programmed : 45000, actual : 45008
Timer : 9, programmed : 50000, actual : 50008
Timer : 10, programmed : 55000, actual : 0
Timer : 11, programmed : 60000, actual : 0
Timer : 12, programmed : 65000, actual : 0
Timer : 13, programmed : 70000, actual : 0
Timer : 14, programmed : 75000, actual : 0
Timer : 15, programmed : 80000, actual : 0
SimpleTimer : 2, ms : 60452, Dms : 10056
Timer : 0, programmed : 5000, actual : 5000
Timer : 1, programmed : 10000, actual : 10000
Timer : 2, programmed : 15000, actual : 15000
Timer : 3, programmed : 20000, actual : 20000
Timer : 4, programmed : 25000, actual : 25000
Timer : 5, programmed : 30000, actual : 30000
Timer : 6, programmed : 35000, actual : 35008
Timer : 7, programmed : 40000, actual : 40008
Timer : 8, programmed : 45000, actual : 45008
Timer : 9, programmed : 50000, actual : 50008
Timer : 10, programmed : 55000, actual : 55008
Timer : 11, programmed : 60000, actual : 60008
Timer : 12, programmed : 65000, actual : 0
Timer : 13, programmed : 70000, actual : 0
Timer : 14, programmed : 75000, actual : 0
Timer : 15, programmed : 80000, actual : 0
SimpleTimer : 2, ms : 70509, Dms : 10057
Timer : 0, programmed : 5000, actual : 5000
Timer : 1, programmed : 10000, actual : 10000
Timer : 2, programmed : 15000, actual : 15000
Timer : 3, programmed : 20000, actual : 20000
Timer : 4, programmed : 25000, actual : 25000
Timer : 5, programmed : 30000, actual : 30000
Timer : 6, programmed : 35000, actual : 35000
Timer : 7, programmed : 40000, actual : 40008
Timer : 8, programmed : 45000, actual : 45008
Timer : 9, programmed : 50000, actual : 50008
Timer : 10, programmed : 55000, actual : 55008
Timer : 11, programmed : 60000, actual : 60008
Timer : 12, programmed : 65000, actual : 65008
Timer : 13, programmed : 70000, actual : 70008
Timer : 14, programmed : 75000, actual : 0
Timer : 15, programmed : 80000, actual : 0
SimpleTimer : 2, ms : 80566, Dms : 10057
Timer : 0, programmed : 5000, actual : 5000
Timer : 1, programmed : 10000, actual : 10000
Timer : 2, programmed : 15000, actual : 15000
Timer : 3, programmed : 20000, actual : 20000
Timer : 4, programmed : 25000, actual : 25000
Timer : 5, programmed : 30000, actual : 30000
Timer : 6, programmed : 35000, actual : 35000
Timer : 7, programmed : 40000, actual : 40000
Timer : 8, programmed : 45000, actual : 45008
Timer : 9, programmed : 50000, actual : 50008
Timer : 10, programmed : 55000, actual : 55008
Timer : 11, programmed : 60000, actual : 60008
Timer : 12, programmed : 65000, actual : 65008
Timer : 13, programmed : 70000, actual : 70008
Timer : 14, programmed : 75000, actual : 75008
Timer : 15, programmed : 80000, actual : 80008

```

---
---

### Debug

Debug is enabled by default on Serial.

You can also change the debugging level (_TIMERINTERRUPT_LOGLEVEL_) from 0 to 4

```cpp
// These define's must be placed at the beginning before #include "ESP8266TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0
```

---

### Troubleshooting

If you get compilation errors, more often than not, you may need to install a newer version of the core for Arduino boards.

Sometimes, the library will only work if you update the board core to the latest version because I am using newly added functions.

---
---

## Releases

### Releases v1.4.0

1. Fix compiler errors due to conflict to some libraries.
2. Add complex examples.

### Releases v1.3.0

1. Update to match new ESP8266 core v3.0.0

### Releases v1.2.0

1. Add better debug feature.
2. Optimize code and examples to reduce RAM usage

### Releases v1.1.1

1. Add [**Change_Interval**](examples/Change_Interval) example to show how to change TimerInterval on-the-fly
2. Add Version String

### Releases v1.1.0

1. Restore cpp code besides Impl.h code to use if Multiple-Definition linker error.
2. Update examples.
3. Enhance README.

### Releases v1.0.3

1. Restructure code.
2. Fix example.
3. Enhance README.

### Releases v1.0.2

1. Basic hardware timers for ESP8266.
2. Fix compatibility issue causing compiler error while using Arduino IDEs before 1.8.10 and ESP8266 cores 2.5.2 and before
3. More hardware-initiated software-enabled timers
4. Longer time interval

### Releases v1.0.1

1. Fixing compiler error

### Releases v1.0.0

1. Initial coding

---
---

### Issues

Submit issues to: [ESP8266TimerInterrupt issues](https://github.com/khoih-prog/ESP8266TimerInterrupt/issues)

---
---

## TO DO

1. Search for bug and improvement.
2. Similar features for remaining Arduino boards such as SAMD21, SAMD51, SAM-DUE, nRF52

---

## DONE

1. Basic hardware timers for ESP8266.
2. More hardware-initiated software-enabled timers
3. Longer time interval
4. Similar features for remaining Arduino boards such as AVR, Teensy, SAMD21, SAMD51, SAM-DUE, nRF52, ESP32, STM32, etc.
5. Update to match new ESP8266 core v3.0.0
6. Fix compiler errors due to conflict to some libraries.
7. Add complex examples.

---
---

## Contributions and thanks

1. Thanks to [Holger Lembke](https://github.com/holgerlembke) to report [ESP8266TimerInterrupt Issue 8: **ESP8266Timer and PWM --> wdt reset**](https://github.com/khoih-prog/ESP8266TimerInterrupt/issues/8), leading to the [HOWTO Use PWM analogWrite() with ESP8266 running Timer1 Interrupt](https://github.com/khoih-prog/ESP8266TimerInterrupt#howto-use-pwm-analogwrite-with-esp8266-running-timer1-interrupt) notes.


<table>
  <tr>
    <td align="center"><a href="https://github.com/holgerlembke"><img src="https://github.com/holgerlembke.png" width="100px;" alt="holgerlembke"/><br /><sub><b>Holger Lembke</b></sub></a><br /></td>
  </tr> 
</table>


---

## Contributing

If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

---

## License

- The library is licensed under [MIT](https://github.com/khoih-prog/ESP8266TimerInterrupt/blob/master/LICENSE)

---

## Copyright

Copyright 2019- Khoi Hoang


