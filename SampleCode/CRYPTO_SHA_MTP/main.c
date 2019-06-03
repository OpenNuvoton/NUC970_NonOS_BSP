/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/05/06 9:54a $
 * @brief    This sample shows how to use SHA engine to compare output digest with MTP key.
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nuc970.h"
#include "crypto.h"
#include "mtp.h"


//#define PROGRAM_MTP          /* Used to program SHA digest of "image.bin". Remeber to disable this once SHA digest have been programmed. */

//#define SHA1_TEST
//#define SHA224_TEST
#define SHA256_TEST

extern UINT32   ImageDataBase, ImageDataLimit;

UINT32  mtp_key[8];

UINT8   g_sha_key_buff[0x100000] __attribute__((aligned(32)));     /* 1 MB buffer which will hold "image.bin". */
UINT32  g_sha_digest[8];
UINT8   *g_sha_key;
int     g_key_len;

int  g_digest_len = 0;


static volatile int g_SHA_done;


void CRYPTO_IRQHandler()
{
    if (SHA_GET_INT_FLAG())
    {
        g_SHA_done = 1;
        SHA_CLR_INT_FLAG();
    }
}

int  do_compare(UINT8 *output, UINT8 *expect, int cmp_len)
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

static void dump_mtp_status()
{
    int    cnt;
    UINT8  user_data;

    sysprintf("MTP_STATUS: 0x%x\n", MTP->STATUS);
    if (MTP->STATUS & MTP_STATUS_MTPEN)
        sysprintf(" ENABLED");
    if (MTP->STATUS & MTP_STATUS_KEYVALID)
        sysprintf(" KEY_VALID");
    if (MTP->STATUS & MTP_STATUS_NONPRG)
        sysprintf(" NO_KEY");
    if (MTP->STATUS & MTP_STATUS_LOCKED)
        sysprintf(" LOCKED");
    if (MTP->STATUS & MTP_STATUS_PRGFAIL)
        sysprintf(" PROG_FAIL");
    if (MTP->STATUS & MTP_STATUS_BUSY)
        sysprintf(" BUSY");
    cnt = MTP_GetPrgCount();
    if (cnt >= 0)
        sysprintf("  PRGCNT=%d\n", MTP_GetPrgCount());
    else
        sysprintf("\n");

    if (MTP_GetUserData(&user_data) == MTP_RET_OK)
        sysprintf("USER_DATA: 0x%x\n", user_data);

    sysprintf("MTP Key is %s.\n", MTP_IsKeyLocked() ? "locked" : "not locked");
}

void  SHA_MTP_compare()
{
    int         i;
    UINT32      *dptr;

    sysprintf("\n\n\n===================================\n");
    sysprintf("START SHA Test...\n\n");

    CRPT->HMAC_CTL = 0;

    /*--------------------------------------------*/
    /*  SHA go....                                */
    /*--------------------------------------------*/

#if defined(SHA1_TEST)
    CRPT->HMAC_CTL |= (SHA_MODE_SHA1 << CRPT_HMAC_CTL_OPMODE_Pos);
    g_digest_len = 20;
#elif defined(SHA224_TEST)
    CRPT->HMAC_CTL |= (SHA_MODE_SHA224 << CRPT_HMAC_CTL_OPMODE_Pos);
    g_digest_len = 28;
#else
    CRPT->HMAC_CTL |= (SHA_MODE_SHA256 << CRPT_HMAC_CTL_OPMODE_Pos);
    g_digest_len = 32;
#endif

    CRPT->HMAC_CTL |= CRPT_HMAC_CTL_INSWAP_Msk;
    CRPT->HMAC_DMACNT = g_key_len;
    CRPT->HMAC_SADDR = (UINT32)g_sha_key;

    sysprintf("SHA byte count = %d\n", g_key_len/8);

    g_SHA_done = 0;
    CRPT->HMAC_CTL |= CRPT_HMAC_CTL_START_Msk | CRPT_HMAC_CTL_DMAEN_Msk | CRPT_HMAC_CTL_DMALAST_Msk;
    while (!g_SHA_done) ;
    while (CRPT->HMAC_STS & CRPT_HMAC_STS_BUSY_Msk)
        sysprintf("!");

    /*--------------------------------------------*/
    /*  SHA hash done, print digest output        */
    /*--------------------------------------------*/
    sysprintf("OUTPUT digest [0x%x] ==> \n", CRPT->HMAC_STS);
    memset((UINT8 *)&g_sha_digest[0], 0, sizeof(g_sha_digest));
    dptr = (UINT32 *)&(CRPT->HMAC_DGST0);
    for (i = 0; i < 8; i++, dptr++)
    {
        g_sha_digest[i] = *dptr;
        sysprintf("    SHA_H%d = 0x%08x\n", i, g_sha_digest[i]);
    }

    /*--------------------------------------------*/
    /*  Write SHA digest to MTP key               */
    /*--------------------------------------------*/
    mtp_key[0] = g_sha_digest[0];
    mtp_key[1] = g_sha_digest[1];
    mtp_key[2] = g_sha_digest[2];
    mtp_key[3] = g_sha_digest[3];
    mtp_key[4] = g_sha_digest[4];
    mtp_key[5] = g_sha_digest[5];
    mtp_key[6] = g_sha_digest[6];
    mtp_key[7] = g_sha_digest[7];

    if (MTP_Enable() != MTP_RET_OK)
    {
        sysprintf("Failed to enable MTP!\n");
        return;
    }
#ifdef PROGRAM_MTP
    MTP_Program(mtp_key, 0x20);    /* Bit 2 of <option> must be 0, otherwise, IBR secure boot will be enabled. */

    while (MTP->STATUS & MTP_STATUS_BUSY) ;
#endif

    dump_mtp_status();

    CRPT->HMAC_CTL = 0;

    /*--------------------------------------------*/
    /*  Trigger SHA again, and enable MTP comapre */
    /*--------------------------------------------*/
#if defined(SHA1_TEST)
    CRPT->HMAC_CTL |= (SHA_MODE_SHA1 << CRPT_HMAC_CTL_OPMODE_Pos);
    g_digest_len = 20;
#elif defined(SHA224_TEST)
    CRPT->HMAC_CTL |= (SHA_MODE_SHA224 << CRPT_HMAC_CTL_OPMODE_Pos);
    g_digest_len = 28;
#else
    CRPT->HMAC_CTL |= (SHA_MODE_SHA256 << CRPT_HMAC_CTL_OPMODE_Pos);
    g_digest_len = 32;
#endif

    CRPT->HMAC_CTL |= CRPT_HMAC_CTL_INSWAP_Msk;
    CRPT->HMAC_DMACNT = g_key_len;
    CRPT->HMAC_SADDR = (UINT32)g_sha_key;

    sysprintf("SHA byte count = %d\n", g_key_len/8);

    g_SHA_done = 0;
    CRPT->HMAC_CTL |= CRPT_HMAC_CTL_COMPEN_Msk;
    CRPT->HMAC_CTL |= CRPT_HMAC_CTL_START_Msk | CRPT_HMAC_CTL_DMAEN_Msk | CRPT_HMAC_CTL_DMALAST_Msk;
    while (!g_SHA_done) ;
    while (CRPT->HMAC_STS & CRPT_HMAC_STS_BUSY_Msk)
        sysprintf("!");

    /*--------------------------------------------*/
    /*  SHA hash done, print digest output        */
    /*--------------------------------------------*/
    sysprintf("OUTPUT digest ==> \n");
    memset((UINT8 *)&g_sha_digest[0], 0, sizeof(g_sha_digest));
    dptr = (UINT32 *)(CRPT_BA + 0x308);
    for (i = 0; i < 8; i++, dptr++)
    {
        g_sha_digest[i] = *dptr;
        sysprintf("    SHA_H%d = 0x%08x\n", i, g_sha_digest[i]);
    }

    if (CRPT->HMAC_STS & CRPT_HMAC_STS_COMPRES_Msk)
    {
        sysprintf("MTP compare OK!  0x%x\n", CRPT->HMAC_STS);
    }
    else
    {
        sysprintf("MTP compare failed!\n");
    }
}


/*-----------------------------------------------------------------------------*/
int main(void)
{
    g_key_len = (UINT32)&ImageDataLimit - (UINT32)&ImageDataBase;
    g_sha_key = (UINT8 *)((UINT32)&g_sha_key_buff[0] | 0x80000000);
    memcpy(g_sha_key, (UINT8 *)&ImageDataBase, g_key_len);

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    /* enable Crypto clock */
    outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | (1 << 23));

    /* enable MTP clock */
    outpw(REG_CLK_PCLKEN1, inpw(REG_CLK_PCLKEN1) | (1 << 26));

    sysprintf("+------------------------------------+\n");
    sysprintf("|     Crypto SHA Sample Program      |\n");
    sysprintf("+------------------------------------+\n");

    sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, CRPT_IRQn, (PVOID)CRYPTO_IRQHandler);
    sysSetLocalInterrupt(ENABLE_IRQ);
    sysEnableInterrupt(CRPT_IRQn);

    SHA_ENABLE_INT();

    SHA_MTP_compare();

    while (1);
}
