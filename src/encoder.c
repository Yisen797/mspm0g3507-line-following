/**
 * @file    encoder.c
 * @brief   双编码器 — TIMG0 QEI (左) + TIMG6 捕获 (右)
 *          已适配 SDK 2.11 API
 */
#include "encoder.h"

static volatile int32_t  enc_left_total  = 0;
static volatile int32_t  enc_right_total = 0;
static volatile uint16_t enc_left_last   = 0;
static volatile uint32_t enc_right_last  = 0;

/* --- TIMG0 QEI (PA12 CCP0, PA13 CCP1) --- */
static const DL_TimerG_ClockConfig qeiClock = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U
};

static void encoder_left_init_qei(void)
{
    DL_TimerG_reset(ENC_L_TIMER);
    DL_TimerG_enablePower(ENC_L_TIMER);

    /* PA12 = TIMG0_CCP0 (PINCM34), PA13 = TIMG0_CCP1 (PINCM35) */
    DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM34, IOMUX_PINCM34_PF_TIMG0_CCP0);
    DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM35, IOMUX_PINCM35_PF_TIMG0_CCP1);

    DL_TimerG_setClockConfig(ENC_L_TIMER, (DL_TimerG_ClockConfig *)&qeiClock);

    /* SDK 2.11: DL_Timer_configQEI(mode, invert, ccIndex) */
    DL_TimerG_configQEI(ENC_L_TIMER, DL_TIMER_QEI_MODE_2_INPUT,
        DL_TIMER_CC_INPUT_INV_NOINVERT,
        DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setLoadValue(ENC_L_TIMER, 0xFFFF);
    DL_TimerG_startCounter(ENC_L_TIMER);
}

int32_t encoder_read_left(void)
{
    uint32_t now = DL_TimerG_getTimerCount(ENC_L_TIMER);
    int16_t delta = (int16_t)((uint16_t)now - enc_left_last);
    enc_left_total += (int32_t)delta;
    enc_left_last = (uint16_t)now;
    return enc_left_total;
}

/* --- TIMG6 32-bit 输入捕获 (PA21 CCP0) --- */
static const DL_TimerG_ClockConfig capClock = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U
};

static const DL_TimerG_CaptureConfig capCfg = {
    .captureMode   = DL_TIMER_CAPTURE_MODE_EDGE_TIME_UP,
    .period        = 0xFFFFFFFF,
    .startTimer    = DL_TIMER_STOP,
    .edgeCaptMode  = DL_TIMER_CAPTURE_EDGE_DETECTION_MODE_RISING,
    .inputChan     = DL_TIMER_INPUT_CHAN_0,
    .inputInvMode  = DL_TIMER_CC_INPUT_INV_NOINVERT,
};

static void encoder_right_init_capture(void)
{
    DL_TimerG_reset(ENC_R_TIMER);
    DL_TimerG_enablePower(ENC_R_TIMER);

    /* PA21 = TIMG6_CCP0 (PINCM46), PA22 = GPIO 方向 (PINCM47) */
    DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM46, IOMUX_PINCM46_PF_TIMG6_CCP0);
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM47,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_TimerG_setClockConfig(ENC_R_TIMER, (DL_TimerG_ClockConfig *)&capClock);
    DL_TimerG_initCaptureMode(ENC_R_TIMER, (DL_TimerG_CaptureConfig *)&capCfg);
    DL_TimerG_setLoadValue(ENC_R_TIMER, 0xFFFFFFFF);
    DL_TimerG_startCounter(ENC_R_TIMER);
}

int32_t encoder_read_right(void)
{
    uint32_t now = DL_TimerG_getTimerCount(ENC_R_TIMER);
    uint32_t b   = DL_GPIO_readPins(ENC_R_B_PORT, ENC_R_B_PIN);
    int32_t delta = (int32_t)(now - enc_right_last);
    if (b != 0) delta = -delta;
    enc_right_total += delta;
    enc_right_last = now;
    return enc_right_total;
}

/* --- 速度 --- */
int32_t encoder_speed_left(float dt) {
    static int32_t last = 0; int32_t now = encoder_read_left();
    int32_t d = now - last; last = now;
    return dt > 0 ? (int32_t)((float)d / dt) : 0;
}
int32_t encoder_speed_right(float dt) {
    static int32_t last = 0; int32_t now = encoder_read_right();
    int32_t d = now - last; last = now;
    return dt > 0 ? -(int32_t)((float)d / dt) : 0;
}

float encoder_speed_left_mm_s(float dt)  {
    return (float)encoder_speed_left(dt)  * WHEEL_CIRCUMFERENCE / (float)PULSES_PER_REV;
}
float encoder_speed_right_mm_s(float dt) {
    return (float)encoder_speed_right(dt) * WHEEL_CIRCUMFERENCE / (float)PULSES_PER_REV;
}

void encoder_init(void)
{
    encoder_left_init_qei();
    encoder_right_init_capture();
    encoder_reset();
}

void encoder_reset(void)
{
    enc_left_total = enc_right_total = 0;
    enc_left_last = enc_right_last = 0;
    DL_TimerG_setTimerCount(ENC_L_TIMER, 0);
    DL_TimerG_setTimerCount(ENC_R_TIMER, 0);
}
