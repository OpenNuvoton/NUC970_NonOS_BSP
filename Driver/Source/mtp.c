/**************************************************************************//**
 * @file     mtp.c
 * @version  V1.10
 * $Revision: 2 $
 * $Date: 15/05/06 3:55p $
 * @brief  NUC970 MTP driver source file
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "nuc970.h"
#include "mtp.h"


/** @addtogroup NUC970_Device_Driver NUC970 Device Driver
  @{
*/

/** @addtogroup NUC970_MTP_Driver MTP Driver
  @{
*/


/** @addtogroup NUC970_MTP_EXPORTED_FUNCTIONS MTP Exported Functions
  @{
*/


/**
  * @brief      Enable MTP key.
  *
  * @return
  *             - \ref MTP_RET_OK              MTP key is enabled.
  *             - \ref MTP_RET_ERR_REG_LOCK    Failed to unlock MTP controller.
  *             - \ref MTP_RET_ERR_ENABLE      Failed to enable MTP key.
  */
int  MTP_Enable(void)
{
    UINT32   loop;

    MTP->REGLCTL = 0x59;
    MTP->REGLCTL = 0x16;
    MTP->REGLCTL = 0x88;
    if (MTP->REGLCTL != 0x1)
    {
        mtp_dbg("MTP_Enable failed, MTP_REGLCTL was not set!\n\n");
        return MTP_RET_ERR_REG_LOCK;
    }

    MTP->KEYEN |= MTP_KEYEN_KEYEN;

    for (loop = 0; loop < POOLING_LOOP; loop++)
    {
        if ((MTP->STATUS & MTP_STATUS_MTPEN) &&
                !(MTP->STATUS & MTP_STATUS_BUSY))
        {
            if (MTP->STATUS & MTP_STATUS_NONPRG)
            {
                mtp_dbg("MTP enabled, no key programmed.\n");
                return MTP_RET_OK;
            }

            if (MTP->STATUS & MTP_STATUS_KEYVALID)
            {
                mtp_dbg("MTP enabled and key valid.\n");
                return MTP_RET_OK;
            }
        }
    }
    mtp_dbg("MTP_Enable failed!");
    return MTP_RET_ERR_ENABLE;
}


/**
  * @brief       Program the MTP key.
  *
  * @param[in]   key     The 256 bits MTP key to be programmed.
  * @param[in]   option  The 8 bits user data to be programmed to MTP_USERDATA.
  *
  * @return
  *              - \ref MTP_RET_OK              Success.
  *              - \ref MTP_RET_ERR_ENABLE      Failed to enable MTP key.
  *              - \ref MTP_RET_ERR_PRG_CNT     Cannot program MTP key, because program count reachs limit.
  *              - \ref MTP_RET_ERR_LOCK        Cannot program MTP key, because MTP key is locked.
  *              - \ref MTP_RET_ERR_PRG_FAIL    Failed to program MTP key.
  */
int MTP_Program(UINT32 key[8], UINT8 option)
{
    int  i, loop;

    if (MTP_Enable() != MTP_RET_OK)
        return MTP_RET_ERR_ENABLE;

    if (MTP_KEY_PROG_COUNT == 15)
        return MTP_RET_ERR_PRG_CNT;

    if (MTP->STATUS & MTP_STATUS_LOCKED)
        return MTP_RET_ERR_LOCK;

    MTP->CTL |= (MTP->CTL & MTP_CTL_MODE_MASK) | MTP_CLT_MODE_PROG;
    MTP->PCYCLE = 0x60AE * 2;
    for (i = 0; i < 8; i++)
        MTP->KEY[i] = key[i];

    MTP->USERDATA = option;

    MTP->PSTART = MTP_PSTART_PSTART;

    for (loop = 0; loop < POOLING_LOOP; loop++)
    {
        if (MTP->PSTART == 0)
            break;
    }
    if (loop >= POOLING_LOOP)
    {
        mtp_dbg("MTP_PSTART not cleared!\n");
        return MTP_RET_ERR_PRG_FAIL;
    }

    if (MTP->STATUS & MTP_STATUS_PRGFAIL)
    {
        mtp_dbg("MTP key program failed!\n");
        return MTP_RET_ERR_PRG_FAIL;
    }

    MTP_Enable();

    mtp_dbg("MPT key program OK, COUNT = %d\n", MTP_KEY_PROG_COUNT);

    return MTP_RET_OK;
}


/**
  * @brief       Lock the MTP key.
  *
  * @return
  *              - \ref MTP_RET_OK              Success.
  *              - \ref MTP_RET_ERR_ENABLE      Failed to enable MTP key.
  *              - \ref MTP_RET_ERR_NO_KEY      MTP is not programmed with any key yet.
  *              - \ref MTP_RET_ERR_LOCK        Failed to lock MTP key.
  */
int MTP_Lock(void)
{
    int  loop;

    if (MTP_Enable() != MTP_RET_OK)
        return MTP_RET_ERR_ENABLE;

    if (MTP->STATUS & MTP_STATUS_NONPRG)
    {
        mtp_dbg("No key in MTP.\n");
        return MTP_RET_ERR_NO_KEY;
    }

    MTP->CTL |= (MTP->CTL & MTP_CTL_MODE_MASK) | MTP_CTL_MODE_LOCK;
    MTP->PCYCLE = 0x60AE * 2;

    MTP->PSTART = MTP_PSTART_PSTART;

    for (loop = 0; loop < POOLING_LOOP; loop++)
    {
        if (MTP->PSTART == 0)
            break;
    }
    if (loop >= POOLING_LOOP)
    {
        mtp_dbg("Failed to lock MTP key!\n");
        return MTP_RET_ERR_LOCK;
    }

    if (MTP_Enable() != MTP_RET_OK)
        return MTP_RET_ERR_ENABLE;

    if ((MTP->STATUS & (MTP_STATUS_MTPEN | MTP_STATUS_KEYVALID | MTP_STATUS_LOCKED)) !=
            (MTP_STATUS_MTPEN | MTP_STATUS_KEYVALID | MTP_STATUS_LOCKED))
    {
        mtp_dbg("MTP lock failed!\n");
        return MTP_RET_ERR_LOCK;
    }

    return MTP_RET_OK;
}


/**
  * @brief       Check MTP key is locked or not.
  *
  * @return
  *          - 1:            MTP key is locked.
  *          - 0:            MTP key is not locked.
  *          - \ref MTP_RET_ERR_ENABLE      Failed to enable MTP key.
  */
int MTP_IsKeyLocked(void)
{
    if (MTP_Enable() != MTP_RET_OK)
        return MTP_RET_ERR_ENABLE;

    if (MTP->STATUS & MTP_STATUS_LOCKED)
        return 1;

    return 0;
}


/**
  * @brief       Get the MTP key program count.
  *
  * @return  The MTP key program count.
  * @return
  *          - \ref MTP_RET_ERR_ENABLE      Failed to enable MTP key.
  *          Otherwise      The MTP key program count.
  */
int MTP_GetPrgCount(void)
{
    if (MTP_Enable() != MTP_RET_OK)
        return MTP_RET_ERR_ENABLE;

    return MTP_KEY_PROG_COUNT;
}


/**
  * @brief       Get user data of the MTP key.
  *
  * @param[in]   usr_data   User data.
  *
  * @return
  *              - \ref MTP_RET_OK              Successfully get the user data..
  *              - \ref MTP_RET_ERR_ENABLE      Failed to enable MTP key.
  */
int MTP_GetUserData(UINT8 *usr_data)
{
    if (MTP_Enable() != MTP_RET_OK)
        return MTP_RET_ERR_ENABLE;

    *usr_data = MTP->USERDATA;

    return MTP_RET_OK;
}


/*@}*/ /* end of group NUC970_MTP_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC970_MTP_Driver */

/*@}*/ /* end of group NUC970_Device_Driver */

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/

