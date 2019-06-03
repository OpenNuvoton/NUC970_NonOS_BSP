/**************************************************************************//**
 * @file     sdh.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 16/06/21 1:58p $
 * @brief    NUC970 SD driver source file
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"
#include "sdh.h"

/** @addtogroup NUC970_Device_Driver NUC970 Device Driver
  @{
*/

/** @addtogroup NUC970_SDH_Driver SDH Driver
  @{
*/


/** @addtogroup NUC970_SDH_EXPORTED_FUNCTIONS SDH Exported Functions
  @{
*/
/// @cond HIDDEN_SYMBOLS

#define SD_BLOCK_SIZE   512

// global variables
// For response R3 (such as ACMD41, CRC-7 is invalid; but SD controller will still
// calculate CRC-7 and get an error result, software should ignore this error and clear SDISR [CRC_IF] flag
// _sd_uR3_CMD is the flag for it. 1 means software should ignore CRC-7 error
unsigned int _sd_uR3_CMD=0;
unsigned int _sd_uR7_CMD=0;
unsigned char volatile _sd_SDDataReady = FALSE;

unsigned char *_sd_pSDHCBuffer;
unsigned int gSDHReferenceClock;

#ifdef __ICCARM__
#pragma data_alignment = 4096
unsigned char _sd_ucSDHCBuffer[512];
#else
unsigned char _sd_ucSDHCBuffer[512] __attribute__((aligned(4096)));
#endif

int sd0_ok = 0;
int sd1_ok = 0;

unsigned char pSD0_offset = 0;
unsigned char pSD1_offset = 0;

SD_INFO_T SD0;
SD_INFO_T SD1;

void SD_CheckRB()
{
    while(1) {
        outpw(REG_SDH_CTL, inpw(REG_SDH_CTL)|SDH_CTL_CLK8OEN_Msk);
        while(inpw(REG_SDH_CTL) & SDH_CTL_CLK8OEN_Msk);
        if (inpw(REG_SDH_INTSTS) & SDH_INTSTS_DAT0STS_Msk)
            break;
    }
}


int SD_SDCommand(SD_INFO_T *pSD, unsigned char ucCmd, unsigned int uArg)
{
    volatile int buf;

    outpw(REG_SDH_CMD, uArg);
    buf = (inpw(REG_SDH_CTL)&(~SDH_CTL_CMDCODE_Msk))|(ucCmd << 8)|(SDH_CTL_COEN_Msk);
    outpw(REG_SDH_CTL, buf);

    while(inpw(REG_SDH_CTL) & SDH_CTL_COEN_Msk) {
        if (pSD->IsCardInsert == FALSE)
            return SD_NO_SD_CARD;
    }
    return 0;
}


int SD_SDCmdAndRsp(SD_INFO_T *pSD, unsigned char ucCmd, unsigned int uArg, int ntickCount)
{
    volatile int buf;

    outpw(REG_SDH_CMD, uArg);
    buf = (inpw(REG_SDH_CTL)&(~SDH_CTL_CMDCODE_Msk))|(ucCmd << 8)|(SDH_CTL_COEN_Msk | SDH_CTL_RIEN_Msk);
    outpw(REG_SDH_CTL, buf);

    if (ntickCount > 0) {
        while(inpw(REG_SDH_CTL) & SDH_CTL_RIEN_Msk) {
            if(ntickCount-- == 0) {
                outpw(REG_SDH_CTL, inpw(REG_SDH_CTL)|SDH_CTL_CTLRST_Msk); // reset SD engine
                return 2;
            }
            if (pSD->IsCardInsert == FALSE)
                return SD_NO_SD_CARD;
        }
    } else {
        while(inpw(REG_SDH_CTL) & SDH_CTL_RIEN_Msk) {
            if (pSD->IsCardInsert == FALSE)
                return SD_NO_SD_CARD;
        }
    }

    if (_sd_uR7_CMD) {
        if (((inpw(REG_SDH_RESP1) & 0xff) != 0x55) && ((inpw(REG_SDH_RESP0) & 0xf) != 0x01)) {
            _sd_uR7_CMD = 0;
            return SD_CMD8_ERROR;
        }
    }

    if (!_sd_uR3_CMD) {
        if (inpw(REG_SDH_INTSTS) & SDH_INTSTS_CRC7_Msk)     // check CRC7
            return 0;
        else
            return SD_CRC7_ERROR;
    } else { // ignore CRC error for R3 case
        _sd_uR3_CMD = 0;
        outpw(REG_SDH_INTSTS, SDH_INTSTS_CRCIF_Msk);
        return 0;
    }
}

int SD_Swap32(int val)
{
    int buf;

    buf = val;
    val <<= 24;
    val |= (buf<<8)&0xff0000;
    val |= (buf>>8)&0xff00;
    val |= (buf>>24)&0xff;
    return val;
}

// Get 16 bytes CID or CSD
int SD_SDCmdAndRsp2(SD_INFO_T *pSD, unsigned char ucCmd, unsigned int uArg, unsigned int *puR2ptr)
{
    unsigned int i, buf;
    unsigned int tmpBuf[5];

    outpw(REG_SDH_CMD, uArg);
    buf = (inpw(REG_SDH_CTL)&(~SDH_CTL_CMDCODE_Msk))|(ucCmd << 8)|(SDH_CTL_COEN_Msk | SDH_CTL_R2EN_Msk);
    outpw(REG_SDH_CTL, buf);

    while(inpw(REG_SDH_CTL) & SDH_CTL_R2EN_Msk) {
        if (pSD->IsCardInsert == FALSE)
            return SD_NO_SD_CARD;
    }

    if (inpw(REG_SDH_INTSTS) & SDH_INTSTS_CRC7_Msk) {
        for (i=0; i<5; i++)
            tmpBuf[i] = SD_Swap32(*(int*)(SDH_BA+i*4));
        for (i=0; i<4; i++)
            *puR2ptr++ = ((tmpBuf[i] & 0x00ffffff)<<8) | ((tmpBuf[i+1] & 0xff000000)>>24);
        return 0;
    } else
        return SD_CRC7_ERROR;
}


int SD_SDCmdAndRspDataIn(SD_INFO_T *pSD, unsigned char ucCmd, unsigned int uArg)
{
    volatile int buf;

    outpw(REG_SDH_CMD, uArg);
    buf = (inpw(REG_SDH_CTL)&(~SDH_CTL_CMDCODE_Msk))|(ucCmd << 8)|(SDH_CTL_COEN_Msk | SDH_CTL_RIEN_Msk | SDH_CTL_DIEN_Msk);
    outpw(REG_SDH_CTL, buf);

    while (inpw(REG_SDH_CTL) & SDH_CTL_RIEN_Msk) {
        if (pSD->IsCardInsert == FALSE)
            return SD_NO_SD_CARD;
    }

    while (inpw(REG_SDH_CTL) & SDH_CTL_DIEN_Msk) {
        if (pSD->IsCardInsert == FALSE)
            return SD_NO_SD_CARD;
    }

    if (!(inpw(REG_SDH_INTSTS) & SDH_INTSTS_CRC7_Msk)) {    // check CRC7
        return SD_CRC7_ERROR;
    }

    if (!(inpw(REG_SDH_INTSTS) & SDH_INTSTS_CRC16_Msk)) {   // check CRC16
        return SD_CRC16_ERROR;
    }
    return 0;
}

// there are 3 bits for divider N0, maximum is 8
#define SD_CLK_DIV0_MAX     8
// there are 8 bits for divider N1, maximum is 256
#define SD_CLK_DIV1_MAX     256

void SD_Set_clock(unsigned int sd_clock_khz)
{
    UINT32 rate, div0, div1, i;

    //--- calculate the rate that 2 divider have to divide
    // _fmi_uFMIReferenceClock is the input clock with unit KHz like as APLL/UPLL and
    //      assign by sicIoctl(SIC_SET_CLOCK, , , );
    if (sd_clock_khz > gSDHReferenceClock)
    {
        sysprintf("ERROR: wrong SD clock %dKHz since it is faster than input clock %dKHz !\n", sd_clock_khz, gSDHReferenceClock);
        return;
    }
    rate = gSDHReferenceClock / sd_clock_khz;
    // choose slower clock if system clock cannot divisible by wanted clock
    if (gSDHReferenceClock % sd_clock_khz != 0)
        rate++;

    if (rate > (SD_CLK_DIV0_MAX * SD_CLK_DIV1_MAX)) // the maximum divider for SD_CLK is (SD_CLK_DIV0_MAX * SD_CLK_DIV1_MAX)
    {
        sysprintf("ERROR: wrong SD clock %dKHz since it is slower than input clock %dKHz/%d !\n", sd_clock_khz, gSDHReferenceClock, SD_CLK_DIV0_MAX * SD_CLK_DIV1_MAX);
        return;
    }

    //--- choose a suitable value for first divider
    for (div0 = SD_CLK_DIV0_MAX; div0 > 0; div0--)    // choose the maximum value if can exact division
    {
        if (rate % div0 == 0)
            break;
    }
    if (div0 == 0) // cannot exact division
    {
        // if rate <= SD_CLK_DIV1_MAX, set div0 to 1 since div1 can exactly divide input clock
        div0 = (rate <= SD_CLK_DIV1_MAX) ? 1 : SD_CLK_DIV0_MAX;
    }

    //--- calculate the second divider 
    div1 = rate / div0;
    div1 &= 0xFF;

//     sysprintf("fmiSD_Set_clock(): wanted clock=%d, rate=%d, div0=%d, div1=%d\n", sd_clock_khz, rate, div0, div1);

    //--- setup register
	outpw(REG_CLK_DIVCTL9, (inpw(REG_CLK_DIVCTL9) & ~0x18) | (0x3 << 3)); 	    // SD clock from UPLL [4:3]
	outpw(REG_CLK_DIVCTL9, (inpw(REG_CLK_DIVCTL9) & ~0x7) | (div0-1)); 			// SD clock divided by CLKDIV3[SD_N] [2:0]
	outpw(REG_CLK_DIVCTL9, (inpw(REG_CLK_DIVCTL9) & ~0xff00) | ((div1-1) << 8)); 	// SD clock divided by CLKDIV3[SD_N] [15:8]
	for(i=0; i<1000; i++);  // waiting for clock become stable
	return;
}

void SD_CardSelect(int cardSel)
{
    if(cardSel == 0)
        outpw(REG_SDH_CTL, inpw(REG_SDH_CTL) & ~SDH_CTL_SDPORT_Msk);
    else if(cardSel == 1)
        outpw(REG_SDH_CTL, (inpw(REG_SDH_CTL)& ~SDH_CTL_SDPORT_Msk)|(1 << SDH_CTL_SDPORT_Pos));
}


// Initial
int SD_Init(SD_INFO_T *pSD)
{
    int volatile i, status;
    unsigned int resp;
    unsigned int CIDBuffer[4];
    unsigned int volatile u32CmdTimeOut;

    // set the clock to 300KHz
    SD_Set_clock(300);

    // power ON 74 clock
    outpw(REG_SDH_CTL, inpw(REG_SDH_CTL) | SDH_CTL_CLK74OEN_Msk);

    while(inpw(REG_SDH_CTL) & SDH_CTL_CLK74OEN_Msk) {
        if (pSD->IsCardInsert == FALSE)
            return SD_NO_SD_CARD;
    }

    SD_SDCommand(pSD, 0, 0);        // reset all cards
    for (i=0x1000; i>0; i--);

    // initial SDHC
    _sd_uR7_CMD = 1;
    u32CmdTimeOut = 5000;

    i = SD_SDCmdAndRsp(pSD, 8, 0x00000155, u32CmdTimeOut);
    if (i == 0) {
        // SD 2.0
        SD_SDCmdAndRsp(pSD, 55, 0x00, u32CmdTimeOut);
        _sd_uR3_CMD = 1;
        SD_SDCmdAndRsp(pSD, 41, 0x40ff8000, u32CmdTimeOut); // 2.7v-3.6v
        resp = inpw(REG_SDH_RESP0);

        while (!(resp & 0x00800000)) {      // check if card is ready
            SD_SDCmdAndRsp(pSD, 55, 0x00, u32CmdTimeOut);
            _sd_uR3_CMD = 1;
            SD_SDCmdAndRsp(pSD, 41, 0x40ff8000, u32CmdTimeOut); // 3.0v-3.4v
            resp = inpw(REG_SDH_RESP0);
        }
        if (resp & 0x00400000)
            pSD->CardType = SD_TYPE_SD_HIGH;
        else
            pSD->CardType = SD_TYPE_SD_LOW;
    } else {
        // SD 1.1
        SD_SDCommand(pSD, 0, 0);        // reset all cards
        for (i=0x100; i>0; i--);

        i = SD_SDCmdAndRsp(pSD, 55, 0x00, u32CmdTimeOut);
        if (i == 2) {   // MMC memory

            SD_SDCommand(pSD, 0, 0);        // reset
            for (i=0x100; i>0; i--);

            _sd_uR3_CMD = 1;

            if (SD_SDCmdAndRsp(pSD, 1, 0x40ff8000, u32CmdTimeOut) != 2) {  // eMMC memory
                resp = inpw(REG_SDH_RESP0);
                while (!(resp & 0x00800000)) {      // check if card is ready
                    _sd_uR3_CMD = 1;

                    SD_SDCmdAndRsp(pSD, 1, 0x40ff8000, u32CmdTimeOut);      // high voltage
                    resp = inpw(REG_SDH_RESP0);
                }

                if(resp & 0x00400000)
                    pSD->CardType = SD_TYPE_EMMC;
                else
                pSD->CardType = SD_TYPE_MMC;
            } else {
                pSD->CardType = SD_TYPE_UNKNOWN;
                return SD_ERR_DEVICE;
            }
        } else if (i == 0) { // SD Memory
            _sd_uR3_CMD = 1;
            SD_SDCmdAndRsp(pSD, 41, 0x00ff8000, u32CmdTimeOut); // 3.0v-3.4v
            resp = inpw(REG_SDH_RESP0);
            while (!(resp & 0x00800000)) {      // check if card is ready
                SD_SDCmdAndRsp(pSD, 55, 0x00,u32CmdTimeOut);
                _sd_uR3_CMD = 1;
                SD_SDCmdAndRsp(pSD, 41, 0x00ff8000, u32CmdTimeOut); // 3.0v-3.4v
                resp = inpw(REG_SDH_RESP0);
            }
            pSD->CardType = SD_TYPE_SD_LOW;
        } else {
            pSD->CardType = SD_TYPE_UNKNOWN;
            return SD_INIT_ERROR;
        }
    }

    // CMD2, CMD3
    if (pSD->CardType != SD_TYPE_UNKNOWN) {
        SD_SDCmdAndRsp2(pSD, 2, 0x00, CIDBuffer);
        if ((pSD->CardType == SD_TYPE_MMC) || (pSD->CardType == SD_TYPE_EMMC)) {
            if ((status = SD_SDCmdAndRsp(pSD, 3, 0x10000, 0)) != 0)        // set RCA
                return status;
            pSD->RCA = 0x10000;
        } else {
            if ((status = SD_SDCmdAndRsp(pSD, 3, 0x00, 0)) != 0)       // get RCA
                return status;
            else
                pSD->RCA = (inpw(REG_SDH_RESP0) << 8) & 0xffff0000;
        }
    }

	if (pSD->CardType == SD_TYPE_SD_HIGH)
		sysprintf("This is high capacity SD memory card\n");
	if (pSD->CardType == SD_TYPE_SD_LOW)
		sysprintf("This is standard capacity SD memory card\n");
	if (pSD->CardType == SD_TYPE_MMC)
		sysprintf("This is MMC memory card\n");
    return 0;
}


int SD_SwitchToHighSpeed(SD_INFO_T *pSD)
{
    int volatile status=0;
    unsigned short current_comsumption, busy_status0;

    outpw(REG_SDH_DMASA, (unsigned int)_sd_pSDHCBuffer);    // set DMA transfer starting address
    outpw(REG_SDH_BLEN, 63);    // 512 bit

    if ((status = SD_SDCmdAndRspDataIn(pSD, 6, 0x00ffff01)) != 0)
        return 1;

    current_comsumption = _sd_pSDHCBuffer[0]<<8 | _sd_pSDHCBuffer[1];
    if (!current_comsumption)
        return 1;

    busy_status0 = _sd_pSDHCBuffer[28]<<8 | _sd_pSDHCBuffer[29];

    if (!busy_status0) { // function ready
        outpw(REG_SDH_DMASA, (unsigned int)_sd_pSDHCBuffer);        // set DMA transfer starting address
        outpw(REG_SDH_BLEN, 63);    // 512 bit

        if ((status = SD_SDCmdAndRspDataIn(pSD, 6, 0x80ffff01)) != 0)
            return 1;

        // function change timing: 8 clocks
        outpw(REG_SDH_CTL, inpw(REG_SDH_CTL)| SDH_CTL_CLK8OEN_Msk);
        while(inpw(REG_SDH_CTL) & SDH_CTL_CLK8OEN_Msk);

        current_comsumption = _sd_pSDHCBuffer[0]<<8 | _sd_pSDHCBuffer[1];
        if (!current_comsumption)
            return 1;

        return 0;
    } else
        return 1;
}


int SD_SelectCardType(SD_INFO_T *pSD)
{
    int volatile status=0;
    unsigned int arg;

    if ((status = SD_SDCmdAndRsp(pSD, 7, pSD->RCA, 0)) != 0)
        return status;

    SD_CheckRB();

    // if SD card set 4bit
    if (pSD->CardType == SD_TYPE_SD_HIGH) {
        _sd_pSDHCBuffer = (unsigned char *)((unsigned int)_sd_ucSDHCBuffer);
        outpw(REG_SDH_DMASA, (unsigned int)_sd_pSDHCBuffer);    // set DMA transfer starting address
        outpw(REG_SDH_BLEN, 0x07);  // 64 bit

        if ((status = SD_SDCmdAndRsp(pSD, 55, pSD->RCA, 0)) != 0)
            return status;
        if ((status = SD_SDCmdAndRspDataIn(pSD, 51, 0x00)) != 0)
            return status;

        if ((_sd_ucSDHCBuffer[0] & 0xf) == 0x2) {
            status = SD_SwitchToHighSpeed(pSD);
            if (status == 0) {
                /* divider */
                SD_Set_clock(SDHC_FREQ);
            }
        }

        if ((status = SD_SDCmdAndRsp(pSD, 55, pSD->RCA, 0)) != 0)
            return status;
        if ((status = SD_SDCmdAndRsp(pSD, 6, 0x02, 0)) != 0)   // set bus width
            return status;

        outpw(REG_SDH_CTL, inpw(REG_SDH_CTL)| SDH_CTL_DBW_Msk);
    } else if (pSD->CardType == SD_TYPE_SD_LOW) {
        _sd_pSDHCBuffer = (unsigned char *)((unsigned int)_sd_ucSDHCBuffer);
        outpw(REG_SDH_DMASA, (unsigned int) _sd_pSDHCBuffer); // set DMA transfer starting address
        outpw(REG_SDH_BLEN, 0x07);  // 64 bit

        if ((status = SD_SDCmdAndRsp(pSD, 55, pSD->RCA, 0)) != 0)
            return status;
        if ((status = SD_SDCmdAndRspDataIn(pSD, 51, 0x00)) != 0)
            return status;

        // set data bus width. ACMD6 for SD card, SDCR_DBW for host.
        if ((status = SD_SDCmdAndRsp(pSD, 55, pSD->RCA, 0)) != 0)
            return status;

        if ((status = SD_SDCmdAndRsp(pSD, 6, 0x02, 0)) != 0)   // set bus width
            return status;

        outpw(REG_SDH_CTL, inpw(REG_SDH_CTL)| SDH_CTL_DBW_Msk);
    } else if ((pSD->CardType == SD_TYPE_MMC) ||(pSD->CardType == SD_TYPE_EMMC)) {
		
        if(pSD->CardType == SD_TYPE_MMC)
        outpw(REG_SDH_CTL, inpw(REG_SDH_CTL) & ~SDH_CTL_DBW_Msk);
		
        //--- sent CMD6 to MMC card to set bus width to 4 bits mode
        // set CMD6 argument Access field to 3, Index to 183, Value to 1 (4-bit mode)
        arg = (3 << 24) | (183 << 16) | (1 << 8);
        if ((status = SD_SDCmdAndRsp(pSD, 6, arg, 0)) != 0)
            return status;
        SD_CheckRB();

        outpw(REG_SDH_CTL, inpw(REG_SDH_CTL)| SDH_CTL_DBW_Msk);
    }

    if ((status = SD_SDCmdAndRsp(pSD, 16, SD_BLOCK_SIZE, 0)) != 0) // set block length
        return status;
    outpw(REG_SDH_BLEN, SD_BLOCK_SIZE - 1);           // set the block size

    SD_SDCommand(pSD, 7, 0);
    outpw(REG_SDH_CTL, inpw(REG_SDH_CTL)|SDH_CTL_CLK8OEN_Msk);
    while(inpw(REG_SDH_CTL) & SDH_CTL_CLK8OEN_Msk);

    outpw(REG_SDH_INTEN, inpw(REG_SDH_INTEN)| SDH_INTEN_BLKDIEN_Msk);

    return 0;
}

void SD_Get_SD_info(SD_INFO_T *pSD)
{
    unsigned int R_LEN, C_Size, MULT, size;
    unsigned int Buffer[4];
    unsigned char *ptr;

    SD_SDCmdAndRsp2(pSD, 9, pSD->RCA, Buffer);

	if ((pSD->CardType == SD_TYPE_MMC) || (pSD->CardType == SD_TYPE_EMMC))
    {
        // for MMC/eMMC card
        if ((Buffer[0] & 0xc0000000) == 0xc0000000)
        {
            // CSD_STRUCTURE [127:126] is 3
            // CSD version depend on EXT_CSD register in eMMC v4.4 for card size > 2GB
            SD_SDCmdAndRsp(pSD, 7, pSD->RCA, 0);

            ptr = (unsigned char *)((unsigned int)_sd_ucSDHCBuffer);
            outpw(REG_SDH_DMASA, (unsigned int)ptr);  // set DMA transfer starting address
            outpw(REG_SDH_BLEN, 511);  // read 512 bytes for EXT_CSD
			
            if (SD_SDCmdAndRspDataIn(pSD, 8, 0x00) != 0)
                return;

            SD_SDCommand(pSD, 7, 0);
            outpw(REG_SDH_CTL, inpw(REG_SDH_CTL)|SDH_CTL_CLK8OEN_Msk);
            while(inpw(REG_SDH_CTL) & SDH_CTL_CLK8OEN_Msk);

            pSD->totalSectorN = (*(unsigned int *)(ptr+212));
            pSD->diskSize = pSD->totalSectorN / 2;
        }
        else
        {
            // CSD version v1.0/1.1/1.2 in eMMC v4.4 spec for card size <= 2GB
            R_LEN = (Buffer[1] & 0x000f0000) >> 16;
            C_Size = ((Buffer[1] & 0x000003ff) << 2) | ((Buffer[2] & 0xc0000000) >> 30);
            MULT = (Buffer[2] & 0x00038000) >> 15;
            size = (C_Size+1) * (1<<(MULT+2)) * (1<<R_LEN);

            pSD->diskSize = size / 1024;
            pSD->totalSectorN = size / 512;
        }
    }
    else
    {
        if (Buffer[0] & 0xc0000000)
        {
            C_Size = ((Buffer[1] & 0x0000003f) << 16) | ((Buffer[2] & 0xffff0000) >> 16);
            size = (C_Size+1) * 512;    // Kbytes

            pSD->diskSize = size;
            pSD->totalSectorN = size << 1;
        }
        else
        {
            R_LEN = (Buffer[1] & 0x000f0000) >> 16;
            C_Size = ((Buffer[1] & 0x000003ff) << 2) | ((Buffer[2] & 0xc0000000) >> 30);
            MULT = (Buffer[2] & 0x00038000) >> 15;
            size = (C_Size+1) * (1<<(MULT+2)) * (1<<R_LEN);

            pSD->diskSize = size / 1024;
            pSD->totalSectorN = size / 512;
        }
    }
    pSD->sectorSize = 512;
    sysprintf("The size is %d KB\n", pSD->diskSize);
}

/// @endcond HIDDEN_SYMBOLS

/**
 *  @brief  This function use to detect SD card is inserted or remove.
 *
 *  @param[in]  u32CardNum  Select initial SD0 or SD1. ( \ref SD_PORT0 / \ref SD_PORT1)
 *
 *  @return   1: Card inserted.
 *            0: Card removed.
 */
unsigned int SD_CardDetection(unsigned int u32CardNum)
{
    unsigned int i;

    if (u32CardNum == SD_PORT0) {
        if(inpw(REG_SDH_INTEN) & SDH_INTEN_CDSRC0_Msk) { // Card detect pin from GPIO
            outpw(REG_SDH_INTEN, inpw(REG_SDH_INTEN) | SDH_INTEN_CDSRC0_Msk | SDH_INTEN_CDIEN0_Msk);
            if(inpw(REG_SDH_INTSTS) & SDH_INTSTS_CDSTS0_Msk) { // Card remove
                SD0.IsCardInsert = FALSE;
                return FALSE;
            } else
                SD0.IsCardInsert = TRUE;
        } else if(!(inpw(REG_SDH_INTEN) & SDH_INTEN_CDSRC0_Msk)) {
            outpw(REG_SDH_INTEN, inpw(REG_SDH_INTEN) & ~SDH_INTEN_CDSRC0_Msk);
            outpw(REG_SDH_CTL, inpw(REG_SDH_CTL) | SDH_CTL_CLKKEEP0_Msk);
            for(i= 0; i < 5000; i++);

            if(inpw(REG_SDH_INTSTS) & SDH_INTSTS_CDSTS0_Msk) // Card insert
                SD0.IsCardInsert = TRUE;
            else {
                SD0.IsCardInsert = FALSE;
                return FALSE;
            }
            outpw(REG_SDH_CTL, inpw(REG_SDH_CTL) & ~SDH_CTL_CLKKEEP0_Msk);
        }

    } else if (u32CardNum == SD_PORT1) {
        if(inpw(REG_SDH_INTEN) & SDH_INTEN_CDSRC1_Msk) { // Card detect pin from GPIO
            outpw(REG_SDH_INTEN, inpw(REG_SDH_INTEN) | SDH_INTEN_CDSRC1_Msk | SDH_INTEN_CDIEN1_Msk);
            if(inpw(REG_SDH_INTSTS) & SDH_INTSTS_CDSTS1_Msk) { // Card remove
                SD1.IsCardInsert = FALSE;
                return FALSE;
            } else
                SD1.IsCardInsert = TRUE;
        } else if(!(inpw(REG_SDH_INTEN) & SDH_INTEN_CDSRC1_Msk)) {
            outpw(REG_SDH_INTEN, inpw(REG_SDH_INTEN) & ~SDH_INTEN_CDSRC1_Msk);
            outpw(REG_SDH_CTL, inpw(REG_SDH_CTL) | SDH_CTL_CLKKEEP1_Msk);
            for(i= 0; i < 5000; i++);

            if(inpw(REG_SDH_INTSTS) & SDH_INTSTS_CDSTS1_Msk) // Card insert
                SD1.IsCardInsert = TRUE;
            else {
                SD1.IsCardInsert = FALSE;
                return FALSE;
            }
            outpw(REG_SDH_CTL, inpw(REG_SDH_CTL) & ~SDH_CTL_CLKKEEP1_Msk);
        }
    }
    return TRUE;
}

/**
 *  @brief  This function use to tell SD engine clock.
 *
 *  @param[in]  u32Clock   Set current SD engine clock
 *
 *  @return None
 */
void SD_SetReferenceClock(unsigned int u32Clock)
{
	gSDHReferenceClock = u32Clock;	// kHz
}

/**
 *  @brief  This function use to reset SD function and select card detection source and pin.
 *
 *  @param[in]  u32CardDetSrc   Select card detection source from SD0 or SD1. ( \ref SD_PORT0 / \ref SD_PORT1) \n
 *                          And also select card detection pin from GPIO or DAT3 pin. ( \ref CardDetect_From_GPIO / \ref CardDetect_From_DAT3)
 *
 *  @return None
 */
void SD_Open(unsigned int u32CardDetSrc)
{
    // enable DMAC
    outpw(REG_SDH_DMACTL, SDH_DMACTL_DMARST_Msk);
    while(inpw(REG_SDH_DMACTL) & SDH_DMACTL_DMARST_Msk);

    outpw(REG_SDH_DMACTL, SDH_DMACTL_DMAEN_Msk);

    //Reset Global
    outpw(REG_SDH_GCTL, SDH_GCTL_GCTLRST_Msk);
    while(inpw(REG_SDH_GCTL) & SDH_GCTL_GCTLRST_Msk);

    // enable SD
    outpw(REG_SDH_GCTL, SDH_GCTL_SDEN_Msk);
    outpw(REG_SDH_ECTL, 0);

    outpw(REG_SDH_CTL, inpw(REG_SDH_CTL) | SDH_CTL_CTLRST_Msk);     // SD software reset
    while(inpw(REG_SDH_CTL) & SDH_CTL_CTLRST_Msk);

    outpw(REG_SDH_CTL, inpw(REG_SDH_CTL) & ~(0xFF|SDH_CTL_CLKKEEP1_Msk));    // disable SD clock output

    if(u32CardDetSrc & SD_PORT0) {
        memset(&SD0, 0, sizeof(SD_INFO_T));
        if(u32CardDetSrc & CardDetect_From_GPIO) {
            outpw(REG_SDH_INTEN, inpw(REG_SDH_INTEN) | SDH_INTEN_CDSRC0_Msk);
        }
        else if (u32CardDetSrc & CardDetect_From_DAT3) {
            outpw(REG_SDH_INTEN, inpw(REG_SDH_INTEN) & ~SDH_INTEN_CDSRC0_Msk);
        }
    }
    else if(u32CardDetSrc & SD_PORT1) {
        memset(&SD1, 0, sizeof(SD_INFO_T));
        if(u32CardDetSrc & CardDetect_From_GPIO) {
            outpw(REG_SDH_INTEN, inpw(REG_SDH_INTEN) | SDH_INTEN_CDSRC1_Msk);
        }
        else if (u32CardDetSrc & CardDetect_From_DAT3) {
            outpw(REG_SDH_INTEN, inpw(REG_SDH_INTEN) & ~SDH_INTEN_CDSRC1_Msk);
        }
    }
}

/**
 *  @brief  This function use to initial SD card.
 *
 *  @param[in]  u32CardNum  Select initial SD0 or SD1. ( \ref SD_PORT0 / \ref SD_PORT1)
 *
 *  @return None
 */
int SD_Probe(unsigned int u32CardNum)
{
    // Disable SD host interrupt
    outpw(REG_SDH_GINTEN, 0);

    outpw(REG_SDH_CTL, inpw(REG_SDH_CTL)& ~(SDH_CTL_SDNWR_Msk|SDH_CTL_BLKCNT_Msk|SDH_CTL_DBW_Msk));
    outpw(REG_SDH_CTL, inpw(REG_SDH_CTL)|(0x09 << SDH_CTL_SDNWR_Pos)|(0x01 << SDH_CTL_BLKCNT_Pos));

    if(!(SD_CardDetection(u32CardNum)))
        return FALSE;

    if (u32CardNum == SD_PORT0) {
        if (SD_Init(&SD0) < 0)
            return FALSE;

        /* divider */
        if (SD0.CardType == SD_TYPE_MMC)
            SD_Set_clock(20000);
        else
            SD_Set_clock(SD_FREQ);

        SD_Get_SD_info(&SD0);

        if (SD_SelectCardType(&SD0))
            return FALSE;

        sd0_ok = 1;
    } else if (u32CardNum == SD_PORT1) {
        if (SD_Init(&SD1) < 0)
            return FALSE;

        /* divider */
        if (SD1.CardType == SD_TYPE_MMC)
            SD_Set_clock(20000);
        else
            SD_Set_clock(SD_FREQ);

        SD_Get_SD_info(&SD1);

        if (SD_SelectCardType(&SD1))
            return FALSE;

        sd1_ok = 1;
    }
    return TRUE;
}

/**
 *  @brief  This function use to read data from SD card.
 *
 *  @param[in]     u32CardNum    Select card: SD0 or SD1. ( \ref SD_PORT0 / \ref SD_PORT1)
 *  @param[out]    pu8BufAddr    The buffer to receive the data from SD card.
 *  @param[in]     u32StartSec   The start read sector address.
 *  @param[in]     u32SecCount   The the read sector number of data
 *
 *  @return None
 */
unsigned int SD_Read(unsigned int u32CardNum, unsigned char *pu8BufAddr, unsigned int u32StartSec, unsigned int u32SecCount)
{
    char volatile bIsSendCmd = FALSE;
    unsigned int volatile reg;
    int volatile i, loop, status;
    unsigned int blksize = SD_BLOCK_SIZE;

    SD_INFO_T *pSD;

    if(u32CardNum == SD_PORT0)
        pSD = &SD0;
    else
        pSD = &SD1;

    //--- check input parameters
    if (u32SecCount == 0)
        return SD_SELECT_ERROR;

    if ((status = SD_SDCmdAndRsp(pSD, 7, pSD->RCA, 0)) != 0)
        return status;
    SD_CheckRB();

    outpw(REG_SDH_BLEN, blksize - 1);       // the actual byte count is equal to (SDBLEN+1)

    if ( (pSD->CardType == SD_TYPE_SD_HIGH) || (pSD->CardType == SD_TYPE_EMMC) )
        outpw(REG_SDH_CMD, u32StartSec);
    else
        outpw(REG_SDH_CMD, u32StartSec * blksize);

    outpw(REG_SDH_DMASA, (unsigned int)pu8BufAddr);
    
    loop = u32SecCount / 255;
    for (i=0; i<loop; i++) {
        _sd_SDDataReady = FALSE;

        reg = inpw(REG_SDH_CTL) & ~SDH_CTL_CMDCODE_Msk;
        reg = reg | 0xff0000;   // set BLK_CNT to 255
        if (bIsSendCmd == FALSE) {
            outpw(REG_SDH_CTL, reg|(18<<8)|(SDH_CTL_COEN_Msk | SDH_CTL_RIEN_Msk | SDH_CTL_DIEN_Msk));
            bIsSendCmd = TRUE;
        } else
            outpw(REG_SDH_CTL, reg | SDH_CTL_DIEN_Msk);

        while(!_sd_SDDataReady)
        {
//             if ((inpw(REG_SDH_INTSTS) & SDH_INTSTS_BLKDIF_Msk) && (!(inpw(REG_SDH_CTL) & SDH_CTL_DIEN_Msk))) {
//                 outpw(REG_SDH_INTSTS, SDH_INTSTS_BLKDIF_Msk);
//                 break;
//             }
            if (pSD->IsCardInsert == FALSE)
                return SD_NO_SD_CARD;
        }

        if (!(inpw(REG_SDH_INTSTS) & SDH_INTSTS_CRC7_Msk)) {    // check CRC7
            //sysprintf("sdioSD_Read_in_blksize(): response error!\n");
            return SD_CRC7_ERROR;
        }

        if (!(inpw(REG_SDH_INTSTS) & SDH_INTSTS_CRC16_Msk)) {   // check CRC16
            //sysprintf("sdioSD_Read_in_blksize() :read data error!\n");
            return SD_CRC16_ERROR;
        }
    }

    loop = u32SecCount % 255;
    if (loop != 0) {
        _sd_SDDataReady = FALSE;

        reg = inpw(REG_SDH_CTL) & (~SDH_CTL_CMDCODE_Msk);
        reg = reg & (~SDH_CTL_BLKCNT_Msk);
        reg |= (loop << 16);    // setup SDCR_BLKCNT

        if (bIsSendCmd == FALSE) {
            outpw(REG_SDH_CTL, reg|(18<<8)|(SDH_CTL_COEN_Msk | SDH_CTL_RIEN_Msk | SDH_CTL_DIEN_Msk));
            bIsSendCmd = TRUE;
        } else
            outpw(REG_SDH_CTL, reg | SDH_CTL_DIEN_Msk);

        while(!_sd_SDDataReady)
        {
            if ((inpw(REG_SDH_INTSTS) & SDH_INTSTS_BLKDIF_Msk) && (!(inpw(REG_SDH_CTL) & SDH_CTL_DIEN_Msk))) {
                outpw(REG_SDH_INTSTS, SDH_INTSTS_BLKDIF_Msk);
                break;
            }
            if (pSD->IsCardInsert == FALSE)
                return SD_NO_SD_CARD;
        }

        if (!(inpw(REG_SDH_INTSTS) & SDH_INTSTS_CRC7_Msk)) {    // check CRC7
            //sysprintf("sdioSD_Read_in_blksize(): response error!\n");
            return SD_CRC7_ERROR;
        }

        if (!(inpw(REG_SDH_INTSTS) & SDH_INTSTS_CRC16_Msk)) {   // check CRC16
            //sysprintf("sdioSD_Read_in_blksize(): read data error!\n");
            return SD_CRC16_ERROR;
        }
    }

    if (SD_SDCmdAndRsp(pSD, 12, 0, 0)) {    // stop command
        //sysprintf("stop command fail !!\n");
        return SD_CRC7_ERROR;
    }
    SD_CheckRB();

    SD_SDCommand(pSD, 7, 0);
    outpw(REG_SDH_CTL, inpw(REG_SDH_CTL)|SDH_CTL_CLK8OEN_Msk);
    while(inpw(REG_SDH_CTL) & SDH_CTL_CLK8OEN_Msk);

    return 0;
}


/**
 *  @brief  This function use to write data to SD card.
 *
 *  @param[in]    u32CardNum  Select card: SD0 or SD1. ( \ref SD_PORT0 / \ref SD_PORT1)
 *  @param[in]    pu8BufAddr    The buffer to send the data to SD card.
 *  @param[in]    u32StartSec   The start write sector address.
 *  @param[in]    u32SecCount   The the write sector number of data.
 *
 *  @return   \ref SD_SELECT_ERROR : u32SecCount is zero. \n
 *            \ref SD_NO_SD_CARD : SD card be removed. \n
 *            \ref SD_CRC_ERROR : CRC error happen. \n
 *            \ref SD_CRC7_ERROR : CRC7 error happen.
 */
unsigned int SD_Write(unsigned int u32CardNum, unsigned char *pu8BufAddr, unsigned int u32StartSec, unsigned int u32SecCount)
{
    char volatile bIsSendCmd = FALSE;
    unsigned int volatile reg;
    int volatile i, loop, status;

    SD_INFO_T *pSD;

    if(u32CardNum == SD_PORT0)
        pSD = &SD0;
    else
        pSD = &SD1;

    //--- check input parameters
    if (u32SecCount == 0)
        return SD_SELECT_ERROR;

    if ((status = SD_SDCmdAndRsp(pSD, 7, pSD->RCA, 0)) != 0)
        return status;

    SD_CheckRB();

    // According to SD Spec v2.0, the write CMD block size MUST be 512, and the start address MUST be 512*n.
    outpw(REG_SDH_BLEN, SD_BLOCK_SIZE - 1);           // set the block size

    if ((pSD->CardType == SD_TYPE_SD_HIGH) || (pSD->CardType == SD_TYPE_EMMC))
        outpw(REG_SDH_CMD, u32StartSec);
    else
        outpw(REG_SDH_CMD, u32StartSec * SD_BLOCK_SIZE);  // set start address for SD CMD

    outpw(REG_SDH_DMASA, (unsigned int)pu8BufAddr);
    loop = u32SecCount / 255;   // the maximum block count is 0xFF=255 for register SDCR[BLK_CNT]
    for (i=0; i<loop; i++) {
        _sd_SDDataReady = FALSE;

        reg = inpw(REG_SDH_CTL) & 0xff00c080;
        reg = reg | 0xff0000;   // set BLK_CNT to 0xFF=255
        if (!bIsSendCmd) {
            outpw(REG_SDH_CTL, reg|(25<<8)|(SDH_CTL_COEN_Msk | SDH_CTL_RIEN_Msk | SDH_CTL_DOEN_Msk));
            bIsSendCmd = TRUE;
        } else
            outpw(REG_SDH_CTL, reg | SDH_CTL_DOEN_Msk);

        while(!_sd_SDDataReady)
        {
//             if ((inpw(REG_SDH_INTSTS) & SDH_INTSTS_BLKDIF_Msk) && (!(inpw(REG_SDH_CTL) & SDH_CTL_DOEN_Msk))) {
//                 outpw(REG_SDH_INTSTS, SDH_INTSTS_BLKDIF_Msk);
//                 break;
//             }
            if (pSD->IsCardInsert == FALSE)
                return SD_NO_SD_CARD;
        }

        if ((inpw(REG_SDH_INTSTS) & SDH_INTSTS_CRCIF_Msk) != 0) {   // check CRC
            outpw(REG_SDH_INTSTS, SDH_INTSTS_CRCIF_Msk);
            return SD_CRC_ERROR;
        }
    }

    loop = u32SecCount % 255;
    if (loop != 0) {
        _sd_SDDataReady = FALSE;

        reg = (inpw(REG_SDH_CTL) & 0xff00c080) | (loop << 16);
        if (!bIsSendCmd) {
            outpw(REG_SDH_CTL, reg|(25<<8)|(SDH_CTL_COEN_Msk | SDH_CTL_RIEN_Msk | SDH_CTL_DOEN_Msk));
            bIsSendCmd = TRUE;
        } else
            outpw(REG_SDH_CTL, reg | SDH_CTL_DOEN_Msk);

        while(!_sd_SDDataReady)
        {
//             if ((inpw(REG_SDH_INTSTS) & SDH_INTSTS_BLKDIF_Msk) && (!(inpw(REG_SDH_CTL) & SDH_CTL_DOEN_Msk))) {
//                 outpw(REG_SDH_INTSTS, SDH_INTSTS_BLKDIF_Msk);
//                 break;
//             }
            if (pSD->IsCardInsert == FALSE)
                return SD_NO_SD_CARD;
        }

        if ((inpw(REG_SDH_INTSTS) & SDH_INTSTS_CRCIF_Msk) != 0) {   // check CRC
            outpw(REG_SDH_INTSTS, SDH_INTSTS_CRCIF_Msk);
            return SD_CRC_ERROR;
        }
    }
    outpw(REG_SDH_INTSTS, SDH_INTSTS_CRCIF_Msk);

    if (SD_SDCmdAndRsp(pSD, 12, 0, 0)) {    // stop command
        return SD_CRC7_ERROR;
    }
    SD_CheckRB();

    SD_SDCommand(pSD, 7, 0);
    outpw(REG_SDH_CTL, inpw(REG_SDH_CTL)|SDH_CTL_CLK8OEN_Msk);
    while(inpw(REG_SDH_CTL) & SDH_CTL_CLK8OEN_Msk);

    return 0;
}


/*@}*/ /* end of group NUC970_SD_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC970_SD_Driver */

/*@}*/ /* end of group NUC970_Device_Driver */

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/








