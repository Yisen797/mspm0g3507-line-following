/**
 * @file    main.c
 * @brief   三轮寻迹小车 — MSPM0G3507 天猛星移植版
 *
 * 显示: 0.96" SSD1306 OLED (I2C0, PA0=SDA, PA1=SCL)
 * 控制: BTN3(PA25) 启动/停止, 8路IR+, PID → TB6612 差速转向
 *
 * 引脚总览见 mspm0_config.h
 */

#include "mspm0_config.h"
#include "delay.h"
#include "oled_ssd1306.h"
#include "sensor.h"
#include "motor.h"
#include "tracker.h"
#include "button.h"

/* --- LED 宏 (PB6=LED1, PB7=LED2, 低电平有效) --- */
#define LED1_ON()   DL_GPIO_clearPins(LED1_PORT, LED1_PIN)
#define LED1_OFF()  DL_GPIO_setPins(LED1_PORT, LED1_PIN)
#define LED2_ON()   DL_GPIO_clearPins(LED2_PORT, LED2_PIN)
#define LED2_OFF()  DL_GPIO_setPins(LED2_PORT, LED2_PIN)

/*===========================================================================
 * 时钟初始化: SYSOSC 32MHz (默认)
 *
 * TODO: SYSOSC(32MHz) → SYSPLL(80MHz)
 * SDK 2.11 API 已变化: 需要 DL_SYSCTL_SYSPLLConfig 结构体 +
 * DL_SYSCTL_enableSYSPLL() + DL_SYSCTL_setMCLKSource(SYSOSC, HSCLK, ...)
 * 目前先用默认 32MHz 运行, 后续再加 PLL。
 *==========================================================================*/
static void clock_init_32mhz(void)
{
    /* SYSOSC 默认即 32MHz Base 频率, 无需额外配置 */
    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
}

/*===========================================================================
 * LED 初始化
 *==========================================================================*/
static void led_init(void)
{
    DL_GPIO_reset(GPIOB);
    DL_GPIO_enablePower(GPIOB);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM23);           /* PB6 = LED1 */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM24);           /* PB7 = LED2 */
    DL_GPIO_enableOutput(GPIOB, LED1_PIN | LED2_PIN);  /* 方向=输出 */
    LED1_OFF(); LED2_OFF();
}

/*===========================================================================
 * OLED 启动画面
 *==========================================================================*/
static void show_splash(void)
{
    oled_chess();
    delay_ms(150);
    oled_clear();
    oled_text(0, 1, "TRACKER READY");
    oled_text(0, 3, "PA25: Start/Stop");
    oled_text(0, 5, "MSPM0G3507");
}

/*===========================================================================
 * OLED 显示 8 路红外状态
 *==========================================================================*/
static void oled_show_ir(uint8_t raw)
{
    char buf[17] = "IR:12345678     ";
    /* 覆盖列0清除旧内容 */
    oled_text(0, 0, "                ");
    oled_text(0, 2, "                ");
    oled_text(0, 4, "                ");
    oled_text(0, 6, "                ");
    /* 显示传感器值: '1'=检测到线 */
    for (int i = 0; i < 8; i++)
        buf[3 + i] = (raw & (1 << i)) ? '1' : '0';
    oled_text(0, 0, buf);

    /* 显示位置估算 */
    int cnt = 0; float sum = 0;
    for (int i = 0; i < 8; i++) {
        if (raw & (1 << i)) { sum += (float)i; cnt++; }
    }
    if (cnt > 0) {
        float pos = sum / (float)cnt;
        int ip = (int)(pos * 100.0f);
        buf[0] = 'P'; buf[1] = 'o'; buf[2] = 's'; buf[3] = ':';
        buf[4] = '0' + ip / 100; buf[5] = '.';
        buf[6] = '0' + (ip / 10) % 10; buf[7] = '0' + ip % 10;
        buf[8] = ' '; buf[9] = 'c'; buf[10] = ':';
        buf[11] = '0' + cnt; buf[12] = 0;
        oled_text(0, 3, buf);
    } else {
        oled_text(0, 3, "Pos: LOST!");
    }
}

/*===========================================================================
 * 主函数
 *==========================================================================*/
int main(void)
{
    /* ---- 1. 初始化 ---- */
    clock_init_32mhz();
    led_init();

    /* 启动闪烁 */
    for (int i = 0; i < 3; i++) {
        LED1_ON(); LED2_ON();  delay_ms(80);
        LED1_OFF(); LED2_OFF(); delay_ms(80);
    }

    oled_init();
    button_init();
    sensor_init();
    motor_init();
    motor_brake();
    tracker_init();

    show_splash();

    /* ---- 2. 状态变量 ---- */
    int       running     = 0;
    int       motors_on   = 0;
    uint32_t  motor_timer = 0;
    uint32_t  lost_ticks  = 0;
    int       line_cnt    = 0;

    /* PWM 范围: 0~10000, base_speed 缩放 10× (原180→现1800) */
    const int16_t base_speed = 1800;
    const float   dt         = 0.02f;

    int dc = 0;

    /* ---- 3. 主循环 ---- */
    while (1)
    {
        /* --- 按键 BTN3(PA25) 启动/停止 --- */
        if (button_falling_edge(2)) {  /* BTN3=PA25 */
            running = !running;
            if (running) {
                tracker_init();
                lost_ticks  = 0;
                motors_on   = 0;
                motor_timer = 0;
                oled_clear();
                oled_text(0, 1, "--- RUNNING ---");
                LED1_ON();
            } else {
                tracker_stop();
                motors_on = 0;
                oled_clear();
                oled_text(0, 1, "TRACKER READY");
                oled_text(0, 3, "  Motor: STOP");
                LED1_OFF();
            }
            delay_ms(200);
        }

        /* --- 控制循环 20ms --- */
        if (running) {
            delay_ms(20);
            if (motors_on) {
                line_cnt = tracker_update(dt, base_speed);
            } else {
                line_cnt = 0;
                sensor_read();
            }
        } else {
            delay_ms(50);
        }

        /* --- 电机延迟启动 1s --- */
        if (running && !motors_on) {
            motor_timer++;
            if (motor_timer > 50) {
                motors_on = 1;
                LED2_ON();
            }
        }

        /* --- 丢线安全: 连续500ms无检测 → 刹车 --- */
        sensor_read();  /* 保持传感器刷新 */
        if (running && line_cnt <= 0) lost_ticks++;
        else lost_ticks = 0;
        if (lost_ticks > 25) {
            tracker_stop();
            lost_ticks = 0;
            LED2_OFF();
        }

        /* --- 显示刷新 --- */
        if (++dc >= 5) {
            dc = 0;
            if (running) {
                oled_show_ir(sensor_read());
            }
        }
    }

    return 0;
}
