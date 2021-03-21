

import network
import time
from wifi import WifiController
from knobs import EC11
from max6675 import MAX6675
from ssd1306_esp import SSD1306_I2C
from pid import PID
import webrepl
from machine import  Pin, PWM, I2C
import machine
from machine import Timer
import gc

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

  def show(self):
    self.oled.show()

class HeatPlatform():
  """
  加热平台控制主类
  """
  def __init__(self, config_dict, LED_Status = True):
    self.cfg = config_dict # 配置文件信息
    self.pwm_out = PWM(Pin(2), freq=5, duty=1023)
    
    self.max6675 = MAX6675(None, None, None, Pin(16, Pin.OUT))
    # max6675使用的是硬件SPI，会设置GPIO13引脚，若使用GPIO13（蜂鸣器）请后初始化
    
    self.beep = Pin(13, Pin.OUT)    # 蜂鸣器引脚
    self.beep.value(1)      # 关闭蜂鸣器
    self.beep_flag1 = True
    self.beep_flag2 = False
    
    self.buf_len = 10
    self.buf_index = 0
    self.buf_sum = 0
    self.temperature_err_flag = True
    self.temperature_buf = [20] * self.buf_len
    
    self.temp_now = self.max6675.read_temperature()
    self.temp_target = 50
    self.temp_DValue = 3
    self.pid_enable = False
    gc.collect()
    
    # 创建OLED显示器
    # construct an I2C bus，软件i2c所有接口都适用
    self.LED_STATUS = LED_Status
    if self.LED_STATUS == True:
      self.interface = Interface(scl=Pin(1), sda=Pin(3))
      self.interface.set_temperature_now( str(self.temp_now) )
      self.interface.set_temperature_target( str(self.temp_target) )
      self.interface.show()
      
    
    # 创建编码器控制对象
    self.knobs = EC11(4, 5, 0)
    
    # 创建pid控制对象
    # self.pid_contorller = PID(0.7, 0.4, 0.25, 0.5)
    self.pid_contorller = PID(0.1, 0.3, 0.1, 0.5)
    
    
    # 创建温度读取的定时器
    self.beep_timer = Timer(-1)  # periodic with 100ms period
    # mode: Timer.ONE_SHOT / Timer.PERIODIC
    self.beep_timer.init(period=500, mode=Timer.PERIODIC, callback=self.short_beep)
    
    # 创建温度读取的定时器
    self.temperature_timer = Timer(-1)  # periodic with 100ms period
    # mode: Timer.ONE_SHOT / Timer.PERIODIC
    self.temperature_timer.init(period=200, mode=Timer.PERIODIC, callback=self.get_temperature)
    
    # 创建按键检测的定时器
    self.knobs_timer = Timer(-1)  # periodic with 100ms period
    # mode: Timer.ONE_SHOT / Timer.PERIODIC
    self.knobs_timer.init(period=1000, mode=Timer.PERIODIC, callback=self.check_knobs)
    
    # 创建pid调节的定时器
    self.adjust_timer = Timer(-1)
    # mode: Timer.ONE_SHOT / Timer.PERIODIC
    self.adjust_timer.init(period=500, mode=Timer.PERIODIC, callback=self.adjust_temperature)
    
    gc.collect()
  
  def get_temperature(self, timer):
    temp = self.max6675.read_temperature()
    if temp == 4095:
      self.temperature_err_flag = True
      return None
    else:
      self.temperature_err_flag = False
    self.buf_sum = self.buf_sum - self.temperature_buf[self.buf_index]
    self.temperature_buf[self.buf_index] = temp
    self.buf_sum = self.buf_sum + self.temperature_buf[self.buf_index]
    self.buf_index = (self.buf_index+1)%self.buf_len
  
  def check_knobs(self, timer):
    increase = self.knobs.get_data()
    # increase为一个长度为2的元组 编码脉冲+按键是否按下 
    #if increase[1] == 1:
    if increase[0] != 0:
      self.temp_target = self.temp_target + increase[0]
      self.beep_flag = True
      
      if self.LED_STATUS == True:
        self.interface.set_temperature_target( str(self.temp_target) )
        self.interface.show()
      
  def short_beep(self, timer):
    if self.beep_flag1 == True and self.beep_flag2 == True:
      self.beep_flag1 = False
      self.beep.value(0)
      time.sleep(1)
      self.beep.value(1)
    
  def adjust_temperature(self, timer):
    self.temp_now = int(self.buf_sum/self.buf_len)
    str_temp_now = str(self.temp_now)
    pwm_val = None  # 输出的PWM值
    print("\ntemp_now = "+str_temp_now)
    if self.temp_target - self.temp_now > self.temp_DValue:
      pwm_val = 0
      self.pid_enable = False
      self.beep_flag2 = False
      self.pid_contorller.set_data(3, 0)
    elif self.temp_now > self.temp_target:
      pwm_val = 1023
      self.pid_enable = False
      self.pid_contorller.set_data(0, 0)
    else: 
      if  self.temp_target == self.temp_now:
        self.beep_flag2 = True
      if self.pid_enable == False:
        self.pid_enable = True
      out_val = self.pid_contorller.get_output(self.temp_target, self.temp_now)
    
      print("str_out = "+str(out_val))
      pwm_val = int(out_val)*60
      if pwm_val>0:
        if pwm_val>1020:
          pwm_val = 0
        else:
          pwm_val = 1023-pwm_val
      else:
        pwm_val = 1023
    self.pwm_out.duty(pwm_val)
    print("pwm_val = "+str(pwm_val))
    
    if self.LED_STATUS == True:
      if self.temperature_err_flag == True:
        str_temp_now = "XXXX"
      self.interface.set_temperature_now(str_temp_now)
      self.interface.set_pwm(str(pwm_val))
      self.interface.show()
    

  def __del__(self):
    self.temperature_timer.deinit()
    self.knobs_timer.deinit()
    self.adjust_timer.deinit()
    print("Stop HeatPlatform")
    
    

class MyBoot():
  def __init__(self, config_dict):
    self.cfg = config_dict # 配置文件信息
    webrepl.start(password = self.cfg['webrepl-password'])
    # self.wifi = WifiController()
    # self.connect(self.cfg)
    # self.disconnect(self.cfg)
    # create and configure in one go
  
  def disconnect(self, cfg_dict):
    ret = self.wifi.get_valid_bs_list(["orangepi"])
    print(ret)
    self.wifi.close_sta()
    print("close_sta succ")
    self.wifi.close_ap()
    print("close_ap succ")
      
  def connect(self, cfg_dict):
    ret = self.wifi.get_valid_bs_list(["orangepi"])
    print(ret)
    #ret = conn.do_sta("Test", "88888888")
    ret = self.wifi.do_sta(cfg_dict["sta-essid"], cfg_dict["sta-password"])
    if ret == True:
      print("do_sta succ")
    ret = self.wifi.create_ap (cfg_dict["ap-essid"], cfg_dict["ap-password"])
    if ret == True:
      print("do_ap succ")
    
def is_ADC_dress():
  from machine import ADC
  adc = ADC(0)            # create ADC object on ADC pin
  if  adc.read()<512:
    return False
  else:
    return True


"""
This is demo
"""
#if __name__ == "test":
if __name__ == '__main__':
  import gc
  gc.collect()
  import json
  cfg = None
  with open('config.json', 'r') as f:
    cfg = json.loads(f.read())
  LED_STATUS = is_ADC_dress()
  # LED_STATUS = False
  print(LED_STATUS)
  hp = HeatPlatform(cfg, LED_STATUS)
  #boot = MyBoot(cfg)
  # os.remove('wifi_config.json') # 鍒犻櫎鏂囦欢
  #webrepl.start(password = cfg['webrepl-password'])
  while True:
    time.sleep(5)
   
   



