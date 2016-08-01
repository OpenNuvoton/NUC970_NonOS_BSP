/**************************************************************************//**
* @file     kpi.h
* @version  V1.00
* $Revision: 1 $
* $Date: 15/05/22 2:59p $
* @brief    NUC970 KPI driver header file
*
* @note
* Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef _KPI_H_
#define _KPI_H_

#include "nuc970.h"

/** @addtogroup NUC970_Device_Driver NUC970 Device Driver
  @{
*/

/** @addtogroup NUC970_KPI_Driver KPI Driver
  @{
*/

/** @addtogroup NUC970_KPI_EXPORTED_CONSTANTS KPI Exported Constants
  @{
*/	
	

//Constants
#define NONBLOCK_MODE           0    /*!< KPI read non-block mode */
#define BLOCK_MODE              1    /*!< KPI read block mode */

//ioctl command table
#define SET_KPICONF         	0    /*!< Set register REG_KPI_CONF */
#define SET_KPI3KCONF       	1    /*!< Set register REG_KPI_3KCONF */
#define SET_KPIRSTC          	2    /*!< Set register REG_KPI_RSTC */
#define CLEAN_KPIKPE            3    /*!< Clearn register REG_KPI_KPE */
#define CLEAN_KPIKRE            4    /*!< Clearn register REG_KPI_KRE */
#define SET_PRESCALDIV          5    /*!< Set Pre-scale */
#define CLEAN_KPI_BUFFER    	6    /*!< Clearn KPI buffer  */
#define CONTINUOUS_MODE 		7    /*!< Set KPI continuous scan mode */

/*@}*/ /* end of group NUC970_KPI_EXPORTED_CONSTANTS */

/** @addtogroup NUC970_KPI_EXPORTED_STRUCTS KPI Exported Structs
  @{
*/

/** \brief  Structure type of REG_KPI_CONF register
 */
typedef union
{
    UINT value;   /*!< REG_KPI_CONF register value */
	struct
    {
        UINT enkp:1,        /*!< Keypad Scan Enable */
		     pkinten:1,     /*!< Press Key Interrupt Enable Control */
		     rkinten:1,     /*!< Release Key Interrupt Enable Control */
		     inten:1,       /*!< Key Interrupt Enable Control */
		     oden:1,        /*!< Open Drain Enable */
		     wakeup:1,      /*!< Wakeup Enable */
		     inpu:1,        /*!< Key Scan In Pull-UP Enable Register */
		     reserved_0:1,  /*!< Reserved */
             prescale:8,    /*!< Row Scan Cycle Pre-Scale Value */
		     dbclksel:4,    /*!< Scan In De-Bounce Sampling Cycle Selection */
		     reserved_1:1,  /*!< Reserved */
		     db_en:1,       /*!< Scan In Signal De-Bounce Enable */
		     reserved_2:2,  /*!< Reserved */
		     kcol:3,        /*!< Keypad Matrix COL Number */
		     reserved_3:1,  /*!< Reserved */
		     krow:2,        /*!< Keypad Matrix ROW Number */
		     reserved:2;    /*!< Reserved */
    }KPICONF_field;
}KPICONF_union;

/*@}*/ /* end of group NUC970_KPI_EXPORTED_STRUCTS */

#define ENKP       (kpiconf.KPICONF_field.enkp)       /*!< Keypad Scan Enable */
#define PKINTEN    (kpiconf.KPICONF_field.pkinten)    /*!< Press Key Interrupt Enable Control */
#define RKINTEN    (kpiconf.KPICONF_field.rkinten)    /*!< Release Key Interrupt Enable Control */
#define INTEN      (kpiconf.KPICONF_field.inten)      /*!< Key Interrupt Enable Control */
#define ODEN       (kpiconf.KPICONF_field.oden)       /*!< Open Drain Enable */
#define WAKEUP     (kpiconf.KPICONF_field.wakeup)     /*!< Wakeup Enable */
#define INPU       (kpiconf.KPICONF_field.inpu)       /*!< Key Scan In Pull-UP Enable Register */
#define PRESCALE   (kpiconf.KPICONF_field.prescale)   /*!< Row Scan Cycle Pre-Scale Value */
#define DBCLKSEL   (kpiconf.KPICONF_field.dbclksel)   /*!< Scan In De-Bounce Sampling Cycle Selection */
#define DB_EN      (kpiconf.KPICONF_field.db_en)      /*!< Scan In Signal De-Bounce Enable */
#define KCOL       (kpiconf.KPICONF_field.kcol)       /*!< Keypad Matrix COL Number */
#define KROW       (kpiconf.KPICONF_field.krow)       /*!< Keypad Matrix ROW Number */

typedef enum
{
	 ENKP_ENABLE         =0x1,   /*!< Keypad Scan Enable */
	 ENKP_DISABLE        =0x0    /*!< Keypad Scan Disable */
}INT_ENKP;

typedef enum
{
	 PKINTEN_ENABLE         =0x1,  /*!< Press key interrupt enable */
	 PKINTEN_DISABLE        =0x0   /*!< Press key interrupt disable */
}INT_PKINTEN;

typedef enum
{
	 RKINTEN_ENABLE         =0x1,   /*!< Release key interrupt enable */
	 RKINTEN_DISABLE        =0x0    /*!< Release key interrupt disable */
}INT_RKINTEN;

typedef enum
{
	 INTEN_ENABLE         =0x1,   /*!< Key interrupt enable */
	 INTEN_DISABLE        =0x0    /*!< Key interrupt disable */
}INT_INTEN;

typedef enum
{
	 ODEN_ENABLE         =0x1,  /*!< Open drain Enable */
	 ODEN_DISABLE        =0x0   /*!< Open drain disable */
}INT_ODEN;

typedef enum
{
	 WAKEUP_ENABLE         =0x1,  /*!< Wake-up Enable */
	 WAKEUP_DISABLE        =0x0   /*!< Wake-up disable */
}INT_WAKEUP;

typedef enum
{
	 INPU_ENABLE         =0x1,   /*!< Key Scan In Pull-UP Enable */
	 INPU_DISABLE        =0x0    /*!< Key Scan In Pull-UP disable */
}INT_INPU;

typedef enum
{
	 DB_EN_ENABLE         =0x1,   /*!< De-Bounce Enable */
	 DB_EN_DISABLE        =0x0    /*!< De-Bounce disable */
}INT_DB_EN;

/** @addtogroup NUC970_KPI_EXPORTED_STRUCTS KPI Exported Structs
  @{
*/

/** \brief  Structure type of REG_KPI_3KCONF register
 */
typedef union
{
    UINT value;   /*!< REG_KPI_3KCONF register value */
    struct
    {
        UINT k30c:3,            /*!< The key_0 Key Column Address */
		     k30r:2,            /*!< The key_0 Key Row Address */   
		     _reserved5:3,      /*!< Reserved */
             k31c:3,            /*!< The key_1 Key Column Address */
		     k31r:2,            /*!< The key_1 Key Row Address */   
		     _reserved13:3,     /*!< Reserved */
             k32c:3,            /*!< The key_2 Key Column Address */
		     k32r:2,            /*!< The key_2 Key Row Address */   
		     _reserved21:3,     /*!< Reserved */
             en3kyrst:1,        /*!< Enable Three-Key Reset */
		     _reserved25_31:7;  /*!< Reserved */
    }KPI3KCONF_field;
} KPI3KCONF_union;

/*@}*/ /* end of group NUC970_KPI_EXPORTED_STRUCTS */

#define K30C       (kpi3kconf.KPI3KCONF_field.k30c)        /*!< The key_0 Key Column Address */
#define K30R       (kpi3kconf.KPI3KCONF_field.k30r)        /*!< The key_0 Key Row Address */   
#define K31C       (kpi3kconf.KPI3KCONF_field.k31c)        /*!< The key_1 Key Column Address */
#define K31R       (kpi3kconf.KPI3KCONF_field.k31r)        /*!< The key_1 Key Row Address */   
#define K32C       (kpi3kconf.KPI3KCONF_field.k32c)        /*!< The key_2 Key Column Address */
#define K32R       (kpi3kconf.KPI3KCONF_field.k32r)        /*!< The key_2 Key Row Address */   
#define EN3KYRST   (kpi3kconf.KPI3KCONF_field.en3kyrst)    /*!< Enable Three-Key Reset */

#define EN3KYRST_ENABLE    0x1   /*!< Enable Three-Key Reset */
#define EN3KYRST_DISABLE   0x0   /*!< Disable Three-Key Reset */


/** @addtogroup NUC970_KPI_EXPORTED_STRUCTS KPI Exported Structs
  @{
*/

/** \brief  Structure type of REG_KPI_STATUS register
 */
typedef union
{
    UINT value;   /*!< REG_KPI_STATUS register value */
    struct
    {
        UINT pdwake:1,             /*!< Power Down Wakeup Flag */      
		     rst_3key:1,           /*!< 3-Keys Reset Flag */        
		     key_int:1,            /*!< Key Interrupt flag */         
		     rkey_int:1,           /*!< Release Key Interrupt flag */
		     pkey_int:1,           /*!< Press Key Interrupt flag */    
		     _reserved5:3,         /*!< Reserved */
             prow0:1,              /*!< Press Key Row 0 Coordinate */
		     prow1:1,              /*!< Press Key Row 1 Coordinate */
		     prow2:1,              /*!< Press Key Row 2 Coordinate */
		     prow3:1,              /*!< Press Key Row 3 Coordinate */
		     _reserved12:4,        /*!< Reserved */
             rrow0:1,              /*!< Release Key Row 0 Coordinate */
		     rrow1:1,              /*!< Release Key Row 1 Coordinate */
		     rrow2:1,              /*!< Release Key Row 2 Coordinate */
		     rrow3:1,              /*!< Release Key Row 3 Coordinate */
		     _reserved20_31:12;    /*!< Reserved */
    }KPISTATUS_field;
}KPISTATUS_union;

/*@}*/ /* end of group NUC970_KPI_EXPORTED_STRUCTS */

#define PDWAKE      (kpistatus.KPISTATUS_field.pdwake)        /*!< Power Down Wakeup Flag */    
#define RST_3KEY    (kpistatus.KPISTATUS_field.rst_3key)      /*!< 3-Keys Reset Flag */        
#define KEY_INT     (kpistatus.KPISTATUS_field.key_int)       /*!< Key Interrupt flag */        
#define RKEY_INT    (kpistatus.KPISTATUS_field.rkey_int)      /*!< Release Key Interrupt flag */
#define PKEY_INT    (kpistatus.KPISTATUS_field.pkey_int)      /*!< Press Key Interrupt flag */  
#define PROW0       (kpistatus.KPISTATUS_field.prow0)         /*!< Press Key Row 0 Coordinate */
#define PROW1       (kpistatus.KPISTATUS_field.prow1)         /*!< Press Key Row 1 Coordinate */
#define PROW2       (kpistatus.KPISTATUS_field.prow2)         /*!< Press Key Row 2 Coordinate */
#define PROW3       (kpistatus.KPISTATUS_field.prow3)         /*!< Press Key Row 3 Coordinate */
#define RROW0       (kpistatus.KPISTATUS_field.rrow0)         /*!< Release Key Row 0 Coordinate */
#define RROW1       (kpistatus.KPISTATUS_field.rrow1)         /*!< Release Key Row 1 Coordinate */
#define RROW2       (kpistatus.KPISTATUS_field.rrow2)         /*!< Release Key Row 2 Coordinate */
#define RROW3       (kpistatus.KPISTATUS_field.rrow3)         /*!< Release Key Row 3 Coordinate */

/** @addtogroup NUC970_KPI_EXPORTED_STRUCTS KPI Exported Structs
  @{
*/

/** \brief  Structure type of REG_KPI_RSTC register
 */
typedef union
{
    UINT value;   /*!< REG_KPI_RSTC register value */
    struct
    {
        UINT rstc:8,            /*!< 3-Key Reset Period Count */
		     _reserved9_31:24;  /*!< Reserved */
    }KPIRSTC_field;
}KPIRSTC_union;

/*@}*/ /* end of group NUC970_KPI_EXPORTED_STRUCTS */

#define RSTC     (kpirstc.KPIRSTC_field.rstc)   /*!< 3-Key Reset Period Count */

/** @addtogroup NUC970_KPI_EXPORTED_STRUCTS KPI Exported Structs
  @{
*/

/** \brief  Structure type of REG_KPI_KPE / REG_KPI_KRE register
 */
typedef union
{
    UINT value;  /*!< REG_KPI_KPE / REG_KPI_KRE register value */
    struct
    {
        UINT key00:1,  /*!< Press / Release Key Event Indicator */
		     key01:1,  /*!< Press / Release Key Event Indicator */
		     key02:1,  /*!< Press / Release Key Event Indicator */
		     key03:1,  /*!< Press / Release Key Event Indicator */
		     key04:1,  /*!< Press / Release Key Event Indicator */ 
		     key05:1,  /*!< Press / Release Key Event Indicator */
		     key06:1,  /*!< Press / Release Key Event Indicator */
		     key07:1,  /*!< Press / Release Key Event Indicator */
             key10:1,  /*!< Press / Release Key Event Indicator */
		     key11:1,  /*!< Press / Release Key Event Indicator */
		     key12:1,  /*!< Press / Release Key Event Indicator */
		     key13:1,  /*!< Press / Release Key Event Indicator */
		     key14:1,  /*!< Press / Release Key Event Indicator */
		     key15:1,  /*!< Press / Release Key Event Indicator */
		     key16:1,  /*!< Press / Release Key Event Indicator */
		     key17:1,  /*!< Press / Release Key Event Indicator */
             key20:1,  /*!< Press / Release Key Event Indicator */
		     key21:1,  /*!< Press / Release Key Event Indicator */
		     key22:1,  /*!< Press / Release Key Event Indicator */
		     key23:1,  /*!< Press / Release Key Event Indicator */
		     key24:1,  /*!< Press / Release Key Event Indicator */
			 key25:1,  /*!< Press / Release Key Event Indicator */
			 key26:1,  /*!< Press / Release Key Event Indicator */
			 key27:1,  /*!< Press / Release Key Event Indicator */
             key30:1,  /*!< Press / Release Key Event Indicator */
			 key31:1,  /*!< Press / Release Key Event Indicator */
			 key32:1,  /*!< Press / Release Key Event Indicator */
			 key33:1,  /*!< Press / Release Key Event Indicator */
			 key34:1,  /*!< Press / Release Key Event Indicator */
			 key35:1,  /*!< Press / Release Key Event Indicator */
			 key36:1,  /*!< Press / Release Key Event Indicator */
			 key37:1;  /*!< Press / Release Key Event Indicator */
    }KPIKEY_field;
}KPIKEY_union;

/*@}*/ /* end of group NUC970_KPI_EXPORTED_STRUCTS */

#define KEY00       (kpikey.KPIKEY_field.key00)  /*!< Press / Release Key Event Indicator */
#define KEY01       (kpikey.KPIKEY_field.key01)  /*!< Press / Release Key Event Indicator */
#define KEY02       (kpikey.KPIKEY_field.key02)  /*!< Press / Release Key Event Indicator */
#define KEY03       (kpikey.KPIKEY_field.key03)  /*!< Press / Release Key Event Indicator */
#define KEY04       (kpikey.KPIKEY_field.key04)  /*!< Press / Release Key Event Indicator */
#define KEY05       (kpikey.KPIKEY_field.key05)  /*!< Press / Release Key Event Indicator */
#define KEY06       (kpikey.KPIKEY_field.key06)  /*!< Press / Release Key Event Indicator */
#define KEY07       (kpikey.KPIKEY_field.key07)  /*!< Press / Release Key Event Indicator */
#define KEY10       (kpikey.KPIKEY_field.key00)  /*!< Press / Release Key Event Indicator */
#define KEY11       (kpikey.KPIKEY_field.key11)  /*!< Press / Release Key Event Indicator */
#define KEY12       (kpikey.KPIKEY_field.key12)  /*!< Press / Release Key Event Indicator */
#define KEY13       (kpikey.KPIKEY_field.key13)  /*!< Press / Release Key Event Indicator */
#define KEY14       (kpikey.KPIKEY_field.key14)  /*!< Press / Release Key Event Indicator */
#define KEY15       (kpikey.KPIKEY_field.key15)  /*!< Press / Release Key Event Indicator */
#define KEY16       (kpikey.KPIKEY_field.key16)  /*!< Press / Release Key Event Indicator */
#define KEY17       (kpikey.KPIKEY_field.key17)  /*!< Press / Release Key Event Indicator */
#define KEY20       (kpikey.KPIKEY_field.key20)  /*!< Press / Release Key Event Indicator */
#define KEY21       (kpikey.KPIKEY_field.key21)  /*!< Press / Release Key Event Indicator */
#define KEY22       (kpikey.KPIKEY_field.key22)  /*!< Press / Release Key Event Indicator */
#define KEY23       (kpikey.KPIKEY_field.key23)  /*!< Press / Release Key Event Indicator */
#define KEY24       (kpikey.KPIKEY_field.key24)  /*!< Press / Release Key Event Indicator */
#define KEY25       (kpikey.KPIKEY_field.key25)  /*!< Press / Release Key Event Indicator */
#define KEY26       (kpikey.KPIKEY_field.key26)  /*!< Press / Release Key Event Indicator */
#define KEY27       (kpikey.KPIKEY_field.key27)  /*!< Press / Release Key Event Indicator */
#define KEY30       (kpikey.KPIKEY_field.key30)  /*!< Press / Release Key Event Indicator */
#define KEY31       (kpikey.KPIKEY_field.key31)  /*!< Press / Release Key Event Indicator */
#define KEY32       (kpikey.KPIKEY_field.key32)  /*!< Press / Release Key Event Indicator */
#define KEY33       (kpikey.KPIKEY_field.key33)  /*!< Press / Release Key Event Indicator */
#define KEY34       (kpikey.KPIKEY_field.key34)  /*!< Press / Release Key Event Indicator */
#define KEY35       (kpikey.KPIKEY_field.key35)  /*!< Press / Release Key Event Indicator */
#define KEY36       (kpikey.KPIKEY_field.key36)  /*!< Press / Release Key Event Indicator */
#define KEY37       (kpikey.KPIKEY_field.key37)  /*!< Press / Release Key Event Indicator */

//KPI Error code
#define KEYPAD_ERR_ID  0xFFFF1400                       /*!< KPI error ID */
#define kpiReadModeError        (KEYPAD_ERR_ID|0x1)     /*!< KPI read mode set error */
#define kpiInvalidIoctlCommand  (KEYPAD_ERR_ID|0x2)     /*!< KPI invalid Ioctl command */
#define kpiNotOpen              (KEYPAD_ERR_ID|0x3)     /*!< KPI not open */
#define kpiBusy                 (KEYPAD_ERR_ID|0x4)     /*!< KPI busy */
#define kpiNoKey                (KEYPAD_ERR_ID|0x5)     /*!< KPI find no press/release key */

/** @addtogroup NUC970_KPI_EXPORTED_FUNCTIONS KPI Exported Functions
  @{
*/

// Function definition
extern INT KPI_Init(UINT8 u8Row, UINT8 u8Col, UINT8 u8Press, UINT8 u8Release);
extern INT KPI_Open(void);
extern INT KPI_Close(void);
extern INT KPI_Read(PUCHAR pucKeyStatus, PUCHAR pucKey, UINT uReadMode);
extern INT KPI_Ioctl(UINT uCommand, UINT uIndication, UINT uValue);

#endif

/*@}*/ /* end of group NUC970_KPI_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC970_KPI_Driver */

/*@}*/ /* end of group NUC970_Device_Driver */

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/

