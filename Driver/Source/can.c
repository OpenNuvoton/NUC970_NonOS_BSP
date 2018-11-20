/**************************************************************************//**
* @file     can.c
* @version  V1.00
* $Revision: 1 $
* $Date: 15/06/11 2:44p $
* @brief    NUC970 CAN driver source file
*
* @note
* Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include "nuc970.h"
#include "sys.h"
#include "can.h"

/** @addtogroup NUC970_Device_Driver NUC970 Device Driver
  @{
*/

/** @addtogroup NUC970_CAN_Driver CAN Driver
  @{
*/

/** @addtogroup NUC970_CAN_EXPORTED_CONSTANTS CAN Exported Constants
  @{
*/

/*@}*/ /* end of group NUC970_CAN_EXPORTED_CONSTANTS */

#include <stdio.h>


/// @cond HIDDEN_SYMBOLS

#define RETRY_COUNTS    (0x10000000)

#define TSEG1_MIN 2
#define TSEG1_MAX 16
#define TSEG2_MIN 1
#define TSEG2_MAX 8
#define BRP_MIN   1
#define BRP_MAX   1024  /* 6-bit BRP field + 4-bit BRPE field*/
#define SJW_MAX   4
#define BRP_INC   1


//#define DEBUG_PRINTF sysprintf
#define DEBUG_PRINTF(...)

/**
  * @brief    Enter initialization mode
  * @param[in]    uCAN    CAN channel.
  * @return   None
  * @details  This function is used to set CAN to enter initialization mode and enable access bit timing
  *           register. After bit timing configuration ready, user must call CAN_LeaveInitMode()
  *           to leave initialization mode and lock bit timing register to let new configuration
  *           take effect.
  */
void CAN_EnterInitMode(UINT32 uCAN)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;
    UINT32 uRegAddress;

    uRegAddress = REG_CAN0_CON + uOffset;
    outpw(uRegAddress, inpw(uRegAddress)|CAN_CON_INIT_Msk);
    outpw(uRegAddress, inpw(uRegAddress)|CAN_CON_CCE_Msk);
}


/**
  * @brief    Leave initialization mode
  * @param[in]    uCAN    CAN channel.
  * @return   None
  * @details  This function is used to set CAN to leave initialization mode to let
  *           bit timing configuration take effect after configuration ready.
  */
void CAN_LeaveInitMode(UINT32 uCAN)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;
    UINT32 uRegAddress, uRegVal;

    uRegAddress = REG_CAN0_CON + uOffset;
    uRegVal = inpw(uRegAddress);
    uRegVal &= (~(CAN_CON_INIT_Msk | CAN_CON_CCE_Msk));
    outpw(uRegAddress, uRegVal);

    while(inpw(uRegAddress) & CAN_CON_INIT_Msk);
}

/**
  * @brief    Wait message into message buffer in basic mode.
  * @param[in]    uCAN    CAN channel.
  * @return   None
  * @details  This function is used to wait message into message buffer in basic mode. Please notice the
  *           function is polling NEWDAT bit of MCON register by while loop and it is used in basic mode.
  */
void CAN_WaitMsg(UINT32 uCAN)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;

    outpw((REG_CAN0_STATUS+uOffset), 0x0);

    while(1)
    {
        if(inpw(REG_CAN0_IF2_MCON+uOffset) & CAN_IF_MCON_NEWDAT_Msk)
        {
            DEBUG_PRINTF("New Data IN\n");
            break;
        }

        if(inpw(REG_CAN0_STATUS+uOffset) & CAN_STATUS_RXOK_Msk)
            DEBUG_PRINTF("Rx OK\n");

        if(inpw(REG_CAN0_STATUS+uOffset) & CAN_STATUS_LEC_Msk)
            DEBUG_PRINTF("Error\n");
    }

}

/**
  * @brief    Get current bit rate
  * @param[in]    uCAN    CAN channel.
  * @return   Current Bit-Rate (kilo bit per second)
  * @details  Return current CAN bit rate according to the user bit-timing parameter settings
  */
uint32_t CAN_GetCANBitRate(UINT32  uCAN)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;
    UINT32 uRegAddress;

    UINT8 u8Tseg1,u8Tseg2;
    UINT32 u32Bpr, u32Value;

    u32Value = sysGetClock(SYS_PCLK)*1000000;

    uRegAddress = REG_CAN0_BTIME + uOffset;
    u8Tseg1 = (inpw(uRegAddress) & CAN_BTIME_TSEG1_Msk) >> CAN_BTIME_TSEG1_Pos;
    u8Tseg2 = (inpw(uRegAddress) & CAN_BTIME_TSEG2_Msk) >> CAN_BTIME_TSEG2_Pos;
    u32Bpr  = (inpw(uRegAddress) & CAN_BTIME_BRP_Msk) | (inpw(REG_CAN0_BRPE+uOffset) << 6);

    return (u32Value/(u32Bpr+1)/(u8Tseg1 + u8Tseg2 + 3));
}

/**
  * @brief    Switch the CAN into test mode.
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u8TestMask  Specifies the configuration in test modes
  *                             CAN_TEST_BASIC_Msk   : Enable basic mode of test mode
  *                             CAN_TESTR_SILENT_Msk  : Enable silent mode of test mode
  *                             CAN_TESTR_LBACK_Msk   : Enable Loop Back Mode of test mode
  *                             CAN_TESTR_TX0_Msk/CAN_TESTR_TX1_Msk: Control CAN_TX pin bit field
  * @return   None
  * @details  Switch the CAN into test mode. There are four test mode (BASIC/SILENT/LOOPBACK/
  *           LOOPBACK combined SILENT/CONTROL_TX_PIN)could be selected. After setting test mode,user
  *           must call CAN_LeaveInitMode() to let the setting take effect.
  */
void CAN_EnterTestMode(UINT32 uCAN, uint8_t u8TestMask)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;
    UINT32 uRegVal;

    uRegVal = inpw(REG_CAN0_CON+uOffset);
    uRegVal |= CAN_CON_TEST_Msk;
    outpw((REG_CAN0_CON+uOffset), uRegVal);

    outpw((REG_CAN0_TEST+uOffset), u8TestMask);
}




/**
  * @brief    Leave the test mode
  * @param[in]    uCAN    CAN channel.
  * @return   None
  * @details  This function is used to Leave the test mode (switch into normal mode).
  */
void CAN_LeaveTestMode(UINT32 uCAN)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;
    UINT32 uRegVal;

    uRegVal = inpw(REG_CAN0_CON+uOffset);
    uRegVal |= CAN_CON_TEST_Msk;
    outpw((REG_CAN0_CON+uOffset), uRegVal);

    uRegVal = inpw(REG_CAN0_TEST+uOffset);
    uRegVal &= ~CAN_CON_TEST_Msk;
    outpw((REG_CAN0_TEST+uOffset), uRegVal);

    uRegVal = inpw(REG_CAN0_CON+uOffset);
    uRegVal &= ~CAN_CON_TEST_Msk;
    outpw((REG_CAN0_CON+uOffset), uRegVal);
}

/**
  * @brief    Get the waiting status of a received message.
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u8MsgObj    Specifies the Message object number, from 0 to 31.
  * @retval   non-zero    The corresponding message object has a new data bit is set.
  * @retval   0           No message object has new data.
  * @details  This function is used to get the waiting status of a received message.
  */
uint32_t CAN_IsNewDataReceived(UINT32 uCAN, uint8_t u8MsgObj)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;

    return (u8MsgObj < 16 ? inpw(REG_CAN0_NDAT1 + uOffset) & (1 << u8MsgObj) : inpw(REG_CAN0_NDAT2 + uOffset) & (1 << (u8MsgObj-16)));
}


/**
  * @brief    Send CAN message in BASIC mode of test mode
  * @param[in]    uCAN    CAN channel.
  * @param[in]    pCanMsg     Pointer to the message structure containing data to transmit.
  * @return   TRUE:  Transmission OK
  *           FALSE: Check busy flag of interface 0 is timeout
  * @details  The function is used to send CAN message in BASIC mode of test mode. Before call the API,
  *           the user should be call CAN_EnterTestMode(CAN_TESTR_BASIC) and let CAN controller enter
  *           basic mode of test mode. Please notice IF1 Registers used as Tx Buffer in basic mode.
  */
int32_t CAN_BasicSendMsg(UINT32 uCAN, STR_CANMSG_T* pCanMsg)
{

    UINT32 uOffset = uCAN * CAN_OFFSET;
    UINT32 uRegVal;

    UINT32 i=0;

    while(inpw(REG_CAN0_IF1_CREQ+uOffset) & CAN_IF_CREQ_BUSY_Msk);

    uRegVal = inpw(REG_CAN0_STATUS+uOffset);
    uRegVal &= ~CAN_STATUS_TXOK_Msk;
    outpw((REG_CAN0_STATUS+uOffset), uRegVal);

    outpw((REG_CAN0_IF1_CMASK+uOffset), CAN_IF_CMASK_WRRD_Msk);

    if (pCanMsg->IdType == CAN_STD_ID)
    {
        /* standard ID*/
        outpw((REG_CAN0_IF1_ARB1+uOffset), 0x0);
        outpw((REG_CAN0_IF1_ARB2+uOffset), (((pCanMsg->Id)&0x7FF)<<2));
    }
    else
    {
        /* extended ID*/
        outpw((REG_CAN0_IF1_ARB1+uOffset), ((pCanMsg->Id)&0xFFFF));
        outpw((REG_CAN0_IF1_ARB2+uOffset), (((pCanMsg->Id)&0x1FFF0000)>>16  | CAN_IF_ARB2_XTD_Msk) );
    }

    if(pCanMsg->FrameType)
    {
        uRegVal = inpw(REG_CAN0_IF1_ARB2+uOffset);
        uRegVal |= CAN_IF_ARB2_DIR_Msk;
        outpw((REG_CAN0_IF1_ARB2+uOffset), uRegVal);
    }
    else
    {
        uRegVal = inpw(REG_CAN0_IF1_ARB2+uOffset);
        uRegVal &= ~CAN_IF_ARB2_DIR_Msk;
        outpw((REG_CAN0_IF1_ARB2+uOffset), uRegVal);
    }

    outpw((REG_CAN0_IF1_MCON+uOffset), (inpw(REG_CAN0_IF1_MCON+uOffset) & (~CAN_IF_MCON_DLC_Msk) | pCanMsg->DLC) );
    outpw((REG_CAN0_IF1_DAT_A1+uOffset), (((uint16_t)pCanMsg->Data[1]<<8) | pCanMsg->Data[0]));
    outpw((REG_CAN0_IF1_DAT_A2+uOffset), (((uint16_t)pCanMsg->Data[3]<<8) | pCanMsg->Data[2]));
    outpw((REG_CAN0_IF1_DAT_B1+uOffset), (((uint16_t)pCanMsg->Data[5]<<8) | pCanMsg->Data[4]));
    outpw((REG_CAN0_IF1_DAT_B2+uOffset), (((uint16_t)pCanMsg->Data[7]<<8) | pCanMsg->Data[6]));

    /* request transmission*/
    outpw((REG_CAN0_IF1_CREQ+uOffset), (inpw(REG_CAN0_IF1_CREQ+uOffset) & (~CAN_IF_CREQ_BUSY_Msk)));

    if(inpw(REG_CAN0_IF1_CREQ+uOffset) & CAN_IF_CREQ_BUSY_Msk)
    {
        DEBUG_PRINTF("Cannot clear busy for sending ...\n");
        return FALSE;
    }

    outpw((REG_CAN0_IF1_CREQ+uOffset), (inpw(REG_CAN0_IF1_CREQ+uOffset) | CAN_IF_CREQ_BUSY_Msk));  // sending

    for ( i=0; i<0xFFFFF; i++)
    {
        if( (inpw(REG_CAN0_IF1_CREQ+uOffset) & CAN_IF_CREQ_BUSY_Msk) == 0 ) break;
    }

    if ( i >= 0xFFFFF )
    {
        DEBUG_PRINTF("Cannot send out...\n");
        return FALSE;
    }

    return TRUE;
}


/**
  * @brief    Get a message information in BASIC mode.
  *
  * @param[in]    uCAN    CAN channel.
  * @param[out]    pCanMsg     Pointer to the message structure where received data is copied.
  *
  * @return   FALSE  No any message received. \n
  *           TRUE   Receive a message success.
  *
  */
int32_t CAN_BasicReceiveMsg(UINT32 uCAN, STR_CANMSG_T* pCanMsg)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;

    if((inpw(REG_CAN0_IF2_MCON+uOffset) & CAN_IF_MCON_NEWDAT_Msk) == 0)  /* In basic mode, receive data always save in IF2 */
    {
        return FALSE;
    }

    outpw((REG_CAN0_STATUS+uOffset), (inpw(REG_CAN0_STATUS+uOffset) & (~CAN_STATUS_RXOK_Msk)));

    outpw((REG_CAN0_IF2_CMASK+uOffset), ( CAN_IF_CMASK_ARB_Msk
                                          | CAN_IF_CMASK_CONTROL_Msk
                                          | CAN_IF_CMASK_DATAA_Msk
                                          | CAN_IF_CMASK_DATAB_Msk));


    if((inpw(REG_CAN0_IF2_ARB2+uOffset) & CAN_IF_ARB2_XTD_Msk) == 0)
    {
        /* standard ID*/
        pCanMsg->IdType = CAN_STD_ID;
        pCanMsg->Id = (inpw(REG_CAN0_IF2_ARB2+uOffset) >> 2) & 0x07ff;

    }
    else
    {
        /* extended ID*/
        pCanMsg->IdType = CAN_EXT_ID;
        pCanMsg->Id  = (inpw(REG_CAN0_IF2_ARB2+uOffset) &0x1FFF) << 16;
        pCanMsg->Id |= inpw(REG_CAN0_IF2_ARB1+uOffset);
    }

    pCanMsg->FrameType = !((inpw(REG_CAN0_IF2_ARB2+uOffset) & CAN_IF_ARB2_DIR_Msk) >> CAN_IF_ARB2_DIR_Pos);

    pCanMsg->DLC     = inpw(REG_CAN0_IF2_MCON+uOffset) & CAN_IF_MCON_DLC_Msk;
    pCanMsg->Data[0] = inpw(REG_CAN0_IF2_DAT_A1+uOffset) & CAN_IF_DAT_A1_DATA0_Msk;
    pCanMsg->Data[1] = (inpw(REG_CAN0_IF2_DAT_A1+uOffset) & CAN_IF_DAT_A1_DATA1_Msk) >> CAN_IF_DAT_A1_DATA1_Pos;
    pCanMsg->Data[2] = inpw(REG_CAN0_IF2_DAT_A2+uOffset) & CAN_IF_DAT_A2_DATA2_Msk;
    pCanMsg->Data[3] = (inpw(REG_CAN0_IF2_DAT_A2+uOffset) & CAN_IF_DAT_A2_DATA3_Msk) >> CAN_IF_DAT_A2_DATA3_Pos;
    pCanMsg->Data[4] = inpw(REG_CAN0_IF2_DAT_B1+uOffset) & CAN_IF_DAT_B1_DATA4_Msk;
    pCanMsg->Data[5] = (inpw(REG_CAN0_IF2_DAT_B1+uOffset) & CAN_IF_DAT_B1_DATA5_Msk) >> CAN_IF_DAT_B1_DATA5_Pos;
    pCanMsg->Data[6] = inpw(REG_CAN0_IF2_DAT_B2+uOffset) & CAN_IF_DAT_B2_DATA6_Msk;
    pCanMsg->Data[7] = (inpw(REG_CAN0_IF2_DAT_B2+uOffset) & CAN_IF_DAT_B2_DATA7_Msk) >> CAN_IF_DAT_B2_DATA7_Pos;

    return TRUE;
}

/**
  * @brief    Set Rx message object
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u8MsgObj    Specifies the Message object number, from 0 to 31.
  * @param[in]    u8idType    Specifies the identifier type of the frames that will be transmitted
  *                       This parameter can be one of the following values:
  *                       CAN_STD_ID (standard ID, 11-bit)
  *                       CAN_EXT_ID (extended ID, 29-bit)
  * @param[in]    u32id       Specifies the identifier used for acceptance filtering.
  * @param[in]    u8singleOrFifoLast  Specifies the end-of-buffer indicator.
  *                                 This parameter can be one of the following values:
  *                                 TRUE: for a single receive object or a FIFO receive object that is the last one of the FIFO.
  *                                 FALSE: for a FIFO receive object that is not the last one.
  * @retval   TRUE           SUCCESS
  * @retval   FALSE   No useful interface
  * @details  The function is used to configure a receive message object.
  */
int32_t CAN_SetRxMsgObj(UINT32  uCAN, uint8_t u8MsgObj, uint8_t u8idType, uint32_t u32id, uint8_t u8singleOrFifoLast)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;

    if((inpw(REG_CAN0_IF2_CREQ+uOffset) & CAN_IF_CREQ_BUSY_Msk) != 0)
    {
        return FALSE;
    }

    /* Command Setting */
    outpw((REG_CAN0_IF2_CMASK+uOffset), (CAN_IF_CMASK_WRRD_Msk | CAN_IF_CMASK_MASK_Msk | CAN_IF_CMASK_ARB_Msk |
                                         CAN_IF_CMASK_CONTROL_Msk | CAN_IF_CMASK_DATAA_Msk | CAN_IF_CMASK_DATAB_Msk));

    if (u8idType == CAN_STD_ID)   /* According STD/EXT ID format,Configure Mask and Arbitration register */
    {
        outpw((REG_CAN0_IF2_ARB1+uOffset), 0x0);
        outpw((REG_CAN0_IF2_ARB2+uOffset), (CAN_IF_ARB2_MSGVAL_Msk | (u32id & 0x7FF)<< 2));
    }
    else
    {
        outpw((REG_CAN0_IF2_ARB1+uOffset), (u32id & 0xFFFF));
        outpw((REG_CAN0_IF2_ARB2+uOffset), (CAN_IF_ARB2_MSGVAL_Msk | CAN_IF_ARB2_XTD_Msk | (u32id & 0x1FFF0000)>>16));
    }

    outpw((REG_CAN0_IF2_MCON+uOffset), (inpw(REG_CAN0_IF2_MCON+uOffset) | CAN_IF_MCON_UMASK_Msk | CAN_IF_MCON_RXIE_Msk));

    if(u8singleOrFifoLast)
    {
        outpw((REG_CAN0_IF2_MCON+uOffset), (inpw(REG_CAN0_IF2_MCON+uOffset) | CAN_IF_MCON_EOB_Msk));
    }
    else
    {
        outpw((REG_CAN0_IF2_MCON+uOffset), (inpw(REG_CAN0_IF2_MCON+uOffset) & ~CAN_IF_MCON_EOB_Msk));
    }

    outpw((REG_CAN0_IF2_DAT_A1+uOffset), 0x0);
    outpw((REG_CAN0_IF2_DAT_A2+uOffset), 0x0);
    outpw((REG_CAN0_IF2_DAT_B1+uOffset), 0x0);
    outpw((REG_CAN0_IF2_DAT_B2+uOffset), 0x0);

    outpw((REG_CAN0_IF2_CREQ+uOffset), (1 + u8MsgObj));

    return TRUE;
}


/**
  * @brief    Gets the message
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u8MsgObj    Specifies the Message object number, from 0 to 31.
  * @param[in]    u8Release   Specifies the message release indicator.
  *                       This parameter can be one of the following values:
  *                        TRUE: the message object is released when getting the data.
  *                        FALSE:the message object is not released.
  * @param[out]    pCanMsg     Pointer to the message structure where received data is copied.
  * @retval   TRUE   Success
  * @retval   FALSE    No any message received
  * @details  Gets the message, if received.
  */
int32_t CAN_ReadMsgObj(UINT32 uCAN, uint8_t u8MsgObj, uint8_t u8Release, STR_CANMSG_T* pCanMsg)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;

    if (!CAN_IsNewDataReceived(uCAN, u8MsgObj))
    {
        return FALSE;
    }

    outpw((REG_CAN0_STATUS+uOffset), (inpw(REG_CAN0_STATUS+uOffset) & ~CAN_STATUS_RXOK_Msk));

    /* read the message contents*/
    outpw((REG_CAN0_IF2_CMASK+uOffset), (CAN_IF_CMASK_MASK_Msk
                                         | CAN_IF_CMASK_ARB_Msk
                                         | CAN_IF_CMASK_CONTROL_Msk
                                         | CAN_IF_CMASK_CLRINTPND_Msk
                                         | (u8Release ? CAN_IF_CMASK_TXRQSTNEWDAT_Msk : 0)
                                         | CAN_IF_CMASK_DATAA_Msk
                                         | CAN_IF_CMASK_DATAB_Msk));

    outpw((REG_CAN0_IF2_CREQ+uOffset), (1 + u8MsgObj));

    while (inpw(REG_CAN0_IF2_CREQ+uOffset) & CAN_IF_CREQ_BUSY_Msk)
    {
        /*Wait*/
    }

    if ((inpw(REG_CAN0_IF2_ARB2+uOffset) & CAN_IF_ARB2_XTD_Msk) == 0)
    {
        /* standard ID*/
        pCanMsg->IdType = CAN_STD_ID;
        pCanMsg->Id     = (inpw(REG_CAN0_IF2_ARB2+uOffset) & CAN_IF_ARB2_ID_Msk) >> 2;
    }
    else
    {
        /* extended ID*/
        pCanMsg->IdType = CAN_EXT_ID;
        pCanMsg->Id  = (((inpw(REG_CAN0_IF2_ARB2+uOffset)) & 0x1FFF)<<16) | inpw(REG_CAN0_IF2_ARB1+uOffset);
    }

    pCanMsg->DLC     = inpw(REG_CAN0_IF2_MCON+uOffset) & CAN_IF_MCON_DLC_Msk;
    pCanMsg->Data[0] = inpw(REG_CAN0_IF2_DAT_A1+uOffset) & CAN_IF_DAT_A1_DATA0_Msk;
    pCanMsg->Data[1] = (inpw(REG_CAN0_IF2_DAT_A1+uOffset) & CAN_IF_DAT_A1_DATA1_Msk) >> CAN_IF_DAT_A1_DATA1_Pos;
    pCanMsg->Data[2] = inpw(REG_CAN0_IF2_DAT_A2+uOffset) & CAN_IF_DAT_A2_DATA2_Msk;
    pCanMsg->Data[3] = (inpw(REG_CAN0_IF2_DAT_A2+uOffset) & CAN_IF_DAT_A2_DATA3_Msk) >> CAN_IF_DAT_A2_DATA3_Pos;
    pCanMsg->Data[4] = inpw(REG_CAN0_IF2_DAT_B1+uOffset) & CAN_IF_DAT_B1_DATA4_Msk;
    pCanMsg->Data[5] = (inpw(REG_CAN0_IF2_DAT_B1+uOffset) & CAN_IF_DAT_B1_DATA5_Msk) >> CAN_IF_DAT_B1_DATA5_Pos;
    pCanMsg->Data[6] = inpw(REG_CAN0_IF2_DAT_B2+uOffset) & CAN_IF_DAT_B2_DATA6_Msk;
    pCanMsg->Data[7] = (inpw(REG_CAN0_IF2_DAT_B2+uOffset) & CAN_IF_DAT_B2_DATA7_Msk) >> CAN_IF_DAT_B2_DATA7_Pos;

    return TRUE;
}

static int can_update_spt(int sampl_pt, int tseg, int *tseg1, int *tseg2)
{
    *tseg2 = tseg + 1 - (sampl_pt * (tseg + 1)) / 1000;
    if (*tseg2 < TSEG2_MIN)
        *tseg2 = TSEG2_MIN;
    if (*tseg2 > TSEG2_MAX)
        *tseg2 = TSEG2_MAX;
    *tseg1 = tseg - *tseg2;
    if (*tseg1 > TSEG1_MAX)
    {
        *tseg1 = TSEG1_MAX;
        *tseg2 = tseg - *tseg1;
    }
    return 1000 * (tseg + 1 - *tseg2) / (tseg + 1);
}

/// @endcond HIDDEN_SYMBOLS

/** @addtogroup NUC970_CAN_EXPORTED_FUNCTIONS CAN Exported Functions
  @{
*/


/**
  * @brief    The function is used to set bus timing parameter according current clock and target baud-rate.
  *
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u32BaudRate The target CAN baud-rate. The range of u32BaudRate is 1~1000KHz
  *
  * @return   u32CurrentBitRate  Real baud-rate value
  */
uint32_t CAN_SetBaudRate(UINT32 uCAN, uint32_t u32BaudRate)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;

    long rate;
    long best_error = 1000000000, error = 0;
    int best_tseg = 0, best_brp = 0, brp = 0;
    int tsegall, tseg = 0, tseg1 = 0, tseg2 = 0;
    int spt_error = 1000, spt = 0, sampl_pt;
    uint64_t clock_freq = 0;
    uint32_t sjw = 1;

    CAN_EnterInitMode(uCAN);

    clock_freq = sysGetClock(SYS_PCLK);
    clock_freq = clock_freq*1000000;

    if(u32BaudRate >= 1000000)
        u32BaudRate = 1000000;

    /* Use CIA recommended sample points */
    if (u32BaudRate > 800000)
        sampl_pt = 750;
    else if (u32BaudRate > 500000)
        sampl_pt = 800;
    else
        sampl_pt = 875;

    /* tseg even = round down, odd = round up */
    for (tseg = (TSEG1_MAX + TSEG2_MAX) * 2 + 1; tseg >= (TSEG1_MIN + TSEG2_MIN) * 2; tseg--)
    {
        tsegall = 1 + tseg / 2;
        /* Compute all possible tseg choices (tseg=tseg1+tseg2) */
        brp = clock_freq / (tsegall * u32BaudRate) + tseg % 2;
        /* chose brp step which is possible in system */
        brp = (brp / BRP_INC) * BRP_INC;

        if ((brp < BRP_MIN) || (brp > BRP_MAX))
            continue;
        rate = clock_freq / (brp * tsegall);

        error = u32BaudRate - rate;

        /* tseg brp biterror */
        if (error < 0)
            error = -error;
        if (error > best_error)
            continue;
        best_error = error;
        if (error == 0)
        {
            spt = can_update_spt(sampl_pt, tseg / 2, &tseg1, &tseg2);
            error = sampl_pt - spt;
            if (error < 0)
                error = -error;
            if (error > spt_error)
                continue;
            spt_error = error;
        }
        best_tseg = tseg / 2;
        best_brp = brp;

        if (error == 0)
            break;
    }

    spt = can_update_spt(sampl_pt, best_tseg, &tseg1, &tseg2);

    /* check for sjw user settings */
    /* bt->sjw is at least 1 -> sanitize upper bound to sjw_max */
    if (sjw > SJW_MAX)
        sjw = SJW_MAX;
    /* bt->sjw must not be higher than tseg2 */
    if (tseg2 < sjw)
        sjw = tseg2;

    /* real bit-rate */
    u32BaudRate = clock_freq / (best_brp * (tseg1 + tseg2 + 1));

    outpw((REG_CAN0_BTIME+uOffset), (((uint32_t)(tseg2 - 1) << CAN_BTIME_TSEG2_Pos) | ((uint32_t)(tseg1 - 1) << CAN_BTIME_TSEG1_Pos) |
                                     ((best_brp - 1) & CAN_BTIME_BRP_Msk) | (sjw << CAN_BTIME_SJW_Pos)));
    outpw((REG_CAN0_BRPE+uOffset), (((best_brp - 1) >> 6) & 0x0F));

    //printf("\n bitrate = %d \n", CAN_GetCANBitRate(uCAN));

    CAN_LeaveInitMode(uCAN);

    return u32BaudRate;
}

/**
  * @brief    The function is used to disable all CAN interrupt.
  *
  * @param[in]     uCAN    CAN channel.
  *
  * @return   None
  */
void CAN_Close(UINT32 uCAN)
{
    CAN_DisableInt(uCAN, (CAN_CON_IE_Msk|CAN_CON_SIE_Msk|CAN_CON_EIE_Msk));
}

/**
  * @brief    The function is sets bus timing parameter according current clock and target baud-rate. And set CAN operation mode.
  *
  * @param[in]     uCAN    CAN channel.
  * @param[in]    u32BaudRate The target CAN baud-rate. The range of u32BaudRate is 1~1000KHz
  * @param[in]    u32Mode     The CAN operation mode. ( \ref CAN_NORMAL_MODE / \ref CAN_BASIC_MODE)
  *
  * @return   u32CurrentBitRate  Real baud-rate value
  */
uint32_t CAN_Open(UINT32 uCAN, uint32_t u32BaudRate, uint32_t u32Mode)
{
    uint32_t u32CurrentBitRate;

    u32CurrentBitRate = CAN_SetBaudRate(uCAN, u32BaudRate);

    if(u32Mode == CAN_BASIC_MODE)
        CAN_EnterTestMode(uCAN, CAN_TEST_BASIC_Msk);

    return u32CurrentBitRate;
}

/**
  * @brief    The function is used to configure a transmit object.
  *
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u32MsgNum   Specifies the Message object number, from 0 to 31
  * @param[in]    pCanMsg     Pointer to the message structure where received data is copied.
  *
  * @return   FALSE: No useful interface. \n
  *           TRUE : Config message object success.
  *
  */
int32_t CAN_SetTxMsg(UINT32 uCAN, uint32_t u32MsgNum , STR_CANMSG_T* pCanMsg)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;

    uint32_t i=0;

    while((inpw(REG_CAN0_IF1_CREQ+uOffset) & CAN_IF_CREQ_BUSY_Msk) != 0)
    {
        i++;
        if(i > 0x10000000)
            return FALSE;
    }

    /* update the contents needed for transmission*/
    outpw((REG_CAN0_IF1_CMASK+uOffset), 0xF3); /*CAN_CMASK_WRRD_Msk | CAN_CMASK_MASK_Msk | CAN_CMASK_ARB_Msk
                                               | CAN_CMASK_CONTROL_Msk | CAN_CMASK_DATAA_Msk  | CAN_CMASK_DATAB_Msk ; */

    if (pCanMsg->IdType == CAN_STD_ID)
    {
        /* standard ID*/
        outpw((REG_CAN0_IF1_ARB1+uOffset), 0x0);
        outpw((REG_CAN0_IF1_ARB2+uOffset), ((((pCanMsg->Id)&0x7FF)<<2) | CAN_IF_ARB2_DIR_Msk | CAN_IF_ARB2_MSGVAL_Msk));

    }
    else
    {
        /* extended ID*/
        outpw((REG_CAN0_IF1_ARB1+uOffset), ((pCanMsg->Id)&0xFFFF));
        outpw((REG_CAN0_IF1_ARB2+uOffset), (((pCanMsg->Id)&0x1FFF0000)>>16 | CAN_IF_ARB2_DIR_Msk
                                            | CAN_IF_ARB2_XTD_Msk | CAN_IF_ARB2_MSGVAL_Msk));
    }

    if(pCanMsg->FrameType)
        outpw((REG_CAN0_IF1_ARB2+uOffset), (inpw(REG_CAN0_IF1_ARB2+uOffset) | CAN_IF_ARB2_DIR_Msk));
    else
        outpw((REG_CAN0_IF1_ARB2+uOffset), (inpw(REG_CAN0_IF1_ARB2+uOffset) & ~CAN_IF_ARB2_DIR_Msk));

    outpw((REG_CAN0_IF1_DAT_A1+uOffset), (((uint16_t)pCanMsg->Data[1]<<8) | pCanMsg->Data[0]));
    outpw((REG_CAN0_IF1_DAT_A2+uOffset), (((uint16_t)pCanMsg->Data[3]<<8) | pCanMsg->Data[2]));
    outpw((REG_CAN0_IF1_DAT_B1+uOffset), (((uint16_t)pCanMsg->Data[5]<<8) | pCanMsg->Data[4]));
    outpw((REG_CAN0_IF1_DAT_B2+uOffset), (((uint16_t)pCanMsg->Data[7]<<8) | pCanMsg->Data[6]));

    outpw((REG_CAN0_IF1_MCON+uOffset), (CAN_IF_MCON_NEWDAT_Msk | pCanMsg->DLC |CAN_IF_MCON_TXIE_Msk | CAN_IF_MCON_EOB_Msk));
    outpw((REG_CAN0_IF1_CREQ+uOffset), (1 + u32MsgNum));

    return TRUE;
}

/**
  * @brief    Set transmit request bit
  *
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u32MsgNum    Specifies the Message object number, from 0 to 31.
  *
  * @return   TRUE: Start transmit message.
  */
int32_t CAN_TriggerTxMsg(UINT32 uCAN, uint32_t u32MsgNum)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;
    STR_CANMSG_T rMsg;

    CAN_ReadMsgObj(uCAN, u32MsgNum,TRUE, &rMsg);

    outpw((REG_CAN0_IF1_CMASK+uOffset), (CAN_IF_CMASK_WRRD_Msk |CAN_IF_CMASK_TXRQSTNEWDAT_Msk));
    outpw((REG_CAN0_IF1_CREQ+uOffset), (1 + u32MsgNum));

    return TRUE;
}

/**
  * @brief    Enable CAN interrupt
  *
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u32Mask    Interrupt Mask. ( \ref CAN_CON_IE_Msk / \ref CAN_CON_SIE_Msk / \ref CAN_CON_EIE_Msk)
  *
  * @return   None
  */
void CAN_EnableInt(UINT32 uCAN, uint32_t u32Mask)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;

    CAN_EnterInitMode(uCAN);

    outpw((REG_CAN0_CON+uOffset), ((inpw(REG_CAN0_CON+uOffset) & 0xF1) | ((u32Mask & CAN_CON_IE_Msk   )? CAN_CON_IE_Msk :0)
                                   | ((u32Mask & CAN_CON_SIE_Msk  )? CAN_CON_SIE_Msk:0)
                                   | ((u32Mask & CAN_CON_EIE_Msk  )? CAN_CON_EIE_Msk:0)));

    CAN_LeaveInitMode(uCAN);
}

/**
  * @brief    Disable CAN interrupt
  *
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u32Mask    Interrupt Mask. ( \ref CAN_CON_IE_Msk / \ref CAN_CON_SIE_Msk / \ref CAN_CON_EIE_Msk)
  *
  * @return   None
  */
void CAN_DisableInt(UINT32 uCAN, uint32_t u32Mask)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;

    CAN_EnterInitMode(uCAN);

    outpw((REG_CAN0_CON+uOffset), (inpw(REG_CAN0_CON+uOffset) & ~(CAN_CON_IE_Msk | ((u32Mask & CAN_CON_SIE_Msk)?CAN_CON_SIE_Msk:0)
                                   | ((u32Mask & CAN_CON_EIE_Msk)?CAN_CON_EIE_Msk:0))));

    CAN_LeaveInitMode(uCAN);
}


/**
  * @brief    The function is used to configure a receive message object
  *
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u32MsgNum   Specifies the Message object number, from 0 to 31
  * @param[in]    u32IDType   Specifies the identifier type of the frames that will be transmitted. ( \ref CAN_STD_ID / \ref CAN_EXT_ID)
  * @param[in]    u32ID       Specifies the identifier used for acceptance filtering.
  *
  * @return   FALSE: No useful interface \n
  *           TRUE : Configure a receive message object success.
  *
  */
int32_t CAN_SetRxMsg(UINT32 uCAN, uint32_t u32MsgNum , uint32_t u32IDType, uint32_t u32ID)
{
    uint32_t u32TimeOutCount = 0;

    while(CAN_SetRxMsgObj(uCAN, u32MsgNum, u32IDType, u32ID, TRUE) == FALSE)
    {
        u32TimeOutCount++;

        if(u32TimeOutCount >= 0x10000000) return FALSE;
    }

    return TRUE;
}

/**
  * @brief    The function is used to configure several receive message objects
  *
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u32MsgNum   The starting MSG RAM number. (0 ~ 31)
  * @param[in]    u32MsgCount the number of MSG RAM of the FIFO.
  * @param[in]    u32IDType   Specifies the identifier type of the frames that will be transmitted. ( \ref CAN_STD_ID / \ref CAN_EXT_ID)
  * @param[in]    u32ID       Specifies the identifier used for acceptance filtering.
  *
  * @return   FALSE: No useful interface \n
  *           TRUE : Configure receive message objects success.
  *
  */
int32_t CAN_SetMultiRxMsg(UINT32 uCAN, uint32_t u32MsgNum , uint32_t u32MsgCount, uint32_t u32IDType, uint32_t u32ID)
{
    uint32_t i = 0;
    uint32_t u32TimeOutCount;
    uint32_t u32EOB_Flag = 0;

    for(i= 1; i < u32MsgCount; i++)
    {
        u32TimeOutCount = 0;

        u32MsgNum += (i - 1);

        if(i == u32MsgCount) u32EOB_Flag = 1;

        while(CAN_SetRxMsgObj(uCAN, u32MsgNum, u32IDType, u32ID, u32EOB_Flag) == FALSE)
        {
            u32TimeOutCount++;

            if(u32TimeOutCount >= 0x10000000) return FALSE;
        }
    }

    return TRUE;
}


/**
  * @brief    Send CAN message.
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u32MsgNum   Specifies the Message object number, from 0 to 31
  * @param[in]    pCanMsg     Pointer to the message structure where received data is copied.
  *
  * @return   FALSE: When operation in basic mode: Transmit message time out, or when operation in normal mode: No useful interface. \n
  *           TRUE : Transmit Message success.
  */
int32_t CAN_Transmit(UINT32 uCAN, uint32_t u32MsgNum , STR_CANMSG_T* pCanMsg)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;

    if((inpw(REG_CAN0_CON+uOffset) & CAN_CON_TEST_Msk) && (inpw(REG_CAN0_TEST+uOffset) & CAN_TEST_BASIC_Msk))
    {
        return (CAN_BasicSendMsg(uCAN, pCanMsg));
    }
    else
    {
        if(CAN_SetTxMsg(uCAN, u32MsgNum, pCanMsg) == FALSE)
            return FALSE;
        CAN_TriggerTxMsg(uCAN, u32MsgNum);
    }

    return TRUE;
}


/**
  * @brief    Gets the message, if received.
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u32MsgNum   Specifies the Message object number, from 0 to 31
  * @param[out]    pCanMsg     Pointer to the message structure where received data is copied.
  *
  * @return   FALSE: No any message received. \n
  *           TRUE : Receive Message success.
  */
int32_t CAN_Receive(UINT32 uCAN, uint32_t u32MsgNum , STR_CANMSG_T* pCanMsg)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;

    if((inpw(REG_CAN0_CON+uOffset) & CAN_CON_TEST_Msk) && (inpw(REG_CAN0_TEST+uOffset) & CAN_TEST_BASIC_Msk))
    {
        return (CAN_BasicReceiveMsg(uCAN, pCanMsg));
    }
    else
    {
        return CAN_ReadMsgObj(uCAN, u32MsgNum, TRUE, pCanMsg);
    }
}

/**
  * @brief    Clear interrupt pending bit.
  * @param[in]    uCAN    CAN channel.
  * @param[in]    u32MsgNum   Specifies the Message object number, from 0 to 31
  *
  * @return   None
  *
  */
void CAN_CLR_INT_PENDING_BIT(UINT32 uCAN, uint8_t u32MsgNum)
{
    UINT32 uOffset = uCAN * CAN_OFFSET;

    uint32_t u32MsgIfNum = 0;
    uint32_t u32IFBusyCount = 0;

    while(u32IFBusyCount < 0x10000000)
    {
        if((inpw(REG_CAN0_IF1_CREQ+uOffset) & CAN_IF_CREQ_BUSY_Msk) == 0)
        {
            u32MsgIfNum = 0;
            break;
        }
        else if((inpw(REG_CAN0_IF2_CREQ+uOffset)  & CAN_IF_CREQ_BUSY_Msk) == 0)
        {
            u32MsgIfNum = 1;
            break;
        }

        u32IFBusyCount++;
    }

    if(u32MsgIfNum == 0)
    {
        outpw((REG_CAN0_IF1_CMASK+uOffset), (CAN_IF_CMASK_CLRINTPND_Msk | CAN_IF_CMASK_TXRQSTNEWDAT_Msk));
        outpw((REG_CAN0_IF1_CREQ+uOffset), (1 + u32MsgNum));
    }
    else if(u32MsgIfNum == 1)
    {
        outpw((REG_CAN0_IF2_CMASK+uOffset), (CAN_IF_CMASK_CLRINTPND_Msk | CAN_IF_CMASK_TXRQSTNEWDAT_Msk));
        outpw((REG_CAN0_IF2_CREQ+uOffset), (1 + u32MsgNum));
    }
}


/*@}*/ /* end of group NUC970_CAN_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC970_CAN_Driver */

/*@}*/ /* end of group NUC970_Device_Driver */


/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/


