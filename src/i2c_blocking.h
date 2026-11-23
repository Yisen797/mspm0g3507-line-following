/**
 * @file    i2c_blocking.h
 * @brief   I2C 阻塞传输封装 (SDK 2.11, 参考 TI 官方 i2c_controller_rw_multibyte_fifo_poll 示例)
 */
#include <ti/driverlib/dl_i2c.h>

/* ---- I2C 通用初始化 (控制器模式, 400kHz) ---- */
static inline void i2c_initController(I2C_Regs *i2c,
    uint32_t sdaPincm, uint32_t sdaFunc,
    uint32_t sclPincm, uint32_t sclFunc,
    uint8_t timerPeriod)      /* 400kHz: 32MHz→7, 24MHz→5 */
{
    DL_GPIO_initPeripheralInputFunctionFeatures(sdaPincm, sdaFunc,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(sclPincm, sclFunc,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(sdaPincm);   /* I2C 开漏必须 HiZ */
    DL_GPIO_enableHiZ(sclPincm);

    DL_I2C_ClockConfig clkCfg = {
        .clockSel    = DL_I2C_CLOCK_BUSCLK,
        .divideRatio = DL_I2C_CLOCK_DIVIDE_1
    };
    DL_I2C_setClockConfig(i2c, &clkCfg);
    DL_I2C_disableAnalogGlitchFilter(i2c);
    DL_I2C_resetControllerTransfer(i2c);
    DL_I2C_setTimerPeriod(i2c, timerPeriod);
    DL_I2C_setControllerTXFIFOThreshold(i2c, DL_I2C_TX_FIFO_LEVEL_EMPTY);
    DL_I2C_setControllerRXFIFOThreshold(i2c, DL_I2C_RX_FIFO_LEVEL_BYTES_1);
    DL_I2C_enableControllerClockStretching(i2c);
    DL_I2C_enableController(i2c);
}

/** 阻塞发送 (支持任意长度, TX FIFO 仅 4 字节, 边发边填) */
static inline void i2c_transmitBlocking(
    I2C_Regs *i2c, uint32_t targetAddr, const uint8_t *data, uint16_t len)
{
    if (len == 0) return;

    /* 填满首批数据 (最多 4 字节) */
    uint16_t first = (len > 4) ? 4 : len;
    DL_I2C_fillControllerTXFIFO(i2c, data, first);

    while (!(DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_startControllerTransfer(i2c, targetAddr,
        DL_I2C_CONTROLLER_DIRECTION_TX, len);
    delay_cycles(30);  /* Errata I2C_ERR_13 */

    /* 边发边填: 等待 FIFO 有空位, 逐字节填入 */
    for (uint16_t i = first; i < len; i++) {
        while (DL_I2C_isControllerTXFIFOFull(i2c));
        DL_I2C_transmitControllerData(i2c, data[i]);
    }

    while (DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_BUSY);
}

/** 阻塞接收 */
static inline void i2c_receiveBlocking(
    I2C_Regs *i2c, uint32_t targetAddr, uint8_t *data, uint16_t len)
{
    while (!(DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_startControllerTransfer(i2c, targetAddr,
        DL_I2C_CONTROLLER_DIRECTION_RX, len);
    for (uint16_t i = 0; i < len; i++) {
        while (DL_I2C_isControllerRXFIFOEmpty(i2c));
        data[i] = DL_I2C_receiveControllerData(i2c);
    }
}
