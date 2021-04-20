from ssd1306_esp import SSD1306_I2C
from machine import I2C
import math


class Interface():
    def __init__(self, conterller, scl, sda, OLED_STATUS):
        """
        界面的初始化
        param: scl为IIC的scl
        param: sda为IIC的sda
        """
        self.conterller = conterller
        # 当前温度需要通过conterller中赋值
        self.temperature_now = 0
        # 设定的温度需要赋值到conterller中
        self.temperature_target = 0
        self.pwm = 0
        # 初始的界面指针指向main页面
        self.windows_point = self.main
        self.OLED_STATUS = OLED_STATUS
        if self.OLED_STATUS == True:
            #print("Init IIC\n")
            i2c = I2C(scl=scl, sda=sda, freq=400000)
            self.oled = SSD1306_I2C(
                128, 64, i2c, addr=0x3c, external_vcc=False)
        self.set_draw_panel()
        self.show()

    def set_draw_panel(self):
        if self.OLED_STATUS == False:
            return None
        weigth = 2
        self.oled.fill_rect(0, 0, 128, weigth, 1)
        self.oled.fill_rect(0, 0, weigth, 64, 1)
        self.oled.fill_rect(126, 0, weigth, 64, 1)
        self.oled.fill_rect(0, 62, 128, weigth, 1)
        tu = b"\x00\x80\xC0\xE0\xF8\xFF\xFE\xFC\x70\x00\x00\xC0\x80\x00\x00\x00\x00\x1F\x3F\x7F\x9F\x07\x03\xE1\xF0\xF8\xFE\xFF\xFF\x7F\x3C\x00"
        self.oled.img_16x16(tu, 107, 3)
        tu = b"\x00\x00\x00\x80\xE0\xF8\xFC\x0E\x0E\xFC\xF8\xE0\x80\x00\x00\x00\x00\x38\x7E\x7F\x7F\x7F\x7F\x66\x66\x7F\x7F\x7F\x7F\x7E\x38\x00"
        self.oled.img_16x16(tu, 107, 20)
        #tu = b"\x00\x00\x00\x80\xE0\xF8\xFC\x0E\x0E\xFC\xF8\xE0\x80\x00\x00\x00\x00\x38\x7E\x7F\x7F\x7F\x7F\x66\x66\x7F\x7F\x7F\x7F\x7E\x38\x00"
        #self.oled.img_16x16(tu, 118, 40)
        #self.oled.fill_rect(0,  31, 96, 2, 1)

    def set_temperature_target(self, dat_str):
        self.oled.text("SET: ", 3, 3)
        self.oled.fill_rect(30, 3, 4*8, 8, 0)
        self.oled.text(dat_str, 30, 3)

    def set_temperature_now(self, dat_str):
        self.oled.fill_rect(30, 24, 4*8, 16, 0)
        self.oled.text_num(dat_str, 30, 24)

    def set_pwm(self, dat_str):
        self.oled.text("PWM: ", 3, 52)
        self.oled.fill_rect(30, 52, len(dat_str)*8, 8, 0)
        self.oled.text(dat_str, 30, 52)

    def show(self, increase=None, enter=None):
        if self.OLED_STATUS == False:
            return None
        self.windows_point(increase, enter)
        self.oled.show()

    def opt_deal(self, increase, enter):
        """
        param increase: 为旋钮的增量
        param enter： 为中间的按钮是否按下
        """
        speed = 0
        if increase != 0:
            speed = int(math.fabs(increase/2)) + 1
        self.show(increase*speed, enter)

    def main(self, increase, enter):
        if increase != None and isinstance(self.temperature_target, str):
            self.temperature_target = self.temperature_target + increase
            # 将相关信息同步到主控制器中
            self.conterller.temp_target = self.temperature_target
            self.conterller.beep_flag = True
        if enter != None:
            pass
        self.set_temperature_target(str(self.temperature_target))
        self.set_temperature_now(str(self.temperature_now))
        self.set_pwm(str(self.pwm))
