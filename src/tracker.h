/**
 * @file    tracker.h
 * @brief   三轮寻迹小车控制算法 (MSPM0G3507 移植版)
 *
 * 算法与 STM32 原版完全一致。硬件相关部分通过 driver 模块抽象。
 *
 * 传感器位置映射:
 *   X1 (PB8, 最左)  → 位置坐标 0
 *   X2 (PB9)        → 位置坐标 1
 *   ...
 *   X8 (PB15, 最右) → 位置坐标 7
 *   轨道中心 ⇒ 位置坐标 3.5
 */

#ifndef __TRACKER_H__
#define __TRACKER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void  tracker_init(void);
int   tracker_update(float dt, int16_t base_speed);
void  tracker_stop(void);
float tracker_get_error(void);
void  tracker_tune_pid(float kp, float ki, float kd);

#ifdef __cplusplus
}
#endif

#endif /* __TRACKER_H__ */
