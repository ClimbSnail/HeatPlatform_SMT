# MicroPython SSD1306 OLED driver, I2C and SPI interfaces
from micropython import const
import time
import framebuf
import sys

# register definitions
SET_CONTRAST = const(0x81)
SET_ENTIRE_ON = const(0xa4)
SET_NORM_INV = const(0xa6)
SET_DISP = const(0xae)
SET_MEM_ADDR = const(0x20)
SET_COL_ADDR = const(0x21)
SET_PAGE_ADDR = const(0x22)
SET_DISP_START_LINE = const(0x40)
SET_SEG_REMAP = const(0xa0)
SET_MUX_RATIO = const(0xa8)
SET_COM_OUT_DIR = const(0xc0)
SET_DISP_OFFSET = const(0xd3)
SET_COM_PIN_CFG = const(0xda)
SET_DISP_CLK_DIV = const(0xd5)
SET_PRECHARGE = const(0xd9)
SET_VCOM_DESEL = const(0xdb)
SET_CHARGE_PUMP = const(0x8d)


class SSD1306:
    def __init__(self, width, height, external_vcc):
        self.width = width
        self.height = height
        self.external_vcc = external_vcc
        self.pages = self.height // 8
        self.buffer = bytearray(self.pages * self.width)
        self.framebuf = framebuf.FrameBuffer(
            self.buffer, self.width, self.height, framebuf.MVLSB)
        self.poweron()
        self.init_display()
        self.num_list = [b'\x00\xE0\x10\x08\x08\x10\xE0\x00\x00\x0F\x10\x20\x20\x10\x0F\x00',  # 0
                         b'\x00\x00\x10\x10\xF8\x00\x00\x00\x00\x00\x20\x20\x3F\x20\x20\x00',
                         b'\x00\x70\x08\x08\x08\x08\xF0\x00\x00\x30\x28\x24\x22\x21\x30\x00',
                         b'\x00\x30\x08\x08\x08\x88\x70\x00\x00\x18\x20\x21\x21\x22\x1C\x00',
                         b'\x00\x00\x80\x40\x30\xF8\x00\x00\x00\x06\x05\x24\x24\x3F\x24\x24',
                         b'\x00\xF8\x88\x88\x88\x08\x08\x00\x00\x19\x20\x20\x20\x11\x0E\x00',
                         b'\x00\xE0\x10\x88\x88\x90\x00\x00\x00\x0F\x11\x20\x20\x20\x1F\x00',
                         b'\x00\x18\x08\x08\x88\x68\x18\x00\x00\x00\x00\x3E\x01\x00\x00\x00',
                         b'\x00\x70\x88\x08\x08\x88\x70\x00\x00\x1C\x22\x21\x21\x22\x1C\x00',
                         b'\x00\xF0\x08\x08\x08\x10\xE0\x00\x00\x01\x12\x22\x22\x11\x0F\x00',  # 9
                         b'\x08\x18\x68\x80\x80\x68\x18\x08\x20\x30\x2C\x03\x03\x2C\x30\x20',  # X
                         b'\x00\x00\x00\xF8\x00\x00\x00\x00\x00\x00\x00\x33\x00\x00\x00\x00'  # !
                         ]

    def init_display(self):
        for cmd in (
            SET_DISP | 0x00,  # off
            # address setting
            SET_MEM_ADDR, 0x00,  # horizontal
            # resolution and layout
            SET_DISP_START_LINE | 0x00,
            SET_SEG_REMAP | 0x01,  # column addr 127 mapped to SEG0
            SET_MUX_RATIO, self.height - 1,
            SET_COM_OUT_DIR | 0x08,  # scan from COM[N] to COM0
            SET_DISP_OFFSET, 0x00,
            SET_COM_PIN_CFG, 0x02 if self.height == 32 else 0x12,
            # timing and driving scheme
            SET_DISP_CLK_DIV, 0x80,
            SET_PRECHARGE, 0x22 if self.external_vcc else 0xf1,
            SET_VCOM_DESEL, 0x30,  # 0.83*Vcc
            # display
            SET_CONTRAST, 0xff,  # maximum
            SET_ENTIRE_ON,  # output follows RAM contents
            SET_NORM_INV,  # not inverted
            # charge pump
            SET_CHARGE_PUMP, 0x10 if self.external_vcc else 0x14,
                SET_DISP | 0x01):  # on
            self.write_cmd(cmd)
        self.fill(0)
        self.show()

    def poweroff(self):
        self.write_cmd(SET_DISP | 0x00)

    def contrast(self, contrast):  # 调整亮度。0最暗，255最亮
        self.write_cmd(SET_CONTRAST)
        self.write_cmd(contrast)

    def invert(self, invert):  # 奇数时反相显示，偶数时正常显示
        self.write_cmd(SET_NORM_INV | (invert & 1))

    # 更新显示内容。大部分函数只是写入数据到缓冲区，并不会直接显示到屏幕，需要调用show()后才能显示出来。
    def show(self):
        x0 = 0
        x1 = self.width - 1
        if self.width == 64:
            # displays with width of 64 pixels are shifted by 32
            x0 += 32
            x1 += 32
        self.write_cmd(SET_COL_ADDR)
        self.write_cmd(x0)
        self.write_cmd(x1)
        self.write_cmd(SET_PAGE_ADDR)
        self.write_cmd(0)
        self.write_cmd(self.pages - 1)
        self.write_data(self.buffer)

    def fill(self, col):  # n=0，清空屏幕，n大于0，填充屏幕
        self.framebuf.fill(col)

    def pixel(self, x, y, col):  # 在(x, y)处画点
        self.framebuf.pixel(x, y, col)

    def scroll(self, dx, dy):  # 滚屏
        self.framebuf.scroll(dx, dy)

    def text(self, string, x, y, col=1):  # 在(x, y)处显示字符串，注意text()函数内置的字体是8x8的，暂时不能替换
        self.framebuf.text(string, x, y, col)

    def text_num(self, string, x, y, col=1):  # 在(x, y)处显示字符串，注意text()函数内置的字体是8x16的，暂时不能替换
        x0 = x % 8
        x1 = x
        y0 = y % 8  # 34 2
        y1 = y//8*128  # 34 4
        index = 0

        for s in string:
            if s == 'X':
                index = 10
            elif s == '!':
                index = 11
            else:
                index = int(s)

            start = y1+x1
            for num in range(8):
                self.buffer[start+num] = (self.buffer[start+num] <<
                                          (8-y0) >> (8-y0)) | (self.num_list[index][num] << y0)
                # buffer低y0位  |  num_list低8-y0位

            start = y1+128+x1
            for num in range(8):
                self.buffer[start+num] = (self.num_list[index][num]
                                          >> (8-y0)) | (self.num_list[index][num+8] << y0)
                # num_list高y0位  |  num_list低8-y0位

            start = y1+256+x1
            x1 = x1+8
            if start >= 8*128:
                continue
            for num in range(8):
                self.buffer[start+num] = (self.num_list[index][num+8]
                                          >> (8-y0)) | (self.buffer[start+num] << y0)

    def img_16x16(self, img, x, y, col=1):  # 在(x, y)处显示字符串，注意text()函数内置的字体是8x16的，暂时不能替换
        x0 = x % 8
        x1 = x
        y0 = y % 8  # 34 2
        y1 = y//8*128  # 34 4
        index = 0

        start = y1+x1
        for num in range(16):
            self.buffer[start+num] = (self.buffer[start+num]
                                      << (8-y0) >> (8-y0)) | (img[num] << y0)
            # buffer低y0位  |  num_list低8-y0位
        start = y1+128+x1
        for num in range(16):
            self.buffer[start+num] = (img[num] >> (8-y0)) | (img[num+16] << y0)
            # num_list高y0位  |  num_list低8-y0位

        start = y1+256+x1
        if start >= 8*128:
            return
        for num in range(16):
            self.buffer[start+num] = (img[num+16] >>
                                      (8-y0)) | (self.buffer[start+num] << y0)

    def hline(self, x, y, w, col):  # 按宽度填充(直线)
        self.framebuf.hline(x, y, w, col)

    def vline(self, x, y, h, col):  # 按高度填充(直线)
        self.framebuf.vline(x, y, h, col)

    def line(self, x1, y1, x2, y2, col):  # 按（起终）对角线范围填充(直线)
        self.framebuf.line(x1, y1, x2, y2, col)

    def rect(self, x, y, w, h, col):  # 按起始点与长宽限定范围画框（空心矩形）
        self.framebuf.rect(x, y, w, h, col)

    def fill_rect(self, x, y, w, h, col):  # 按起始点与长宽限定范围画框并填充(矩形)
        self.framebuf.fill_rect(x, y, w, h, col)

    def blit(self, fbuf, x, y):
        self.framebuf.blit(fbuf, x, y)


class SSD1306_I2C(SSD1306):
    def __init__(self, width, height, i2c, addr=0x3c, external_vcc=False):
        self.i2c = i2c
        self.addr = addr
        self.temp = bytearray(2)
        super().__init__(width, height, external_vcc)

    def write_cmd(self, cmd):
        self.temp[0] = 0x80  # Co=1, D/C#=0
        self.temp[1] = cmd
        self.i2c.writeto(self.addr, self.temp)

    def write_data(self, buf):
        self.temp[0] = self.addr << 1
        self.temp[1] = 0x40  # Co=0, D/C#=1
        self.i2c.start()
        self.i2c.write(self.temp)
        self.i2c.write(buf)
        self.i2c.stop()

    def poweron(self):
        pass


class SSD1306_SPI(SSD1306):
    def __init__(self, width, height, spi, dc, res, cs, external_vcc=False):
        self.rate = 10 * 1024 * 1024
        dc.init(dc.OUT, value=0)
        res.init(res.OUT, value=0)
        cs.init(cs.OUT, value=1)
        self.spi = spi
        self.dc = dc
        self.res = res
        self.cs = cs
        super().__init__(width, height, external_vcc)

    def write_cmd(self, cmd):
        self.spi.init(baudrate=self.rate, polarity=0, phase=0)
        self.cs.high()
        self.dc.low()
        self.cs.low()
        self.spi.write(bytearray([cmd]))
        self.cs.high()

    def write_data(self, buf):
        self.spi.init(baudrate=self.rate, polarity=0, phase=0)
        self.cs.high()
        self.dc.high()
        self.cs.low()
        self.spi.write(buf)
        self.cs.high()

    def poweron(self):
        self.res.high()
        time.sleep_ms(1)
        self.res.low()
        time.sleep_ms(10)
        self.res.high()


"""
class Interface():
  def __init__(self, scl, sda):
    i2c = I2C(scl=scl, sda=sda, freq=400000)
    self.oled = SSD1306_I2C(128, 64, i2c, addr=0x3c, external_vcc=False)
    self.set_draw_panel()
    self.show()
	
  def set_draw_panel(self):
    weigth = 2
    self.oled.fill_rect(0, 0, 128, weigth, 1)
    self.oled.fill_rect(0, 0, weigth, 64, 1)
    self.oled.fill_rect(126, 0, weigth, 64, 1)
    self.oled.fill_rect(0, 62, 128, weigth, 1)
    #self.oled.fill_rect(0,  31, 96, 2, 1)
    
  def set_temperature_target(self, dat_str):
    self.oled.text("SET: ", 3, 3)
    self.oled.fill_rect(30, 3, 4*8, 8, 0)
    self.oled.text(dat_str, 30, 3)
        
  def set_temperature_now(self, dat_str):
    self.oled.text("Now: ", 3, 20)
    self.oled.fill_rect(30, 20, 4*8, 8, 0)
    self.oled.text(dat_str, 30, 20)
     
  def set_pwm(self, dat_str):
    self.oled.text("PWM: ", 3, 52)
    self.oled.fill_rect(30, 52, len(dat_str)*8, 8, 0)
    self.oled.text(dat_str, 30, 52)

  def show(self):
    self.oled.show()
"""

"""
This is demo
"""


def main():
    import gc
    gc.collect()
    from machine import Pin,  I2C
    i2c = I2C(scl=Pin(5), sda=Pin(4), freq=100000)
    #i2c = I2C(scl=Pin(1), sda=Pin(3), freq=100000)
    oled = SSD1306_I2C(128, 64, i2c, addr=0x3c, external_vcc=False)
    oled.text_num("12345", 64, 34)
    oled.show()
    while True:
        oled.text("Hello PYB Nano", 0, 0)
        oled.show()
        time.sleep(1)
        oled.text("Nano PYB Hello", 0, 0)
        oled.show()
        time.sleep(1)


if __name__ == "test":
    # if __name__ == '__main__':
    main()
