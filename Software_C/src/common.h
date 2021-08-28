#include <Arduino.h>
#include <EEPROM.h> //导入Flash库文件

struct UserData
{
    int save_temperature;
    int mode;
};

UserData user_data = {0, 0};

void save_config() //保存函数
{
    // EEPROM.begin(512); //向系统申请1024kb ROM
    //开始写入
    uint8_t *p = (uint8_t *)(&user_data);
<<<<<<< HEAD
    EEPROM.put(0, user_data);
=======
    for (int i = 0; i < sizeof(user_data); i++)
    {
        EEPROM.write(i, *(p + i)); //在闪存内模拟写入
    }
>>>>>>> 13cc9d57966b95a90323786ae48897e56b318414
    EEPROM.commit(); //执行写入ROM
}

void load_config() //读取函数
{
    EEPROM.begin(512);
<<<<<<< HEAD
    // EEPROM.get();
    uint8_t *p = (uint8_t *)(&user_data);
    user_data = EEPROM.get(0, user_data);
    // for (int i = 0; i < sizeof(UserData); i++)
    // {
    //     *(p + i) = EEPROM.read(i);
    // }
=======
    uint8_t *p = (uint8_t *)(&user_data);
    for (int i = 0; i < sizeof(user_data); i++)
    {
        *(p + i) = EEPROM.read(i);
    }
>>>>>>> 13cc9d57966b95a90323786ae48897e56b318414
    // EEPROM.commit();
}