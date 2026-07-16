#include "LaserCtrl.h"
#include <Arduino.h>

#define LASER_CTRL_PIN 3   // GPIO3 控制激光笔
#define LASER_TOGGLE_PIN 0 // GPIO0 下降沿触发使能切换

// 内部状态
static volatile bool laser_enabled = true; // 使能标志

/**
 * GPIO0 下降沿中断服务函数
 * 每次触发时翻转使能状态；若变为未使能，立即关闭激光
 */
void IRAM_ATTR laser_toggle_isr()
{
    laser_enabled = !laser_enabled;
    if (laser_enabled)
    {
        digitalWrite(LASER_CTRL_PIN, HIGH); // 使能后默认打开
    }
    else
    {
        digitalWrite(LASER_CTRL_PIN, LOW); // 关闭激光
    }
}

void laserCtrl_init()
{
    pinMode(LASER_CTRL_PIN, OUTPUT);
    digitalWrite(LASER_CTRL_PIN, HIGH);

    // 配置触发引脚，内部上拉，下降沿中断
    pinMode(LASER_TOGGLE_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(LASER_TOGGLE_PIN), laser_toggle_isr, FALLING);
}

void laser_ON()
{
    if (laser_enabled)
    {
        digitalWrite(LASER_CTRL_PIN, HIGH);
    }
}

void laser_OFF()
{
    digitalWrite(LASER_CTRL_PIN, LOW);
}

bool laserCtrl_isEnabled()
{
    return laser_enabled;
}
