/**
 * MPU6050 I2C1 (原工作版)
 */
#include <ti/driverlib/dl_i2c.h>
#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_common.h>
#define MPU_ADDR 0x68

static int i2c_write(uint8_t *d, int len) {
    DL_I2C_fillControllerTXFIFO(I2C1,d,len);
    int to=0; while(!(DL_I2C_getControllerStatus(I2C1)&DL_I2C_CONTROLLER_STATUS_IDLE))if(++to>500000)return -1;
    DL_I2C_startControllerTransfer(I2C1,MPU_ADDR,DL_I2C_CONTROLLER_DIRECTION_TX,len);
    DL_Common_delayCycles(30); to=0; while(DL_I2C_getControllerStatus(I2C1)&DL_I2C_CONTROLLER_STATUS_BUSY)if(++to>200000)return -2;
    return 0;
}

void mpu_reset_i2c1(void) { DL_I2C_reset(I2C1); }

void mpu_init(void) {
    DL_I2C_enablePower(I2C1); DL_Common_delayCycles(16);
    DL_GPIO_initPeripheralInputFunctionFeatures(IOMUX_PINCM4, IOMUX_PINCM4_PF_I2C1_SCL,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(IOMUX_PINCM5, IOMUX_PINCM5_PF_I2C1_SDA,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(IOMUX_PINCM4); DL_GPIO_enableHiZ(IOMUX_PINCM5);
    DL_I2C_ClockConfig c={DL_I2C_CLOCK_BUSCLK,DL_I2C_CLOCK_DIVIDE_1}; DL_I2C_setClockConfig(I2C1,&c);
    DL_I2C_disableAnalogGlitchFilter(I2C1); DL_I2C_resetControllerTransfer(I2C1);
    DL_I2C_setTimerPeriod(I2C1,7);
    DL_I2C_setControllerTXFIFOThreshold(I2C1,DL_I2C_TX_FIFO_LEVEL_EMPTY);
    DL_I2C_setControllerRXFIFOThreshold(I2C1,DL_I2C_RX_FIFO_LEVEL_BYTES_1);
    DL_I2C_enableControllerClockStretching(I2C1);
    DL_I2C_enableController(I2C1);

    uint8_t d[2];
    d[0]=0x6B;d[1]=0x00; i2c_write(d,2);
    d[0]=0x1B;d[1]=0x00; i2c_write(d,2);
}

int16_t mpu_read_yaw(void) {
    uint8_t reg=0x43,r[2]={0,0};int to;
    DL_I2C_fillControllerTXFIFO(I2C1,&reg,1);
    to=0; while(!(DL_I2C_getControllerStatus(I2C1)&DL_I2C_CONTROLLER_STATUS_IDLE))if(++to>200000)return 0;
    DL_I2C_startControllerTransfer(I2C1,MPU_ADDR,DL_I2C_CONTROLLER_DIRECTION_TX,1);
    DL_Common_delayCycles(30); to=0; while(DL_I2C_getControllerStatus(I2C1)&DL_I2C_CONTROLLER_STATUS_BUSY)if(++to>200000)return 0;
    while(!(DL_I2C_getControllerStatus(I2C1)&DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_startControllerTransfer(I2C1,MPU_ADDR,DL_I2C_CONTROLLER_DIRECTION_RX,2);
    for(int i=0;i<2;i++){to=0; while(DL_I2C_isControllerRXFIFOEmpty(I2C1))if(++to>200000)return 0; r[i]=DL_I2C_receiveControllerData(I2C1);}
    return (int16_t)((r[0]<<8)|r[1]);
}
