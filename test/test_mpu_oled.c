/**
 * OLED(I2C0 PA0/PA1) + MPU6050(I2C1 PA29/PA30)
 * I2C init 完全按 TI 官方示例
 */
#include "mspm0_config.h"
#include "delay.h"
#include "oled_ssd1306.h"
#include "i2c_blocking.h"

#define MPU_ADDR 0x68

/* 等宽格式化: 固定 7 字符 "±XX.XX" */
static void f2s(char *b, float v) {
    int n = (int)(v * 100.0f + (v >= 0 ? 0.5f : -0.5f));
    char *p = b;
    *p++ = (n < 0) ? '-' : ' '; if (n < 0) n = -n;
    int hi = n / 100, lo = n % 100;
    if (hi < 10) *p++ = ' ';
    else        *p++ = '0' + hi / 10;
    *p++ = '0' + hi % 10;
    *p++ = '.';
    *p++ = '0' + lo / 10;
    *p++ = '0' + lo % 10;
    *p++ = ' '; *p = 0;
}

int main(void) {
    DL_GPIO_reset(GPIOB); DL_GPIO_enablePower(GPIOB);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM50);
    DL_GPIO_enableOutput(GPIOB, DL_GPIO_PIN_22);
    DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_22);

    /* ---- I2C0 → OLED ---- */
    oled_init();
    oled_clear();
    oled_text(0, 1, "Init I2C1...");

    /* ---- I2C1 → MPU6050 (和 I2C0 完全一样的初始化方式) ---- */
    DL_I2C_reset(I2C1_BUS);
    DL_I2C_enablePower(I2C1_BUS);
    delay_cycles(16);
    i2c_initController(I2C1_BUS,
        IOMUX_PINCM5, IOMUX_PINCM5_PF_I2C1_SDA,
        IOMUX_PINCM4, IOMUX_PINCM4_PF_I2C1_SCL, 7);

    /* 唤醒 MPU6050 */
    uint8_t d[2];
    d[0]=0x6B; d[1]=0x00;   /* PWR_MGMT1=0 */
    DL_I2C_fillControllerTXFIFO(I2C1_BUS, d, 2);
    while(!(DL_I2C_getControllerStatus(I2C1_BUS) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_startControllerTransfer(I2C1_BUS, MPU_ADDR, DL_I2C_CONTROLLER_DIRECTION_TX, 2);
    delay_cycles(30);
    while(DL_I2C_getControllerStatus(I2C1_BUS) & DL_I2C_CONTROLLER_STATUS_BUSY);

    d[0]=0x1C; d[1]=0x00;   /* ACCEL ±2g */
    DL_I2C_fillControllerTXFIFO(I2C1_BUS, d, 2);
    while(!(DL_I2C_getControllerStatus(I2C1_BUS) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_startControllerTransfer(I2C1_BUS, MPU_ADDR, DL_I2C_CONTROLLER_DIRECTION_TX, 2);
    delay_cycles(30);
    while(DL_I2C_getControllerStatus(I2C1_BUS) & DL_I2C_CONTROLLER_STATUS_BUSY);

    d[0]=0x1B; d[1]=0x00;   /* GYRO ±250dps */
    DL_I2C_fillControllerTXFIFO(I2C1_BUS, d, 2);
    while(!(DL_I2C_getControllerStatus(I2C1_BUS) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_startControllerTransfer(I2C1_BUS, MPU_ADDR, DL_I2C_CONTROLLER_DIRECTION_TX, 2);
    delay_cycles(30);
    while(DL_I2C_getControllerStatus(I2C1_BUS) & DL_I2C_CONTROLLER_STATUS_BUSY);

    oled_text(0, 3, "MPU init OK");
    delay_ms(500);
    oled_clear();

    char b[20]; uint8_t r[14];
    while(1) {
        uint8_t reg=0x3B;
        DL_I2C_fillControllerTXFIFO(I2C1_BUS, &reg, 1);
        while(!(DL_I2C_getControllerStatus(I2C1_BUS) & DL_I2C_CONTROLLER_STATUS_IDLE));
        DL_I2C_startControllerTransfer(I2C1_BUS, MPU_ADDR, DL_I2C_CONTROLLER_DIRECTION_TX, 1);
        delay_cycles(30);
        while(DL_I2C_getControllerStatus(I2C1_BUS) & DL_I2C_CONTROLLER_STATUS_BUSY);

        while(!(DL_I2C_getControllerStatus(I2C1_BUS) & DL_I2C_CONTROLLER_STATUS_IDLE));
        DL_I2C_startControllerTransfer(I2C1_BUS, MPU_ADDR, DL_I2C_CONTROLLER_DIRECTION_RX, 14);
        for(int i=0;i<14;i++){while(DL_I2C_isControllerRXFIFOEmpty(I2C1_BUS));r[i]=DL_I2C_receiveControllerData(I2C1_BUS);}

        f2s(b, (float)(int16_t)((r[0]<<8)|r[1])/16384.0f); oled_text(0,0,b);
        f2s(b, (float)(int16_t)((r[2]<<8)|r[3])/16384.0f); oled_text(42,0,b);
        f2s(b, (float)(int16_t)((r[4]<<8)|r[5])/16384.0f); oled_text(84,0,b);
        f2s(b, (float)(int16_t)((r[8]<<8)|r[9])/131.0f);   oled_text(0,3,b);
        f2s(b, (float)(int16_t)((r[10]<<8)|r[11])/131.0f); oled_text(42,3,b);
        f2s(b, (float)(int16_t)((r[12]<<8)|r[13])/131.0f); oled_text(84,3,b);
        delay_ms(30);
    }
}
