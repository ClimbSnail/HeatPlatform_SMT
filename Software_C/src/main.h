#ifndef __MAIN_H
#define __MAIN_H

void interrupt_read_temperature();
void interrupt_check_knobs();
void interrupt_ctrl_pid();

void interrupt_save_param();
void interrupt_pwm();


#define NONE_TYPE 0
#define ENCODER_TYPE 1
#define BUTTON_TYPE 2
// 加热板的主控制页面
void main_process(int type, int16_t value = 0);
// 设置页面
void setting_process(int type, int16_t value = 0);

void read_temperature();
void pid_ctrl();
void knobs_deal();


#endif