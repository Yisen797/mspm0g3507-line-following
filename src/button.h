/**
 * @file    button.h
 * @brief   3按键驱动 — PA31(BTN1) / PA7(BTN2) / PA25(BTN3)
 *
 * 注: BTN1(PA28) 已舍弃 — 与 IR X6(PA2) 共用 PINCM3
 */

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "mspm0_config.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void button_init(void);
bool button_pressed(int n);       /* n=0~2, 去抖后状态 */
bool button_falling_edge(int n);  /* 下降沿检测 */

#ifdef __cplusplus
}
#endif

#endif
