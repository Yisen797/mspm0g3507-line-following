/**
 * @file    delay.h
 * @brief   毫秒/微秒延时 (MSPM0G3507 移植版)
 *
 * 原始 STM32 版本使用忙等待 (~42000次循环/ms @ 168MHz)。
 * MSPM0 版本使用 DriverLib 的 delay_cycles() 函数, 基于 CPU 周期精确延时。
 *
 * 注意: 与原始项目一样, 这里不使用 SysTick 中断,
 *        delay_cycles() 是硬件精确的忙等待。
 */

#ifndef __DELAY_H__
#define __DELAY_H__

#include "mspm0_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  毫秒级延时
 * @param  ms  延时毫秒数
 * @note   基于 delay_cycles(), 80MHz 下 1ms = 80000 cycles
 */
static inline void delay_ms(uint32_t ms)
{
    delay_cycles((CPUCLK_FREQ / 1000UL) * ms);
}

/**
 * @brief  微秒级延时
 * @param  us  延时微秒数
 * @note   基于 delay_cycles(), 80MHz 下 1us = 80 cycles
 */
static inline void delay_us(uint32_t us)
{
    delay_cycles((CPUCLK_FREQ / 1000000UL) * us);
}

#ifdef __cplusplus
}
#endif

#endif /* __DELAY_H__ */
