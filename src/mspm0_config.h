/**
 * @file    mspm0_config.h
 * @brief   MSPM0G3507 天猛星开发板 — 寻迹小车完整引脚映射 (2026-07-19 修正版)
 *
 * ⚠️ 本文件已与 PINOUT.md 完全对齐。修改前请确认不影响已有 PCB。
 *
 * 芯片:  MSPM0G3507, Cortex-M0+, 80MHz, 128KB Flash, 32KB SRAM
 * 板子:  立创天猛星 TMX-MSPM0G3507 (LQFP64)
 *
 * 物理引脚共享注意事项:
 *   PA2/PA28 共用 PINCM3 — 使用 PA2(IR X6), 舍弃 PA28(BTN1)
 *   PA5/PA31 共用 PINCM6 — PA5不在排针, PA31 可用作 BTN2
 *   PA3/PA29 共用 PINCM4 — PA3空闲, PA29 可用
 *   PA4/PA30 共用 PINCM5 — PA4空闲, PA30 可用
 */

#ifndef __MSPM0_CONFIG_H__
#define __MSPM0_CONFIG_H__

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * 系统时钟: SYSOSC(32MHz) → SYSPLL → 80MHz
 *==========================================================================*/
#define SYSTEM_CLOCK        80000000UL
#define CPUCLK_FREQ         80000000UL
#define DELAY_MS(ms)        delay_cycles((CPUCLK_FREQ / 1000UL) * (ms))
#define DELAY_US(us)        delay_cycles((CPUCLK_FREQ / 1000000UL) * (us))

/*===========================================================================
 * 编码器参数 (与 STM32 原版一致)
 *==========================================================================*/
#define PULSES_PER_REV      390UL
#define WHEEL_DIA_MM        65U
#define WHEEL_CIRCUMFERENCE 204.2f

/*===========================================================================
 * 1. I2C0 — OLED (0x3C)
 *    PA0: I2C0_SDA (PINCM1, PF3)
 *    PA1: I2C0_SCL (PINCM2, PF3)
 *==========================================================================*/
#define I2C0_BUS                I2C0
#define I2C0_SDA_PORT           GPIOA
#define I2C0_SDA_PIN            DL_GPIO_PIN_0       /* PA0 */
#define I2C0_SDA_IOMUX          IOMUX_PINCM1
#define I2C0_SCL_PORT           GPIOA
#define I2C0_SCL_PIN            DL_GPIO_PIN_1       /* PA1 */
#define I2C0_SCL_IOMUX          IOMUX_PINCM2
#define OLED_7BIT_ADDR          0x3C
#define I2C_SPEED_HZ            400000U

/*===========================================================================
 * 2. I2C1 — MPU6050 (0x68)
 *    PA30: I2C1_SDA (PINCM5, PF2)
 *    PA29: I2C1_SCL (PINCM4, PF2)
 *    ⚠️ PA30 与 PA4 共用 PINCM5, PA29 与 PA3 共用 PINCM4 (均无冲突)
 *==========================================================================*/
#define I2C1_BUS                I2C1
#define I2C1_SDA_PORT           GPIOA
#define I2C1_SDA_PIN            DL_GPIO_PIN_30      /* PA30 */
#define I2C1_SDA_IOMUX          IOMUX_PINCM5
#define I2C1_SCL_PORT           GPIOA
#define I2C1_SCL_PIN            DL_GPIO_PIN_29      /* PA29 */
#define I2C1_SCL_IOMUX          IOMUX_PINCM4
#define MPU6050_7BIT_ADDR       0x68

/*===========================================================================
 * 3. 左编码器 — TIMG0 硬件 QEI (正交编码器, 4倍频)
 *    PA12: TIMG0_C0 (PINCM34, PF4)
 *    PA13: TIMG0_C1 (PINCM35, PF5)
 *==========================================================================*/
#define ENC_L_TIMER             TIMG0
#define ENC_L_A_PORT            GPIOA
#define ENC_L_A_PIN             DL_GPIO_PIN_12      /* PA12 */
#define ENC_L_A_IOMUX           IOMUX_PINCM34
#define ENC_L_B_PORT            GPIOA
#define ENC_L_B_PIN             DL_GPIO_PIN_13      /* PA13 */
#define ENC_L_B_IOMUX           IOMUX_PINCM35

/*===========================================================================
 * 4. 右编码器 — TIMG6 32-bit 输入捕获 + GPIO 方向判断
 *    PA21: TIMG6_C0 (PINCM46, PF6) — A相
 *    PA22: GPIO (PINCM47) — B相
 *    ⚠️ PA21 标为特殊引脚(VREF-), 检查板上跳线帽
 *==========================================================================*/
#define ENC_R_TIMER             TIMG6
#define ENC_R_A_PORT            GPIOA
#define ENC_R_A_PIN             DL_GPIO_PIN_21      /* PA21 */
#define ENC_R_A_IOMUX           IOMUX_PINCM46
#define ENC_R_B_PORT            GPIOA
#define ENC_R_B_PIN             DL_GPIO_PIN_22      /* PA22 */
#define ENC_R_B_IOMUX           IOMUX_PINCM47

/*===========================================================================
 * 5. TB6612 电机驱动 — TIMA0 高级定时器 PWM
 *    PB8: TIMA0_C0 (PINCM25, PF4) — 左轮 PWM
 *    PB9: TIMA0_C1 (PINCM26, PF4) — 右轮 PWM
 *    PB2: AIN1 (右正转)      PB3: AIN2 (右反转)
 *    PB4: BIN1 (左正转)      PB5: BIN2 (左反转)
 *    PWM: 8kHz, 10000步精度
 *==========================================================================*/
#define MOTOR_PWM_TIMER         TIMA0
#define MOTOR_PWM_FREQ_HZ       8000U
#define MOTOR_PWM_PRESCALER     0U
#define MOTOR_PWM_PERIOD        9999U
#define MOTOR_PWM_MAX_DUTY      9999U

#define MOTOR_L_PWM_PORT        GPIOB
#define MOTOR_L_PWM_PIN         DL_GPIO_PIN_8       /* PB8 */
#define MOTOR_L_PWM_IOMUX       IOMUX_PINCM25
#define MOTOR_R_PWM_PORT        GPIOB
#define MOTOR_R_PWM_PIN         DL_GPIO_PIN_9       /* PB9 */
#define MOTOR_R_PWM_IOMUX       IOMUX_PINCM26

/* 方向引脚 */
#define MOTOR_DIR_PORT          GPIOB
#define MOTOR_AIN1_PIN          DL_GPIO_PIN_2       /* PB2 — 右正转 */
#define MOTOR_AIN2_PIN          DL_GPIO_PIN_3       /* PB3 — 右反转 */
#define MOTOR_BIN1_PIN          DL_GPIO_PIN_4       /* PB4 — 左正转 */
#define MOTOR_BIN2_PIN          DL_GPIO_PIN_5       /* PB5 — 左反转 */

/*===========================================================================
 * 6. 8路红外传感器 — GPIO 输入 (低电平=检测到黑线)
 *    X1=PB11  X2=PB12  X3=PB13  X4=PB14  X5=PB15
 *    X6=PA2   X7=PB10  X8=PB16
 *    ⚠️ PA2(X6) 只能做输入 (与 ROSC 校准电阻共用)
 *==========================================================================*/
#define IR_PORT_A               GPIOB
#define IR_PINS_A               (DL_GPIO_PIN_11 | DL_GPIO_PIN_12 | \
                                 DL_GPIO_PIN_13 | DL_GPIO_PIN_14 | \
                                 DL_GPIO_PIN_15)    /* PB11~PB15 = X1~X5 */
#define IR_SHIFT_A              11U

#define IR_PORT_B               GPIOB
#define IR_PINS_B               (DL_GPIO_PIN_10 | DL_GPIO_PIN_16)  /* PB10=X7, PB16=X8 */
#define IR_SHIFT_B              10U

#define IR_PORT_C               GPIOA
#define IR_PIN_C                DL_GPIO_PIN_2       /* PA2 = X6 */
#define IR_SHIFT_C              2U

/*===========================================================================
 * 7. 串口
 *    UART0: PA10(TX, PINCM21), PA11(RX, PINCM22) — 板载CH340E占用
 *    UART2: PA23(TX, PINCM53), PA24(RX, PINCM54)
 *    ⚠️ PA10/PA11 勿外接串口设备
 *    ⚠️ PA23 标为特殊引脚(VREF+), 检查板上跳线帽
 *==========================================================================*/
#define UART0_TX_PORT           GPIOA
#define UART0_TX_PIN            DL_GPIO_PIN_10      /* PA10 */
#define UART0_TX_IOMUX          IOMUX_PINCM21
#define UART0_RX_PORT           GPIOA
#define UART0_RX_PIN            DL_GPIO_PIN_11      /* PA11 */
#define UART0_RX_IOMUX          IOMUX_PINCM22

#define UART2_TX_PORT           GPIOA
#define UART2_TX_PIN            DL_GPIO_PIN_23      /* PA23 */
#define UART2_TX_IOMUX          IOMUX_PINCM53
#define UART2_RX_PORT           GPIOA
#define UART2_RX_PIN            DL_GPIO_PIN_24      /* PA24 */
#define UART2_RX_IOMUX          IOMUX_PINCM54

/*===========================================================================
 * 8. 按键 (3个, BTN1已舍弃)
 *    BTN2: PA31   BTN3: PA7   BTN4: PA25
 *    ⚠️ BTN1(PA28) 与 IR X6(PA2) 共用 PINCM3, 已舍弃
 *    ⚠️ PA31 与 PA5 共用 PINCM6 (PA5不在排针,但PA31可用)
 *==========================================================================*/
#define BTN_NUM                 3   /* 从4减为3 */

#define BTN1_PORT               GPIOA
#define BTN1_PIN                DL_GPIO_PIN_31      /* PA31 (原BTN2 → 现BTN1) */
#define BTN2_PORT               GPIOA
#define BTN2_PIN                DL_GPIO_PIN_7       /* PA7  (原BTN3 → 现BTN2) */
#define BTN3_PORT               GPIOA
#define BTN3_PIN                DL_GPIO_PIN_25      /* PA25 (原BTN4 → 现BTN3) */

/*===========================================================================
 * 9. LED (2个) + 蜂鸣器
 *    PB6: LED1   PB7: LED2   PA17: 蜂鸣器
 *==========================================================================*/
#define LED1_PORT               GPIOB
#define LED1_PIN                DL_GPIO_PIN_6       /* PB6 */
#define LED2_PORT               GPIOB
#define LED2_PIN                DL_GPIO_PIN_7       /* PB7 */
#define BUZZER_PORT             GPIOA
#define BUZZER_PIN              DL_GPIO_PIN_17      /* PA17 */

/*===========================================================================
 * 10. 舵机 (2路 PWM) — TIMG7
 *     PA26: TIMG7_C0 (PINCM59, PF4)
 *     PA27: TIMG7_C1 (PINCM60, PF4)
 *==========================================================================*/
#define SERVO_TIMER             TIMG7
#define SERVO1_PORT             GPIOA
#define SERVO1_PIN              DL_GPIO_PIN_26      /* PA26 */
#define SERVO1_IOMUX            IOMUX_PINCM59
#define SERVO2_PORT             GPIOA
#define SERVO2_PIN              DL_GPIO_PIN_27      /* PA27 */
#define SERVO2_IOMUX            IOMUX_PINCM60
#define SERVO_PWM_FREQ_HZ       50U

/*===========================================================================
 * 12. HC-SR04 超声波
 *     PA14: Trig (GPIO输出)   PA15: Echo (GPIO输入)
 *     TIMG12: 1MHz 自由计数器 (TIMG1不存在于MSPM0G3507)
 *==========================================================================*/
#define US_TRIG_PORT            GPIOA
#define US_TRIG_PIN             DL_GPIO_PIN_14      /* PA14 */
#define US_ECHO_PORT            GPIOA
#define US_ECHO_PIN             DL_GPIO_PIN_15      /* PA15 */
#define US_TIMER                TIMG12
#define US_TIMER_FREQ_HZ        1000000U

/*===========================================================================
 * 12. CCD 模拟信号采集 — ADC0
 *     PA16: ADC0_A1_1 (PINCM38)    PB0: CLK    PB1: SI
 *==========================================================================*/
#define CCD_ADC                 ADC0
#define CCD_ADC_PORT            GPIOA
#define CCD_ADC_PIN             DL_GPIO_PIN_16      /* PA16 */
#define CCD_ADC_IOMUX           IOMUX_PINCM38
#define CCD_ADC_CHANNEL         DL_ADC_CHANNEL_1
#define CCD_CLK_PORT            GPIOB
#define CCD_CLK_PIN             DL_GPIO_PIN_0       /* PB0 */
#define CCD_SI_PORT             GPIOB
#define CCD_SI_PIN              DL_GPIO_PIN_1       /* PB1 */

/*===========================================================================
 * 13. 禁区 & 特殊引脚
 *     PA5,PA6  — 不在排针
 *     PA18     — ⛔ BSL锁芯片, 代码永不触碰
 *     PA19     — SWDIO
 *     PA20     — SWCLK
 *     PA10,PA11— 板载CH340E占用
 *     PA21     — 特殊(VREF-), 检查跳线
 *     PA23     — 特殊(VREF+), 检查跳线
 *==========================================================================*/

/*===========================================================================
 * 便捷宏
 *==========================================================================*/
#define GPIO_READ(port, pin)        DL_GPIO_readPins((port), (pin))
#define GPIO_WRITE(port, pin, val)  \
    do { if (val) DL_GPIO_setPins((port), (pin)); \
         else DL_GPIO_clearPins((port), (pin)); } while(0)
#define GPIO_TOGGLE(port, pin)      DL_GPIO_togglePins((port), (pin))

#ifdef __cplusplus
}
#endif

#endif /* __MSPM0_CONFIG_H__ */
