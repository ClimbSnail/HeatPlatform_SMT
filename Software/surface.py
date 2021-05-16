from ssd1306_esp import SSD1306_I2C
from machine import I2C
import math
import gc


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
        # 标记当前界面是否需要初始化
        self.is_init_windows = True
        # 用于记录界面中当前运行到哪了，再切换页面时记得清零
        self.now_status = 0
        # 用于记录界面中上一回运行到哪了
        self.history_status = 0
        # 当前选项是否选中
        self.is_choose = True
        self.OLED_STATUS = OLED_STATUS
        if self.OLED_STATUS == True:
            i2c = I2C(scl=scl, sda=sda, freq=400000)
            self.oled = SSD1306_I2C(
                128, 64, i2c, addr=0x3c, external_vcc=False)

    def opt_deal(self, increase=0, enter=0):
        """
        param increase: 为旋钮的增量
        param enter： 为中间的按钮是否按下
        """
        speed = 0
        if increase != 0:
            speed = int(math.fabs(increase/2)) + 1
        self.show(increase*speed, enter)

    def show(self, increase=0, enter=0):
        self.windows_point(increase, enter)
        gc.collect()
        if self.OLED_STATUS == False:
            return None
        self.oled.show()

    """
    以下是主界面的操作方法
    """

    def main(self, increase=0, enter=0):

        if self.OLED_STATUS == True:
            self.set_temperature_target(str(self.temperature_target))
            self.set_temperature_now(str(self.temperature_now))
            self.set_pwm(str(self.pwm))

        op_status_num = 3
        if increase != 0 and self.is_choose == False:
            self.now_status += increase
            if self.now_status < 0:
                self.now_status = 0
            elif self.now_status >= op_status_num:
                self.now_status = op_status_num-1
        
        if self.is_init_windows == True:
            self.is_init_windows = False
            self.draw_main_panel()
            # 恢复上一回状态
            self.now_status = self.history_status
        
        if self.now_status == 0:
            if increase != 0 and self.is_choose == True:
                self.temperature_target = self.temperature_target + increase
                # 将相关信息同步到主控制器中
                self.conterller.temp_target = self.temperature_target
                self.conterller.beep_flag2 = True
            if enter == 1:  # 进入菜单选择状态
                if self.is_choose == True:
                    self.is_choose = False
                    UnCursorImage_8x8 = b"\xFF\xC3\x42\x66\x24\x3C\x18\x18"
                    self.oled.img_8x8(UnCursorImage_8x8, 92, 28)
                else:
                    # 进入运行状态
                    self.is_choose = True
                    CursorImage_8x8 = b"\xFF\xFF\x7E\x7E\x3C\x3C\x18\x18"
                    self.oled.img_8x8(CursorImage_8x8, 92, 28)
        elif enter == 1:  # 进入菜单选择状态
            if self.now_status == 1:
                # 初始的界面指针指向main页面
                self.windows_point = self.heat_plate_setting
                # 跳转前的状态标记
                self.history_status = self.now_status
                self.now_status = 0
                self.is_choose == False
                self.is_init_windows = True
            elif self.now_status == 2:
                self.history_status = self.now_status
                self.now_status = 0
                self.is_choose == False
                self.is_init_windows = True

        self.set_pwm(str(self.now_status))
        # 绘制滑动选项
        HeatImage = b"\x00\xC0\xE0\xB8\x88\x88\x88\x88\x88\x88\x88\xC8\x78\x18\x08\x00\x00\x30\x38\x2E\x22\x22\x22\x22\x22\x22\x32\x1A\x0E\x02\x00\x00"
        RunImage = b"\x00\x80\xC0\xE0\xF8\xFF\xFE\xFC\x70\x00\x00\xC0\x80\x00\x00\x00\x00\x1F\x3F\x7F\x9F\x07\x03\xE1\xF0\xF8\xFE\xFF\xFF\x7F\x3C\x00"
        CurveImage = b"\x00\x00\x00\x80\xC0\x30\x30\xE0\x00\x00\x80\xC0\x60\x30\x0C\x06\x1C\x06\x03\x01\x00\x00\x00\x01\x07\x07\x01\x00\x00\x00\x00\x00"
        # SettingImage = b"\x80\xC0\xDC\xFC\xFC\x38\x1E\x1F\x1F\x1E\x38\xFC\xFC\xDC\xC0\x80\x01\x03\x3B\x3F\x3F\x1C\x78\xF8\xF8\x78\x1C\x3F\x3F\x3B\x03\x01"
        op_list = [CurveImage, RunImage, HeatImage]
        op_size = len(op_list)
        op_offsize = self.now_status
        op_list = op_list[op_offsize:op_size] + \
            op_list[0:op_offsize]
        for num in range(op_size):
            self.oled.img_16x16(op_list[num], 106, 3+17*num)

    def draw_main_panel(self):
        if self.OLED_STATUS == False:
            return None
        weigth = 2
        self.oled.fill_rect(0, 0, 126, 62, 0)
        # 绘制矩形框
        self.oled.fill_rect(0, 0, 128, weigth, 1)
        self.oled.fill_rect(0, 0, weigth, 64, 1)
        self.oled.fill_rect(126, 0, weigth, 64, 1)
        self.oled.fill_rect(0, 62, 128, weigth, 1)
        # 绘制光标
        if self.is_choose == True:
            CursorImage_8x8 = b"\xFF\xFF\x7E\x7E\x3C\x3C\x18\x18"
            self.oled.img_8x8(CursorImage_8x8, 92, 28)
        else:
            UnCursorImage_8x8 = b"\xFF\xC3\x42\x66\x24\x3C\x18\x18"
            self.oled.img_8x8(UnCursorImage_8x8, 92, 28)

    def set_temperature_target(self, dat_str):
        self.oled.text("SET: ", 3, 3)
        self.oled.fill_rect(30, 3, 4*8, 8, 0)
        self.oled.text(dat_str, 30, 3)

    def set_temperature_now(self, dat_str):
        self.oled.fill_rect(30, 24, 4*8, 16, 0)
        self.oled.text_num(dat_str, 30, 24)

    def set_pwm(self, dat_str):
        self.oled.text("PWM: ", 3, 52)
        self.oled.fill_rect(30, 52, 4*8, 8, 0)
        self.oled.text(dat_str, 30, 52)

    """
    以下是加热板的设置界面
    """

    def heat_plate_setting(self, increase=0, enter=0):
        op_status_num = 3
        if increase != 0 and self.is_choose == False:
            self.now_status += increase
            if self.now_status < 0:
                self.now_status = 0
            elif self.now_status >= op_status_num:
                self.now_status = op_status_num-1
        
        if self.is_init_windows == True:
            # init windows
            self.is_init_windows = False
            self.draw_heat_plate()
        
        if self.now_status == 0:
            if enter == 1:  # 进入菜单选择状态
                if self.is_choose == True:
                    self.is_choose = False
                    UnCursorImage_8x8 = b"\xFF\xC3\x42\x66\x24\x3C\x18\x18"
                    self.oled.img_8x8(UnCursorImage_8x8, 92, 28)
                else:
                    # 进入运行状态
                    self.is_choose = True
                    CursorImage_8x8 = b"\xFF\xFF\x7E\x7E\x3C\x3C\x18\x18"
                    self.oled.img_8x8(CursorImage_8x8, 92, 28)
        elif self.now_status == 1:
            pass
        elif self.now_status == 2:
            if enter == 1:  # 按下返回
                self.now_status = 0
                self.windows_point = self.main
                self.is_init_windows = True
        
        self.set_pwm(str(10+self.now_status))

    def draw_heat_plate(self):
        if self.OLED_STATUS == False:
            return None
        weigth = 2
        self.oled.fill_rect(2, 2, 124, 60, 0)
        # 绘制光标
        CursorImage_8x8 = b"\xFF\xFF\x7E\x7E\x3C\x3C\x18\x18"
        self.oled.img_8x8(CursorImage_8x8, 92, 28)
