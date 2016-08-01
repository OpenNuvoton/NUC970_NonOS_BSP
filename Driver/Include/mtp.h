/**************************************************************************//**
 * @file     mtp.h
 * @version  V1.10
 * $Revision: 3 $
 * $Date: 15/06/12 9:48a $
 * @brief    Cryptographic Accelerator driver header file
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __MTP_H__
#define __MTP_H__

#include "nuc970.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** @addtogroup NUC970_Device_Driver NUC970 Device Driver
  @{
*/

/** @addtogroup NUC970_MTP_Driver MTP Driver
  @{
*/

/// @cond HIDDEN_SYMBOLS

//#define MTP_DEBUG

#ifdef MTP_DEBUG
#define mtp_dbg		sysprintf
#else
#define mtp_dbg(...)
#endif

typedef struct
{
    volatile UINT32  KEYEN;             /*!< Offset: 0x0000   MTP Key Enable                        */ 
    volatile UINT32  Reserved1[2];      /*!< Offset: 0x0004 ~ 0x0008  Reserved                      */
    volatile UINT32  USERDATA;          /*!< Offset: 0x000C   MTP user data (IBR option)            */ 
    volatile UINT32  KEY[8];            /*!< Offset: 0x0010 ~ 0x002C  MTP Key Value 0~7             */ 
    volatile UINT32  PCYCLE;            /*!< Offset: 0x0030   MTP Program Cycle Control register    */ 
    volatile UINT32  CTL;               /*!< Offset: 0x0034   MTP Control register                  */ 
    volatile UINT32  PSTART;            /*!< Offset: 0x0038   MTP Program Start register            */ 
    volatile UINT32  Reserved2;         /*!< Offset: 0x003C   Reserved                              */
    volatile UINT32  STATUS;            /*!< Offset: 0x0040   MTP Status register                   */ 
    volatile UINT32  Reserved3[3];      /*!< Offset: 0x0044 ~ 0x004C   Reserved                     */
    volatile UINT32  REGLCTL;           /*!< Offset: 0x0050   MTP Register Write-Protection Control Register  */ 
}   MTP_TypeDef;

#define MTP                         ((MTP_TypeDef *) 0xB800C000)

#define MTP_KEY_PROG_COUNT          ((MTP->STATUS >> 16) & 0xf)
#define POOLING_LOOP			    0x100000

/// @endcond HIDDEN_SYMBOLS



/** @addtogroup NUC970_MTP_EXPORTED_CONSTANTS MTP Exported Constants
  @{
*/

/*----------------------------------------------------*/
/*  Function return code                              */
/*----------------------------------------------------*/
#define MTP_RET_OK              0       /*!< No error.                                */
#define MTP_RET_ERR_ENABLE     -1       /*!< Failed to enable MTP key.                */
#define MTP_RET_ERR_PRG_CNT    -2       /*!< MTP key program count reachs limit.      */
#define MTP_RET_ERR_LOCK       -3       /*!< MTP key is locked.                       */
#define MTP_RET_ERR_REG_LOCK   -4       /*!< Failed to unlock MTP controller.         */
#define MTP_RET_ERR_PRG_FAIL   -5       /*!< Failed to program MTP key.               */
#define MTP_RET_ERR_NO_KEY     -6       /*!< MTP is not programmed with any key yet.  */

/*----------------------------------------------------*/
/*  MTP controller register bit fields                */
/*----------------------------------------------------*/

/* MTP_KEYEN  */
#define MTP_KEYEN_KEYEN             ((UINT32)0x00000001)    /*!< MTP Key Enable                 \hideinitializer */

/* MTP_CTL    */
#define MTP_CTL_MODE_MASK           ((UINT32)0x00000003)    /*!< MTP_CTL MODE control bits mask \hideinitializer */
#define MTP_CLT_MODE_IDLE           ((UINT32)0x00000000)    /*!< MTP_CTL MTP idle mode          \hideinitializer */
#define MTP_CLT_MODE_PROG           ((UINT32)0x00000002)    /*!< MTP_CTL program key mode       \hideinitializer */
#define MTP_CTL_MODE_LOCK           ((UINT32)0x00000003)    /*!< MTP lock key mode              \hideinitializer */

/* MTP_PSTART */
#define MTP_PSTART_PSTART           ((UINT32)0x00000001)    /*!< MTP start program              \hideinitializer */

/* MTP_STATUS */
#define MTP_STATUS_MTPEN            ((UINT32)0x00000001)    /*!< MTP enable status              \hideinitializer */
#define MTP_STATUS_KEYVALID         ((UINT32)0x00000002)    /*!< MTP key valid status           \hideinitializer */
#define MTP_STATUS_NONPRG           ((UINT32)0x00000004)    /*!< MTP non-program status         \hideinitializer */
#define MTP_STATUS_LOCKED           ((UINT32)0x00000008)    /*!< MTP key lock status            \hideinitializer */
#define MTP_STATUS_PRGFAIL          ((UINT32)0x00000010)    /*!< MTP program fail status        \hideinitializer */
#define MTP_STATUS_BUSY             ((UINT32)0x01000000)    /*!< MTP busy status                \hideinitializer */


/*@}*/ /* end of group NUC970_MTP_EXPORTED_CONSTANTS */


/** @addtogroup NUC970_MTP_EXPORTED_FUNCTIONS MTP Exported Functions
  @{
*/

/*---------------------------------------------------------------------------------------------------------*/
/*  Functions                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/

int  MTP_Enable(void);
int  MTP_Program(UINT32 key[8], UINT8 option);
int  MTP_Lock(void);
int  MTP_IsKeyLocked(void);
int  MTP_GetPrgCount(void);
int  MTP_GetUserData(UINT8 *user_data);


/*@}*/ /* end of group NUC970_MTP_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC970_MTP_Driver */

/*@}*/ /* end of group NUC970_Device_Driver */

#ifdef __cplusplus
}
#endif

#endif  // __MTP_H__

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/

