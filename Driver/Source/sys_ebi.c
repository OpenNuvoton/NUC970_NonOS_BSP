/**************************************************************************//**
* @file     sys_ebi.c
* @version  V1.00
* $Revision: 2 $
* $Date: 15/05/18 4:57p $
* @brief    NUC970 SYS EBI bus driver source file
*
* @note
* Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include "nuc970.h"
#include "sys.h"

INT32 sysSetExternalIO(INT extNo, UINT32 extBaseAddr, UINT32 extSize, INT extBusWidth)
{
    UINT32 volatile reg, extiobase;

    switch (extNo)
    {
        case EXT0:
            extiobase = REG_EBI_BNKCTL0;
            break;
        case EXT1:
            extiobase = REG_EBI_BNKCTL1;
            break;
        case EXT2:
            extiobase = REG_EBI_BNKCTL2;
            break;
        case EXT3:
            extiobase = REG_EBI_BNKCTL3;
            break;
        case EXT4:
            extiobase = REG_EBI_BNKCTL4;
            break;
        default:
            return 1;
    }

    reg = inpw(extiobase);

    // Set Bus width
    switch (extBusWidth)
    {
        case BUS_BIT_8:
            reg = (reg & 0xfffffffc) | 0x01;
            break;

        case BUS_BIT_16:
            reg = (reg & 0xfffffffc) | 0x02;
            break;

        case BUS_DISABLE:
            reg = reg & 0xfffffffc;
            break;
    }

    // Set size
    switch (extSize)
    {
        case SIZE_256K:
            reg = reg & 0xfff8ffff;
            break;
    }

    // Set Base address
    extBaseAddr = (extBaseAddr << 1) & 0xfff80000;
    reg = reg | extBaseAddr;

    // set the reg value into register
    outpw(extiobase, reg);

    return 0;
}

INT32 sysSetExternalIOTiming1(INT extNo, INT tACC, INT tACS)
{
    UINT32 volatile reg, extiobase;

    switch (extNo)
    {
        case EXT0:
            extiobase = REG_EBI_BNKCTL0;
            break;
        case EXT1:
            extiobase = REG_EBI_BNKCTL1;
            break;
        case EXT2:
            extiobase = REG_EBI_BNKCTL2;
            break;
        case EXT3:
            extiobase = REG_EBI_BNKCTL3;
            break;
        case EXT4:
            extiobase = REG_EBI_BNKCTL4;
            break;
        default:
            return 1;
    }

    reg = inpw(extiobase);

    if ((tACC >= 0) && (tACC <= 0xf))
        reg = (reg & 0xffff87ff) | (tACC << 11);

    if ((tACS >= 0) && (tACS <= 0x7))
        reg = (reg & 0xffffff1f) | (tACS << 5);

    outpw(extiobase, reg);

    return 0;

}

INT32 sysSetExternalIOTiming2(INT extNo, INT tCOH, INT tCOS)
{
    UINT32 volatile reg, extiobase;

    switch (extNo)
    {
        case EXT0:
            extiobase = REG_EBI_BNKCTL0;
            break;
        case EXT1:
            extiobase = REG_EBI_BNKCTL1;
            break;
        case EXT2:
            extiobase = REG_EBI_BNKCTL2;
            break;
        case EXT3:
            extiobase = REG_EBI_BNKCTL3;
            break;
        case EXT4:
            extiobase = REG_EBI_BNKCTL4;
            break;
        default:
            return 1;
    }

    reg = inpw(extiobase);

    if ((tCOH >= 0) && (tCOH <= 0x7))
        reg = (reg & 0xfffff8ff) | (tCOH << 8);

    if ((tCOS >= 0) && (tCOS <= 0x7))
        reg = (reg & 0xffffffe3) | (tCOS << 2);

    outpw(extiobase, reg);

    return 0;
}

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
