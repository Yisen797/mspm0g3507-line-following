/**
 * @file    ccd.c
 * @brief   CCD 模拟采集 — ADC0, PA16
 *
 * 12-bit ADC, 单通道采样
 * PA16 = ADC0_A1_1 (PINCM38, PF=0 进入模拟模式)
 */

#include "ccd.h"

void ccd_init(void)
{
    DL_ADC12_reset(CCD_ADC);
    DL_ADC12_enablePower(CCD_ADC);

    /* PA16 → 模拟模式 (PF=0, PC=0) */
    DL_GPIO_initAnalogMode(IOMUX_PINCM38);

    /* ADC 参考: VDDA (3.3V), 采样时间: 500ns (最短) */
    DL_ADC12_setReference(CCD_ADC, DL_ADC12_REF_VDDA);
    DL_ADC12_setSamplingTime(CCD_ADC, DL_ADC12_SAMPLING_TIME_500NS);
    DL_ADC12_setResolution(CCD_ADC, DL_ADC12_RESOLUTION_12BIT);

    /* 配置单通道 A1_1 */
    DL_ADC12_configChannel(CCD_ADC, DL_ADC12_CHANNEL_1,
        DL_ADC12_MEMORY_IDX_0, DL_ADC12_SINGLE_CHANNEL);

    DL_ADC12_enable(CCD_ADC);
}

uint16_t ccd_read(void)
{
    /* 软件触发单次转换 */
    DL_ADC12_startConversion(CCD_ADC, DL_ADC12_SOFTWARE_TRIGGER);

    /* 等待转换完成 */
    while (!DL_ADC12_getStatus(CCD_ADC) & DL_ADC12_STATUS_CONVERSION_COMPLETE);

    return (uint16_t)DL_ADC12_getConversionResult(CCD_ADC, DL_ADC12_MEMORY_IDX_0);
}

float ccd_read_voltage(void)
{
    return (float)ccd_read() * 3.3f / 4095.0f;
}
