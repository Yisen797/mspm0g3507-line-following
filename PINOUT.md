# 天猛星 MSPM0G3507 — 寻迹小车引脚分配 (数据手册验证版)

> 基于 MSPM0G3507 数据手册 SLASEX6B 引脚复用表交叉验证
> 2026-07-16

## I2C

| 总线 | 设备 | SDA | SCL | 备注 |
|---|---|---|---|---|
| I2C0 | OLED (0x3C) | PA0 (PINCM1 PF3) | PA1 (PINCM2 PF3) | BSL默认引脚, 5V耐压开漏 |
| I2C1 | MPU6050 (0x68) | PA30 (PINCM5 PF2) | PA29 (PINCM4 PF2) | |

## 电机 TB6612

| 功能 | 引脚 | 外设 |
|---|---|---|
| 左轮 PWM | PB8 | TIMA0_C0 (PINCM25 PF4) |
| 右轮 PWM | PB9 | TIMA0_C1 (PINCM26 PF4) |
| AIN1 (右正转) | PB2 | GPIO |
| AIN2 (右反转) | PB3 | GPIO |
| BIN1 (左正转) | PB4 | GPIO |
| BIN2 (左反转) | PB5 | GPIO |

## 编码器

| 功能 | 引脚 | 外设 |
|---|---|---|
| 左编码器 A | PA12 | TIMG0_C0 QEI (PINCM34 PF4) |
| 左编码器 B | PA13 | TIMG0_C1 QEI (PINCM35 PF5) |
| 右编码器 A | PA21 | TIMG6_C0 捕获 (PINCM46 PF6) |
| 右编码器 B | PA22 | GPIO 方向判断 |

## 红外 8 路

| 通道 | 引脚 | 逻辑位置 |
|---|---|---|
| X1 | PB11 | bit0 (最左) |
| X2 | PB12 | bit1 |
| X3 | PB13 | bit2 |
| X4 | PB14 | bit3 |
| X5 | PB15 | bit4 |
| X6 | PA2 | bit5 |
| X7 | PB10 | bit6 |
| X8 | PB16 | bit7 (最右) |

## 串口

| 串口 | TX | RX |
|---|---|---|
| UART0 | PA10 (PINCM21 PF2) | PA11 (PINCM22 PF2) |
| UART1 | PA8 (PINCM19 PF2) | PA9 (PINCM20 PF2) |
| UART2 | PA23 (PINCM53 PF2) | PA24 (PINCM54 PF2) |

## 按键

| 编号 | 引脚 | 功能 |
|---|---|---|
| BTN1 | PA31 | 启动/停止 (原BTN1=PA28与IR X6共用PINCM3, 已舍弃) |
| BTN2 | PA7 | |
| BTN3 | PA25 | |
> 注: PA28 与 PA2(IR X6) 共用物理引脚 PINCM3, 保留红外，舍弃按键

## LED + 蜂鸣器

| 功能 | 引脚 |
|---|---|
| LED1 | PB6 |
| LED2 | PB7 |
| BUZZER | PA17 |

## 舵机

| 通道 | 引脚 | 外设 |
|---|---|---|
| SERVO1 | PA26 | TIMG7_C0 (PINCM59 PF7) |
| SERVO2 | PA27 | TIMG7_C1 (PINCM60 PF7) |

## 超声波 HC-SR04

| 功能 | 引脚 |
|---|---|
| TRIG | PA14 |
| ECHO | PA15 |

## CCD

| 功能 | 引脚 |
|---|---|
| AO (模拟) | PA16 (ADC0_A1_1) |
| CLK | PB0 |
| SI | PB1 |

## 禁区

| 引脚 | 原因 |
|---|---|
| PA5, PA6 | 不在排针 |
| PA18 | 上电高电平锁芯片 (BSL_Invoke) |
| PA19 | SWDIO |
| PA20 | SWCLK |

## 定时器占用

| 定时器 | 用途 | 引脚 |
|---|---|---|
| TIMA0 | 电机 PWM | PB8, PB9 |
| TIMG0 | 左编码器 QEI | PA12, PA13 |
| TIMG1 | 超声波计时 | 内部 1MHz |
| TIMG6 | 右编码器捕获 | PA21, PA22 |
| TIMG7 | 舵机 PWM | PA26, PA27 |
