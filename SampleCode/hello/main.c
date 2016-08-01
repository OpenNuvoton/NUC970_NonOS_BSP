/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/05/07 5:38p $
 * @brief    NUC970 Driver Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "nuc970.h"
#include "sys.h"

/*-----------------------------------------------------------------------------*/
int main(void)
{
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();
	sysprintf("\n\n Hello NUC970 !!!\n");

    sysprintf("APLL    clock %d MHz\n", sysGetClock(SYS_APLL));
    sysprintf("UPLL    clock %d MHz\n", sysGetClock(SYS_UPLL));
    sysprintf("CPU     clock %d MHz\n", sysGetClock(SYS_CPU));
    sysprintf("System  clock %d MHz\n", sysGetClock(SYS_SYSTEM));
    sysprintf("HCLK1   clock %d MHz\n", sysGetClock(SYS_HCLK1));
    sysprintf("HCLK234 clock %d MHz\n", sysGetClock(SYS_HCLK234));
    sysprintf("PCLK    clock %d MHz\n", sysGetClock(SYS_PCLK));

    return 0;
}
