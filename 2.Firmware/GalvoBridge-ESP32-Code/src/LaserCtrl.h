#pragma once

// 初始化激光控制模块（配置 GPIO 和中断）
void laserCtrl_init();

// 点亮激光（仅在使能状态下生效）
void laser_ON();

// 关闭激光
void laser_OFF();

// 查询当前是否处于使能状态
bool laserCtrl_isEnabled();
