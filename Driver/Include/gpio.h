/**************************************************************************//**
* @file     gpio.h
* @version  V1.00
* $Revision: 9 $
* $Date: 15/05/18 5:38p $
* @brief    NUC970 GPIO driver header file
*
* @note
* Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C"
{
#endif


/** @addtogroup NUC970_Device_Driver NUC970 Device Driver
  @{
*/

/** @addtogroup NUC970_GPIO_Driver GPIO Driver
  @{
*/

/** @addtogroup NUC970_GPIO_EXPORTED_CONSTANTS GPIO Exported Constants
  @{
*/

/*---------------------------------------------------------------------------------------------------------*/
/*  MODE Constant Definitions                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
/// @cond HIDDEN_SYMBOLS
#ifndef GPIO_ERR_PORT_BUSY
  #define GPIO_ERR_PORT_BUSY      -1
  #define GPIO_ERR_UNSUPPORTED    -2
  #define GPIO_ERR_BIT_BUSY       -3
  #define SUCCESSFUL              0
#endif
/// @endcond HIDDEN_SYMBOLS

#define MAX_PORT 10  /*!< GPIO Port Number */

#define GPIOA_MASK  0x0000FFFF  /*!< GPIO Port A Mask */
#define GPIOB_MASK  0x0000FFFF  /*!< GPIO Port B Mask */
#define GPIOC_MASK  0x00007FFF  /*!< GPIO Port C Mask */
#define GPIOD_MASK  0x0000FFFF  /*!< GPIO Port D Mask */
#define GPIOE_MASK  0x0000FFFF  /*!< GPIO Port E Mask */
#define GPIOF_MASK  0x0000FFFF  /*!< GPIO Port F Mask */
#define GPIOG_MASK  0x0000FFFF  /*!< GPIO Port G Mask */
#define GPIOH_MASK  0x0000FFFF  /*!< GPIO Port H Mask */
#define GPIOI_MASK  0x0000FFFF  /*!< GPIO Port I Mask */
#define GPIOJ_MASK  0x0000003F  /*!< GPIO Port J Mask */

/// @cond HIDDEN_SYMBOLS
typedef INT32 (*GPIO_CALLBACK)(UINT32 status, UINT32 userData);
typedef INT32 (*EINT_CALLBACK)(UINT32 status, UINT32 userData);
/// @endcond HIDDEN_SYMBOLS

/** \brief  Structure type of GPIO_PORT
 */
typedef enum {
    GPIOA=0x000,   /*!< Port A offset of GPIO base address      */
    GPIOB=0x040,   /*!< Port B offset of GPIO base address      */
    GPIOC=0x080,   /*!< Port C offset of GPIO base address      */
    GPIOD=0x0C0,   /*!< Port D offset of GPIO base address      */
    GPIOE=0x100,   /*!< Port E offset of GPIO base address      */
    GPIOF=0x140,   /*!< Port F offset of GPIO base address      */
    GPIOG=0x180,   /*!< Port G offset of GPIO base address      */
    GPIOH=0x1C0,   /*!< Port H offset of GPIO base address      */
    GPIOI=0x200,   /*!< Port I offset of GPIO base address      */
    GPIOJ=0x240,   /*!< Port J offset of GPIO base address      */
} GPIO_PORT;

/** \brief  Structure type of GPIO_DIR
 */
typedef enum {
DIR_INPUT,   /*!< GPIO Output mode      */
DIR_OUTPUT   /*!< GPIO Input mode      */
} GPIO_DIR;

/** \brief  Structure type of GPIO_PULL
 */
typedef enum {
  NO_PULL_UP, /*!< GPIO Pull-Up Disable */
  PULL_UP     /*!< GPIO Pull-Up Enable */
} GPIO_PULL;

/** \brief  Structure type of GPIO_DRV
 */
typedef enum {
  DRV_LOW,   /*!< GPIO Set to Low */
  DRV_HIGH   /*!< GPIO Set to High */
} GPIO_DRV;

/** \brief  Structure type of GPIO_NIRQ
 */
typedef enum {
  NIRQ0=0,   /*!< External interrupt 0 */
  NIRQ1,     /*!< External interrupt 1 */
  NIRQ2,     /*!< External interrupt 2 */
  NIRQ3,     /*!< External interrupt 3 */
  NIRQ4,     /*!< External interrupt 4 */
  NIRQ5,     /*!< External interrupt 5 */
  NIRQ6,     /*!< External interrupt 6 */
  NIRQ7,     /*!< External interrupt 7 */
} GPIO_NIRQ;

/** \brief  Structure type of GPIO_TRIGGER_TYPE
 */
typedef enum {
  LOW,                   /*!< Trigger type set low */
  HIGH,                  /*!< Trigger type set high */
  FALLING,               /*!< Trigger type set falling edge */
  RISING,                /*!< Trigger type set rising edge */
  BOTH_EDGE              /*!< Trigger type set falling edge and rising edge */
} GPIO_TRIGGER_TYPE;

/// @cond HIDDEN_SYMBOLS
typedef struct 
{
    UINT16          bitBusyFlag[MAX_PORT];
    GPIO_CALLBACK   IRQCallback[MAX_PORT];
    UINT32          IRQUserData[MAX_PORT];
    EINT_CALLBACK   EINTIRQCallback[8];
    UINT32          EINTIRQUserData[8];
} GPIO_CFG;
/// @endcond HIDDEN_SYMBOLS

/*@}*/ /* end of group NUC970_GPIO_EXPORTED_CONSTANTS */


/** @addtogroup NUC970_GPIO_EXPORTED_FUNCTIONS GPIO Exported Functions
  @{
*/

INT32 GPIO_Open(GPIO_PORT port, GPIO_DIR direction, GPIO_PULL pull);
INT32 GPIO_Close(GPIO_PORT port);
INT32 GPIO_Set(GPIO_PORT port, UINT32 bitMap);
INT32 GPIO_Clr(GPIO_PORT port, UINT32 bitMap);
UINT32 GPIO_ReadPort(GPIO_PORT port);
INT32 GPIO_SetPortDir(GPIO_PORT port, GPIO_DIR direction);

/* General GPIO bit function */
INT32 GPIO_OpenBit(GPIO_PORT port, UINT32 bit, GPIO_DIR direction, GPIO_PULL pull);
INT32 GPIO_CloseBit(GPIO_PORT port, UINT32 bit);
INT32 GPIO_SetBit(GPIO_PORT port, UINT32 bit);
INT32 GPIO_ClrBit(GPIO_PORT port, UINT32 bit);
INT32 GPIO_ReadBit(GPIO_PORT port, UINT32 bit);
INT32 GPIO_SetBitDir(GPIO_PORT port, UINT32 bit, GPIO_DIR direction);
INT32 GPIO_EnableTriggerType(GPIO_PORT port, UINT32 bit, GPIO_TRIGGER_TYPE triggerType);
INT32 GPIO_DisableTriggerType(GPIO_PORT port, UINT32 bit);
INT32 GPIO_EnableInt(GPIO_PORT port, GPIO_CALLBACK callback, UINT32 userData);
INT32 GPIO_DisableInt(GPIO_PORT port);
INT32 GPIO_ClrISR(GPIO_PORT port, UINT32 bit);
INT32 GPIO_ClrISRBit(GPIO_PORT port, UINT32 bit);

/* GPIO status function */
BOOL GPIO_BitIsUsed(GPIO_PORT port, UINT32 bit);

/* External GPIO interrupt function */
INT32 GPIO_EnableEINT(GPIO_NIRQ nIrq, GPIO_CALLBACK callback, UINT32 userData);
INT32 GPIO_DisableEINT(GPIO_NIRQ nIrq);
INT32 GPIO_EnableDebounce(INT32 debounceClkSel);
INT32 GPIO_DisableDebounce(void);

/*@}*/ /* end of group NUC970_GPIO_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC970_GPIO_Driver */

/*@}*/ /* end of group NUC970_Device_Driver */

#ifdef __cplusplus
}
#endif

#endif //__GPIO_H__

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
