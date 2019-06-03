/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/04/30 10:49a $
 * @brief    NUC970 Driver Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nuc970.h"
#include "mtp.h"


// This key table contains just monkey input keys. These keys are just for test.
// You can replace them with any keys you desired.
UINT32  key_table[15][8] =
{
    0x14454543, 0x7e1f66a0, 0x8901c2a0, 0x8812a832, 0x9122334c, 0x1dc55666, 0x233454ac, 0xaaaaaaaa,
    0x61239000, 0x00000000, 0x134ad8c9, 0xaaaaaaaa, 0x87342acb, 0x8fca134a, 0x879ac76d, 0xacbfd565,
    0xfffddddd, 0x134a8902, 0xaaaaaaaa, 0x00000000, 0x34341129, 0x56ac8baf, 0x980c1af4, 0x00000000,
    0x55555555, 0x89812323, 0xac341bdf, 0x00000000, 0x34341129, 0x56ac8baf, 0x980c1af4, 0x00000000,
    0x12232344, 0x72134344, 0x98766112, 0x9011234f, 0x55555555, 0x89812323, 0xac341bdf, 0x00000000,
    0x567ac123, 0xbc459000, 0x11111111, 0xa5a5a5a5, 0x5a5a5a5a, 0x5a5a5a5a, 0x00000000, 0xffffffff,
    0x5a5a5a5a, 0x5a5a5a5a, 0x00000000, 0xffffffff, 0x34341129, 0x56ac8baf, 0x980c1af4, 0x00000000,
    0x34341129, 0x56ac8baf, 0x980c1af4, 0x00000000, 0x567ac123, 0xbc459000, 0x11111111, 0xa5a5a5a5,
    0x87342acb, 0x8fca134a, 0x879ac76d, 0xacbfd565, 0xfffddddd, 0x134a8902, 0xaaaaaaaa, 0x00000000,
    0x9122334c, 0x1dc55666, 0x233454ac, 0xaaaaaaaa, 0x61239000, 0x00000000, 0x134ad8c9, 0xaaaaaaaa,
    0x9011234f, 0x88111112, 0x3a0099b3, 0x09ae71c2, 0xf2d08c7a, 0x10901111, 0x22222222, 0xe35aa111,
    0x0977465a, 0x87448ca1, 0xaf0987f7, 0x24344acb, 0x675b89cc, 0x128987ca, 0xb98098e0, 0xef98787a,
    0x98fc0099, 0x3424a76c, 0xac9879f8, 0xef9a8c80, 0xc76c76c8, 0x9f898f8f, 0xa987f87a, 0x897c87b8,
    0x98f98c89, 0x908d9a9f, 0xff989fc0, 0x2298dac3, 0x1109999c, 0xbbbbbbbb, 0x11122222, 0x5656aaaa,
    0xfcfcf122, 0xafa22333, 0xccbbaa23, 0x08765432, 0x13456776, 0x35367585, 0x34536765, 0x53465777
};


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


/*-----------------------------------------------------------------------------*/
int main(void)
{
    int  i, chr;

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    /* enable MTP clock */
    outpw(REG_CLK_PCLKEN1, inpw(REG_CLK_PCLKEN1) | (1 << 26));

    i = 0;
    while (1)
    {
        sysprintf("+-------------------------------------------+\n");
        sysprintf("|  MTP menu                                 |\n");
        sysprintf("+-------------------------------------------+\n");
        sysprintf("| [1] On-chip MTP key status                |\n");
        sysprintf("| [2] Program MTP key                       |\n");
        sysprintf("| [3] Lock MTP key                          |\n");
        sysprintf("+-------------------------------------------+\n");

        chr = sysGetChar();

        switch (chr)
        {
        case '1':
            if (MTP_Enable() != MTP_RET_OK)
            {
                sysprintf("Failed to enable MTP!\n");
                break;
            }
            dump_mtp_status();
            break;

        case '2':
            if (MTP_Program(key_table[i++], 0x40) == MTP_RET_OK)
                sysprintf("MTP key program done.\n");
            else
                sysprintf("Failed to program MTP key!\n");
            break;

        case '3':
            if (MTP_Lock() == MTP_RET_OK)
                sysprintf("MTP key is locked.\n");
            else
                sysprintf("Failed to lock MTP key!\n");
            break;
        }
        sysprintf("\nPress any key...\n");
        sysGetChar();
    }
}
