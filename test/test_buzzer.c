/**
 * 蜂鸣器: PA17拉低就响, 响一声
 */
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_common.h>

int main(void) {
    DL_GPIO_reset(GPIOA); DL_GPIO_enablePower(GPIOA);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM39);
    DL_GPIO_enableOutput(GPIOA, DL_GPIO_PIN_17);
    DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_17);   /* 关 */

    DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_17);  /* 响 */
    DL_Common_delayCycles(16000000);            /* 0.5s */
    DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_17);    /* 停 */

    while(1);
}
