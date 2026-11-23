/**
 * @file    buzzer.c
 * @brief   蜂鸣器 — PA17, 推挽输出, 低电平有效
 */

#include "buzzer.h"
#include "delay.h"

void buzzer_init(void)
{
    DL_GPIO_reset(BUZZER_PORT);
    DL_GPIO_enablePower(BUZZER_PORT);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM39);  /* PA17 */
    buzzer_off();
}

void buzzer_on(void)  { DL_GPIO_clearPins(BUZZER_PORT, BUZZER_PIN); }
void buzzer_off(void) { DL_GPIO_setPins(BUZZER_PORT, BUZZER_PIN); }

void buzzer_beep(uint32_t ms)
{
    buzzer_on();
    delay_ms(ms);
    buzzer_off();
}
