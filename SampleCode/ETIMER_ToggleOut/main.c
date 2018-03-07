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

/*-----------------------------------------------------------------------------*/
int main(void)
{
    
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | (1 << 4)); // Enable ETIMER0 engine clock
    outpw(REG_SYS_GPC_MFPL, (inpw(REG_SYS_GPC_MFPL) & ~(0xF << 24)) | (0xD << 24)); // Enable ETIMER0 toggle out pin @ PC6

    sysprintf("\nThis sample code use timer 0 to generate 500Hz toggle output to PC.6 pin\n");
    
    /* To generate 500HZ toggle output, timer frequency must set to 1000Hz.
       Because toggle output state change on every timer timeout event */
    ETIMER_Open(0, ETIMER_TOGGLE_MODE, 1000);
    ETIMER_Start(0);

    while(1);

}
