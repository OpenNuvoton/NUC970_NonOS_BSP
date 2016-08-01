/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/04/29 10:00a $
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


#define GENERATE_COUNT      10


static volatile int  g_PRNG_done;

void CRYPTO_IRQHandler()
{
    if (PRNG_GET_INT_FLAG()) {
        g_PRNG_done = 1;
        PRNG_CLR_INT_FLAG();
    }
}


/*-----------------------------------------------------------------------------*/
int main(void)
{
	uint32_t  i, u32KeySize;
	uint32_t  au32PrngData[8];
	
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();
    
    /* enable Crypto clock */
	outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | (1 << 23));

    sysprintf("+------------------------------------+\n");
    sysprintf("|     Crypto PRNG Sample Program     |\n");
    sysprintf("+------------------------------------+\n");

	sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, CRPT_IRQn, (PVOID)CRYPTO_IRQHandler);
	//sysDisableInterrupt(TMR0_IRQn);
  	sysSetLocalInterrupt(ENABLE_IRQ);
	sysEnableInterrupt(CRPT_IRQn);

    PRNG_ENABLE_INT();

    for (u32KeySize = PRNG_KEY_SIZE_64; u32KeySize <= PRNG_KEY_SIZE_256; u32KeySize++) {
        sysprintf("\n\nPRNG Key size = %s\n\n",(u32KeySize == PRNG_KEY_SIZE_64) ? "64" :
               (u32KeySize == PRNG_KEY_SIZE_128) ? "128" :
               (u32KeySize == PRNG_KEY_SIZE_192) ? "192" :
               (u32KeySize == PRNG_KEY_SIZE_256) ? "256" : "unknown");
        PRNG_Open(u32KeySize, 0, 0);

        for (i = 0; i < GENERATE_COUNT; i++) {
            g_PRNG_done = 0;
            PRNG_Start();
            while (!g_PRNG_done);

            memset(au32PrngData, 0, sizeof(au32PrngData));
            PRNG_Read(au32PrngData);

            sysprintf("PRNG DATA ==>\n");
            sysprintf("    0x%08x  0x%08x  0x%08x  0x%08x\n", au32PrngData[0], au32PrngData[1], au32PrngData[2], au32PrngData[3]);
            sysprintf("    0x%08x  0x%08x  0x%08x  0x%08x\n", au32PrngData[4], au32PrngData[5], au32PrngData[6], au32PrngData[7]);
        }
    }

    sysprintf("\nAll done.\n");

    return 0;
}
