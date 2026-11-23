/**
 * @file    sensor.c
 * @brief   8路红外传感器 — 三组读取, 拼成 8-bit
 *
 * X1~X5: PB11~PB15 → bits 0~4
 * X6:    PA2       → bit 5    (⚠️ 只能输入, ROSC电阻)
 * X7:    PB10      → bit 6
 * X8:    PB16      → bit 7
 *
 * 黑线=低电平, 取反后 1=检测到线
 */
#include "sensor.h"

static uint8_t sensor_raw_mask = 0x00;

void sensor_init(void)
{
    DL_GPIO_reset(IR_PORT_A);
    DL_GPIO_enablePower(IR_PORT_A);

    /* PB11~PB15 (X1~X5): PINCM28~32 */
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM28,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM29,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM30,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM31,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM32,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    /* PA2 (X6): PINCM3 — ⚠️ 只能输入, ROSC校准电阻100kΩ→GND */
    DL_GPIO_reset(IR_PORT_C);
    DL_GPIO_enablePower(IR_PORT_C);
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM3,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    /* PB10 (X7): PINCM27, PB16 (X8): PINCM33 */
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM27,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM33,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
}

uint8_t sensor_read(void)
{
    /* PB11~PB15 → X1~X5 (bit0~bit4) */
    uint32_t raw_a = DL_GPIO_readPins(IR_PORT_A, IR_PINS_A);
    uint8_t bits_a = (uint8_t)(~(raw_a >> IR_SHIFT_A)) & 0x1F;

    /* PA2 → X6 (bit5) — 单引脚: 低=检测到线 */
    uint32_t raw_c = DL_GPIO_readPins(IR_PORT_C, IR_PIN_C);
    uint8_t bit5 = (raw_c == 0) ? 1 : 0;

    /* PB10/PB16 → X7/X8 (bit6~bit7), not contiguous, extract manually */
    uint32_t raw_b = DL_GPIO_readPins(IR_PORT_B, IR_PINS_B);
    uint8_t bit6 = (raw_b & DL_GPIO_PIN_10) ? 0 : 1;
    uint8_t bit7 = (raw_b & DL_GPIO_PIN_16) ? 0 : 1;

    sensor_raw_mask = bits_a | (bit5 << 5) | (bit6 << 6) | (bit7 << 7);
    return sensor_raw_mask;
}

bool sensor_line_detected(void)
{
    sensor_read();
    return (sensor_raw_mask != 0x00);
}

uint8_t sensor_active_count(void)
{
    uint8_t cnt = 0, m = sensor_raw_mask;
    while (m) { cnt += (m & 1); m >>= 1; }
    return cnt;
}
