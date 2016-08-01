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


extern void open_test_vector(void);
extern int  get_next_pattern(void);

extern uint8_t  *_au8ShaData;
extern uint8_t  _au8ShaDigest[64];
extern int      _i32DataLen;

static int  _i32DigestLength = 0;

static volatile int g_SHA_done;


void CRYPTO_IRQHandler()
{
    if (SHA_GET_INT_FLAG()) {
        g_SHA_done = 1;
        SHA_CLR_INT_FLAG();
    }
}


int  do_compare(uint8_t *output, uint8_t *expect, int cmp_len)
{
    int   i;

    if (memcmp(expect, output, cmp_len)) {
        sysprintf("\nMismatch!! - %d\n", cmp_len);
        for (i = 0; i < cmp_len; i++)
            sysprintf("0x%02x    0x%02x\n", expect[i], output[i]);
        return -1;
    }
    return 0;
}


int  run_sha()
{
    uint32_t  au32OutputDigest[8];

    SHA_Open(SHA_MODE_SHA1, SHA_IN_SWAP, 0);

    SHA_SetDMATransfer((uint32_t)&_au8ShaData[0],  _i32DataLen/8);

    sysprintf("Key len= %d bits\n", _i32DataLen);

    g_SHA_done = 0;
    SHA_Start(CRYPTO_DMA_ONE_SHOT);
    while (!g_SHA_done) ;

    SHA_Read(au32OutputDigest);

    /*--------------------------------------------*/
    /*  Compare                                   */
    /*--------------------------------------------*/
    if (do_compare((uint8_t *)&au32OutputDigest[0], &_au8ShaDigest[0], _i32DigestLength) < 0) {
        sysprintf("Compare error!\n");
        while (1);
    }
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

    sysprintf("+------------------------------------+\n");
    sysprintf("|     Crypto SHA Sample Program      |\n");
    sysprintf("+------------------------------------+\n");

	sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, CRPT_IRQn, (PVOID)CRYPTO_IRQHandler);
  	sysSetLocalInterrupt(ENABLE_IRQ);
	sysEnableInterrupt(CRPT_IRQn);

    SHA_ENABLE_INT();

    open_test_vector();

    while (1) {
        if (get_next_pattern() < 0)
            break;

        run_sha();
    }

    sysprintf("SHA test done.\n");

    return 0;
}
