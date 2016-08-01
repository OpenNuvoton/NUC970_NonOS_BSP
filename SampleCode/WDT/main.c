/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/05/08 5:46p $
 * @brief    NUC970 WDT Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "nuc970.h"
#include "sys.h"


void WDT_IRQHandler(void)
{
    // Reload WWDT counter and clear WWDT interrupt flag
    sysClearWatchDogTimerCount();
    sysClearWatchDogTimerInterruptStatus();
    sysprintf("Reset WDT counter\n");

}

int main(void)
{
    // Disable all interrupts.
    outpw(REG_AIC_MDCR, 0xFFFFFFFE);
    outpw(REG_AIC_MDCRH, 0x3FFFFFFF);
    
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();
    
    sysprintf("\nThis sample code demonstrate reset WDT function\n");
    
    outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 1); // Enable WDT engine clock

    sysSetWatchDogTimerInterval(6);     // Set WDT time out interval to 2^16 Twdt = 0.7 sec. Where Twdt = 12MHZ / 128
    sysInstallWatchDogTimerISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, WDT_IRQHandler);
    sysEnableWatchDogTimerReset();
    sysEnableWatchDogTimer();

    while(1);
}
