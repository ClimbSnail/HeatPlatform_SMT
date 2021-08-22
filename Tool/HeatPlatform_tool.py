import sys
import os
import serial
from esptool import esptool

VERSION = "Ver1.0"


if __name__ == '__main__':
    try:
        input("\n1. 刷机前请确保串口连接上控制板.\n2. 请将固件程序bin文件放在本软件的同级目录下。 \n3. 刷机过程中需要按住旋钮，接着按一下板子复位键，过0.5秒即可松开旋钮。\n\n准备好后敲下回车键继续。\n")
        com_obj_list = list(serial.tools.list_ports.comports())

        select_com = None
        # 获取可用COM口名字
        com_list = [com_obj[0] for com_obj in com_obj_list]
        print("您本机的串口设备有：", end='')
        print(com_list)
        if len(com_list) == 1:
            select_com = com_list[0]
        else:
            select_com = input("input COM（例如 COM7）: ")

        
        #列出文件夹下所有的目录与文件
        list_file = os.listdir("./")
        firmware_path = 'HeatPlatform_C_v1.0.bin'
        for file_name in list_file:
            if 'HeatPlatform_C' in file_name:
                firmware_path = file_name.strip()
        
        cmd = ['HeatPlatform_tool.py', '--port', select_com,
            '--baud', '460800',
            'write_flash',
            '0x0', firmware_path]
        
        # sys.argv = cmd
        esptool.main(cmd[1:])
    except Exception as err:
        print(err)
    
    input("刷机完毕，按回车以关闭软件。")