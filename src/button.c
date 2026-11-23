/**
 * @file    button.c
 * @brief   3按键驱动 — PA31(BTN1) / PA7(BTN2) / PA25(BTN3)
 *          BTN1(PA28) 已舍弃 — 与 IR X6(PA2) 共用 PINCM3
 *          上拉输入, 低电平有效
 */
#include "button.h"

static uint8_t btn_history[BTN_NUM] = {0xFF, 0xFF, 0xFF};
static bool    btn_stable[BTN_NUM]  = {false, false, false};
static bool    btn_prev[BTN_NUM]    = {false, false, false};

static const struct { GPIO_Regs *port; uint32_t pin; } btn_cfg[BTN_NUM] = {
    {BTN1_PORT, BTN1_PIN},  /* PA31 */
    {BTN2_PORT, BTN2_PIN},  /* PA7  */
    {BTN3_PORT, BTN3_PIN},  /* PA25 */
};

void button_init(void)
{
    /* PA31 = PINCM6 (与 PA5 共用, PA5不在排针但 PINCM6 引出为 PA31) */
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM6,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    /* PA7 = PINCM14 */
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM14,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    /* PA25 = PINCM55 */
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM55,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    for (int i = 0; i < BTN_NUM; i++) {
        btn_history[i] = 0xFF;
        btn_stable[i]  = false;
        btn_prev[i]    = false;
    }
}

bool button_pressed(int n)
{
    if (n < 0 || n >= BTN_NUM) return false;
    uint32_t raw = DL_GPIO_readPins(btn_cfg[n].port, btn_cfg[n].pin);
    uint8_t cur = (raw == 0) ? 0 : 1;
    btn_history[n] = (uint8_t)((btn_history[n] << 1) | cur);
    if (btn_history[n] == 0x00)      btn_stable[n] = true;
    else if (btn_history[n] == 0xFF) btn_stable[n] = false;
    return btn_stable[n];
}

bool button_falling_edge(int n)
{
    bool cur = button_pressed(n);
    bool edge = (cur && !btn_prev[n]);
    btn_prev[n] = cur;
    return edge;
}
