/**
 * @file    pid.h
 * @brief   PID 控制器 — 通用位置式 PID (平台无关)
 *
 * 本模块无硬件依赖, 与原始 STM32 版本完全兼容。
 * 仅修改了 include 路径。
 */

#ifndef __PID_H__
#define __PID_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float kp;           /* 比例系数 */
    float ki;           /* 积分系数 */
    float kd;           /* 微分系数 */
    float dt;           /* 控制周期 (秒) */

    float integral;     /* 积分累加值 */
    float integral_max; /* 积分上限 */
    float integral_min; /* 积分下限 */
    float prev_error;   /* 上次误差 (用于微分) */

    float output_min;   /* 输出下限 */
    float output_max;   /* 输出上限 */
} PID;

void pid_init(PID *pid, float kp, float ki, float kd, float dt,
              float integral_limit, float output_limit);

float pid_calc(PID *pid, float setpoint, float measurement);

void pid_reset(PID *pid);

void pid_set_gains(PID *pid, float kp, float ki, float kd);

#ifdef __cplusplus
}
#endif

#endif /* __PID_H__ */
