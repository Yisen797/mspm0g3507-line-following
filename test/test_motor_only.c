/**
 * 步骤1: 纯电机, 确认GPIO软件PWM正常
 */
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_common.h>
#include "oled_ssd1306.h"

int main(void) {
    oled_init();
    oled_clear();
    oled_text(0,3,"PURE MOTOR TEST");

    DL_GPIO_reset(GPIOB); DL_GPIO_enablePower(GPIOB); delay_cycles(16);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM25); DL_GPIO_initDigitalOutput(IOMUX_PINCM26);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM15); DL_GPIO_initDigitalOutput(IOMUX_PINCM16);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM17); DL_GPIO_initDigitalOutput(IOMUX_PINCM18);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM50);
    DL_GPIO_enableOutput(GPIOB, 0x3FC|DL_GPIO_PIN_22);
    DL_GPIO_clearPins(GPIOB, 0x3FC|DL_GPIO_PIN_22);
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM49, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    int running=0, prev=1, pwm=0;
    while(1) {
        int btn=(DL_GPIO_readPins(GPIOB,DL_GPIO_PIN_21)==0)?0:1;
        if(btn==0&&prev==1){
            running=!running;
            if(running){
                DL_GPIO_setPins(GPIOB,DL_GPIO_PIN_22|DL_GPIO_PIN_4|DL_GPIO_PIN_3);
                DL_GPIO_clearPins(GPIOB,DL_GPIO_PIN_5|DL_GPIO_PIN_2);
            }else{
                DL_GPIO_setPins(GPIOB,DL_GPIO_PIN_2|DL_GPIO_PIN_3|DL_GPIO_PIN_4|DL_GPIO_PIN_5);
                DL_GPIO_clearPins(GPIOB,DL_GPIO_PIN_8|DL_GPIO_PIN_9|DL_GPIO_PIN_22);
            }
            DL_Common_delayCycles(8000000);
        }
        prev=btn;
        if(running){
            if(pwm<30)DL_GPIO_setPins(GPIOB,DL_GPIO_PIN_8|DL_GPIO_PIN_9);
            else DL_GPIO_clearPins(GPIOB,DL_GPIO_PIN_8|DL_GPIO_PIN_9);
            if(++pwm>=100)pwm=0;
        }else{pwm=0;}
        DL_Common_delayCycles(640);
    }
}
