/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/05/08 5:40p $
 * @brief    NUC970 WWDT Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "nuc970.h"
#include "sys.h"
#include "wwdt.h"

void WWDT_IRQHandler(void)
{

    // Reload WWDT counter and clear WWDT interrupt flag
    WWDT_RELOAD_COUNTER();
    WWDT_CLEAR_INT_FLAG();
    sysprintf("WWDT counter reload\n");

}

int main(void)
{

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();
    
    sysprintf("\nThis sample code demonstrate WWDT reload function\n");

    
    sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, WWDT_IRQn, (PVOID)WWDT_IRQHandler);
    sysSetLocalInterrupt(ENABLE_IRQ);
    sysEnableInterrupt(WWDT_IRQn);
    
    outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 2); // Enable WWDT engine clock
    
    // WWDT timeout every 2048 * 64 WWDT clock, compare interrupt trigger at 2048 * 32 WWDT clock. About every 0.7 sec
    // enable WWDT counter compare interrupt. Default WWDT clock source is 12MHz / 128.
    WWDT_Open(WWDT_PRESCALER_2048, 0x20, TRUE);

    while(1);
}
