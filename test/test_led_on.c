/* PB22 板载LED常亮 (低电平有效) */
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/dl_common.h>
#include <ti/driverlib/dl_gpio.h>

int main(void)
{
    DL_GPIO_reset(GPIOB);
    DL_GPIO_enablePower(GPIOB);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM50);
    DL_GPIO_enableOutput(GPIOB, DL_GPIO_PIN_22);

    /* PB22 置高 → LED 亮 (高电平有效) */
    DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_22);

    while (1) {
        /* 常亮, 不操作 */
    }
    return 0;
}
