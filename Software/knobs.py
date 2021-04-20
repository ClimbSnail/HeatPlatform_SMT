from machine import Pin
import gc
import time

"""
开关编码器功能文件
"""

"""
外部中断mode：
        Pin.IN  输入
        Pin.OUT 输出
      value：输出电平
      Pin.value（[value]）不带参数时是读取输入电平，带参数时是设置输出电平,参数是1/0.
      Pin.irq(*,trigger, handler=None)
中断irq接口
      trigger，触发方式(不指定 默认为上升下降沿)
      Pin.IRQ_FALLING，下降沿
      Pin.IRQ_RISING，上升沿
      Pin.IRQ_RISING | Pin.IRQ_FALLING，上升下降沿
      handler，回调函数
"""


class EC11:
    def __init__(self, pinA_num, pinB_num, pinSw_num=None):
        """
        编码器类的初始化   注：在大多数情况下，只能使用引脚0、2、4、5、12、13、14、15和16
        :param pinA_num: 接收编码器脉冲的引脚A
        :param pinB_num: 接收编码器脉冲的引脚B
        :param pinSw_num: 按键触发的引脚（此为可选参数）
        """
        self.pinA_Status = 0
        self.pinB_Status = 0
        self.pulse_count = 0
        self.switch_status = 0

        # self.cnt_value = 0  # 编码器距离上一轮的脉冲增量
        self.pinA = Pin(pinA_num, Pin.IN, Pin.PULL_UP)
        self.pinA.irq(trigger=Pin.IRQ_RISING | Pin.IRQ_FALLING,
                      handler=self.interruter_funcA)
        self.pinB = Pin(pinB_num, Pin.IN, Pin.PULL_UP)
        self.pinSw = None
        if pinSw_num != None:
            self.pinSw = Pin(pinSw_num, Pin.IN, Pin.PULL_UP)
            self.pinSw.irq(trigger=Pin.IRQ_FALLING,
                           handler=self.interruter_funcSW)
        gc.collect()

    def interruter_funcA(self, events):
        """
        发生外部中断后执行的函数
        :return:
        """
        cnt = 0
        while self.pinA.value() and cnt < 5:
            cnt += 1
        if self.pinA.value() == 1:
            return None

        if self.pinB.value() == 1:
            self.pulse_count += 1
        else:
            self.pulse_count -= 1

        gc.collect()

    def interruter_funcSW(self, events):
        """
        发生外部中断后执行的函数
        :return:
        """
        self.switch_status = 1

    def get_data(self):
        """
        获取本次检测到的脉冲数量
        :return: 距离上一次获取所产生的脉冲数、按键状态
        """
        pulse = self.pulse_count
        self.pulse_count = 0  # clear
        sw_status = self.switch_status
        self.switch_status = 0  # clear
        gc.collect()
        return pulse, sw_status


"""
This is demo
"""


def main():
    import gc
    gc.collect()
    knobs = EC11(4, 5, 0)
    import time
    while True:
        print(knobs.get_data())
        time.sleep(0.2)


if __name__ == "test":
    # if __name__ == '__main__':
    main()
