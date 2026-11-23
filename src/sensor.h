/**
 * @file    sensor.h
 * @brief   8路红外传感器 — 三组读取
 *
 * PB11~PB15 → X1~X5, PA2 → X6, PB10 → X7, PB16 → X8
 * 合并为 8-bit: bit0=最左, bit7=最右, 1=检测到黑线
 */

#ifndef __SENSOR_H__
#define __SENSOR_H__

#include "mspm0_config.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void     sensor_init(void);
uint8_t  sensor_read(void);
bool     sensor_line_detected(void);
uint8_t  sensor_active_count(void);

#ifdef __cplusplus
}
#endif

#endif
