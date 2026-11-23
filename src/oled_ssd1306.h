/**
 * @file    oled_ssd1306.h
 * @brief   SSD1306 OLED 128x64 I2C 驱动
 *
 * I2C0: PA0(SDA), PA1(SCL) @ 400kHz, 地址 0x3C
 * 与 MPU6050 共用 I2C 总线 (MPU6050 地址 0x68)
 */

#ifndef __OLED_SSD1306_H__
#define __OLED_SSD1306_H__

#include "mspm0_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void oled_init(void);
void oled_clear(void);
void oled_chess(void);
void oled_text(uint8_t col, uint8_t page, const char *str);

#ifdef __cplusplus
}
#endif

#endif
