/**
 * @file    encoder.h
 * @brief   霍尔编码器双电机测速 (MSPM0G3507 移植版)
 *
 * 关键差异: MSPM0G3507 只有 1 个硬件 QEI (TIMG0)!
 *   左编码器: TIMG0 硬件 QEI 模式 (16-bit, 4倍频)
 *   右编码器: TIMG6 输入捕获模式 + GPIO 方向判断 (32-bit 定时器)
 *
 * 原始 STM32:
 *   左电机: TIM2 编码器模式  CH1=PA0, CH2=PA1 (32-bit)
 *   右电机: TIM8 编码器模式  CH1=PC6, CH2=PC7 (16-bit)
 *
 * MSPM0:
 *   左电机: TIMG0 QEI  PA12(A), PA13(B)   (16-bit, 注意溢出!)
 *   右电机: TIMG6 捕获 PA21(A), PA22(B)   (32-bit, 软件方向解码)
 *
 * 注意: 16-bit QEI 计数器在高速下容易溢出 (390 PPR * 100 RPM = 650 counts/s)
 *       采样周期必须足够短以避免溢出。20ms 周期下溢出风险低。
 *
 * 编码器参数:
 *   PULSES_PER_REV = 390  (11 PPR 霍尔编码器 * 减速比)
 *   轮径: 65mm, 周长: 204.2mm
 */

#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "mspm0_config.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void     encoder_init(void);

int32_t  encoder_read_left(void);          /* 左编码器 32-bit 累计计数值 */
int32_t  encoder_read_right(void);         /* 右编码器 32-bit 累计计数值 */

int32_t  encoder_speed_left(float dt);     /* 左轮速度 (脉冲/秒) */
int32_t  encoder_speed_right(float dt);    /* 右轮速度 (脉冲/秒) */

float    encoder_speed_left_mm_s(float dt);  /* 左轮速度 (mm/s) */
float    encoder_speed_right_mm_s(float dt); /* 右轮速度 (mm/s) */

void     encoder_reset(void);              /* 清零所有计数值 */

#ifdef __cplusplus
}
#endif

#endif /* __ENCODER_H__ */
