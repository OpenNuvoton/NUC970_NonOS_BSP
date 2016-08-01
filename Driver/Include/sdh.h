/**************************************************************************//**
 * @file     sdh.h
 * @version  V1.00
 * $Revision: 3 $
 * $Date: 15/06/12 10:02a $
 * @brief    NUC970 SDH driver header file
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>

#ifndef __SDH_H__
#define __SDH_H__

/** @addtogroup NUC970_Device_Driver NUC970 Device Driver
  @{
*/

/** @addtogroup NUC970_SDH_Driver SDH Driver
  @{
*/


/** @addtogroup NUC970_SDH_EXPORTED_CONSTANTS SDH Exported Constants
  @{
*/

/**
    @addtogroup SDH_CONST SDH Bit Field Definition
    Constant Definitions for SDH Controller
@{ */

#define SDH_DMACTL_DMAEN_Pos             (0)                                               /*!< SDH DMACTL: DMAEN Position             */
#define SDH_DMACTL_DMAEN_Msk             (0x1ul << SDH_DMACTL_DMAEN_Pos)                   /*!< SDH DMACTL: DMAEN Mask                 */

#define SDH_DMACTL_DMARST_Pos            (1)                                               /*!< SDH DMACTL: DMARST Position            */
#define SDH_DMACTL_DMARST_Msk            (0x1ul << SDH_DMACTL_DMARST_Pos)                  /*!< SDH DMACTL: DMARST Mask                */

#define SDH_DMACTL_SGEN_Pos              (3)                                               /*!< SDH DMACTL: SGEN Position              */
#define SDH_DMACTL_SGEN_Msk              (0x1ul << SDH_DMACTL_SGEN_Pos)                    /*!< SDH DMACTL: SGEN Mask                  */

#define SDH_DMACTL_DMABUSY_Pos           (9)                                               /*!< SDH DMACTL: DMABUSY Position           */
#define SDH_DMACTL_DMABUSY_Msk           (0x1ul << SDH_DMACTL_DMABUSY_Pos)                 /*!< SDH DMACTL: DMABUSY Mask               */

#define SDH_DMASA_ORDER_Pos              (0)                                               /*!< SDH DMASA: ORDER Position              */
#define SDH_DMASA_ORDER_Msk              (0x1ul << SDH_DMASA_ORDER_Pos)                    /*!< SDH DMASA: ORDER Mask                  */

#define SDH_DMASA_DMASA_Pos              (1)                                               /*!< SDH DMASA: DMASA Position              */
#define SDH_DMASA_DMASA_Msk              (0x7ffffffful << SDH_DMASA_DMASA_Pos)             /*!< SDH DMASA: DMASA Mask                  */

#define SDH_DMABCNT_BCNT_Pos             (0)                                               /*!< SDH DMABCNT: BCNT Position             */
#define SDH_DMABCNT_BCNT_Msk             (0x3fffffful << SDH_DMABCNT_BCNT_Pos)             /*!< SDH DMABCNT: BCNT Mask                 */

#define SDH_DMAINTEN_ABORTIEN_Pos        (0)                                               /*!< SDH DMAINTEN: ABORTIEN Position        */
#define SDH_DMAINTEN_ABORTIEN_Msk        (0x1ul << SDH_DMAINTEN_ABORTIEN_Pos)              /*!< SDH DMAINTEN: ABORTIEN Mask            */

#define SDH_DMAINTEN_WEOTIEN_Pos         (1)                                               /*!< SDH DMAINTEN: WEOTIEN Position         */
#define SDH_DMAINTEN_WEOTIEN_Msk         (0x1ul << SDH_DMAINTEN_WEOTIEN_Pos)               /*!< SDH DMAINTEN: WEOTIEN Mask             */

#define SDH_DMAINTSTS_ABORTIF_Pos        (0)                                               /*!< SDH DMAINTSTS: ABORTIF Position        */
#define SDH_DMAINTSTS_ABORTIF_Msk        (0x1ul << SDH_DMAINTSTS_ABORTIF_Pos)              /*!< SDH DMAINTSTS: ABORTIF Mask            */

#define SDH_DMAINTSTS_WEOTIF_Pos         (1)                                               /*!< SDH DMAINTSTS: WEOTIF Position         */
#define SDH_DMAINTSTS_WEOTIF_Msk         (0x1ul << SDH_DMAINTSTS_WEOTIF_Pos)               /*!< SDH DMAINTSTS: WEOTIF Mask             */

#define SDH_GCTL_GCTLRST_Pos             (0)                                               /*!< SDH GCTL: GCTLRST Position             */
#define SDH_GCTL_GCTLRST_Msk             (0x1ul << SDH_GCTL_GCTLRST_Pos)                   /*!< SDH GCTL: GCTLRST Mask                 */

#define SDH_GCTL_SDEN_Pos                (1)                                               /*!< SDH GCTL: SDEN Position                */
#define SDH_GCTL_SDEN_Msk                (0x1ul << SDH_GCTL_SDEN_Pos)                      /*!< SDH GCTL: SDEN Mask                    */

#define SDH_GINTEN_DTAIEN_Pos            (0)                                               /*!< SDH GINTEN: DTAIEN Position            */
#define SDH_GINTEN_DTAIEN_Msk            (0x1ul << SDH_GINTEN_DTAIEN_Pos)                  /*!< SDH GINTEN: DTAIEN Mask                */

#define SDH_GINTSTS_DTAIF_Pos            (0)                                               /*!< SDH GINTSTS: DTAIF Position            */
#define SDH_GINTSTS_DTAIF_Msk            (0x1ul << SDH_GINTSTS_DTAIF_Pos)                  /*!< SDH GINTSTS: DTAIF Mask                */

#define SDH_CTL_COEN_Pos                 (0)                                               /*!< SDH CTL: COEN Position                 */
#define SDH_CTL_COEN_Msk                 (0x1ul << SDH_CTL_COEN_Pos)                       /*!< SDH CTL: COEN Mask                     */

#define SDH_CTL_RIEN_Pos                 (1)                                               /*!< SDH CTL: RIEN Position                 */
#define SDH_CTL_RIEN_Msk                 (0x1ul << SDH_CTL_RIEN_Pos)                       /*!< SDH CTL: RIEN Mask                     */

#define SDH_CTL_DIEN_Pos                 (2)                                               /*!< SDH CTL: DIEN Position                 */
#define SDH_CTL_DIEN_Msk                 (0x1ul << SDH_CTL_DIEN_Pos)                       /*!< SDH CTL: DIEN Mask                     */

#define SDH_CTL_DOEN_Pos                 (3)                                               /*!< SDH CTL: DOEN Position                 */
#define SDH_CTL_DOEN_Msk                 (0x1ul << SDH_CTL_DOEN_Pos)                       /*!< SDH CTL: DOEN Mask                     */

#define SDH_CTL_R2EN_Pos                 (4)                                               /*!< SDH CTL: R2EN Position                 */
#define SDH_CTL_R2EN_Msk                 (0x1ul << SDH_CTL_R2EN_Pos)                       /*!< SDH CTL: R2EN Mask                     */

#define SDH_CTL_CLK74OEN_Pos             (5)                                               /*!< SDH CTL: CLK74OEN Position             */
#define SDH_CTL_CLK74OEN_Msk             (0x1ul << SDH_CTL_CLK74OEN_Pos)                   /*!< SDH CTL: CLK74OEN Mask                 */

#define SDH_CTL_CLK8OEN_Pos              (6)                                               /*!< SDH CTL: CLK8OEN Position              */
#define SDH_CTL_CLK8OEN_Msk              (0x1ul << SDH_CTL_CLK8OEN_Pos)                    /*!< SDH CTL: CLK8OEN Mask                  */

#define SDH_CTL_CLKKEEP0_Pos             (7)                                               /*!< SDH CTL: CLKKEEP0 Position             */
#define SDH_CTL_CLKKEEP0_Msk             (0x1ul << SDH_CTL_CLKKEEP0_Pos)                   /*!< SDH CTL: CLKKEEP0 Mask                 */

#define SDH_CTL_CMDCODE_Pos              (8)                                               /*!< SDH CTL: CMDCODE Position              */
#define SDH_CTL_CMDCODE_Msk              (0x3ful << SDH_CTL_CMDCODE_Pos)                   /*!< SDH CTL: CMDCODE Mask                  */

#define SDH_CTL_CTLRST_Pos               (14)                                              /*!< SDH CTL: CTLRST Position               */
#define SDH_CTL_CTLRST_Msk               (0x1ul << SDH_CTL_CTLRST_Pos)                     /*!< SDH CTL: CTLRST Mask                   */

#define SDH_CTL_DBW_Pos                  (15)                                              /*!< SDH CTL: DBW Position                  */
#define SDH_CTL_DBW_Msk                  (0x1ul << SDH_CTL_DBW_Pos)                        /*!< SDH CTL: DBW Mask                      */

#define SDH_CTL_BLKCNT_Pos               (16)                                              /*!< SDH CTL: BLKCNT Position               */
#define SDH_CTL_BLKCNT_Msk               (0xfful << SDH_CTL_BLKCNT_Pos)                    /*!< SDH CTL: BLKCNT Mask                   */

#define SDH_CTL_SDNWR_Pos                (24)                                              /*!< SDH CTL: SDNWR Position                */
#define SDH_CTL_SDNWR_Msk                (0xful << SDH_CTL_SDNWR_Pos)                      /*!< SDH CTL: SDNWR Mask                    */

#define SDH_CTL_SDPORT_Pos               (29)                                              /*!< SDH CTL: SDPORT Position               */
#define SDH_CTL_SDPORT_Msk               (0x3ul << SDH_CTL_SDPORT_Pos)                     /*!< SDH CTL: SDPORT Mask                   */

#define SDH_CTL_CLKKEEP1_Pos             (31)                                              /*!< SDH CTL: CLKKEEP1 Position             */
#define SDH_CTL_CLKKEEP1_Msk             (0x1ul << SDH_CTL_CLKKEEP1_Pos)                   /*!< SDH CTL: CLKKEEP1 Mask                 */

#define SDH_CMDARG_ARGUMENT_Pos          (0)                                               /*!< SDH CMDARG: ARGUMENT Position          */
#define SDH_CMDARG_ARGUMENT_Msk          (0xfffffffful << SDH_CMDARG_ARGUMENT_Pos)         /*!< SDH CMDARG: ARGUMENT Mask              */

#define SDH_INTEN_BLKDIEN_Pos            (0)                                               /*!< SDH INTEN: BLKDIEN Position            */
#define SDH_INTEN_BLKDIEN_Msk            (0x1ul << SDH_INTEN_BLKDIEN_Pos)                  /*!< SDH INTEN: BLKDIEN Mask                */

#define SDH_INTEN_CRCIEN_Pos             (1)                                               /*!< SDH INTEN: CRCIEN Position             */
#define SDH_INTEN_CRCIEN_Msk             (0x1ul << SDH_INTEN_CRCIEN_Pos)                   /*!< SDH INTEN: CRCIEN Mask                 */

#define SDH_INTEN_CDIEN0_Pos             (8)                                               /*!< SDH INTEN: CDIEN0 Position             */
#define SDH_INTEN_CDIEN0_Msk             (0x1ul << SDH_INTEN_CDIEN0_Pos)                   /*!< SDH INTEN: CDIEN0 Mask                 */

#define SDH_INTEN_CDIEN1_Pos             (9)                                               /*!< SDH INTEN: CDIEN1 Position             */
#define SDH_INTEN_CDIEN1_Msk             (0x1ul << SDH_INTEN_CDIEN1_Pos)                   /*!< SDH INTEN: CDIEN1 Mask                 */

#define SDH_INTEN_SDHOST0IEN_Pos         (10)                                              /*!< SDH INTSTS: SDHOST0IEN Position        */
#define SDH_INTEN_SDHOST0IEN_Msk         (0x1ul << SDH_INTEN_SDHOST0IEN_Pos)               /*!< SDH INTSTS: SDHOST0IEN Mask            */

#define SDH_INTEN_SDHOST1IEN_Pos         (11)                                              /*!< SDH INTSTS: SDHOST1IEN Position        */
#define SDH_INTEN_SDHOST1IEN_Msk         (0x1ul << SDH_INTEN_SDHOST1IEN_Pos)               /*!< SDH INTSTS: SDHOST1IEN Mask            */

#define SDH_INTEN_RTOIEN_Pos             (12)                                              /*!< SDH INTEN: RTOIEN Position             */
#define SDH_INTEN_RTOIEN_Msk             (0x1ul << SDH_INTEN_RTOIEN_Pos)                   /*!< SDH INTEN: RTOIEN Mask                 */

#define SDH_INTEN_DITOIEN_Pos            (13)                                              /*!< SDH INTEN: DITOIEN Position            */
#define SDH_INTEN_DITOIEN_Msk            (0x1ul << SDH_INTEN_DITOIEN_Pos)                  /*!< SDH INTEN: DITOIEN Mask                */

#define SDH_INTEN_WKIEN_Pos              (14)                                              /*!< SDH INTEN: WKIEN Position              */
#define SDH_INTEN_WKIEN_Msk              (0x1ul << SDH_INTEN_WKIEN_Pos)                    /*!< SDH INTEN: WKIEN Mask                  */

#define SDH_INTEN_CDSRC0_Pos             (30)                                              /*!< SDH INTEN: CDSRC0 Position             */
#define SDH_INTEN_CDSRC0_Msk             (0x1ul << SDH_INTEN_CDSRC0_Pos)                   /*!< SDH INTEN: CDSRC0 Mask                 */

#define SDH_INTEN_CDSRC1_Pos             (31)                                              /*!< SDH INTEN: CDSRC1 Position             */
#define SDH_INTEN_CDSRC1_Msk             (0x1ul << SDH_INTEN_CDSRC1_Pos)                   /*!< SDH INTEN: CDSRC1 Mask                 */

#define SDH_INTSTS_BLKDIF_Pos            (0)                                               /*!< SDH INTSTS: BLKDIF Position            */
#define SDH_INTSTS_BLKDIF_Msk            (0x1ul << SDH_INTSTS_BLKDIF_Pos)                  /*!< SDH INTSTS: BLKDIF Mask                */

#define SDH_INTSTS_CRCIF_Pos             (1)                                               /*!< SDH INTSTS: CRCIF Position             */
#define SDH_INTSTS_CRCIF_Msk             (0x1ul << SDH_INTSTS_CRCIF_Pos)                   /*!< SDH INTSTS: CRCIF Mask                 */

#define SDH_INTSTS_CRC7_Pos              (2)                                               /*!< SDH INTSTS: CRC7 Position              */
#define SDH_INTSTS_CRC7_Msk              (0x1ul << SDH_INTSTS_CRC7_Pos)                    /*!< SDH INTSTS: CRC7 Mask                  */

#define SDH_INTSTS_CRC16_Pos             (3)                                               /*!< SDH INTSTS: CRC16 Position             */
#define SDH_INTSTS_CRC16_Msk             (0x1ul << SDH_INTSTS_CRC16_Pos)                   /*!< SDH INTSTS: CRC16 Mask                 */

#define SDH_INTSTS_CRCSTS_Pos            (4)                                               /*!< SDH INTSTS: CRCSTS Position            */
#define SDH_INTSTS_CRCSTS_Msk            (0x7ul << SDH_INTSTS_CRCSTS_Pos)                  /*!< SDH INTSTS: CRCSTS Mask                */

#define SDH_INTSTS_DAT0STS_Pos           (7)                                               /*!< SDH INTSTS: DAT0STS Position           */
#define SDH_INTSTS_DAT0STS_Msk           (0x1ul << SDH_INTSTS_DAT0STS_Pos)                 /*!< SDH INTSTS: DAT0STS Mask               */

#define SDH_INTSTS_CDIF0_Pos             (8)                                               /*!< SDH INTSTS: CDIF0 Position             */
#define SDH_INTSTS_CDIF0_Msk             (0x1ul << SDH_INTSTS_CDIF0_Pos)                   /*!< SDH INTSTS: CDIF0 Mask                 */

#define SDH_INTSTS_CDIF1_Pos             (9)                                               /*!< SDH INTSTS: CDIF1 Position             */
#define SDH_INTSTS_CDIF1_Msk             (0x1ul << SDH_INTSTS_CDIF1_Pos)                   /*!< SDH INTSTS: CDIF1 Mask                 */

#define SDH_INTSTS_SDHOST0IF_Pos         (10)                                              /*!< SDH INTSTS: SDHOST0IF Position         */
#define SDH_INTSTS_SDHOST0IF_Msk         (0x1ul << SDH_INTSTS_SDHOST0IF_Pos)               /*!< SDH INTSTS: SDHOST0IF Mask             */

#define SDH_INTSTS_SDHOST1IF_Pos         (11)                                              /*!< SDH INTSTS: SDHOST1IF Position         */
#define SDH_INTSTS_SDHOST1IF_Msk         (0x1ul << SDH_INTSTS_SDHOST1IF_Pos)               /*!< SDH INTSTS: SDHOST1IF Mask             */

#define SDH_INTSTS_RTOIF_Pos             (12)                                              /*!< SDH INTSTS: RTOIF Position             */
#define SDH_INTSTS_RTOIF_Msk             (0x1ul << SDH_INTSTS_RTOIF_Pos)                   /*!< SDH INTSTS: RTOIF Mask                 */

#define SDH_INTSTS_DINTOIF_Pos           (13)                                              /*!< SDH INTSTS: DINTOIF Position           */
#define SDH_INTSTS_DINTOIF_Msk           (0x1ul << SDH_INTSTS_DINTOIF_Pos)                 /*!< SDH INTSTS: DINTOIF Mask               */

#define SDH_INTSTS_CDSTS0_Pos            (16)                                              /*!< SDH INTSTS: CDSTS0 Position            */
#define SDH_INTSTS_CDSTS0_Msk            (0x1ul << SDH_INTSTS_CDSTS0_Pos)                  /*!< SDH INTSTS: CDSTS0 Mask                */

#define SDH_INTSTS_CDSTS1_Pos            (17)                                              /*!< SDH INTSTS: CDSTS1 Position            */
#define SDH_INTSTS_CDSTS1_Msk            (0x1ul << SDH_INTSTS_CDSTS1_Pos)                  /*!< SDH INTSTS: CDSTS1 Mask                */

#define SDH_INTSTS_DAT1STS_Pos           (18)                                              /*!< SDH INTSTS: DAT1STS Position           */
#define SDH_INTSTS_DAT1STS_Msk           (0x1ul << SDH_INTSTS_DAT1STS_Pos)                 /*!< SDH INTSTS: DAT1STS Mask               */

#define SDH_RESP0_RESPTK0_Pos            (0)                                               /*!< SDH RESP0: RESPTK0 Position            */
#define SDH_RESP0_RESPTK0_Msk            (0xfffffffful << SDH_RESP0_RESPTK0_Pos)           /*!< SDH RESP0: RESPTK0 Mask                */

#define SDH_RESP1_RESPTK1_Pos            (0)                                               /*!< SDH RESP1: RESPTK1 Position            */
#define SDH_RESP1_RESPTK1_Msk            (0xfful << SDH_RESP1_RESPTK1_Pos)                 /*!< SDH RESP1: RESPTK1 Mask                */

#define SDH_BLEN_BLKLEN_Pos              (0)                                               /*!< SDH BLEN: BLKLEN Position              */
#define SDH_BLEN_BLKLEN_Msk              (0x7fful << SDH_BLEN_BLKLEN_Pos)                  /*!< SDH BLEN: BLKLEN Mask                  */

#define SDH_TOUT_TOUT_Pos                (0)                                               /*!< SDH TOUT: TOUT Position                */
#define SDH_TOUT_TOUT_Msk                (0xfffffful << SDH_TOUT_TOUT_Pos)                 /*!< SDH TOUT: TOUT Mask                    */

/**@}*/ /* SDH_CONST */


//--- define type of SD card or MMC
#define SD_TYPE_UNKNOWN     0           /*!< Card Type - Unknoen \hideinitializer */
#define SD_TYPE_SD_HIGH     1           /*!< Card Type - SDH     \hideinitializer */
#define SD_TYPE_SD_LOW      2           /*!< Card Type - SD      \hideinitializer */
#define SD_TYPE_MMC         3           /*!< Card Type - MMC     \hideinitializer */
#define SD_TYPE_EMMC        4           /*!< Card Type - eMMC    \hideinitializer */

#define SD_ERR_ID           0xFFFF0100          /*!< SDH Error ID          \hideinitializer */
#define SD_TIMEOUT          (SD_ERR_ID|0x01)    /*!< SDH Error - Timeout   \hideinitializer */
#define SD_NO_MEMORY        (SD_ERR_ID|0x02)    /*!< SDH Error - No Memory \hideinitializer */
/* SD error */
#define SD_NO_SD_CARD       (SD_ERR_ID|0x10)    /*!< SDH Error - No card   \hideinitializer */
#define SD_ERR_DEVICE       (SD_ERR_ID|0x11)    /*!< SDH Error - device err \hideinitializer */
#define SD_INIT_TIMEOUT     (SD_ERR_ID|0x12)    /*!< SDH Error - init timeout \hideinitializer */
#define SD_SELECT_ERROR     (SD_ERR_ID|0x13)    /*!< SDH Error - select err \hideinitializer */
#define SD_WRITE_PROTECT    (SD_ERR_ID|0x14)    /*!< SDH Error - write protect \hideinitializer */
#define SD_INIT_ERROR       (SD_ERR_ID|0x15)    /*!< SDH Error - init err \hideinitializer */
#define SD_CRC7_ERROR       (SD_ERR_ID|0x16)    /*!< SDH Error - crc7 err \hideinitializer */
#define SD_CRC16_ERROR      (SD_ERR_ID|0x17)    /*!< SDH Error - crc16 err \hideinitializer */
#define SD_CRC_ERROR        (SD_ERR_ID|0x18)    /*!< SDH Error - crc err \hideinitializer */
#define SD_CMD8_ERROR       (SD_ERR_ID|0x19)    /*!< SDH Error - CMD8 err \hideinitializer */

#define SD_FREQ         25000   /*!< output 25MHz to SD  \hideinitializer */
#define SDHC_FREQ       50000   /*!< output 50MHz to SDH \hideinitializer */

#define SD_PORT0        (1 << 0)  /*!< Card select SD0 \hideinitializer */
#define SD_PORT1        (1 << 2)  /*!< Card select SD1 \hideinitializer */

#define CardDetect_From_GPIO  (1 << 8)   /*!< Card detection pin is GPIO \hideinitializer */
#define CardDetect_From_DAT3  (1 << 9)   /*!< Card detection pin is DAT3 \hideinitializer */

/*@}*/ /* end of group NUC970_SDH_EXPORTED_CONSTANTS */

/** @addtogroup NUC970_SDH_EXPORTED_TYPEDEF SDH Exported Type Defines
  @{
*/

/** \brief  Structure type of inserted Card information.
 */
typedef struct SD_info_t {
    unsigned int    CardType;       /*!< SDHC, SD, or MMC */
    unsigned int    RCA;            /*!< relative card address */
    unsigned char   IsCardInsert;   /*!< card insert state */
    unsigned int    totalSectorN;   /*!< total sector number */
    unsigned int    diskSize;       /*!< disk size in Kbytes */
    int             sectorSize;     /*!< sector size in bytes */
} SD_INFO_T;

/*@}*/ /* end of group NUC970_SDH_EXPORTED_TYPEDEF */

/// @cond HIDDEN_SYMBOLS
extern SD_INFO_T SD0;
extern SD_INFO_T SD1;
extern unsigned char volatile _sd_SDDataReady;

/// @endcond HIDDEN_SYMBOLS

/** @addtogroup NUC970_SDH_EXPORTED_FUNCTIONS SDH Exported Functions
  @{
*/


/**
 *  @brief    Enable specified interrupt.
 *
 *  @param[in]    u32IntMask    Interrupt type mask:
 *                           \ref SDH_INTEN_BLKDIEN_Msk / \ref SDH_INTEN_CRCIEN_Msk / \ref SDH_INTEN_CDIEN0_Msk / \ref SDH_INTEN_CDIEN1_Msk /
 *                           \ref SDH_INTEN_CDSRC0_Msk / \ref SDH_INTEN_CDSRC1_Msk / \ref SDH_INTEN_RTOIEN_Msk / \ref SDH_INTEN_DITOIEN_Msk /
 *                           \ref SDH_INTEN_WKIEN_Msk
 *
 *  @return   None.
 * \hideinitializer
 */
#define SD_ENABLE_INT(u32IntMask)    (outpw(REG_SDH_INTEN, inpw(REG_SDH_INTEN)|(u32IntMask)))

/**
 *  @brief    Disable specified interrupt.
 *
 *  @param[in]    u32IntMask    Interrupt type mask:
 *                           \ref SDH_INTEN_BLKDIEN_Msk / \ref SDH_INTEN_CRCIEN_Msk / \ref SDH_INTEN_CDIEN0_Msk / \ref SDH_INTEN_CDIEN1_Msk /
 *                           \ref SDH_INTEN_SDHOST0IEN_Msk / \ref SDH_INTEN_SDHOST1IEN_Msk / \ref SDH_INTEN_RTOIEN_Msk / \ref SDH_INTEN_DITOIEN_Msk /
 *                           \ref SDH_INTEN_WKIEN_Msk / \ref SDH_INTEN_CDSRC0_Msk / \ref SDH_INTEN_CDSRC1_Msk
 *
 *  @return   None.
 * \hideinitializer
 */
#define SD_DISABLE_INT(u32IntMask)    (outpw(REG_SDH_INTEN, inpw(REG_SDH_INTEN) & ~(u32IntMask)))

/**
 *  @brief    Get specified interrupt flag/status.
 *
 *  @param[in]    u32IntMask    Interrupt type mask:
 *                           \ref SDH_INTSTS_BLKDIF_Msk / \ref SDH_INTSTS_CRCIF_Msk / \ref SDH_INTSTS_CRC7_Msk /
 *                           \ref SDH_INTSTS_CRC16_Msk / \ref SDH_INTSTS_CRCSTS_Msk / \ref SDH_INTSTS_DAT0STS_Msk / \ref SDH_INTSTS_CDIF0_Msk /
 *                           \ref SDH_INTSTS_CDIF1_Msk / \ref SDH_INTSTS_SDHOST0IF_Msk / \ref SDH_INTSTS_SDHOST1IF_Msk / \ref SDH_INTSTS_RTOIF_Msk /
 *                           \ref SDH_INTSTS_DINTOIF_Msk / \ref SDH_INTSTS_CDSTS0_Msk / \ref SDH_INTSTS_CDSTS1_Msk / \ref SDH_INTSTS_DAT1STS_Msk
 *
 *
 *  @return  0 = The specified interrupt is not happened.
 *            1 = The specified interrupt is happened.
 * \hideinitializer
 */
#define SD_GET_INT_FLAG(u32IntMask) ((inpw(REG_SDH_INTSTS)&(u32IntMask))?1:0)


/**
 *  @brief    Clear specified interrupt flag/status.
 *
 *  @param[in]    u32IntMask    Interrupt type mask:
 *                           \ref SDH_INTSTS_BLKDIF_Msk / \ref SDH_INTSTS_CRCIF_Msk / \ref SDH_INTSTS_CDIF0_Msk /
 *                           \ref SDH_INTSTS_CDIF1_Msk / \ref SDH_INTSTS_SDHOST0IF_Msk / \ref SDH_INTSTS_SDHOST1IF_Msk /
 *                           \ref SDH_INTSTS_RTOIF_Msk / \ref SDH_INTSTS_DINTOIF_Msk
 *
 *
 *  @return   None.
 * \hideinitializer
 */
#define SD_CLR_INT_FLAG(u32IntMask) (outpw(REG_SDH_INTSTS, u32IntMask))


/**
 *  @brief    Check SD Card inserted or removed.
 *
 *  @param[in]    u32CardNum    Select SD0 or SD1. ( \ref SD_PORT0 / \ref SD_PORT1)
 *
 *  @return   1: Card inserted.
 *            0: Card removed.
 * \hideinitializer
 */
#define SD_IS_CARD_PRESENT(u32CardNum) ((u32CardNum & (SD_PORT0))?(SD0.IsCardInsert):(SD1.IsCardInsert))

/**
 *  @brief    Get SD Card capacity.
 *
 *  @param[in]    u32CardNum    Select SD0 or SD1. ( \ref SD_PORT0 / \ref SD_PORT1)
 *
 *  @return   SD Card capacity. (unit: KByte)
 * \hideinitializer
 */
#define SD_GET_CARD_CAPACITY(u32CardNum)  ((u32CardNum & (SD_PORT0))?(SD0.diskSize):(SD1.diskSize))


void SD_Open(unsigned int u32CardDetSrc);
int SD_Probe(unsigned int u32CardNum);
unsigned int SD_Read(unsigned int u32CardNum, unsigned char *pu8BufAddr, unsigned int u32StartSec, unsigned int u32SecCount);
unsigned int SD_Write(unsigned int u32CardNum, unsigned char *pu8BufAddr, unsigned int u32StartSec, unsigned int u32SecCount);
void SD_SetReferenceClock(unsigned int u32Clock);
unsigned int SD_CardDetection(unsigned int u32CardNum);
void SD_Open_Disk(unsigned int cardSel);
void SD_Close_Disk(unsigned int cardSel);


/*@}*/ /* end of group NUC970_SDH_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC970_SDH_Driver */

/*@}*/ /* end of group NUC970_Device_Driver */

#endif  //end of __SDH_H__
/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
