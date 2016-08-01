/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 15/05/20 1:46p $
 * @brief    Demonstrate smartcard UART mode by connecting PG.11 and PG.12 pins
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "nuc970.h"
#include "sys.h"
#include "scuart.h"

char au8TxBuf[] = "Hello World!";


/**
  * @brief  The interrupt services routine of smartcard port 0
  * @param  None
  * @retval None
  */
void SC0_IRQHandler(void)
{
    // Print SCUART received data to UART port
    // Data length here is short, so we're not care about UART FIFO over flow.
    sysPutChar(SCUART_READ(0));

    // RDA is the only interrupt enabled in this sample, this status bit
    // automatically cleared after Rx FIFO empty. So no need to clear interrupt
    // status here.

    return;
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

    outpw(REG_CLK_PCLKEN1, inpw(REG_CLK_PCLKEN1) | (1 << 12)); // Enable SC0 engine clock
    outpw(REG_SYS_GPG_MFPH, (inpw(REG_SYS_GPG_MFPH) & ~(0xFF000)) | 0xAA000); // Enable SCUART Tx/Rx pin

    
    sysprintf("This sample code demos smartcard interface UART mode\n");
    sysprintf("Please connect SC0 CLK pin(PG.11) with SC0 I/O pin(PG.12)\n");
    sysprintf("Hit any key to continue\n");
    sysGetChar();

    // Open smartcard interface 0 in UART mode.
    SCUART_Open(0, 115200);
    // Enable receive interrupt
    SCUART_ENABLE_INT(0, SC_INTEN_RDAIEN_Msk);
    
    sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, SC0_IRQn, (PVOID)SC0_IRQHandler);
    sysSetLocalInterrupt(ENABLE_IRQ);
    sysEnableInterrupt(SC0_IRQn);


    SCUART_Write(0, au8TxBuf, sizeof(au8TxBuf));

    while(1);
}

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/


