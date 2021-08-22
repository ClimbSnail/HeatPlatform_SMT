/****************************************************************************************************************************
  ISR_Timer_Switch.ino
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
/* Notes:
   Special design is necessary to share data between interrupt code and the rest of your program.
   Variables usually need to be "volatile" types. Volatile tells the compiler to avoid optimizations that assume
   variable can not spontaneously change. Because your function may change variables while your program is using them,
   the compiler needs this hint. But volatile alone is often not enough.

   When accessing shared variables, usually interrupts must be disabled. Even with volatile,
   if the interrupt changes a multi-byte variable between a sequence of instructions, it can be read incorrectly.
   If your data is multiple variables, such as an array and a count, usually interrupts need to be disabled
   or the entire sequence of your code which accesses the data.

   This example will demonstrate the nearly perfect accuracy compared to software timers by printing the actual elapsed millisecs.
   Being ISR-based timers, their executions are not blocked by bad-behaving functions / tasks, such as connecting to WiFi, Internet
   and Blynk services. You can also have many (up to 16) timers to use.
   This non-being-blocked important feature is absolutely necessary for mission-critical tasks.
   You'll see blynkTimer is blocked while connecting to WiFi / Internet / Blynk, and elapsed time is very unaccurate
   In this super simple example, you don't see much different after Blynk is connected, because of no competing task is
   written

   ISR_Timer_Switch demontrates the use of ISR combining with Timer Interrupt to avoid being blocked by
   other CPU-monopolizing task

   In this complex example: CPU is connecting to WiFi, Internet and finally Blynk service (https://docs.blynk.cc/)
   Many important tasks are fighting for limited CPU resource in this no-controlled single-tasking environment.
   In certain period, mission-critical tasks (you name it) could be deprived of CPU time and have no chance
   to be executed. This can lead to disastrous results at critical time.
   We hereby will use interrupt to detect whenever the SW is active, then use a hardware timer to poll and switch
   ON/OFF a sample relay (lamp)
   We'll see this ISR-based operation will have highest priority, preempts all remaining tasks to assure its
   functionality.
*/

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

// Init ESP8266 timer
ESP8266Timer ITimer;
#define TIMER_INTERVAL_MS           100

#define DEBOUNCE_TIME               25
#define LONG_BUTTON_PRESS_TIME_MS   100
#define DEBUG_ISR                   0

#define VPIN                        V70     //V1
#define LAMPSTATE_PIN               V76     //V2

//Blynk Color in format #RRGGBB
#define BLYNK_GREEN     "#23C48E"
#define BLYNK_RED       "#D3435C"

WidgetLED  LampStatus(LAMPSTATE_PIN);

volatile unsigned long  lastDebounceTime  = 0;
volatile bool           buttonPressed     = false;
volatile bool           alreadyTriggered  = false;

volatile bool LampState    = false;
volatile bool SwitchReset  = true;

#define RELAY_PIN     D1
#define BUTTON_PIN    D3

unsigned int myWiFiTimeout        =  3200L;  //  3.2s WiFi connection timeout   (WCT)
unsigned int buttonInterval       =  511L;   //  0.5s update button state

void IRAM_ATTR Falling();
void IRAM_ATTR Rising();

void IRAM_ATTR lightOn();
void IRAM_ATTR lightOff();
void IRAM_ATTR ButtonCheck();
void IRAM_ATTR ToggleRelay();

BlynkTimer Timer;

BLYNK_CONNECTED()
{
  LampStatus.on();
  Blynk.setProperty(LAMPSTATE_PIN, "color", LampState ? BLYNK_RED : BLYNK_GREEN );
  Blynk.syncVirtual(VPIN);
}

// Make this a autoreleased pushbutton
BLYNK_WRITE(VPIN)
{
  if (param.asInt())
    ToggleRelay();
}

void IRAM_ATTR Rising()
{
  unsigned long currentTime  = millis();
  unsigned long TimeDiff;

  TimeDiff = currentTime - lastDebounceTime;
  if ( digitalRead(BUTTON_PIN) && (TimeDiff > DEBOUNCE_TIME) )
  {
    buttonPressed = false;
    lastDebounceTime = currentTime;
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), Falling, FALLING);
  }
}

void IRAM_ATTR Falling()
{
  unsigned long currentTime  = millis();

  if ( !digitalRead(BUTTON_PIN) && (currentTime > lastDebounceTime + DEBOUNCE_TIME))
  {
    lastDebounceTime = currentTime;
    buttonPressed = true;
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), Rising, RISING);
  }
}

void IRAM_ATTR HWCheckButton()
{
  if (!alreadyTriggered && buttonPressed)
  {
    alreadyTriggered = true;
    ButtonCheck();

  }
  else if (!buttonPressed)
  {
    // Reset flag when button released to avoid triggered repeatedly
    alreadyTriggered = false;
    ButtonCheck();
  }
}

void heartBeatPrint()
{
  static int num = 1;

  if (Blynk.connected())
  {
    Serial.print(F("B"));
  }
  else
  {
    Serial.print(F("F"));
  }

  if (num == 40)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    Serial.print(F(" "));
  }
}

void checkButton()
{
  heartBeatPrint();

  if (LampState)
    Blynk.setProperty(LAMPSTATE_PIN, "color", BLYNK_RED);
  else
    Blynk.setProperty(LAMPSTATE_PIN, "color", BLYNK_GREEN);
}

void IRAM_ATTR ButtonCheck()
{
  boolean SwitchState = (digitalRead(BUTTON_PIN));

  if (!SwitchState && SwitchReset)
  {
    ToggleRelay();
    SwitchReset = false;
  }
  else if (SwitchState)
  {
    SwitchReset = true;
  }
}

void IRAM_ATTR ToggleRelay()
{
  if (LampState)
    lightOff();
  else
    lightOn();
}

void IRAM_ATTR lightOn()
{
  digitalWrite(RELAY_PIN, HIGH);
  LampState = true;
}

void IRAM_ATTR lightOff()
{
  digitalWrite(RELAY_PIN, LOW);
  LampState = false;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  
  delay(200);

  Serial.print(F("\nStarting ISR_Timer_Switch on ")); Serial.println(ARDUINO_BOARD);
  Serial.println(ESP8266_TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), Falling, FALLING);

  // Interval in microsecs
  // Interval in microsecs, so MS to multiply by 1000
  // Be sure to place this HW Timer well ahead blocking calls, because it needs to be initialized.
  if (ITimer.attachInterruptInterval(TIMER_INTERVAL_MS * 1000, HWCheckButton))
  {
    Serial.print(F("Starting  ITimer OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));

  unsigned long startWiFi = millis();

  WiFi.begin(ssid, pass);

  do
  {
    delay(200);
    if ( (WiFi.status() == WL_CONNECTED) || (millis() > startWiFi + myWiFiTimeout) )
      break;
  } while (WiFi.status() != WL_CONNECTED);

  Blynk.config(auth, blynk_server, BLYNK_HARDWARE_PORT);
  Blynk.connect();

  if (Blynk.connected())
    Serial.println(F("Blynk connected"));
  else
    Serial.println(F("Blynk not connected yet"));

  Timer.setInterval(buttonInterval, checkButton);
}

void loop()
{
  Blynk.run();
  Timer.run();
}
