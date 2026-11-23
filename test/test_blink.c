/* PB22 板载LED闪烁 */
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/dl_common.h>
#include <ti/driverlib/dl_gpio.h>

#define DELAY_MS    32000000  /* ~1s @ 32MHz */

int main(void)
{
    DL_GPIO_reset(GPIOB);
    DL_GPIO_enablePower(GPIOB);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM50);
    DL_GPIO_enableOutput(GPIOB, DL_GPIO_PIN_22);

    while (1) {
        DL_GPIO_togglePins(GPIOB, DL_GPIO_PIN_22);
        DL_Common_delayCycles(DELAY_MS);
    }
    return 0;
}
