/******************************************************************************
 * @file     SDGlue.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 15/06/08 11:48a $
 * @brief    SD glue functions for FATFS
 *
 * @note
 * Copyright (C) 2013 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"
#include "sdh.h"
#include "ff.h"
#include "diskio.h"

extern int sd0_ok;
extern int sd1_ok;

FATFS  _FatfsVolSd0;
FATFS  _FatfsVolSd1;

static TCHAR  _Path[3] = { '0', ':', 0 };

void SD_Open_(unsigned int cardSel)
{
    switch (cardSel & 0xff)
    {
        case SD_PORT0:
            SD_Open(cardSel);

            if (SD_Probe(cardSel & 0x00ff) != TRUE)
            {
                sysprintf("SD0 initial fail!!\n");
                return;
            }

            f_mount(&_FatfsVolSd0, _Path, 1);
            break;

        case SD_PORT1:
            SD_Open(cardSel);

            if (SD_Probe(cardSel & 0x00ff) != TRUE)
            {
                sysprintf("SD1 initial fail!!\n");
                return;
            }

            _Path[0] =  1 + '0';
            f_mount(&_FatfsVolSd1, _Path, 1);
            break;
    }
}

void SD_Close_(unsigned int cardSel)
{
    if (cardSel == SD_PORT0)
    {
        sd0_ok = 0;
        memset(&SD0, 0, sizeof(SD_INFO_T));
        f_mount(NULL, _Path, 1);
        memset(&_FatfsVolSd0, 0, sizeof(FATFS));
    }
    else if (cardSel == SD_PORT1)
    {
        sd1_ok = 0;
        memset(&SD1, 0, sizeof(SD_INFO_T));
        _Path[0] =  1 + '0';
        f_mount(NULL, _Path, 1);
        memset(&_FatfsVolSd1, 0, sizeof(FATFS));
    }
}

