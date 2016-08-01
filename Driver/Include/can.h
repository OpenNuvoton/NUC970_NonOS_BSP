/**************************************************************************//**
* @file     can.h
* @version  V1.00
* $Revision: 1 $
* $Date: 15/06/11 2:44p $
* @brief    NUC970 CAN driver header file
*
* @note
* Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __CAN_H__
#define __CAN_H__

#include "nuc970.h"

#ifdef  __cplusplus
extern "C"
{
#endif

/** @addtogroup NUC970_Device_Driver NUC970 Device Driver
  @{
*/

/** @addtogroup NUC970_CAN_Driver CAN Driver
  @{
*/

/** @addtogroup NUC970_CAN_EXPORTED_CONSTANTS CAN Exported Constants
  @{
*/


#define CAN_CON_TEST_Pos           7                                    /*!< CAN CON: TEST Position */
#define CAN_CON_TEST_Msk           (1ul << CAN_CON_TEST_Pos)            /*!< CAN CON: TEST Mask     */

#define CAN_CON_CCE_Pos            6                                    /*!< CAN CON: CCE Position  */
#define CAN_CON_CCE_Msk            (1ul << CAN_CON_CCE_Pos)             /*!< CAN CON: CCE Mask      */

#define CAN_CON_DAR_Pos            5                                    /*!< CAN CON: DAR Position  */
#define CAN_CON_DAR_Msk            (1ul << CAN_CON_DAR_Pos)             /*!< CAN CON: DAR Mask      */

#define CAN_CON_EIE_Pos            3                                    /*!< CAN CON: EIE Position  */
#define CAN_CON_EIE_Msk            (1ul << CAN_CON_EIE_Pos)             /*!< CAN CON: EIE Mask      */

#define CAN_CON_SIE_Pos            2                                    /*!< CAN CON: SIE Position  */
#define CAN_CON_SIE_Msk            (1ul << CAN_CON_SIE_Pos)             /*!< CAN CON: SIE Mask      */

#define CAN_CON_IE_Pos             1                                    /*!< CAN CON: IE Position   */
#define CAN_CON_IE_Msk             (1ul << CAN_CON_IE_Pos)              /*!< CAN CON: IE Mask       */

#define CAN_CON_INIT_Pos           0                                    /*!< CAN CON: INIT Position */
#define CAN_CON_INIT_Msk           (1ul << CAN_CON_INIT_Pos)            /*!< CAN CON: INIT Mask     */

#define CAN_STATUS_BOFF_Pos        7                                    /*!< CAN STATUS: BOFF Position  */
#define CAN_STATUS_BOFF_Msk        (1ul << CAN_STATUS_BOFF_Pos)         /*!< CAN STATUS: BOFF Mask      */

#define CAN_STATUS_EWARN_Pos       6                                    /*!< CAN STATUS: EWARN Position */
#define CAN_STATUS_EWARN_Msk       (1ul << CAN_STATUS_EWARN_Pos)        /*!< CAN STATUS: EWARN Mask     */

#define CAN_STATUS_EPASS_Pos       5                                    /*!< CAN STATUS: EPASS Position */
#define CAN_STATUS_EPASS_Msk       (1ul << CAN_STATUS_EPASS_Pos)        /*!< CAN STATUS: EPASS Mask     */

#define CAN_STATUS_RXOK_Pos        4                                    /*!< CAN STATUS: RXOK Position  */
#define CAN_STATUS_RXOK_Msk        (1ul << CAN_STATUS_RXOK_Pos)         /*!< CAN STATUS: RXOK Mask      */

#define CAN_STATUS_TXOK_Pos        3                                    /*!< CAN STATUS: TXOK Position  */
#define CAN_STATUS_TXOK_Msk        (1ul << CAN_STATUS_TXOK_Pos)         /*!< CAN STATUS: TXOK Mask      */

#define CAN_STATUS_LEC_Pos         0                                    /*!< CAN STATUS: LEC Position   */
#define CAN_STATUS_LEC_Msk         (0x3ul << CAN_STATUS_LEC_Pos)        /*!< CAN STATUS: LEC Mask       */

#define CAN_ERR_RP_Pos             15                                   /*!< CAN ERR: RP Position       */
#define CAN_ERR_RP_Msk             (1ul << CAN_ERR_RP_Pos)              /*!< CAN ERR: RP Mask           */

#define CAN_ERR_REC_Pos            8                                    /*!< CAN ERR: REC Position      */
#define CAN_ERR_REC_Msk            (0x7Ful << CAN_ERR_REC_Pos)          /*!< CAN ERR: REC Mask          */

#define CAN_ERR_TEC_Pos            0                                    /*!< CAN ERR: TEC Position      */
#define CAN_ERR_TEC_Msk            (0xFFul << CAN_ERR_TEC_Pos)          /*!< CAN ERR: TEC Mask          */

#define CAN_BTIME_TSEG2_Pos        12                                   /*!< CAN BTIME: TSEG2 Position  */
#define CAN_BTIME_TSEG2_Msk        (0x7ul << CAN_BTIME_TSEG2_Pos)       /*!< CAN BTIME: TSEG2 Mask      */

#define CAN_BTIME_TSEG1_Pos        8                                    /*!< CAN BTIME: TSEG1 Position  */
#define CAN_BTIME_TSEG1_Msk        (0xFul << CAN_BTIME_TSEG1_Pos)       /*!< CAN BTIME: TSEG1 Mask      */

#define CAN_BTIME_SJW_Pos          6                                    /*!< CAN BTIME: SJW Position    */
#define CAN_BTIME_SJW_Msk          (0x3ul << CAN_BTIME_SJW_Pos)         /*!< CAN BTIME: SJW Mask        */

#define CAN_BTIME_BRP_Pos          0                                    /*!< CAN BTIME: BRP Position    */
#define CAN_BTIME_BRP_Msk          (0x3Ful << CAN_BTIME_BRP_Pos)        /*!< CAN BTIME: BRP Mask        */

#define CAN_IIDR_INTID_Pos         0                                    /*!< CAN IIDR: INTID Position   */
#define CAN_IIDR_INTID_Msk         (0xFFFFul << CAN_IIDR_INTID_Pos)     /*!< CAN IIDR: INTID Mask       */

#define CAN_TEST_RX_Pos            7                                    /*!< CAN TEST: RX Position      */
#define CAN_TEST_RX_Msk            (1ul << CAN_TEST_RX_Pos)             /*!< CAN TEST: RX Mask          */

#define CAN_TEST_TX_Pos            5                                    /*!< CAN TEST: TX Position      */
#define CAN_TEST_TX_Msk            (0x3ul << CAN_TEST_TX_Pos)           /*!< CAN TEST: TX Mask          */

#define CAN_TEST_LBACK_Pos         4                                    /*!< CAN TEST: LBACK Position   */
#define CAN_TEST_LBACK_Msk         (1ul << CAN_TEST_LBACK_Pos)          /*!< CAN TEST: LBACK Mask       */

#define CAN_TEST_SILENT_Pos        3                                    /*!< CAN TEST: Silent Position  */
#define CAN_TEST_SILENT_Msk        (1ul << CAN_TEST_SILENT_Pos)         /*!< CAN TEST: Silent Mask      */

#define CAN_TEST_BASIC_Pos         2                                    /*!< CAN TEST: Basic Position   */
#define CAN_TEST_BASIC_Msk         (1ul << CAN_TEST_BASIC_Pos)          /*!< CAN TEST: Basic Mask       */

#define CAN_BRPE_BRPE_Pos          0                                    /*!< CAN BRPE: BRPE Position    */
#define CAN_BRPE_BRPE_Msk          (0xFul << CAN_BRPE_BRPE_Pos)         /*!< CAN BRPE: BRPE Mask        */

#define CAN_IF_CREQ_BUSY_Pos       15                                   /*!< CAN IFnCREQ: BUSY Position */
#define CAN_IF_CREQ_BUSY_Msk       (1ul << CAN_IF_CREQ_BUSY_Pos)        /*!< CAN IFnCREQ: BUSY Mask     */

#define CAN_IF_CREQ_MSGNUM_Pos     0                                    /*!< CAN IFnCREQ: MSGNUM Position */
#define CAN_IF_CREQ_MSGNUM_Msk     (0x3Ful << CAN_IF_CREQ_MSGNUM_Pos)   /*!< CAN IFnCREQ: MSGNUM Mask     */

#define CAN_IF_CMASK_WRRD_Pos      7                                    /*!< CAN IFnCMASK: WRRD Position */
#define CAN_IF_CMASK_WRRD_Msk      (1ul << CAN_IF_CMASK_WRRD_Pos)       /*!< CAN IFnCMASK: WRRD Mask     */

#define CAN_IF_CMASK_MASK_Pos      6                                    /*!< CAN IFnCMASK: MASK Position */
#define CAN_IF_CMASK_MASK_Msk      (1ul << CAN_IF_CMASK_MASK_Pos)       /*!< CAN IFnCMASK: MASK Mask     */

#define CAN_IF_CMASK_ARB_Pos       5                                    /*!< CAN IFnCMASK: ARB Position  */
#define CAN_IF_CMASK_ARB_Msk       (1ul << CAN_IF_CMASK_ARB_Pos)        /*!< CAN IFnCMASK: ARB Mask      */

#define CAN_IF_CMASK_CONTROL_Pos   4                                    /*!< CAN IFnCMASK: CONTROL Position */
#define CAN_IF_CMASK_CONTROL_Msk   (1ul << CAN_IF_CMASK_CONTROL_Pos)    /*!< CAN IFnCMASK: CONTROL Mask */

#define CAN_IF_CMASK_CLRINTPND_Pos 3                                    /*!< CAN IFnCMASK: CLRINTPND Position */
#define CAN_IF_CMASK_CLRINTPND_Msk (1ul << CAN_IF_CMASK_CLRINTPND_Pos)  /*!< CAN IFnCMASK: CLRINTPND Mask */

#define CAN_IF_CMASK_TXRQSTNEWDAT_Pos 2                                         /*!< CAN IFnCMASK: TXRQSTNEWDAT Position */
#define CAN_IF_CMASK_TXRQSTNEWDAT_Msk (1ul << CAN_IF_CMASK_TXRQSTNEWDAT_Pos)    /*!< CAN IFnCMASK: TXRQSTNEWDAT Mask     */

#define CAN_IF_CMASK_DATAA_Pos     1                                    /*!< CAN IFnCMASK: DATAA Position */
#define CAN_IF_CMASK_DATAA_Msk     (1ul << CAN_IF_CMASK_DATAA_Pos)      /*!< CAN IFnCMASK: DATAA Mask     */

#define CAN_IF_CMASK_DATAB_Pos     0                                    /*!< CAN IFnCMASK: DATAB Position */
#define CAN_IF_CMASK_DATAB_Msk     (1ul << CAN_IF_CMASK_DATAB_Pos)      /*!< CAN IFnCMASK: DATAB Mask     */

#define CAN_IF_MASK1_MSK_Pos       0                                    /*!< CAN IFnMASK1: MSK Position   */
#define CAN_IF_MASK1_MSK_Msk       (0xFFul << CAN_IF_MASK1_MSK_Pos)     /*!< CAN IFnMASK1: MSK Mask       */

#define CAN_IF_MASK2_MXTD_Pos      15                                   /*!< CAN IFnMASK2: MXTD Position */
#define CAN_IF_MASK2_MXTD_Msk      (1ul << CAN_IF_MASK2_MXTD_Pos)       /*!< CAN IFnMASK2: MXTD Mask     */

#define CAN_IF_MASK2_MDIR_Pos      14                                   /*!< CAN IFnMASK2: MDIR Position */
#define CAN_IF_MASK2_MDIR_Msk      (1ul << CAN_IF_MASK2_MDIR_Pos)       /*!< CAN IFnMASK2: MDIR Mask     */

#define CAN_IF_MASK2_MSK_Pos       0                                    /*!< CAN IFnMASK2: MSK Position */
#define CAN_IF_MASK2_MSK_Msk       (0x1FFul << CAN_IF_MASK2_MSK_Pos)    /*!< CAN IFnMASK2: MSK Mask     */

#define CAN_IF_ARB1_ID_Pos         0                                    /*!< CAN IFnARB1: ID Position   */
#define CAN_IF_ARB1_ID_Msk         (0xFFFFul << CAN_IF_ARB1_ID_Pos)     /*!< CAN IFnARB1: ID Mask       */

#define CAN_IF_ARB2_MSGVAL_Pos     15                                   /*!< CAN IFnARB2: MSGVAL Position */
#define CAN_IF_ARB2_MSGVAL_Msk     (1ul << CAN_IF_ARB2_MSGVAL_Pos)      /*!< CAN IFnARB2: MSGVAL Mask     */

#define CAN_IF_ARB2_XTD_Pos        14                                   /*!< CAN IFnARB2: XTD Position    */
#define CAN_IF_ARB2_XTD_Msk        (1ul << CAN_IF_ARB2_XTD_Pos)         /*!< CAN IFnARB2: XTD Mask        */

#define CAN_IF_ARB2_DIR_Pos        13                                   /*!< CAN IFnARB2: DIR Position    */
#define CAN_IF_ARB2_DIR_Msk        (1ul << CAN_IF_ARB2_DIR_Pos)         /*!< CAN IFnARB2: DIR Mask        */

#define CAN_IF_ARB2_ID_Pos         0                                    /*!< CAN IFnARB2: ID Position     */
#define CAN_IF_ARB2_ID_Msk         (0x1FFFul << CAN_IF_ARB2_ID_Pos)     /*!< CAN IFnARB2: ID Mask         */

#define CAN_IF_MCON_NEWDAT_Pos     15                                   /*!< CAN IFnMCON: NEWDAT Position */
#define CAN_IF_MCON_NEWDAT_Msk     (1ul << CAN_IF_MCON_NEWDAT_Pos)      /*!< CAN IFnMCON: NEWDAT Mask     */

#define CAN_IF_MCON_MSGLST_Pos     14                                   /*!< CAN IFnMCON: MSGLST Position */
#define CAN_IF_MCON_MSGLST_Msk     (1ul << CAN_IF_MCON_MSGLST_Pos)      /*!< CAN IFnMCON: MSGLST Mask     */

#define CAN_IF_MCON_INTPND_Pos     13                                   /*!< CAN IFnMCON: INTPND Position */
#define CAN_IF_MCON_INTPND_Msk     (1ul << CAN_IF_MCON_INTPND_Pos)      /*!< CAN IFnMCON: INTPND Mask     */

#define CAN_IF_MCON_UMASK_Pos      12                                   /*!< CAN IFnMCON: UMASK Position  */
#define CAN_IF_MCON_UMASK_Msk      (1ul << CAN_IF_MCON_UMASK_Pos)       /*!< CAN IFnMCON: UMASK Mask      */

#define CAN_IF_MCON_TXIE_Pos       11                                   /*!< CAN IFnMCON: TXIE Position   */
#define CAN_IF_MCON_TXIE_Msk       (1ul << CAN_IF_MCON_TXIE_Pos)        /*!< CAN IFnMCON: TXIE Mask       */

#define CAN_IF_MCON_RXIE_Pos       10                                   /*!< CAN IFnMCON: RXIE Position   */
#define CAN_IF_MCON_RXIE_Msk       (1ul << CAN_IF_MCON_RXIE_Pos)        /*!< CAN IFnMCON: RXIE Mask       */

#define CAN_IF_MCON_RMTEN_Pos      9                                    /*!< CAN IFnMCON: RMTEN Position  */
#define CAN_IF_MCON_RMTEN_Msk      (1ul << CAN_IF_MCON_RMTEN_Pos)       /*!< CAN IFnMCON: RMTEN Mask      */

#define CAN_IF_MCON_TXRQST_Pos     8                                    /*!< CAN IFnMCON: TXRQST Position */
#define CAN_IF_MCON_TXRQST_Msk     (1ul << CAN_IF_MCON_TXRQST_Pos)      /*!< CAN IFnMCON: TXRQST Mask     */

#define CAN_IF_MCON_EOB_Pos        7                                    /*!< CAN IFnMCON: EOB Position    */
#define CAN_IF_MCON_EOB_Msk        (1ul << CAN_IF_MCON_EOB_Pos)         /*!< CAN IFnMCON: EOB Mask        */

#define CAN_IF_MCON_DLC_Pos        0                                    /*!< CAN IFnMCON: DLC Position    */
#define CAN_IF_MCON_DLC_Msk        (0xFul << CAN_IF_MCON_DLC_Pos)       /*!< CAN IFnMCON: DLC Mask        */

#define CAN_IF_DAT_A1_DATA1_Pos    8                                    /*!< CAN IFnDATAA1: DATA1 Position */
#define CAN_IF_DAT_A1_DATA1_Msk    (0xFFul << CAN_IF_DAT_A1_DATA1_Pos)  /*!< CAN IFnDATAA1: DATA1 Mask     */

#define CAN_IF_DAT_A1_DATA0_Pos    0                                    /*!< CAN IFnDATAA1: DATA0 Position */
#define CAN_IF_DAT_A1_DATA0_Msk    (0xFFul << CAN_IF_DAT_A1_DATA0_Pos)  /*!< CAN IFnDATAA1: DATA0 Mask     */

#define CAN_IF_DAT_A2_DATA3_Pos    8                                    /*!< CAN IFnDATAA1: DATA3 Position */
#define CAN_IF_DAT_A2_DATA3_Msk    (0xFFul << CAN_IF_DAT_A2_DATA3_Pos)  /*!< CAN IFnDATAA1: DATA3 Mask     */

#define CAN_IF_DAT_A2_DATA2_Pos    0                                    /*!< CAN IFnDATAA1: DATA2 Position */
#define CAN_IF_DAT_A2_DATA2_Msk    (0xFFul << CAN_IF_DAT_A2_DATA2_Pos)  /*!< CAN IFnDATAA1: DATA2 Mask     */

#define CAN_IF_DAT_B1_DATA5_Pos    8                                    /*!< CAN IFnDATAB1: DATA5 Position */
#define CAN_IF_DAT_B1_DATA5_Msk    (0xFFul << CAN_IF_DAT_B1_DATA5_Pos)  /*!< CAN IFnDATAB1: DATA5 Mask */

#define CAN_IF_DAT_B1_DATA4_Pos    0                                    /*!< CAN IFnDATAB1: DATA4 Position */
#define CAN_IF_DAT_B1_DATA4_Msk    (0xFFul << CAN_IF_DAT_B1_DATA4_Pos)  /*!< CAN IFnDATAB1: DATA4 Mask */

#define CAN_IF_DAT_B2_DATA7_Pos    8                                    /*!< CAN IFnDATAB2: DATA7 Position */
#define CAN_IF_DAT_B2_DATA7_Msk    (0xFFul << CAN_IF_DAT_B2_DATA7_Pos)  /*!< CAN IFnDATAB2: DATA7 Mask     */

#define CAN_IF_DAT_B2_DATA6_Pos    0                                    /*!< CAN IFnDATAB2: DATA6 Position */
#define CAN_IF_DAT_B2_DATA6_Msk    (0xFFul << CAN_IF_DAT_B2_DATA6_Pos)  /*!< CAN IFnDATAB2: DATA6 Mask     */

#define CAN_IF_TXRQST1_TXRQST_Pos  0                                        /*!< CAN IFnTXRQST1: TXRQST Position */
#define CAN_IF_TXRQST1_TXRQST_Msk  (0xFFFFul << CAN_IF_TXRQST1_TXRQST_Pos)  /*!< CAN IFnTXRQST1: TXRQST Mask     */

#define CAN_IF_TXRQST2_TXRQST_Pos  0                                        /*!< CAN IFnTXRQST2: TXRQST Position  */
#define CAN_IF_TXRQST2_TXRQST_Msk  (0xFFFFul << CAN_IF_TXRQST2_TXRQST_Pos)  /*!< CAN IFnTXRQST2: TXRQST Mask      */

#define CAN_IF_NDAT1_NEWDATA_Pos   0                                        /*!< CAN IFnNDAT1: NEWDATA Position */
#define CAN_IF_NDAT1_NEWDATA_Msk   (0xFFFFul << CAN_IF_NDAT1_NEWDATA_Pos)   /*!< CAN IFnNDAT1: NEWDATA Mask     */

#define CAN_IF_NDAT2_NEWDATA_Pos   0                                        /*!< CAN IFnNDAT2: NEWDATA Position */
#define CAN_IF_NDAT2_NEWDATA_Msk   (0xFFFFul << CAN_IF_NDAT2_NEWDATA_Pos)   /*!< CAN IFnNDAT2: NEWDATA Mask     */

#define CAN_IF_IPND1_INTPND_Pos   0                                         /*!< CAN IFnIPND1: INTPND Position */
#define CAN_IF_IPND1_INTPND_Msk   (0xFFFFul << CAN_IF_IPND1_INTPND_Pos)     /*!< CAN IFnIPND1: INTPND Mask     */

#define CAN_IF_IPND2_INTPND_Pos   0                                         /*!< CAN IFnIPND2: INTPND Position */
#define CAN_IF_IPND2_INTPND_Msk   (0xFFFFul << CAN_IF_IPND2_INTPND_Pos)     /*!< CAN IFnIPND2: INTPND Mask     */

#define CAN_IF_MVLD1_MSGVAL_Pos   0                                         /*!< CAN IFnMVLD1: MSGVAL Position */
#define CAN_IF_MVLD1_MSGVAL_Msk   (0xFFFFul << CAN_IF_MVLD1_MSGVAL_Pos)     /*!< CAN IFnMVLD1: MSGVAL Mask     */

#define CAN_IF_MVLD2_MSGVAL_Pos   0                                         /*!< CAN IFnMVLD2: MSGVAL Position */
#define CAN_IF_MVLD2_MSGVAL_Msk   (0xFFFFul << CAN_IF_MVLD2_MSGVAL_Pos)     /*!< CAN IFnMVLD2: MSGVAL Mask     */

#define CAN_WUEN_WAKUP_EN_Pos     0                                         /*!< CAN WUEN: WAKUP_EN Position */
#define CAN_WUEN_WAKUP_EN_Msk    (1ul << CAN_WUEN_WAKUP_EN_Pos)             /*!< CAN WUEN: WAKUP_EN Mask     */

#define CAN_WUSTATUS_WAKUP_STS_Pos     0                                    /*!< CAN WUSTATUS: WAKUP_STS Position */
#define CAN_WUSTATUS_WAKUP_STS_Msk    (1ul << CAN_WUSTATUS_WAKUP_STS_Pos)   /*!< CAN WUSTATUS: WAKUP_STS Mask     */

/// @cond HIDDEN_SYMBOLS
#define CAN_OFFSET 0x400
/// @endcond HIDDEN_SYMBOLS
#define CAN0                 0  /*!< CAN0 channel */
#define CAN1                 1	/*!< CAN1 channel */
	
/** \brief  Message ID types
 */
typedef enum {
    CAN_STD_ID = 0, /*!< Standard Identifier  */
    CAN_EXT_ID = 1  /*!< Extended Identifier  */
} E_CAN_ID_TYPE;

/** \brief  Message Frame types
 */
typedef enum {
    REMOTE_FRAME = 0,  /*!< Remote Frame  */
    DATA_FRAME   = 1   /*!< Data Frame    */
} E_CAN_FRAME_TYPE;

/** \brief  CAN message structure
 */
typedef struct {
    uint32_t  IdType;     /*!< Identifier Type     */
    uint32_t  FrameType;  /*!< Frame Type          */
    uint32_t  Id;         /*!< Message Identifier  */
    uint8_t   DLC;        /*!< Data Length Code    */
    uint8_t   Data[8];    /*!< Data byte 0 ~ 7     */
} STR_CANMSG_T;

/** \brief  CAN mask message structure
 */
typedef struct {
    uint8_t   u8Xtd;     /*!< Extended Identifier  */
    uint8_t   u8Dir;     /*!< Message Direction    */
    uint32_t  u32Id;     /*!< Message Identifier   */
    uint8_t   u8IdType;  /*!< Identifier Type      */
} STR_CANMASK_T;

/** \brief  CAN operation mode: normal/basic mode
 */
typedef enum {
    CAN_NORMAL_MODE = 1, /*!< Normal Mode  */
    CAN_BASIC_MODE = 2   /*!< Basic Mode   */
} CAN_MODE_SELECT;

#define ALL_MSG  32  /*!< All Message ram number   */
#define MSG(id)  id  /*!< Message ram number       */


/*@}*/ /* end of group NUC970_CAN_EXPORTED_CONSTANTS */


/** @addtogroup NUC970_CAN_EXPORTED_FUNCTIONS CAN Exported Functions
  @{
*/

/**
 *  @brief    Get interrupt status
 *
 *  @param[in]    can  CAN channel
 *
 *  @return   CAN module status register value
 *  \hideinitializer
 */
#define CAN_GET_INT_STATUS(can)    inpw(REG_CAN0_STATUS+(can*CAN_OFFSET))  //(can->STATUS)

/**
 *  @brief    Get specified interrupt pending status
 *
 *  @param[in]    can  CAN channel
 *
 *  @return   The source of the interrupt.
 *  \hideinitializer 
 */
#define CAN_GET_INT_PENDING_STATUS(can)     inpw(REG_CAN0_IIDR+(can*CAN_OFFSET))  //(can->IIDR)

/**
 *  @brief    Disable Wakeup function
 *
 *  @param[in]    can  CAN channel
 *
 *  @return   None
 * \hideinitializer 
 */
#define CAN_DISABLE_WAKEUP(can)             outpw((REG_CAN0_WU_EN+(can*CAN_OFFSET)), 0x0)  //(can->WU_IE = 0)

/**
 *  @brief    Enable Wakeup function
 *
 *  @param[in]    can  CAN channel
 *
 *  @return   None
 * \hideinitializer 
 */
#define CAN_ENABLE_WAKEUP(can)              outpw((REG_CAN0_WU_EN+(can*CAN_OFFSET)), CAN_WUEN_WAKUP_EN_Msk)  //(can->WU_IE = CAN_WUEN_WAKUP_EN_Msk)

/**
 *  @brief    Get specified Message Object new data into bit value
 *
 *  @param[in]    can  CAN channel
 *  @param[in]    u32MsgNum  Specified Message Object number. (0 ~ 31)
 *
 *  @return   Specified Message Object new data into bit value.
 * \hideinitializer 
 */
#define CAN_GET_NEW_DATA_IN_BIT(can, u32MsgNum)    (u32MsgNum < 16 ? inpw(REG_CAN0_NDAT1+(can*CAN_OFFSET)) & (1 << u32MsgNum) : inpw(REG_CAN0_NDAT2+(can*CAN_OFFSET)) & (1 << (u32MsgNum-16))) 


/*---------------------------------------------------------------------------------------------------------*/
/* Define CAN functions prototype                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
uint32_t CAN_SetBaudRate(UINT32 uCAN, uint32_t u32BaudRate);
uint32_t CAN_Open(UINT32 uCAN, uint32_t u32BaudRate, uint32_t u32Mode);
int32_t CAN_Transmit(UINT32 uCAN, uint32_t u32MsgNum , STR_CANMSG_T* pCanMsg);
int32_t CAN_Receive(UINT32 uCAN, uint32_t u32MsgNum , STR_CANMSG_T* pCanMsg);
void CAN_CLR_INT_PENDING_BIT(UINT32 uCAN, uint8_t u32MsgNum);
void CAN_EnableInt(UINT32 uCAN, uint32_t u32Mask);
void CAN_DisableInt(UINT32 uCAN, uint32_t u32Mask);
int32_t CAN_SetMultiRxMsg(UINT32 uCAN, uint32_t u32MsgNum , uint32_t u32MsgCount, uint32_t u32IDType, uint32_t u32ID);
int32_t CAN_SetRxMsg(UINT32 uCAN, uint32_t u32MsgNum , uint32_t u32IDType, uint32_t u32ID);
int32_t CAN_SetTxMsg(UINT32 uCAN, uint32_t u32MsgNum , STR_CANMSG_T* pCanMsg);
int32_t CAN_TriggerTxMsg(UINT32 uCAN, uint32_t u32MsgNum);

/*@}*/ /* end of group NUC970_CAN_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC970_CAN_Driver */

/*@}*/ /* end of group NUC970_Device_Driver */

#ifdef __cplusplus
} 
#endif

#endif //__CAN_H__

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/





