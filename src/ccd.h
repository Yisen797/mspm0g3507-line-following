/**
 * @file    ccd.h
 * @brief   CCD 线阵传感器 — ADC 模拟采集 (PA16, ADC0_A1_1)
 */

#ifndef __CCD_H__
#define __CCD_H__

#include "mspm0_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void     ccd_init(void);
uint16_t ccd_read(void);       /* 读取 ADC 原始值 (12-bit) */
float    ccd_read_voltage(void); /* 读取电压值 (V) */

#ifdef __cplusplus
}
#endif

#endif
