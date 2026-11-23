/**
 * 最简电机测试: 纯GPIO PWM, 无传感器/无I2C/无看门狗
 * 验证: 基本GPIO翻转能否平稳驱动电机
 */
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_common.h>

int main(void) {
    DL_GPIO_reset(GPIOB); DL_GPIO_enablePower(GPIOB);
    DL_Common_delayCycles(16);

    /* PB8/9 = PWM, PB2-5 = 方向 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM25); /* PB8 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM26); /* PB9 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM15); /* PB2 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM16); /* PB3 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM17); /* PB4 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM18); /* PB5 */
    DL_GPIO_enableOutput(GPIOB,
        DL_GPIO_PIN_2|DL_GPIO_PIN_3|DL_GPIO_PIN_4|DL_GPIO_PIN_5|
        DL_GPIO_PIN_8|DL_GPIO_PIN_9);

    /* 前进方向 */
    DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_4|DL_GPIO_PIN_3);
    DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_5|DL_GPIO_PIN_2);
    DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_8|DL_GPIO_PIN_9);

    while(1) {
        /* 20%占空比 PWM, 500步 */
        for(int s=0; s<500; s++) {
            if(s < 100) {
                DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_8|DL_GPIO_PIN_9);
            } else {
                DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_8|DL_GPIO_PIN_9);
            }
        }
        /* 无传感器读取, 无PID, 无I2C, 无WWDT */
    }
}
