/**
 * @file ultrasonic.c HC-SR04 PA14/PA15 TIMG1 (SDK 2.11)
 */
#include "ultrasonic.h"
static const DL_TimerG_ClockConfig usClock = {.clockSel=DL_TIMER_CLOCK_BUSCLK,.divideRatio=DL_TIMER_CLOCK_DIVIDE_8,.prescale=0U};

void ultrasonic_init(void)
{
    DL_GPIO_reset(US_TRIG_PORT); DL_GPIO_enablePower(US_TRIG_PORT);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM36);
    DL_GPIO_clearPins(US_TRIG_PORT, US_TRIG_PIN);
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM37, DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN, DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_TimerG_reset(US_TIMER); DL_TimerG_enablePower(US_TIMER);
    DL_TimerG_setClockConfig(US_TIMER, (DL_TimerG_ClockConfig *)&usClock);
    DL_TimerG_setLoadValue(US_TIMER, 0xFFFFFFFF);
    DL_TimerG_startCounter(US_TIMER);
}

float ultrasonic_read(void)
{
    uint32_t ts,te;
    DL_GPIO_setPins(US_TRIG_PORT, US_TRIG_PIN);
    { uint32_t t0=DL_TimerG_getTimerCount(US_TIMER); while((DL_TimerG_getTimerCount(US_TIMER)-t0)<15); }
    DL_GPIO_clearPins(US_TRIG_PORT, US_TRIG_PIN);
    uint32_t t0=DL_TimerG_getTimerCount(US_TIMER);
    while(!DL_GPIO_readPins(US_ECHO_PORT,US_ECHO_PIN)) if((DL_TimerG_getTimerCount(US_TIMER)-t0)>38000) return -1.0f;
    ts=DL_TimerG_getTimerCount(US_TIMER);
    while(DL_GPIO_readPins(US_ECHO_PORT,US_ECHO_PIN)) if((DL_TimerG_getTimerCount(US_TIMER)-ts)>38000) return -1.0f;
    te=DL_TimerG_getTimerCount(US_TIMER);
    uint32_t pulse=(te>=ts)?(te-ts):(0xFFFFFFFF-ts+te+1);
    return (float)pulse/58.0f;
}
