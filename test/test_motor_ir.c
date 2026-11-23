/**
 * 电机 + PB21: 30%占空比, 按下前进/停止
 * 方向: 左=BIN1(H)/BIN2(L), 右=AIN1(L)/AIN2(H)
 */
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_common.h>

int main(void) {
    DL_GPIO_reset(GPIOB); DL_GPIO_enablePower(GPIOB);

    DL_GPIO_initDigitalOutput(IOMUX_PINCM25);  /* PB8 PWM */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM26);  /* PB9 PWM */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM15);  /* PB2 AIN1 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM16);  /* PB3 AIN2 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM17);  /* PB4 BIN1 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM18);  /* PB5 BIN2 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM50);  /* PB22 LED */
    DL_GPIO_enableOutput(GPIOB, (0xFF << 2) | DL_GPIO_PIN_22);
    DL_GPIO_clearPins(GPIOB, (0xFF << 2) | DL_GPIO_PIN_22);

    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM49, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    int running = 0, prev = 1, pwm_cnt = 0;
    while(1) {
        int btn = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_21) == 0) ? 0 : 1;
        if (btn == 0 && prev == 1) running = !running;
        prev = btn;

        if (running) {
            DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_22 | DL_GPIO_PIN_4 | DL_GPIO_PIN_3); /* LED+BIN1+AIN2 */
            DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_5 | DL_GPIO_PIN_2);                  /* BIN2+AIN1 */
            if (pwm_cnt < 30) DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_8 | DL_GPIO_PIN_9);
            else              DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_8 | DL_GPIO_PIN_9);
            if (++pwm_cnt >= 100) pwm_cnt = 0;
        } else {
            DL_GPIO_clearPins(GPIOB, (0xFF << 2) | DL_GPIO_PIN_22);
            pwm_cnt = 0;
        }
        DL_Common_delayCycles(1280);  /* ~0.8ms per step */
    }
}
