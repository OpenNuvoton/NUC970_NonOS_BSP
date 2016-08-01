/******************************************************************************
 * @file     eMMCGlue.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 15/06/12 9:56a $
 * @brief    FMI eMMC glue functions for FATFS
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nuc970.h"
#include "fmi.h"
#include "ff.h"
#include "diskio.h"

extern int emmc_ok;

FATFS  _FatfsVoleMMC;
static TCHAR  _Path[3] = { '2', ':', 0 };

void eMMC_Open_Disk(void)
{
    eMMC_Open();
    eMMC_Probe();
    f_mount(&_FatfsVoleMMC, _Path, 1);
}

void eMMC_Close_Disk(void)
{
    emmc_ok = 0;
    memset(&eMMC, 0, sizeof(EMMC_INFO_T));
    f_mount(NULL, _Path, 1);
    memset(&_FatfsVoleMMC, 0, sizeof(FATFS));
}

