# HeatPlatform_SMT
DIY个人简易的SMT平板加热台

![HeatPlatform_Entity](https://gitee.com/ClimbSnailQ/Project_Image/raw/master/Note/HeatPlatform_Entity.jpg)

![HeatPlatform_Entity](Image/HeatPlatform_Entity.jpg)

#### 项目起因

平板加热台可以方便手工对PCB进行贴片焊接。本人是一名北漂软件工程师，同时也是一名电子爱好者，为了搬家时方便，工具的选择总是偏向于便捷化。网络上成品的平板加热台往往较为厚重，故自己动手实现一台。

#### 特色

1. 屏幕使用0.96寸的OLED液晶显示屏（即将支持LCD1602）。
2. 支持通过wifi进行固件升级。[链接](ws://192.168.4.1:8266/) ws://192.168.4.1:8266/
3. 使用旋转编码器代替按键式操作，提高用户体验。
4. 控制板面向不同用户群体，分为模块化和集成化两种电路设计。

## 工程包含
1. 加热平台的物理拼装，Bilibli视频[链接](https://www.bilibili.com/video/BV1ky4y1m7ZF) https://www.bilibili.com/video/BV1ky4y1m7ZF
2. 基于ESP8266的温控系统。
3. 控温系统的硬件控制电路。
4. 控制器外壳打印文件。（未更新）

## ESP8266温控器

![HeatPlatform_PCB](https://gitee.com/ClimbSnailQ/Project_Image/raw/master/Note/HeatPlatform_PCB.png)

![HeatPlatform_PCB](Image/HeatPlatform_PCB.png)

1. 主控使用的是ESP8266-12F。
2. 使用MAX6675驱动型热电偶。
3. 使用PID控制算法控制双向可控硅的导通状态。
4. 使用5脚的EC11旋转编码器作为用户操作的按钮。

#### 硬件设计要点
1. 天线附近要保持足够的净空区，否则影响天线的性能。
2. bootloader启动加载程序时，GPIO0为低电平，则进入固件烧录模式。
3. 正常启动时，GPIO0高、GPIO2高、GPIO15低、EN高、RST高。
4. GPIO16不支持中断、PWM、I2C以及One-Wire功能，只能作为普通的输入、输出端口。

#### 在线更新固件脚本
1. 使用webrepl进行更新固件(双击webrepl工具里的"webrepl.html")
2. [链接](ws://192.168.4.1:8266/) ws://192.168.4.1:8266/（链接加热控制器的wifi后方可链接）
3. 密码预设为`88888888`（在config.json文件中已注明）


## 资料
[PID动画演示](https://rossning92.github.io/pid-simulation/)

## 学习资料传送门
1. [教程 in Github](https://github.com/lvidarte/esp8266/wiki)
2. [ESP8266固件](http://micropython.org/download#esp8266)
3. [uPyCraft IDE](http://docs.dfrobot.com.cn/upycraft/)
4. [官方参考文档](https://docs.micropython.org/en/latest/esp8266/quickref.html#pins-and-gpio)
5. [MicroPython API](https://makeblock-micropython-api.readthedocs.io/zh/latest/library/)
6. [非官方中文参考文档](http://docs.dfrobot.com.cn/upycraft/3.2.2%20Pin.html)