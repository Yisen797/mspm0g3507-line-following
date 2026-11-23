/**
 * @file servo.c 双舵机 TIMG7 PWM PA26/PA27 (SDK 2.11)
 */
#include "servo.h"
#define PRD 39999U
#define MIN 1000U
#define MAX 5000U

static const DL_TimerG_ClockConfig svClock = {.clockSel=DL_TIMER_CLOCK_BUSCLK,.divideRatio=DL_TIMER_CLOCK_DIVIDE_1,.prescale=0U};
static const DL_TimerG_PWMConfig svPWM = {.pwmMode=DL_TIMER_PWM_MODE_EDGE_ALIGN,.period=PRD,.isTimerWithFourCC=false,.startTimer=DL_TIMER_STOP};

void servo_init(void)
{
    DL_TimerG_reset(SERVO_TIMER); DL_TimerG_enablePower(SERVO_TIMER);
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM59, IOMUX_PINCM59_PF_TIMG7_CCP0);
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM60, IOMUX_PINCM60_PF_TIMG7_CCP1);
    DL_TimerG_setClockConfig(SERVO_TIMER, (DL_TimerG_ClockConfig *)&svClock);
    DL_TimerG_initPWMMode(SERVO_TIMER, (DL_TimerG_PWMConfig *)&svPWM);
    DL_TimerG_setCaptureCompareOutCtl(SERVO_TIMER,DL_TIMER_CC_OCTL_INIT_VAL_LOW,DL_TIMER_CC_OCTL_INV_OUT_DISABLED,DL_TIMER_CC_OCTL_SRC_FUNCVAL,DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptureCompareOutCtl(SERVO_TIMER,DL_TIMER_CC_OCTL_INIT_VAL_LOW,DL_TIMER_CC_OCTL_INV_OUT_DISABLED,DL_TIMER_CC_OCTL_SRC_FUNCVAL,DL_TIMERG_CAPTURE_COMPARE_1_INDEX);
    DL_TimerG_setCaptureCompareValue(SERVO_TIMER,MIN,DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptureCompareValue(SERVO_TIMER,MIN,DL_TIMERG_CAPTURE_COMPARE_1_INDEX);
    DL_TimerG_enableClock(SERVO_TIMER);
}

void servo_set(uint8_t ch, uint8_t deg) {
    if(deg>180)deg=180;
    uint32_t p=MIN+(uint32_t)deg*(MAX-MIN)/180U;
    DL_TimerG_setCaptureCompareValue(SERVO_TIMER,p,ch?DL_TIMERG_CAPTURE_COMPARE_1_INDEX:DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
}
