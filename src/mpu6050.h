/**
 * @file    mpu6050.h
 * @brief   MPU6050 六轴传感器 I2C 驱动
 *
 * I2C 地址: 0x68 (与 OLED 共用 I2C0 总线)
 * 输出: 加速度 (g), 角速度 (°/s), 温度 (°C)
 */

#ifndef __MPU6050_H__
#define __MPU6050_H__

#include "mspm0_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float ax, ay, az;    /* 加速度 (g) */
    float gx, gy, gz;    /* 角速度 (°/s) */
    float temp;          /* 温度 (°C) */
} MPU6050_Data;

void mpu6050_init(void);
bool mpu6050_read(MPU6050_Data *data);

#ifdef __cplusplus
}
#endif

#endif
