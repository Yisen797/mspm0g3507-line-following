/**
 * PB6/PB7(扩展板LED) 关闭, PB22(板载LED) 闪3下后常亮
 */
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_common.h>

int main(void) {
    DL_GPIO_reset(GPIOB); DL_GPIO_enablePower(GPIOB);

    /* 扩展板LED: PB6 PB7 全关 (高电平=灭) */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM23);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM24);
    DL_GPIO_enableOutput(GPIOB, DL_GPIO_PIN_6 | DL_GPIO_PIN_7);
    DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_6 | DL_GPIO_PIN_7);

    /* 板载LED: PB22 闪3下后常亮 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM50);
    DL_GPIO_enableOutput(GPIOB, DL_GPIO_PIN_22);

    for (int i = 0; i < 3; i++) {
        DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_22);     /* 亮 (高电平) */
        DL_Common_delayCycles(8000000);
        DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_22);   /* 灭 */
        DL_Common_delayCycles(8000000);
    }
    DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_22);  /* 常亮 */

    while(1);
}
