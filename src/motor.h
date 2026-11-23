/**
 * @file    motor.h
 * @brief   TB6612 双路电机驱动 (MSPM0G3507 移植版)
 *
 * 原始 STM32: TIM3 PWM (CH1=PA6, CH2=PA7) + PC0-PC3 方向控制
 * MSPM0:     TIMA0 PWM (C0=PB8, C1=PB9) + PB2-PB5 方向控制
 *
 * PWM 分辨率: 10000 步 (0~9999), 频率 8kHz
 * 速度范围:   -10000 ~ 10000 (负数=反转, 正数=正转)
 *
 * TB6612 控制逻辑:
 *   speed > 0: 正转 (AIN1=1,AIN2=0), PWM=|speed|
 *   speed < 0: 反转 (AIN1=0,AIN2=1), PWM=|speed|
 *   speed = 0: 惰行 (AIN1=0,AIN2=0)
 *   AIN1=AIN2=1: 短路刹车
 *
 * 注意: 左电机用 B 通道 (BIN1/BIN2), 右电机用 A 通道 (AIN1/AIN2)
 *       存在物理接线交叉, 方向在代码中修正。
 */

#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "mspm0_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void motor_init(void);

/**
 * @brief  设置左电机速度
 * @param  speed  -10000 ~ 10000
 */
void motor_left_set(int16_t speed);

/**
 * @brief  设置右电机速度
 * @param  speed  -10000 ~ 10000
 */
void motor_right_set(int16_t speed);

/** 两轮同时刹车 */
void motor_brake(void);

/** 两轮惰行 (自由滑行) */
void motor_coast(void);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_H__ */
