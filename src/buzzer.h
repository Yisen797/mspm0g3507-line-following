/**
 * @file    buzzer.h
 * @brief   蜂鸣器驱动 (MSPM0G3507 移植版)
 *
 * PA17 推挽输出, 低电平有效
 * 原始 STM32: PF3 推挽输出
 */

#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "mspm0_config.h"

#ifdef __cplusplus
extern "C" {
#endif

void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);
void buzzer_beep(uint32_t ms);   /* 响一声 (阻塞延时) */

#ifdef __cplusplus
}
#endif

#endif /* __BUZZER_H__ */
