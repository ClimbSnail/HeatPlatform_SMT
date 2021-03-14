"""
热电偶测温
"""

from machine import SPI, Pin
import time
import gc

MAX6675_CONNECT = 0x04

# SPI API
# SPI.read(len, data=0x00) # 读取len个数据的同时写入len个data数据，以数组的形式返回读取到的数据。
# SPI.readinto(buf, data=0x00)	# 读取buf.len个数据并存入buf中，同时写入buf.len个data数据，函数返回None。
# SPI.write(buf)	# 将 buf 中的所有数据写入到总线。 
# SPI.write_readinto(write_buf, read_buf) # 写入write_buf并读取到 read_buf，写入并读取的长度为buf长度，要求两个缓冲区长度相同。

class MAX6675:
  def __init__(self, sck, mosi, miso, cs):
    '''
    self.spi = SPI(1, baudrate=200000, polarity=0, phase=0)
    self.spi.init(baudrate=200000) # set the baudrate
    # (SPI(0) 仅用于内部的 FlashROM。)
    '''
    self.cs = cs
    self.cs.on()  # 使能引脚拉高self.cs = cs
    self.spi = SPI(1, baudrate=100000, polarity=1, phase=0)
    #self.spi = SPI(-1, baudrate=1000000, polarity=1,
    #        phase=0, sck=sck, mosi=mosi, miso=miso)
    self.connect = False

  def read_temperature(self, ):
    """
    读取一次温度（摄氏度），读取错误或者热电偶异常均返回4095
    """
    self.cs.off() # CS引脚检测到下降沿后停止转换。随后在sck时钟控制下输出16位数据
    value = self.spi.read(2)	# 读取两个字节
    self.cs.on()
    
    temp = value[0]<<8|value[1]
    print(value[0], value[1])
    self.connect = True if (temp & MAX6675_CONNECT) == 0 else False
    gc.collect()
    if self.connect == True:  # 没检测到电热偶
        return ((temp&0x7FFF)>>3)>>2	# 测得的温度单位是0.25，所以要乘以0.25（即除以4）
    else:
        return 4095
  
  
  def __del__(self, ):
    self.spi.deinit()   # 关闭spi
    


def main():
  import gc
  gc.collect()
  max6675 = MAX6675(Pin(0), Pin(2), Pin(13), Pin(16, Pin.OUT))
  while True:
    val = max6675.read_temperature()
    print(val)
    time.sleep(1)
  

"""
This is demo
"""
if __name__ == "test":
#if __name__ == "__main__":
  main()













