/* OLED先, 延迟再电机 */
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_common.h>
#include "oled_ssd1306.h"
int main(void) {
    oled_init();  /* OLED先 - 内部会GPIOA/I2C reset */
    oled_clear();
    oled_text(0,3,"OLED FIRST");

    for(volatile int i=0;i<100000;i++);  /* 等一会儿 */

    /* 电机: 重新reset GPIOB */
    DL_GPIO_reset(GPIOB); DL_GPIO_enablePower(GPIOB);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM25); DL_GPIO_initDigitalOutput(IOMUX_PINCM26);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM15); DL_GPIO_initDigitalOutput(IOMUX_PINCM16);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM17); DL_GPIO_initDigitalOutput(IOMUX_PINCM18);
    DL_GPIO_enableOutput(GPIOB, 0x3FC);
    DL_GPIO_clearPins(GPIOB, 0x3FC);
    DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_4|DL_GPIO_PIN_3);

    oled_text(0,5,"MOTOR ON");

    while(1) {
        DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_8|DL_GPIO_PIN_9);
        for(volatile int i=0;i<15000;i++);
        DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_8|DL_GPIO_PIN_9);
        for(volatile int i=0;i<35000;i++);
    }
}
