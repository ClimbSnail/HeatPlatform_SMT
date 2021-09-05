#include <Arduino.h>
#include <EEPROM.h> //导入Flash库文件

struct UserData
{
    int save_temperature; // 保存的温度
    int mode;             //
    int direction;        // 编码器方向 0正 1反
    int err_temperature;  // 误差温度
    double kp;
    double ki;
    double kd;
    double kt;
};

UserData user_data = {0, 0, 0, 0, 0, 0, 0, 0};

void init_user_data(void)
{
    user_data.save_temperature = 20;
    user_data.mode = 0;
    user_data.direction = 0;
    user_data.err_temperature = 0;
    user_data.kp = 4.5;
    user_data.ki = 0.15;
    user_data.kd = 0.2;
    user_data.kt = 0.2;
}

void save_config() //保存函数
{
    // EEPROM.begin(512); //向系统申请1024kb ROM
    //开始写入
    EEPROM.put(0, user_data);
    EEPROM.commit(); //执行写入ROM
}

void load_config() //读取函数
{
    EEPROM.begin(512);
    user_data = EEPROM.get(0, user_data);
    if (user_data.save_temperature < 0 || user_data.save_temperature > 1000)
    {
        init_user_data();
        save_config();
    }
    // for (int i = 0; i < sizeof(UserData); i++)
    // {
    //     *(p + i) = EEPROM.read(i);
    // }
    // EEPROM.commit();
}

boolean doDelayMillisTime(unsigned long interval, unsigned long *previousMillis, boolean state)
{
    unsigned long currentMillis = millis();
    if (currentMillis - *previousMillis >= interval)
    {
        *previousMillis = currentMillis;
        state = !state;
    }
    return state;
}