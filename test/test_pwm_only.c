/**
 * TIMA0 PWM: 完全复制蜂鸣器TIMA1的工作模式
 */
#include "mspm0_config.h"

int main(void) {
    DL_GPIO_reset(GPIOB); DL_GPIO_enablePower(GPIOB);
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM25, IOMUX_PINCM25_PF_TIMA0_CCP0);
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM26, IOMUX_PINCM26_PF_TIMA0_CCP1);

    /* 方向引脚 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM15);  /* PB2 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM16);  /* PB3 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM17);  /* PB4 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM18);  /* PB5 */
    DL_GPIO_enableOutput(GPIOB, DL_GPIO_PIN_2|DL_GPIO_PIN_3|DL_GPIO_PIN_4|DL_GPIO_PIN_5);
    DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_4|DL_GPIO_PIN_3);     /* 方向: 前进 */
    DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_5|DL_GPIO_PIN_2);

    /* TIMA0 (完全复制TIMA1蜂鸣器模式) */
    DL_TimerA_reset(TIMA0);
    DL_TimerA_enablePower(TIMA0);
    DL_TimerA_ClockConfig clk = {DL_TIMER_CLOCK_BUSCLK, DL_TIMER_CLOCK_DIVIDE_1, 0};
    DL_TimerA_setClockConfig(TIMA0, &clk);
    DL_TimerA_PWMConfig pwm = {.pwmMode=DL_TIMER_PWM_MODE_EDGE_ALIGN,.period=9999,.startTimer=DL_TIMER_STOP};
    DL_TimerA_initPWMMode(TIMA0, &pwm);
    DL_TimerA_setCaptureCompareOutCtl(TIMA0, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
        DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
        DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(TIMA0, 5000, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareOutCtl(TIMA0, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
        DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
        DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptureCompareValue(TIMA0, 5000, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_enableClock(TIMA0);
    DL_TimerA_startCounter(TIMA0);

    while(1);
}
