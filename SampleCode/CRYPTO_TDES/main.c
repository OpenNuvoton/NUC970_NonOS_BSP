/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/05/06 9:54a $
 * @brief    NUC970 Driver Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nuc970.h"
#include "crypto.h"


uint8_t au8MyTDESKey[3][8] = {      /* TDES key = 0001020304050607 1011121314151617 2021222324252627 */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
};

//uint32_t au32MyTDESIV[2] = { 0x3dafba42, 0x9d9eb430 };   /* Initial vector = 3dafba429d9eb430 */
uint32_t au32MyTDESIV[2] = { 0x00000000, 0x00000000 };     /* Initial vector = 0000000000000000 */

uint8_t au8InputData_Pool[] __attribute__((aligned(32))) = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};
uint8_t  *au8InputData;

uint8_t au8OutputData_Pool[1024] __attribute__((aligned(32)));
uint8_t  *au8OutputData;



static volatile int  g_TDES_done;

void CRYPTO_IRQHandler()
{
    if (TDES_GET_INT_FLAG()) {
        g_TDES_done = 1;
        TDES_CLR_INT_FLAG();
    }
}


void  dump_buff_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0) {
        sysprintf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            sysprintf("%02x ", pucBuff[nIdx + i]);
        sysprintf("  ");
        for (i = 0; i < 16; i++) {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                sysprintf("%c", pucBuff[nIdx + i]);
            else
                sysprintf(".");
            nBytes--;
        }
        nIdx += 16;
        sysprintf("\n");
    }
    sysprintf("\n");
}


/*-----------------------------------------------------------------------------*/
int main(void)
{
    int  data_len;

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    /* enable Crypto clock */
    outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | (1 << 23));

    sysprintf("+------------------------------------+\n");
    sysprintf("|   Crypto DES/TDES Sample Program    |\n");
    sysprintf("+------------------------------------+\n");

    au8InputData = (uint8_t *)((uint32_t)au8InputData_Pool | 0x80000000);
    au8OutputData = (uint8_t *)((uint32_t)au8OutputData_Pool | 0x80000000);

    data_len = sizeof(au8InputData_Pool);

    sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, CRPT_IRQn, (PVOID)CRYPTO_IRQHandler);
    sysSetLocalInterrupt(ENABLE_IRQ);
    sysEnableInterrupt(CRPT_IRQn);

    TDES_ENABLE_INT();

    sysprintf("\n\n[Plain text] =>\n");
    dump_buff_hex(au8InputData, data_len);

    /*---------------------------------------
     *  TDES CBC mode encrypt
     *---------------------------------------*/
    TDES_Open(0, 1, TDES_MODE_CBC, TDES_IN_OUT_WHL_SWAP);
    TDES_SetKey(0, au8MyTDESKey);
    TDES_SetInitVect(0, au32MyTDESIV[0], au32MyTDESIV[1]);
    TDES_SetDMATransfer(0, (uint32_t)au8InputData, (uint32_t)au8OutputData, data_len);

    g_TDES_done = 0;
    TDES_Start(0, CRYPTO_DMA_ONE_SHOT);
    while (!g_TDES_done);

    sysprintf("TDES encrypt done.\n\n");

    sysprintf("[Cypher text] =>\n");
    dump_buff_hex(au8OutputData, data_len);

    memset(au8InputData, 0, data_len);     /* To prove it, clear plain text data. */

    /*---------------------------------------
     *  TDES CBC mode decrypt
     *---------------------------------------*/
    TDES_Open(0, 0, TDES_MODE_CBC, TDES_IN_OUT_WHL_SWAP);
    TDES_SetKey(0, au8MyTDESKey);
    TDES_SetInitVect(0, au32MyTDESIV[0], au32MyTDESIV[1]);
    TDES_SetDMATransfer(0, (uint32_t)au8OutputData, (uint32_t)au8InputData, data_len);

    g_TDES_done = 0;
    TDES_Start(0, CRYPTO_DMA_ONE_SHOT);
    while (!g_TDES_done);

    sysprintf("TDES decrypt done.\n\n");

    sysprintf("[Cypher text back to plain text] =>\n");
    dump_buff_hex(au8InputData, data_len);

    sysprintf("TDES test done.\n");

    return 0;
}
