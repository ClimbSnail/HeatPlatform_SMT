import gc


class PID:
    def __init__(self, Kp, Ki, Kd, Dt):
        """
        PID类的初始化并传递必要的参数
        :param Kp: 比例参数 浮点数
        :param Ki: 积分参数 浮点数
        :param Kd: 微分参数 浮点数
        :param Dt: PID调节周期  浮点数（s）
        """
        self.Kp = Kp
        self.Ki = Ki
        self.Kd = Kd
        self.Dt = Dt
        self.previous_error = 0
        self.integral = 0

    def set_pidparam(self, Kp, Ki, Kd, Dt):
        """
        控制过程中PID参数的设定
        :param Kp: 比例参数 浮点数
        :param Ki: 积分参数 浮点数
        :param Kd: 微分参数 浮点数
        :param Dt: PID调节周期  浮点数（s）
        :return: None
        """
        self.Kp = Kp
        self.Ki = Ki
        self.Kd = Kd
        self.Dt = Dt

    def set_data(self, pre_error, integral=0):
        """
        清空数据
        """
        self.previous_error = pre_error
        self.integral = integral

    def get_output(self, target_value, now_value):
        """
        获取将要调节的控制量
        :param target_value: 目标值
        :param now_value: 当前监测到的值
        :return: 调节量
        """
        error = target_value - now_value
        self.integral = self.integral + error * self.Dt
        derivative = (error - self.previous_error) / self.Dt
        output = self.Kp*error + self.Ki*self.integral + self.Kd*derivative
        self.previous_error = error
        gc.collect()
        return output

    def __del__(self, ):
        pass


class Auto_PID(PID):
    pass


"""
This is demo
"""


def main():
    import gc
    gc.collect()
    from machine import Timer

    pid_contorller = PID(0.1, 0.1, 0.1, 0.01)

    tim = Timer(-1)
  # mode: Timer.ONE_SHOT / Timer.PERIODIC
    tim.init(period=2000, mode=Timer.PERIODIC, callback=lambda t: print(2))
    while True:
        output_val = pid_contorller.get_output(100, 200)


if __name__ == "test":
    # if __name__ == '__main__':
    main()
