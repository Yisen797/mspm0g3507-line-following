/**
 * @file    motor.c
 * @brief   TB6612 电机驱动 — TIMA0 PWM (PB8=左 CCP0, PB9=右 CCP1)
 *          方向引脚: PB2=AIN1(右正转) PB3=AIN2(右反转)
 *                    PB4=BIN1(左正转) PB5=BIN2(左反转)
 */
#include "motor.h"

static const DL_TimerA_ClockConfig mtrClock = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U
};
static const DL_TimerA_PWMConfig mtrPWM = {
    .pwmMode     = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period      = MOTOR_PWM_PERIOD,
    .startTimer  = DL_TIMER_STOP
};

static void motor_hw_init(void)
{
    /* === 第1步: Reset + Power (严格按TI SysConfig顺序, 先reset再power) === */
    DL_GPIO_reset(GPIOB);
    DL_TimerA_reset(MOTOR_PWM_TIMER);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerA_enablePower(MOTOR_PWM_TIMER);
    delay_cycles(16);

    /* === 第2步: PWM引脚 (PB8 CCP0, PB9 CCP1) === */
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM25, IOMUX_PINCM25_PF_TIMA0_CCP0);
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM26, IOMUX_PINCM26_PF_TIMA0_CCP1);
    DL_GPIO_enableOutput(GPIOB, DL_GPIO_PIN_8 | DL_GPIO_PIN_9);

    /* === 第3步: TIMA0 PWM 配置 === */
    DL_TimerA_setClockConfig(MOTOR_PWM_TIMER, (DL_TimerA_ClockConfig *)&mtrClock);
    DL_TimerA_initPWMMode(MOTOR_PWM_TIMER, (DL_TimerA_PWMConfig *)&mtrPWM);

    /* 方向引脚: PB2~PB5 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM15);  /* PB2 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM16);  /* PB3 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM17);  /* PB4 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM18);  /* PB5 */
    DL_GPIO_enableOutput(GPIOB,
        DL_GPIO_PIN_2 | DL_GPIO_PIN_3 | DL_GPIO_PIN_4 | DL_GPIO_PIN_5);
    DL_GPIO_clearPins(GPIOB,
        DL_GPIO_PIN_2 | DL_GPIO_PIN_3 | DL_GPIO_PIN_4 | DL_GPIO_PIN_5);

    /* CCP0输出控制 */
    DL_TimerA_setCaptureCompareOutCtl(MOTOR_PWM_TIMER, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
        DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
        DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptCompUpdateMethod(MOTOR_PWM_TIMER,
        DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(MOTOR_PWM_TIMER, 0, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    /* CCP1输出控制 */
    DL_TimerA_setCaptureCompareOutCtl(MOTOR_PWM_TIMER, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
        DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
        DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptCompUpdateMethod(MOTOR_PWM_TIMER,
        DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptureCompareValue(MOTOR_PWM_TIMER, 0, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);

    /* === 第4步: 启动 === */
    DL_TimerA_setCCPDirection(MOTOR_PWM_TIMER,
        DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT);  /* 关键! 设CCP为输出 */
    DL_TimerA_enableClock(MOTOR_PWM_TIMER);
    DL_TimerA_startCounter(MOTOR_PWM_TIMER);
}

void motor_init(void) { motor_hw_init(); }

void motor_left_set(int16_t speed)
{
    uint16_t pwm;
    if (speed > 0) {
        /* 左电机正转: BIN1=H, BIN2=L */
        DL_GPIO_setPins(MOTOR_DIR_PORT, MOTOR_BIN1_PIN);
        DL_GPIO_clearPins(MOTOR_DIR_PORT, MOTOR_BIN2_PIN);
        pwm = (speed > (int16_t)MOTOR_PWM_MAX_DUTY) ? MOTOR_PWM_MAX_DUTY : (uint16_t)speed;
    } else if (speed < 0) {
        /* 左电机反转: BIN1=L, BIN2=H */
        DL_GPIO_setPins(MOTOR_DIR_PORT, MOTOR_BIN2_PIN);
        DL_GPIO_clearPins(MOTOR_DIR_PORT, MOTOR_BIN1_PIN);
        uint16_t a = (uint16_t)(-speed);
        pwm = (a > MOTOR_PWM_MAX_DUTY) ? MOTOR_PWM_MAX_DUTY : a;
    } else {
        DL_GPIO_clearPins(MOTOR_DIR_PORT, MOTOR_BIN1_PIN | MOTOR_BIN2_PIN);
        pwm = 0;
    }
    DL_TimerA_setCaptureCompareValue(MOTOR_PWM_TIMER, pwm,
        DL_TIMERA_CAPTURE_COMPARE_0_INDEX);  /* CCP0 = 左轮 */
}

void motor_right_set(int16_t speed)
{
    uint16_t pwm;
    if (speed > 0) {
        /* 右电机正转: AIN1=L, AIN2=H (实测校正 2026-07-22) */
        DL_GPIO_setPins(MOTOR_DIR_PORT, MOTOR_AIN2_PIN);
        DL_GPIO_clearPins(MOTOR_DIR_PORT, MOTOR_AIN1_PIN);
        pwm = (speed > (int16_t)MOTOR_PWM_MAX_DUTY) ? MOTOR_PWM_MAX_DUTY : (uint16_t)speed;
    } else if (speed < 0) {
        /* 右电机反转: AIN1=H, AIN2=L */
        DL_GPIO_setPins(MOTOR_DIR_PORT, MOTOR_AIN1_PIN);
        DL_GPIO_clearPins(MOTOR_DIR_PORT, MOTOR_AIN2_PIN);
        uint16_t a = (uint16_t)(-speed);
        pwm = (a > MOTOR_PWM_MAX_DUTY) ? MOTOR_PWM_MAX_DUTY : a;
    } else {
        DL_GPIO_clearPins(MOTOR_DIR_PORT, MOTOR_AIN1_PIN | MOTOR_AIN2_PIN);
        pwm = 0;
    }
    DL_TimerA_setCaptureCompareValue(MOTOR_PWM_TIMER, pwm,
        DL_TIMERA_CAPTURE_COMPARE_1_INDEX);  /* CCP1 = 右轮 */
}

void motor_brake(void)
{
    DL_GPIO_setPins(MOTOR_DIR_PORT,
        MOTOR_AIN1_PIN | MOTOR_AIN2_PIN | MOTOR_BIN1_PIN | MOTOR_BIN2_PIN);
    DL_TimerA_setCaptureCompareValue(MOTOR_PWM_TIMER, 0, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(MOTOR_PWM_TIMER, 0, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
}

void motor_coast(void)
{
    DL_GPIO_clearPins(MOTOR_DIR_PORT,
        MOTOR_AIN1_PIN | MOTOR_AIN2_PIN | MOTOR_BIN1_PIN | MOTOR_BIN2_PIN);
    DL_TimerA_setCaptureCompareValue(MOTOR_PWM_TIMER, 0, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(MOTOR_PWM_TIMER, 0, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
}
