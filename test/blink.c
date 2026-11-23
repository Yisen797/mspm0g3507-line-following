#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
static void d(volatile unsigned x){while(x--);}
int main(void){
    DL_GPIO_enablePower(GPIOA); DL_GPIO_enablePower(GPIOB);
    /* PA0 output */  IOMUX->SECCFG.PINCM[1]  = 0x00040001;
    /* PA1 output */  IOMUX->SECCFG.PINCM[2]  = 0x00040001;
    /* PB22 output */ IOMUX->SECCFG.PINCM[50] = 0x00040001;
    while(1){
        GPIOB->DOUT31_0 &= ~(1UL<<22);   /* PB22 ON (low) */
        GPIOA->DOUT31_0 |=  (1UL<<1);    /* PA1  OFF (high) */
        GPIOA->DOUT31_0 &= ~(1UL<<0);    /* PA0  ON (low) */
        d(100000);
        GPIOB->DOUT31_0 |=  (1UL<<22);   /* PB22 OFF */
        GPIOA->DOUT31_0 |=  (1UL<<0);    /* PA0  OFF */
        GPIOA->DOUT31_0 &= ~(1UL<<1);    /* PA1  ON */
        d(100000);
    }
}
