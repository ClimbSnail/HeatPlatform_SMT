#include <Arduino.h>

#include "ESP8266TimerInterrupt.h"
#include "ESP8266_ISR_Timer.h"

#include "common.h"
#include "max6675.h"
#include "knobs.h"
#include "buzzer.h"
#include "pid.h"
#include "surface.h"

MAX6675 max6675(14, 12, 16);
Knobs knobs(4, 5, 0);
Buzzer buzzer(13, 10);
PID pid_contorller(0.1, 0.3, 0.1, 0.5);
Surface surface(3, 1);

// Init ESP8266 timer 0
ESP8266Timer iTimer;
// Init BlynkTimer
ESP8266_ISR_Timer ISR_Timer;
#define HW_TIMER_INTERVAL_MS 1L

// 以下为运行时的参数
#define BUFFER_SIZE 10
int buf_index = 0;
int buf_sum = 200;
volatile boolean temperature_err_flag = true;
int temperature_buf[BUFFER_SIZE] = {20, 20, 20, 20, 20,
                                    20, 20, 20, 20, 20};
#define MAX_ERR_TEMPERATURE 20            // 超温度的报警温差
volatile int temperature_setting = 0;     // 设定的目标温度
volatile int temperature_now = 4095;      // 当前温度
double temp_dvalue = 3;                   // PID开始控制的阈值范围
volatile boolean start_read_temp = false; // 标志是否开始读取温度
volatile boolean pid_enable = false;      // 标志pid控制是否开启

KeyInfo knobs_val = {0, 0};            // 按键的值
volatile boolean is_knobs_opt = false; // 按键是否操作

#define SAVE_DELAY_TIME 10          // 延迟保存数据 单位为秒
volatile int save_counter = 0;      // 保存配置计数器
volatile boolean save_flag = false; // 是否需要保存设置

#define PWM_FREQ 5                        // pwm的频率/
#define PWM_PRECISION 200                 // pwm的精度
#define PWM_PIN 2                         // pwm的pin引脚
volatile int out_pwm = PWM_PRECISION - 1; // 输出的PWM 1023为最大值
volatile int pwm_count = 0;               // pwm的计数值

void IRAM_ATTR TimerHandler()
{
    static bool toggle = false;
    static bool started = false;
    static int timeRun = 0;

    ISR_Timer.run();

    // // Toggle LED every LED_TOGGLE_INTERVAL_MS = 2000ms = 2s
    // if (++timeRun == (LED_TOGGLE_INTERVAL_MS / HW_TIMER_INTERVAL_MS))
    // {
    //     timeRun = 0;

    //     if (!started)
    //     {
    //         started = true;
    //         pinMode(LED_BUILTIN, OUTPUT);
    //     }

    //     //timer interrupt toggles pin LED_BUILTIN
    //     digitalWrite(LED_BUILTIN, toggle);
    //     toggle = !toggle;
    // }
}

typedef void (*irqCallback)();

void interrupt_read_temperature()
{
    start_read_temp = true;
}

void interrupt_check_knobs()
{
    is_knobs_opt = true;
}

void interrupt_ctrl_pid()
{
    pid_enable = true;
}

void interrupt_save_param()
{
    if (0 < save_counter)
    {
        --save_counter;
    }

    if (1 == save_counter)
    {
        save_flag = true;
    }
}

void interrupt_pwm()
{
    if (0 == pwm_count)
    {
        digitalWrite(PWM_PIN, HIGH);
    }
    if (out_pwm == pwm_count)
    {
        digitalWrite(PWM_PIN, LOW);
    }
    pwm_count = (pwm_count + 1) % PWM_PRECISION;
}

#define NUMBER_ISR_TIMERS 6

// You can assign any interval for any timer here, in milliseconds
uint32_t TimerInterval[NUMBER_ISR_TIMERS] = {
    buzzer.m_cycle_time,
    200L,
    200L,
    500L,
    1000L,
    1000L / PWM_FREQ / PWM_PRECISION};

irqCallback irqCallbackFunc[NUMBER_ISR_TIMERS] = {
    buzzer.timer_handler, interrupt_read_temperature,
    interrupt_check_knobs, interrupt_ctrl_pid,
    interrupt_save_param, interrupt_pwm};

void setup()
{
    // put your setup code here, to run once:
    // Serial.begin(74880);

    load_config();
    temperature_setting = user_data.save_temperature; // 设定的温度

    pinMode(PWM_PIN, OUTPUT);

    // Interval in microsecs
    bool ret = iTimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS * 1000, TimerHandler);
    // if (true == ret)
    // {
    //     Serial.print(F("Starting ITimer OK."));
    // }
    // else
    //     Serial.println(F("Can't set ITimer. Select another freq. or timer"));

    // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
    // You can use up to 16 timer for each SAMD_ISR_Timer
    for (uint16_t i = 0; i < NUMBER_ISR_TIMERS; i++)
    {
        ISR_Timer.setInterval(TimerInterval[i], irqCallbackFunc[i]);
    }

    buzzer.set_beep_time(700);
    surface.init();
    surface.set_temperature(temperature_setting, temperature_now, out_pwm);
}

void read_temperature()
{
    if (false == start_read_temp)
    {
        return;
    }

    start_read_temp = false;

    int temp = max6675.read_temperature_mock();
    if (temp == 4095)
    {
        temperature_err_flag = true;
        temperature_now = temp;
        surface.set_temperature(temperature_setting, temperature_now, out_pwm);
        return;
    }
    temperature_err_flag = false;

    buf_sum = buf_sum - temperature_buf[buf_index];
    temperature_buf[buf_index] = temp;
    buf_sum = buf_sum + temperature_buf[buf_index];

    buf_index = (buf_index + 1) % BUFFER_SIZE;
    temperature_now = buf_sum / BUFFER_SIZE;
    surface.set_temperature(temperature_setting, temperature_now, out_pwm);
}

void pid_ctrl()
{
    if (false == pid_enable)
    {
        return;
    }

    pid_enable = false;

    if (true == temperature_err_flag)
    {
        // 未接加热板
        String s = "XXXX";
        // md 控制pwm关闭
        // md 页面显示
        // surface.set_temperature(temperature_setting, s.c_str());
        return;
    }
    if (temperature_setting - temperature_now > temp_dvalue)
    {
        // 还未到达PID开始调控的温度范围
        out_pwm = 0; // 最大值输出
        pid_contorller.set_data(3, 0);
    }
    else if (temperature_now > temperature_setting)
    {
        // 当前温度大于实际温度
        out_pwm = PWM_PRECISION - 1; // 最小值输出
        pid_contorller.set_data(0, 0);
        if (temperature_now - temperature_setting > MAX_ERR_TEMPERATURE)
        {
            // 报警
        }
    }
    else
    {
        if (temperature_setting == temperature_now)
        {
            // 报警
        }
        int ret_out = pid_contorller.get_output(temperature_setting, temperature_now);
        out_pwm = ((int)ret_out) * 60;
        // 以下处理pwm的高低电平反向
        if (out_pwm > 0)
        {
            if (out_pwm > PWM_PRECISION - 1)
                out_pwm = 0;
            else
                out_pwm = PWM_PRECISION - 1 - out_pwm;
        }
        else
            out_pwm = PWM_PRECISION - 1;
    }

    // 输出温度
    // analogWrite(2, 127);
}

void knobs_deal()
{
    if (false == is_knobs_opt)
    {
        return;
    }
    
    is_knobs_opt = false;

    knobs_val = knobs.get_data();
    if (0 != knobs_val.pulse_count)
    {
        save_counter = SAVE_DELAY_TIME;
        int speed = int(fabs(knobs_val.pulse_count / 2)) + 1;
        temperature_setting += (speed * knobs_val.pulse_count);
        surface.set_temperature(temperature_setting, temperature_now, out_pwm);
    }
    if (0 != knobs_val.switch_status)
    {
        buzzer.set_beep_time(300);
    }
}

void loop()
{
    read_temperature();
    pid_ctrl();
    knobs_deal();
    if (true == save_flag)
    {
        save_flag = false;
        // 保存设定的温度
        user_data.save_temperature = temperature_setting;

        // iTimer.disableTimer();
        // ISR_Timer.disableAll();
        // delay(100);
        // save_config();
        // ISR_Timer.enableAll();
        // iTimer.enableTimer();
    }
}