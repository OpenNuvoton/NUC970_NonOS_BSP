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

#include "parser.c"



static volatile int g_HMAC_done;


void CRYPTO_IRQHandler()
{
    if (SHA_GET_INT_FLAG())
    {
        g_HMAC_done = 1;
        SHA_CLR_INT_FLAG();
    }
}


int  do_compare(uint8_t *output, uint8_t *expect, int cmp_len)
{
    int   i;

    if (memcmp(expect, output, cmp_len))
    {
        sysprintf("\nMismatch!! - %d\n", cmp_len);
        for (i = 0; i < cmp_len; i++)
            sysprintf("0x%02x    0x%02x\n", expect[i], output[i]);
        return -1;
    }
    return 0;
}


int  HMAC_test()
{
    uint32_t  au32OutputDigest[16];

    SHA_Open(g_sha_mode, SHA_IN_OUT_SWAP, g_key_len);

    SHA_SetDMATransfer((uint32_t)&g_hmac_msg[0],  g_msg_len + ((g_key_len+3)&0xfffffffc));

    sysprintf("Start HMAC...\n");

    g_HMAC_done = 0;
    SHA_Start(CRYPTO_DMA_ONE_SHOT);
    while (!g_HMAC_done) ;

    SHA_Read(au32OutputDigest);

    /*--------------------------------------------*/
    /*  Compare                                   */
    /*--------------------------------------------*/
    sysprintf("Comparing result...");
    if (do_compare((uint8_t *)&au32OutputDigest[0], &g_hmac_mac[0], g_mac_len) < 0)
    {
        sysprintf("Compare error!\n");
        while (1);
    }
    sysprintf("OK.\n");
    return 0;
}


/*-----------------------------------------------------------------------------*/
int main(void)
{
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    /* enable Crypto clock */
    outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | (1 << 23));

    sysprintf("+-------------------------------------+\n");
    sysprintf("|     Crypto HMAC Sample Program      |\n");
    sysprintf("+-------------------------------------+\n");

    sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, CRPT_IRQn, (PVOID)CRYPTO_IRQHandler);
    sysSetLocalInterrupt(ENABLE_IRQ);
    sysEnableInterrupt(CRPT_IRQn);

    SHA_ENABLE_INT();

    open_test_file();

    while (1)
    {
        if (get_next_pattern() < 0)
            break;

        HMAC_test();
    }
    close_test_file();

    sysprintf("\n\nHMAC test done.\n");

    while (1);
}
