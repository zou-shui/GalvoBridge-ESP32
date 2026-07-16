#include <Arduino.h>
#include "DAC8562.h"
#include "GalvoParser.h"
#include "LaserCtrl.h"

DAC8562 dac;
hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer()
{
    static uint32_t current_point_idx = 0; // 当前播放到哪一个点

    point_t *current_frame = NULL;
    uint32_t total_points = 0;

    // 从解析器模块直接抓取当前的“播放缓冲区”指针和点数
    galvoParser_getRenderFrame(&current_frame, &total_points);

    // 如果没有数据，或者数据点数为0，维持输出中心点
    if (total_points == 0 || current_frame == NULL)
    {
        dac.sync_outputs(32768, 32768);
        current_point_idx = 0;
        return;
    }

    // 安全边界检查：如果串口突然切换了帧，并且新帧点数变少了，防止索引越界
    if (current_point_idx >= total_points)
    {
        current_point_idx = 0;
    }

    // 吐出当前点的 X, Y 坐标给 DAC 芯片
    dac.sync_outputs(current_frame[current_point_idx].x, current_frame[current_point_idx].y);

    // 递增索引，如果到了画面末尾，自动重头循环（振镜需要高频不断地重绘画面）
    current_point_idx++;
    if (current_point_idx >= total_points)
    {
        current_point_idx = 0;
    }
}

void setup()
{
    Serial.begin(921600);

    dac.begin();
    dac.sync_outputs(32768, 32768);

    delay(1000);

    // 1. 初始化解析器：这会在后台建立一个独立运行的串口扫描任务
    galvoParser_init();

    // 2. 启动硬件定时器
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 40, true);
    timerAlarmEnable(timer);

    Serial.println("Ready @ 921600 bps");

    laserCtrl_init();
}

void loop()
{
}
