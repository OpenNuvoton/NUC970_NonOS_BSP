/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/05/11 3:23p $
 * @brief    NUC970 Driver Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "nuc970.h"
#include "sys.h"
#include "etimer.h"

void ETMR0_IRQHandler(void)
{
    // printf takes long time and affect the freq. calculation, we only print out once a while
    static int cnt = 0;
    static UINT t0, t1;

    if(cnt == 0) {
        t0 = ETIMER_GetCaptureData(0);
        cnt++;
    } else if(cnt == 1) {
        t1 = ETIMER_GetCaptureData(0);
        cnt++;
        if(t0 > t1) {
            // over run, drop this data and do nothing
        } else {
            sysprintf("Input frequency is %dHz\n", 12000000 / (t1 - t0));
        }
    } else {
        cnt = 0;
    }

    ETIMER_ClearCaptureIntFlag(0);
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
    
    outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | (1 << 4)); // Enable ETIMER0 engine clock
    outpw(REG_SYS_GPC_MFPL, (inpw(REG_SYS_GPC_MFPL) & ~(0xFUL << 28)) | (0xDUL << 28)); // Enable ETIMER0 toggle out pin @ PC7

    sysprintf("\nThis sample code demonstrate timer free counting mode.\n");
    sysprintf("Please connect input source with Timer 0 capture pin PC.7, press any key to continue\n");
    sysGetChar();
    
    // Give a dummy target frequency here. Will over write capture resolution with macro
    ETIMER_Open(0, ETIMER_PERIODIC_MODE, 1000000);

    // Update prescale for better resolution.
    ETIMER_SET_PRESCALE_VALUE(0, 0);

    // Set compare value as large as possible, so don't need to worry about counter overrun too frequently.
    ETIMER_SET_CMP_VALUE(0, 0xFFFFFF);

    // Configure Timer 0 free counting mode, capture TDR value on rising edge
    ETIMER_EnableCapture(0, ETIMER_CAPTURE_FREE_COUNTING_MODE, ETIMER_CAPTURE_RISING_EDGE);

    // Start Timer 0
    ETIMER_Start(0);

    // Enable timer interrupt
    ETIMER_EnableCaptureInt(0);
    
    sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, ETMR0_IRQn, (PVOID)ETMR0_IRQHandler);
    sysSetLocalInterrupt(ENABLE_IRQ);
    sysEnableInterrupt(ETMR0_IRQn);

    while(1);
}
