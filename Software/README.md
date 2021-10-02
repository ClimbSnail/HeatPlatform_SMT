### 固件刷写

##### 完全刷写
1. 使用USB-TTL下载器链接板子的下载接口（已标识引脚，RXD TXD需要反接）。
2. `uPyCraft`是代码编写工具，同时也是固件刷写工具，打开`uPyCraft`工具，会提示安装字体（随意安装就行）。
3. 打开软件后，选择工具栏`Tools`->`BurnFirmware`，打开固件烧录的提示框。
	* `board`选择`esp8266`
	* `burn_addr`保持默认
	* `erase_flash`可以选择`yes`
	* `com`选择你TTL下载器的`com口`
	* `Firmware_Choose`中选择`Users`，然后点击`Choose`去选择`Firmware`文件夹中找后缀名为`bin`的固件。
4. 点击`OK`后，弹出刷写固件的页面。
5. 可以看到进度条始终不动。这时按住控制板上的旋钮，接着按下`RST`键（之后松开），紧接着松开刚刚按住的旋钮。这时候可以看到`EraseFlash`进度条开始变化了。
6. 等`EraseFlash`进度条走完，会发现`Burn`进度条卡住了，这时候再按住旋钮，然后再按一下`RST`松开，之后再松开旋钮按键，等进度条走完就刷写成功了。
7. 选择`Tools`->`Serial`中下载器对应的端口之后，下方输出区出现`>>`就表示`uPyCraft`工具自动连接了我们的控制器，即表示第6步的固件刷写过程是成功的。
8. 上传代码文件：双击`uPyCraft`工具左侧的`workSpace`文件夹（第一次），会让选择一个文件夹，这时我们浏览到`Software`文件夹的上级目录就行。随后点击`File`->`reflush`刷新下左侧的文件目录。再次点开`workSpace`就可以看到`Software`里的程序文件了。
9. 双击打开任意除`main.py`外的文件，点击`Tools`->`Download`，若上传成功 可以在下方的输出区看到`download ok`字样。重复此过程，使`Software`中的文件依次上传。
10. `uPyCraft`工具左侧的`device`目录中显示的是当前控制器中已有的程序，由此可以判断文件是否上传成功。在等到上传结束，最后再上传`main.py`的文件。
11. 完成。控制板接上铝加热板就可以工作了。当然不需要铝加热板也可以简单接上oled屏幕，重新上电或者按下RST复位，看oled是否正常显示。

##### 更新程序
更新程序是建立在之前进行过_"固件刷写"_操作。

按住控制器上的`MODE`按键，然后按下`RST`松开，`MODE`按键需等待背部ESP8266的指示灯常亮`（5-7s）`左右再松开，控制器即可进入程序更新模式。可跳过前6步骤，直接从第7步开始 更新想要更新的程序文件。


### 在线更新固件脚本（在计划中）
1. 使用webrepl进行更新固件(双击webrepl工具里的"webrepl.html")
2. [链接](ws://192.168.4.1:8266/) ws://192.168.4.1:8266/（链接加热控制器的wifi后方可链接）
3. 密码预设为`88888888`（在config.json文件中已注明）


### 学习资料传送门
1. [教程 in Github](https://github.com/lvidarte/esp8266/wiki)
2. [ESP8266固件](http://micropython.org/download#esp8266)
3. [uPyCraft IDE](http://docs.dfrobot.com.cn/upycraft/)
4. [官方参考文档](https://docs.micropython.org/en/latest/esp8266/quickref.html#pins-and-gpio)
5. [MicroPython API](https://makeblock-micropython-api.readthedocs.io/zh/latest/library/)

