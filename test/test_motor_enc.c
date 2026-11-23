/**
 * 步骤3: 排查TIMG8哪一步导致电机失败
 * 测试A: 只reset+power TIMG8, 不配置QEI
 */
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_common.h>
#include <ti/driverlib/dl_timerg.h>
#include "oled_ssd1306.h"

int main(void) {
    oled_init();
    oled_clear();

    /* TIMG8 完整QEI (按官方例程) */
    DL_TimerG_reset(TIMG8);
    DL_TimerG_enablePower(TIMG8);
    DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM4, IOMUX_PINCM4_PF_TIMG8_CCP0);
    DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM5, IOMUX_PINCM5_PF_TIMG8_CCP1);
    DL_TimerG_setClockConfig(TIMG8, &(DL_TimerG_ClockConfig){
        DL_TIMER_CLOCK_BUSCLK, DL_TIMER_CLOCK_DIVIDE_1, 0});
    DL_TimerG_configQEI(TIMG8, DL_TIMER_QEI_MODE_2_INPUT,
        DL_TIMER_CC_INPUT_INV_NOINVERT, DL_TIMER_CC_0_INDEX);
    DL_TimerG_configQEI(TIMG8, DL_TIMER_QEI_MODE_2_INPUT,
        DL_TIMER_CC_INPUT_INV_NOINVERT, DL_TIMER_CC_1_INDEX);
    DL_TimerG_setLoadValue(TIMG8, 65535);
    DL_TimerG_enableClock(TIMG8);
    /* 不调startCounter, 看是不是这行的问题 */

    /* startCounter 放到电机启动后, 避免初始化时冲突 */
    oled_text(0,3,"T8 QEI deferred");

    /* 电机 (完全不变的代码) */
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
                DL_TimerG_startCounter(TIMG8);  /* 电机启动后再开编码器 */
                DL_TimerG_setTimerCount(TIMG8,0);
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
