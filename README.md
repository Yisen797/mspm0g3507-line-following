# MSPM0G3507 三轮寻迹小车 — STM32F407 移植项目

天猛星 MSPM0G3507 开发板 (立创 LCKFB-TMX-MSPM0G3507)。

> **状态**: 预移植版本，引脚已对照数据手册验证，尚未在实板上调试。

---

## 完整引脚分配

```
┌──────────────────────────────────────────────────────────────┐
│ PA 端口                                                     │
├────────┬─────────────────────────────────────────────────────┤
│ PA0    │ I2C0_SDA    OLED (0x3C)                            │
│ PA1    │ I2C0_SCL    OLED (0x3C)                            │
│ PA2    │ IR X6       ⚠️ 只能输入 (ROSC电阻)                  │
│ PA3    │ (空闲)     与PA29共用PINCM4                          │
│ PA4    │ (空闲)     与PA30共用PINCM5                          │
│ PA5    │ ⛔ 不在排针                                        │
│ PA6    │ ⛔ 不在排针                                        │
│ PA7    │ BTN2       按键2                                   │
│ PA8    │ (空闲)     UART1_TX可用                             │
│ PA9    │ (空闲)     UART1_RX可用                             │
│ PA10   │ UART0_TX   板载CH340E占用, 勿外接                   │
│ PA11   │ UART0_RX   板载CH340E占用, 勿外接                   │
│ PA12   │ TIMG0_C0   左编码器 A (QEI)                         │
│ PA13   │ TIMG0_C1   左编码器 B (QEI)                         │
│ PA14   │ US_TRIG    超声波触发                               │
│ PA15   │ US_ECHO    超声波回波                               │
│ PA16   │ CCD_ADC    CCD模拟 (ADC0_A1_1)                      │
│ PA17   │ BUZZER     蜂鸣器                                   │
│ PA18   │ ⛔ 禁止! 上电锁芯片 (BSL_Invoke)                     │
│ PA19   │ SWDIO      调试, 永不配置为GPIO                      │
│ PA20   │ SWCLK      调试, 永不配置为GPIO                      │
│ PA21   │ TIMG6_C0   右编码器 A (32-bit捕获) ⚠️ VREF-检查跳线  │
│ PA22   │  GPIO      右编码器 B (方向判断)                     │
│ PA23   │ UART2_TX   ⚠️ VREF+检查跳线                         │
│ PA24   │ UART2_RX                                           │
│ PA25   │ BTN3       按键3                                   │
│ PA26   │ TIMG7_C0   舵机1 PWM                                │
│ PA27   │ TIMG7_C1   舵机2 PWM                                │
│ PA28   │ ⛔ 舍弃 — 与 IR X6(PA2) 共用 PINCM3                 │
│ PA29   │ I2C1_SCL   MPU6050 (0x68)                          │
│ PA30   │ I2C1_SDA   MPU6050 (0x68)                          │
│ PA31   │ BTN1       按键1 (启动/停止)                         │
├────────┼─────────────────────────────────────────────────────┤
│ PB 端口                                                     │
├────────┼─────────────────────────────────────────────────────┤
│ PB0    │ CCD_CLK    CCD时钟                                   │
│ PB1    │ CCD_SI     CCD信号输入                                │
│ PB2    │ AIN1       TB6612 右电机正转                        │
│ PB3    │ AIN2       TB6612 右电机反转                        │
│ PB4    │ BIN1       TB6612 左电机正转                        │
│ PB5    │ BIN2       TB6612 左电机反转                        │
│ PB6    │ LED1       用户LED                                  │
│ PB7    │ LED2       用户LED                                  │
│ PB8    │ TIMA0_C0   左轮 PWM  ⚠️ LCD/SPI Flash共用(不插屏可用)│
│ PB9    │ TIMA0_C1   右轮 PWM  ⚠️ LCD/SPI Flash共用(不插屏可用)│
│ PB10   │ IR X7      红外                                    │
│ PB11   │ IR X1      红外最左  ⚠️ LCD共用(不插屏可用)          │
│ PB12   │ IR X2      红外                                    │
│ PB13   │ IR X3      红外                                    │
│ PB14   │ IR X4      红外  ⚠️ LCD共用(不插屏可用)              │
│ PB15   │ IR X5      红外                                    │
│ PB16   │ IR X8      红外最右                                 │
│ PB22   │ 板载LED    仅测试用, 方案未占用                      │
└────────┴─────────────────────────────────────────────────────┘

定时器: TIMA0(电机PWM) TIMG0(左QEI) TIMG1(超声) TIMG6(右捕获) TIMG7(舵机PWM)
I2C总线: I2C0=OLED(0x3C), I2C1=MPU6050(0x68)
```

---

## 目录结构

```
├── README.md
├── Makefile                   # GCC 构建
├── flash.jlink                # J-Link 烧录脚本
├── .vscode/                   # VS Code 配置
├── src/
│   ├── mspm0_config.h         # 引脚映射 (所有硬件配置入口, 权威)
│   ├── main.c                 # 主循环
│   ├── delay.h                # delay_cycles 延时
│   ├── button.h/c             # 3按键 (PA31/PA7/PA25)
│   ├── buzzer.h/c             # 蜂鸣器 (PA17)
│   ├── sensor.h/c             # 8路IR (PB11~PB15+PA2+PB10+PB16)
│   ├── motor.h/c              # TB6612 (PB8/PB9 PWM + PB2~PB5 GPIO)
│   ├── encoder.h/c            # 双编码器 (TIMG0 QEI + TIMG6捕获)
│   ├── servo.h/c              # 双舵机 PWM (PA26/PA27, TIMG7)
│   ├── ultrasonic.h/c         # HC-SR04 (PA14/PA15 + TIMG1)
│   ├── ccd.h/c                # CCD ADC (PA16/PB0/PB1)
│   ├── pid.h/c                # PID 控制器 (平台无关)
│   ├── tracker.h/c            # 寻迹算法 (平台无关)
│   ├── oled_ssd1306.h/c       # OLED I2C (PA0/PA1, I2C0)
│   └── mpu6050.h/c            # MPU6050 I2C (PA29/PA30, I2C1)
```

---

## 构建

### ⚠️ PlatformIO 不支持 MSPM0，替代方案：

**GCC Makefile** (已配好):

```bash
export MSPM0_SDK="C:/ti/mspm0_sdk_2_01_00_03"
make          # 编译
make flash    # 烧录 (J-Link)
```

**CCS Theia** (TI 官方): 导入 SDK empty 示例 → 替换 src/ → SysConfig 配置引脚 → Debug

---

## 待办 (板子到货后)

1. CCS 导入项目，用 SysConfig 验证引脚配置
2. 逐个外设测试: LED → 按键 → OLED → 传感器 → 电机 → 编码器
3. PID 实地调参
4. 直角弯阈值微调
5. MPU6050/舵机/步进/CCD 驱动补充
