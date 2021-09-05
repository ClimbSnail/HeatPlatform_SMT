#include <Arduino.h>

#include "ESP8266TimerInterrupt.h"
#include "ESP8266_ISR_Timer.h"

#include "common.h"
#include "max6675.h"
#include "knobs.h"
#include "buzzer.h"
#include "pid.h"
#include "surface.h"
#include "main.h"

MAX6675 max6675(14, 12, 16);
Knobs knobs(4, 5, 0);
Buzzer buzzer(13, 10);
PID pid_contorller(4.5, 0.15, 0.2, 0.2);
Surface surface(3, 1);

// Init ESP8266 timer 0
ESP8266Timer iTimer;
// Init BlynkTimer
ESP8266_ISR_Timer ISR_Timer;
#define HW_TIMER_INTERVAL_MS 1L

#define SWITCH_TIME 1000 // 检测长按的时间
// unsigned long currentMillis;    // 当前按键按下时间
unsigned long previousMillis; // 上次刷新屏幕的时间戳

// 以下为运行时的参数
#define BUFFER_MAX_SIZE 10 //
int buffer_size = 5;       // 缓冲区的实际大小
int buf_index = 0;
int buf_sum = 0; // 需要在setup中初始化赋值
volatile boolean temperature_err_flag = true;
int temperature_buf[BUFFER_MAX_SIZE] = {20, 20, 20, 20, 20,
                                        20, 20, 20, 20, 20};
#define MAX_ERR_TEMPERATURE 20            // 超温度的报警温差
volatile int temperature_setting = 0;     // 设定的目标温度
volatile int temperature_now = 4095;      // 当前温度
double temp_dvalue = 10;                  // PID开始控制的阈值范围
volatile boolean start_read_temp = false; // 标志是否开始读取温度
volatile boolean pid_enable = false;      // 标志pid控制是否开启

KeyInfo knobs_val = {0, 0};            // 按键的值
volatile boolean is_knobs_opt = false; // 按键是否操作

#define SAVE_DELAY_TIME 10             // 延迟保存数据 单位为秒
volatile int save_timeout_counter = 0; // 保存配置超时计数器
volatile boolean save_flag = false;    // 是否需要保存设置

#define PWM_FREQ 5                        // pwm的频率/
#define PWM_PRECISION 200                 // pwm的精度
#define PWM_PIN 2                         // pwm的pin引脚
volatile int out_pwm = PWM_PRECISION - 1; // 输出的PWM 199为最大值
volatile int pwm_count = 0;               // pwm的计数值

int process_index = 0;    // 控制页面
#define PROCESS_MAX_NUM 2 // 页面最大数
void (*pProcessArray[PROCESS_MAX_NUM])(int type, int16_t value) = {
    main_process, setting_process};

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
    if (0 < save_timeout_counter)
    {
        --save_timeout_counter;
    }

    if (1 == save_timeout_counter)
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
    150L,
    200L,
    1000L,
    1000L / PWM_FREQ / PWM_PRECISION};

irqCallback irqCallbackFunc[NUMBER_ISR_TIMERS] = {
    buzzer.timer_handler, interrupt_read_temperature,
    interrupt_check_knobs, interrupt_ctrl_pid,
    interrupt_save_param, interrupt_pwm};

// 加热板的主控制页面
void main_process(int type, int16_t value)
{
    switch (type)
    {
    case NONE_TYPE:
    {
        surface.set_temperature(temperature_setting, temperature_now, out_pwm);
        surface.display_main();
    }
    break;
    case ENCODER_TYPE:
    {
        save_timeout_counter = SAVE_DELAY_TIME;
        int speed = int(fabs(value / 2)) + 1;
        temperature_setting += (speed * value);
        // 限制温度调整范围
        if (temperature_setting < 0)
        {
            temperature_setting = 0;
        }
        if (temperature_setting > 1000)
        {
            temperature_setting = 1000;
        }
    }
    break;
    case BUTTON_TYPE:
    {
        if (value > SWITCH_TIME)
        {
            process_index = (process_index + 1) % PROCESS_MAX_NUM;
        }
    }
    break;
    default:
        break;
    }
}

// 设置页面
void setting_process(int type, int16_t value)
{
    static uint8 setting_info_index = 0;
    static boolean setting_info_choose = false; // 不为选中状态

    switch (type)
    {
    case NONE_TYPE:
    {
        // surface.set_temperature(temperature_setting, temperature_now, out_pwm);
        // surface.display_main();
    }
    break;
    case ENCODER_TYPE:
    {
        if (setting_info_choose)
        {
            // 设置数据
            switch (setting_info_index)
            {
            case 0:
            {
                user_data.save_temperature += value;
            }
            break;
            case 1:
            {
                user_data.mode += value;
            }
            break;
            case 2:
            {
                user_data.direction = (user_data.direction + value) % 2;
            }
            break;
            case 3:
            {
                user_data.err_temperature += value;
            }
            break;
            case 4:
            {
                user_data.kp += (0.1 * value);
            }
            break;
            case 5:
            {
                user_data.ki += (0.1 * value);
            }
            break;
            case 6:
            {
                user_data.kd += (0.1 * value);
            }
            break;
            case 7:
            {
                user_data.kt += value;
            }
            break;
            case 8:
            {
            }
            break;
            case 9:
            {
            }
            break;
            }
        }
        else
        {
            // +10 的作用主要为了让
            setting_info_index = (setting_info_index + 10 + value) % 10;
        }
    }
    break;
    case BUTTON_TYPE:
    {
        if (value > SWITCH_TIME)
        {
            // 切换页面
            process_index = (process_index + 1) % PROCESS_MAX_NUM;
        }
        else
        {
            if (8 == setting_info_index)
            {
                init_user_data();
                return;
            }
            else if (9 == setting_info_index)
            {
                // 保存并重启
                iTimer.disableTimer();
                ISR_Timer.disableAll();
                delay(100);
                save_config();
                ISR_Timer.enableAll();
                iTimer.enableTimer();
                return;
            }
            // 设置当前设置项为选中状态
            setting_info_choose = !setting_info_choose;
        }
    }
    break;
    default:
        break;
    }
    double setting_info_data[8] = {
        user_data.save_temperature, user_data.mode,
        user_data.direction, user_data.err_temperature,
        user_data.kp, user_data.ki, user_data.kd, user_data.kt};

    surface.set_setting(8, setting_info_data, setting_info_index, setting_info_choose);
}

void setup()
{
    // put your setup code here, to run once:
    // Serial.begin(74880);
    // Serial.print(F("Starting OK."));

    for (int pos = 0, buf_sum = 0; pos < buffer_size; ++pos)
    {
        buf_sum += temperature_buf[pos];
    }

    load_config();
    temperature_setting = user_data.save_temperature; // 设定的温度
    pid_contorller.set_pid_param(user_data.kp, user_data.ki, user_data.kd, 0.2);

    // 初始化PWM引脚
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

    buzzer.set_beep_time(700); // 开机提示音
    surface.init();
    // surface.set_temperature(temperature_setting, temperature_now, out_pwm);
}

void read_temperature()
{
    if (false == start_read_temp)
    {
        return;
    }

    start_read_temp = false;
    // user_data.err_temperature 为用户设定的温度误差
    int temp = max6675.read_temperature_mock() - user_data.err_temperature;
    if (temp == 4095)
    {
        temperature_err_flag = true;
        return;
    }
    temperature_err_flag = false;

    buf_sum = buf_sum - temperature_buf[buf_index];
    temperature_buf[buf_index] = temp;
    buf_sum = buf_sum + temperature_buf[buf_index];

    buf_index = (buf_index + 1) % buffer_size;
    temperature_now = buf_sum / buffer_size;
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
        out_pwm = PWM_PRECISION;
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
        out_pwm = PWM_PRECISION; // 最小值输出
        pid_contorller.set_data(0, 0);
        if (temperature_now - temperature_setting > MAX_ERR_TEMPERATURE && temperature_now > 50)
        {
            // 报警提示音
            buzzer.set_beep_time(700);
        }
    }
    else
    {
        if (temperature_setting == temperature_now)
        {
            // 报警
        }
        int ret_out = pid_contorller.get_output(temperature_setting, temperature_now);
        // Serial.print("\nret_out:");
        // Serial.println(ret_out);
        // 以下处理pwm的高低电平反向
        if (ret_out > 0)
        {
            if (ret_out > PWM_PRECISION - 1)
                out_pwm = 0;
            else
                out_pwm = PWM_PRECISION - ret_out;
        }
        else
            out_pwm = PWM_PRECISION;
    }
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
        // 刚开始process_index=0，首页指向控制加热板的控制页面
        if (1 == user_data.direction)
        {
            // user_data.direction 为编码器旋转方向的设置
            knobs_val.pulse_count = -knobs_val.pulse_count;
        }
        pProcessArray[process_index](ENCODER_TYPE, knobs_val.pulse_count);
    }
    if (0 != knobs_val.switch_status)
    {
        if (knobs_val.switch_time > SWITCH_TIME)
        {
            buzzer.set_beep_time(1000);
        }
        else
        {
            buzzer.set_beep_time(200);
        }
        pProcessArray[process_index](BUTTON_TYPE, knobs_val.switch_time);
    }
}

void loop()
{
    read_temperature();
    pid_ctrl();
    knobs_deal();
    if (doDelayMillisTime(500, &previousMillis, false))
    {
        previousMillis = millis();
        // 刚开始process_index=0，首页指向控制加热板的控制页面
        pProcessArray[process_index](NONE_TYPE, 0); // 更新动作
    }
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