/**
 * 最小测试: LED PB22 闪烁 + OLED 显示 (无 snprintf, 只用 libc 的 memcpy/memset)
 */
#include "mspm0_config.h"
#include "delay.h"
#include "oled_ssd1306.h"

int main(void)
{
    /* 启动闪烁 PB22 ×3 */
    DL_GPIO_reset(GPIOB);
    DL_GPIO_enablePower(GPIOB);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM50);
    DL_GPIO_enableOutput(GPIOB, DL_GPIO_PIN_22);
    for (int i = 0; i < 3; i++) {
        DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_22);
        for (volatile uint32_t d = 0; d < 400000; d++);
        DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_22);
        for (volatile uint32_t d = 0; d < 400000; d++);
    }

    /* OLED */
    oled_init();
    oled_clear();
    oled_text(0, 2, "OLED WORKS!");
    oled_text(0, 5, "MSPM0G3507");

    /* 常亮表示完成 */
    DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_22);

    while (1);
    return 0;
}
