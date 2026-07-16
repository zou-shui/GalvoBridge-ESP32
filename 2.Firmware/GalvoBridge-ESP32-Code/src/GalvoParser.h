#ifndef GALVO_PARSER_H
#define GALVO_PARSER_H

#include <Arduino.h>

#define MAX_POINTS_PER_FRAME 25000 // 一帧最大点数量

// 点坐标结构体
typedef struct
{
    uint16_t x;
    uint16_t y;
} __attribute__((packed)) point_t;

// 初始化解析器与串口接收任务（内部会创建FreeRTOS任务）
void galvoParser_init();

// 供DAC中断调用的接口：获取当前正在播放的数据源指针和总点数
void galvoParser_getRenderFrame(point_t **out_points, uint32_t *out_count);

#endif // GALVO_PARSER_H