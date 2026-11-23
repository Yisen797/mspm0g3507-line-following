/**
 * @file    servo.h
 * @brief   双路舵机 PWM 驱动 — TIMG7
 *
 * PA26: TIMG7_C0 → 舵机1
 * PA27: TIMG7_C1 → 舵机2
 * PWM: 50Hz, 0.5~2.5ms 脉宽 → 0°~180°
 */

#ifndef __SERVO_H__
#define __SERVO_H__

#include "mspm0_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void servo_init(void);

/**
 * @brief  设置舵机角度
 * @param  ch   通道 (0=舵机1, 1=舵机2)
 * @param  deg  角度 0~180
 */
void servo_set(uint8_t ch, uint8_t deg);

#ifdef __cplusplus
}
#endif

#endif
