/**
 * @file    tracker.c
 * @brief   寻迹控制算法实现 (与 STM32 原版完全一致)
 *
 * 算法流程:
 *   传感器 → 插值计算线位置 → 低通滤波 → PID(带死区) → 输出平滑 → 差速驱动
 *
 * 与原版仅有的差异:
 *   - include 路径改为 mspm0_config.h
 *   - PWM 范围: 1000(原始) → 10000(移植)
 *     因此 base_speed 和 correction 范围不同。通过比例缩放解决。
 *     推荐 base_speed = 1800 (等效原版 180)
 */

#include "tracker.h"
#include "pid.h"
#include "sensor.h"
#include "motor.h"
#include "mspm0_config.h"
#include <math.h>

/*===========================================================================
 * 常量
 *==========================================================================*/
#define SENSOR_COUNT   8
#define CENTER_POS     3.5f

/* PID 参数 (与原版完全一致) */
#define TRACK_KP       2.5f
#define TRACK_KI       0.05f
#define TRACK_KD       5.0f
#define INTEGRAL_MAX   150.0f
#define CORRECTION_MAX 500.0f

/*===========================================================================
 * 全局状态
 *==========================================================================*/
static PID   track_pid;
static float current_error    = 0.0f;
static float prev_correction  = 0.0f;
static float filtered_error   = 0.0f;
static float last_pos         = CENTER_POS;
static int   corner_mode      = 0;   /* 0=正常 1=左转 2=右转 */
static int   corner_timer     = 0;

/*===========================================================================
 * 带插值的线位置计算
 *==========================================================================*/
static float calc_position(uint8_t raw)
{
    float sum_weight = 0.0f, sum_count = 0.0f;

    for (int i = 0; i < SENSOR_COUNT; i++) {
        if (raw & (1 << i)) {
            float fine = (float)i;
            if (i > 0 && (raw & (1 << (i - 1)))) fine -= 0.25f;
            if (i < SENSOR_COUNT - 1 && (raw & (1 << (i + 1)))) fine += 0.25f;
            sum_weight += fine;
            sum_count  += 1.0f;
        }
    }

    if (sum_count == 0.0f) return -1.0f;
    return sum_weight / sum_count;
}

/*===========================================================================
 * 公共接口
 *==========================================================================*/

void tracker_init(void)
{
    pid_init(&track_pid, TRACK_KP, TRACK_KI, TRACK_KD,
             0.02f, INTEGRAL_MAX, CORRECTION_MAX);
    prev_correction = 0.0f;
    filtered_error  = 0.0f;
    last_pos        = CENTER_POS;
    corner_mode     = 0;
    corner_timer    = 0;
}

int tracker_update(float dt, int16_t base_speed)
{
    uint8_t raw = sensor_read();

    /* ---- 完全丢线: 慢速直行等线恢复 ---- */
    if (raw == 0) {
        motor_left_set(base_speed / 3);
        motor_right_set(base_speed / 3);
        filtered_error  = 0.0f;
        prev_correction = 0.0f;
        pid_reset(&track_pid);
        corner_mode = 0;
        return -1;
    }

    /* ---- 直角弯模式: 单轮停转, 等2+传感器检测到线 ---- */
    if (corner_mode > 0) {
        int16_t ts = base_speed / 3;
        if (corner_mode == 1) {
            motor_left_set(ts); motor_right_set(0);   /* 左转 */
        } else {
            motor_left_set(0);  motor_right_set(ts);  /* 右转 */
        }
        corner_timer++;
        int cnt = 0;
        for (int i = 0; i < 8; i++) if (raw & (1 << i)) cnt++;
        if (cnt >= 2) {
            corner_mode = 0;
            pid_reset(&track_pid);
            prev_correction = 0.0f; filtered_error = 0.0f;
            last_pos = CENTER_POS;
        }
        if (corner_timer > 300) { corner_mode = 0; }  /* 超时 ~6s */
        return 0;
    }

    float pos = calc_position(raw);
    float raw_error = CENTER_POS - pos;

    /* ---- 直角弯触发: 位置跳变 > 2.5 ---- */
    float jump = pos - last_pos;
    if (fabsf(jump) > 2.5f && (pos < 1.0f || pos > 6.0f)) {
        corner_mode = (jump < 0) ? 1 : 2;
        corner_timer = 0;
        int16_t ts = base_speed / 3;
        if (corner_mode == 1) {
            motor_left_set(ts); motor_right_set(0);
        } else {
            motor_left_set(0);  motor_right_set(ts);
        }
        pid_reset(&track_pid);
        prev_correction = 0.0f; filtered_error = 0.0f;
        last_pos = pos;
        return 0;
    }
    last_pos = pos;

    /* ---- 误差低通滤波 ---- */
    #define ERR_SMOOTH  0.4f
    filtered_error = ERR_SMOOTH * raw_error
                   + (1.0f - ERR_SMOOTH) * filtered_error;

    /* ---- PID 死区 ---- */
    #define DEADBAND 0.2f
    float pid_input = (fabsf(filtered_error) < DEADBAND) ? 0.0f : filtered_error;

    current_error = filtered_error;
    track_pid.dt = dt;
    float raw_correction = pid_calc(&track_pid, 0.0f, pid_input);

    /* ---- 输出变化率限制 (柔和转向) ---- */
    #define MAX_DELTA  60.0f
    float delta = raw_correction - prev_correction;
    if (delta >  MAX_DELTA) raw_correction = prev_correction + MAX_DELTA;
    if (delta < -MAX_DELTA) raw_correction = prev_correction - MAX_DELTA;
    prev_correction = raw_correction;

    float correction = raw_correction;

    /* ---- 速度混合 ---- */
    if (base_speed <= 0) {
        motor_coast();
        return 0;
    }

    /* 修正量限幅: ±40% base_speed */
    float limit = (float)base_speed * 0.4f;
    if (correction >  limit) correction =  limit;
    if (correction < -limit) correction = -limit;

    int16_t left  = (int16_t)((float)base_speed + correction);
    int16_t right = (int16_t)((float)base_speed - correction);

    #define MIN_SPEED  60
    if (left  > 0 && left  <  MIN_SPEED) left  =  MIN_SPEED;
    if (right > 0 && right <  MIN_SPEED) right =  MIN_SPEED;
    if (left  < 0 && left  > -MIN_SPEED) left  = -MIN_SPEED;
    if (right < 0 && right > -MIN_SPEED) right = -MIN_SPEED;

    motor_left_set(left);
    motor_right_set(right);

    int cnt = 0;
    for (int i = 0; i < 8; i++) if (raw & (1 << i)) cnt++;
    return cnt;
}

void tracker_stop(void)
{
    motor_brake();
    pid_reset(&track_pid);
    prev_correction = 0.0f;
    filtered_error  = 0.0f;
}

float tracker_get_error(void)  { return current_error; }

void tracker_tune_pid(float kp, float ki, float kd)
{
    pid_set_gains(&track_pid, kp, ki, kd);
}
