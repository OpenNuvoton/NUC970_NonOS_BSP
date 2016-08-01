/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/05/11 3:23p $
 * @brief    NUC970 ETIMER Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "nuc970.h"
#include "sys.h"
#include "etimer.h"

void ETMR0_IRQHandler(void)
{
    static uint32_t sec = 1;
    sysprintf("%d sec\n", sec++);

    // clear timer interrupt flag
    ETIMER_ClearIntFlag(0);

}

/*-----------------------------------------------------------------------------*/
int main(void)
{
    // Disable all interrupts.
    outpw(REG_AIC_MDCR, 0xFFFFFFFE);
    outpw(REG_AIC_MDCRH, 0x3FFFFFFF);

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    sysprintf("\nThis sample code use timer to generate interrupt every 1 second \n");
    
    outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | (1 << 4)); // Enable ETIMER0 engine clock

    // Set timer frequency to 1HZ
    ETIMER_Open(0, ETIMER_PERIODIC_MODE, 1);

    // Enable timer interrupt
    ETIMER_EnableInt(0);
    sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, ETMR0_IRQn, (PVOID)ETMR0_IRQHandler);
    sysSetLocalInterrupt(ENABLE_IRQ);
    sysEnableInterrupt(ETMR0_IRQn);

    // Start Timer 0
    ETIMER_Start(0);

    while(1);
}
