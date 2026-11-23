/**
 * @file    ultrasonic.h
 * @brief   HC-SR04 超声波测距 (MSPM0G3507 移植版)
 *
 * Trig=PA14, Echo=PA15 (使用 TIMG1 测量回波脉宽)
 * 原项目未使用超声波 (硬件未连接), 本模块保留接口。
 */

#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__

#include "mspm0_config.h"

#ifdef __cplusplus
extern "C" {
#endif

void  ultrasonic_init(void);
float ultrasonic_read(void);   /* 返回距离 (cm), 超时返回 -1 */

#ifdef __cplusplus
}
#endif

#endif /* __ULTRASONIC_H__ */
