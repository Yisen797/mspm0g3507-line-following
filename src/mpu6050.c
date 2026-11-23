/**
 * @file    mpu6050.c
 * @brief   MPU6050 I2C 驱动实现
 *
 * 寄存器映射:
 *   0x6B PWR_MGMT1   — 电源管理 (写0唤醒)
 *   0x1C ACCEL_CONFIG — 加速度量程 (±2g/4g/8g/16g)
 *   0x1B GYRO_CONFIG  — 陀螺仪量程 (±250/500/1000/2000 °/s)
 *   0x3B ACCEL_XOUT_H — 加速度数据起始 (14字节: ax,ay,az,temp,gx,gy,gz)
 *
 * 使用独立 I2C1 总线 (PA30=SDA, PA29=SCL)。
 * MPU6050 地址: 0x68 (AD0 引脚接地)
 */

#include "mpu6050.h"
#include "delay.h"
#include "i2c_blocking.h"

#define MPU_ADDR        0x68
#define REG_PWR_MGMT1   0x6B
#define REG_ACCEL_CFG   0x1C
#define REG_GYRO_CFG    0x1B
#define REG_ACCEL_XOUT  0x3B

/* 量程换算因子 */
#define ACCEL_SCALE     (16384.0f)   /* ±2g → 16384 LSB/g */
#define GYRO_SCALE      (131.0f)     /* ±250°/s → 131 LSB/(°/s) */
#define TEMP_OFFSET     36.53f       /* 温度偏移 */
#define TEMP_SCALE      340.0f       /* 340 LSB/°C */

/* I2C 底层: 写寄存器 */
static void mpu_write(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    i2c_transmitBlocking(I2C1_BUS, MPU_ADDR, buf, 2);
}

/* I2C 底层: 读多字节 */
static void mpu_read(uint8_t reg, uint8_t *data, uint8_t len)
{
    i2c_transmitBlocking(I2C1_BUS, MPU_ADDR, &reg, 1);  /* 先发寄存器地址 */
    i2c_receiveBlocking(I2C1_BUS, MPU_ADDR, data, len); /* 再读数据 */
}

void mpu6050_init(void)
{
    DL_I2C_reset(I2C1_BUS);
    DL_I2C_enablePower(I2C1_BUS);
    delay_cycles(16);

    i2c_initController(I2C1_BUS,
        IOMUX_PINCM5, IOMUX_PINCM5_PF_I2C1_SDA,
        IOMUX_PINCM4, IOMUX_PINCM4_PF_I2C1_SCL,
        7);

    delay_ms(10);

    /* 唤醒 MPU6050 (清除 SLEEP 位) */
    mpu_write(REG_PWR_MGMT1, 0x00);
    delay_ms(10);

    /* 加速度计: ±2g */
    mpu_write(REG_ACCEL_CFG, 0x00);

    /* 陀螺仪: ±250 °/s */
    mpu_write(REG_GYRO_CFG, 0x00);

    delay_ms(10);
}

bool mpu6050_read(MPU6050_Data *data)
{
    uint8_t buf[14];
    mpu_read(REG_ACCEL_XOUT, buf, 14);

    /* 大端序 → 主机序 */
    int16_t ax_raw = (int16_t)((buf[0] << 8) | buf[1]);
    int16_t ay_raw = (int16_t)((buf[2] << 8) | buf[3]);
    int16_t az_raw = (int16_t)((buf[4] << 8) | buf[5]);
    int16_t tp_raw = (int16_t)((buf[6] << 8) | buf[7]);
    int16_t gx_raw = (int16_t)((buf[8] << 8) | buf[9]);
    int16_t gy_raw = (int16_t)((buf[10] << 8) | buf[11]);
    int16_t gz_raw = (int16_t)((buf[12] << 8) | buf[13]);

    data->ax   = (float)ax_raw / ACCEL_SCALE;
    data->ay   = (float)ay_raw / ACCEL_SCALE;
    data->az   = (float)az_raw / ACCEL_SCALE;
    data->gx   = (float)gx_raw / GYRO_SCALE;
    data->gy   = (float)gy_raw / GYRO_SCALE;
    data->gz   = (float)gz_raw / GYRO_SCALE;
    data->temp = ((float)tp_raw - TEMP_OFFSET) / TEMP_SCALE + TEMP_OFFSET;

    return true;
}
