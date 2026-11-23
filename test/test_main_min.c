/**
 * v11 Clean: v6寻迹 + WWDT
 */
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_common.h>
#include <ti/driverlib/m0p/dl_sysctl.h>
#include <ti/driverlib/dl_wwdt.h>
#include "oled_ssd1306.h"
#include "sensor.h"

#define KP    10
#define KD    50
#define KI    0
#define PD_DIV 1024
#define IMAX  5
#define VMAX  100
#define VMIN  30
#define KV    34

extern void mpu_reset_i2c1(void);
extern void mpu_init(void);
extern int16_t mpu_read_yaw(void);

__attribute__((section(".ramfunc"),noinline))
static void tracker_loop(void) {
    int run=0, dbnc=0, angle=0, last_err=0, integral=0, pid_out=0;
    int gyro_tick=0; int16_t gz=0;
    int lost_cnt=0;

    while(1) {
        WWDT0->WWDTCNTRST = 0x00A7;

        if((GPIOB->DIN31_0 & DL_GPIO_PIN_21)==0){
            if(++dbnc==60){ run=!run; dbnc=0;
                if(run){
                    GPIOB->DOUTSET31_0 = DL_GPIO_PIN_4|DL_GPIO_PIN_3;
                    GPIOB->DOUTCLR31_0 = DL_GPIO_PIN_5|DL_GPIO_PIN_2;
                }else{
                    GPIOB->DOUTSET31_0 = DL_GPIO_PIN_2|DL_GPIO_PIN_3|DL_GPIO_PIN_4|DL_GPIO_PIN_5;
                    GPIOB->DOUTCLR31_0 = DL_GPIO_PIN_8|DL_GPIO_PIN_9;
                }
                while((GPIOB->DIN31_0 & DL_GPIO_PIN_21)==0){ WWDT0->WWDTCNTRST=0x00A7; }
            }
        }else{ dbnc=0; }

        if(!run){
            GPIOB->DOUTCLR31_0 = DL_GPIO_PIN_8|DL_GPIO_PIN_9;
            continue;
        }

        uint32_t raw_a = GPIOB->DIN31_0 & (DL_GPIO_PIN_11|DL_GPIO_PIN_12|DL_GPIO_PIN_13|DL_GPIO_PIN_14|DL_GPIO_PIN_15);
        uint8_t bits_a = (uint8_t)(~(raw_a >> 11)) & 0x1F;
        uint8_t bit5 = ((GPIOA->DIN31_0 & DL_GPIO_PIN_2)==0) ? 1 : 0;
        uint8_t bit6 = (GPIOB->DIN31_0 & DL_GPIO_PIN_10) ? 0 : 1;
        uint8_t bit7 = (GPIOB->DIN31_0 & DL_GPIO_PIN_16) ? 0 : 1;
        uint8_t ir = bits_a | (bit5<<5) | (bit6<<6) | (bit7<<7);

        int w[8]={-7,-5,-3,-1,1,3,5,7};
        int wsum=0,rc=0;
        for(int i=0;i<8;i++){ if(ir&(1<<i)){ wsum+=w[i]; rc++; } }
        { static int K[9]={0,1048576,524288,349525,262144,209715,174763,149797,131072};
          if(rc>0) angle=(wsum*K[rc])>>10; }

        if(rc>0){
            lost_cnt=0;
            int err=0-angle;
            integral+=err; if(integral>(IMAX<<10))integral=(IMAX<<10); if(integral<-(IMAX<<10))integral=-(IMAX<<10);
            int d=err-last_err; last_err=err;
            if(++gyro_tick>=50){ gyro_tick=0;
                int16_t raw=mpu_read_yaw(); gz=(gz*7+raw)>>3; }
            d += (int)gz>>3;
            pid_out=(KP*err + KD*d + KI*integral)>>10;
            if(pid_out>100)pid_out=100; if(pid_out<-100)pid_out=-100;
        }else{
            integral=0; angle=0; last_err=0;
            if(++lost_cnt>10) pid_out=0;
        }
        int ag=angle; if(ag<0)ag=-ag;
        int speed=VMAX - ((KV*ag)>>10); if(speed<VMIN)speed=VMIN;

        int L=speed+pid_out, R=speed-pid_out;
        if(L<10)L=10;if(L>400)L=400;if(R<10)R=10;if(R>400)R=400;

        /* OLED: 每300循环显示IR, I2C前喂狗防阻塞 */
        { static int dt=0; if(++dt>=300){ dt=0;
            WWDT0->WWDTCNTRST = 0x00A7;
            char b[17]="IR:........     ";
            for(int i=0;i<8;i++) b[3+i]=(ir&(1<<i))?'1':'0';
            oled_text(0,5,b); } }

        for(int cyc=0; cyc<5; cyc++) {
            for(int s=0; s<500; s++) {
                if(s<L) GPIOB->DOUTSET31_0 = DL_GPIO_PIN_8;
                else    GPIOB->DOUTCLR31_0 = DL_GPIO_PIN_8;
                if(s<R) GPIOB->DOUTSET31_0 = DL_GPIO_PIN_9;
                else    GPIOB->DOUTCLR31_0 = DL_GPIO_PIN_9;
            }
        }
    }
}

int main(void) {
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_setPowerPolicyRUN0SLEEP0();
    DL_Common_delayCycles(160000);

    mpu_reset_i2c1();
    DL_GPIO_reset(GPIOB); DL_GPIO_enablePower(GPIOB);
    DL_Common_delayCycles(16);
    sensor_init();

    oled_init();
    oled_clear(); oled_text(0,2,"V1");

    DL_GPIO_enablePower(GPIOB); DL_Common_delayCycles(16);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM25); DL_GPIO_initDigitalOutput(IOMUX_PINCM26);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM15); DL_GPIO_initDigitalOutput(IOMUX_PINCM16);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM17); DL_GPIO_initDigitalOutput(IOMUX_PINCM18);
    DL_GPIO_enableOutput(GPIOB, DL_GPIO_PIN_2|DL_GPIO_PIN_3|DL_GPIO_PIN_4|DL_GPIO_PIN_5|DL_GPIO_PIN_8|DL_GPIO_PIN_9);
    DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_2|DL_GPIO_PIN_3|DL_GPIO_PIN_4|DL_GPIO_PIN_5);
    DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_8|DL_GPIO_PIN_9);
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM49, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    for(volatile int d=0; d<200000; d++);
    mpu_init();
    for(volatile int d=0; d<200000; d++);

    DL_WWDT_reset(WWDT0);
    for(volatile int d=0; d<1000; d++);
    DL_WWDT_initWatchdogMode(WWDT0,
        DL_WWDT_CLOCK_DIVIDE_2, DL_WWDT_TIMER_PERIOD_15_BITS,
        DL_WWDT_RUN_IN_SLEEP,
        DL_WWDT_WINDOW_PERIOD_0, DL_WWDT_WINDOW_PERIOD_0);
    WWDT0->WWDTCNTRST = 0x00A7;

    tracker_loop();
}
