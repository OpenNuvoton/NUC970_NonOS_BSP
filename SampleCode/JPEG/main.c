/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/06/08 11:54a $
 * @brief    NUC970 Driver Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nuc970.h"
#include "sys.h"

#include "sdh.h"
#include "ff.h"
#include "diskio.h"
#include "jpegcodec.h"
#include "jpeg.h"
#include "jpegsample.h"

/*-----------------------------------------------------------------------*/
/*  JPEG Demo Code ReadMe                                                */
/*-----------------------------------------------------------------------*/
/*  Encode Operation                                                     */
/*  Please put the raw data file in SD card root folder, change the      */
/*  definition by Item 7 for the encode size. Select Item 8 to do the    */
/*  Encode operation. In Item 8, please input the file name for encode   */
/*  source. After encode operation complete, the jpeg file will write to */
/*  the input file name with file name exetension ".jpg" in SD card root */
/*  folder                                                               */
/*-----------------------------------------------------------------------*/
/*  Decode Operation                                                     */
/*  Please put the bitstream file into SD card root folder. User can     */
/*  change decode properties by the following items                      */
/*      Item 0 Enable/Disable Panel Test                                 */
/*             Downscale to fit the Panel size, and display on Panel     */
/*      Item 1 Enable/Disable Input Wait function                        */
/*      Item 2 Enable/Disable Output Wait function                       */
/*      Item 3 Set Decode Output format                                  */
/*      Item 4 Start Docode operation                                    */
/*             Do decode operation after input Jpeg file name            */
/*  After Decode operation complete, the decoded raw data file will      */
/*  write to the input file name with file name exetension ".dat" into   */
/*  SD card root folder                                                  */
/*-----------------------------------------------------------------------*/

BOOL g_bDecPanelTest = FALSE, g_bDecIpwTest = FALSE, g_bDecOpwTest = FALSE, g_bEncUpTest = FALSE, g_bEncSwReserveTest = FALSE;

//LCDFORMATEX lcdInfo;

extern void SD_Open_(unsigned int cardSel);

BYTE SD_Drv; // select SD0

void SDH_IRQHandler(void)
{
    unsigned int volatile isr;

    // FMI data abort interrupt
    if (inpw(REG_SDH_GINTSTS) & SDH_GINTSTS_DTAIF_Msk)
    {
        /* ResetAllEngine() */
        outpw(REG_SDH_GCTL, inpw(REG_SDH_GCTL) | SDH_GCTL_GCTLRST_Msk);
        outpw(REG_SDH_GINTSTS, SDH_GINTSTS_DTAIF_Msk);
    }

    //----- SD interrupt status
    isr = inpw(REG_SDH_INTSTS);

    if (isr & SDH_INTSTS_BLKDIF_Msk)        // block down
    {
        _sd_SDDataReady = TRUE;
        outpw(REG_SDH_INTSTS, SDH_INTSTS_BLKDIF_Msk);
    }

    if (isr & SDH_INTSTS_CDIF0_Msk)   // port 0 card detect
    {
        //----- SD interrupt status
        // it is work to delay 50 times for SD_CLK = 200KHz
        {
            volatile int i;         // delay 30 fail, 50 OK

            for (i = 0; i < 0x500; i++); // delay to make sure got updated value from REG_SDISR.

            isr = inpw(REG_SDH_INTSTS);
        }

#ifdef _USE_DAT3_DETECT_

        if (!(isr & SDH_INTSTS_CDSTS0_Msk))
        {
            SD0.IsCardInsert = FALSE;
            sysprintf("\nCard Remove!\n");
            SD_Close_(0);
        }
        else
        {
            SD_Open_(SD_PORT0 | CardDetect_From_DAT3);
        }

#else

        if (isr & SDH_INTSTS_CDSTS0_Msk)
        {
            SD0.IsCardInsert = FALSE;   // SDISR_CD_Card = 1 means card remove for GPIO mode
            sysprintf("\nCard Remove!\n");
            //SD_Close_(0);
        }
        else
        {
            SD_Open_(SD_PORT0 | CardDetect_From_GPIO);
        }

#endif

        outpw(REG_SDH_INTSTS, SDH_INTSTS_CDIF0_Msk);
    }

    if (isr & SDH_INTSTS_CDIF1_Msk)   // port 1 card detect
    {
        //----- SD interrupt status
        // it is work to delay 50 times for SD_CLK = 200KHz
        {
            volatile int i;         // delay 30 fail, 50 OK

            for (i = 0; i < 0x500; i++); // delay to make sure got updated value from REG_SDISR.

            isr = inpw(REG_SDH_INTSTS);
        }

#ifdef _USE_DAT3_DETECT_

        if (!(isr & SDH_INTSTS_CDSTS1_Msk))
        {
            SD0.IsCardInsert = FALSE;
            sysprintf("\nCard Remove!\n");
            SD_Close_(1);
        }
        else
        {
            SD_Open_(SD_PORT1 | CardDetect_From_DAT3);
        }

#else

        if (isr & SDH_INTSTS_CDSTS1_Msk)
        {
            SD0.IsCardInsert = FALSE;   // SDISR_CD_Card = 1 means card remove for GPIO mode
            sysprintf("\nCard Remove!\n");
            //SD_Close_(1);
        }
        else
        {
            SD_Open_(SD_PORT1 | CardDetect_From_GPIO);
        }

#endif

        outpw(REG_SDH_INTSTS, SDH_INTSTS_CDIF1_Msk);
    }

    // CRC error interrupt
    if (isr & SDH_INTSTS_CRCIF_Msk)
    {
        if (!(isr & SDH_INTSTS_CRC16_Msk))
        {
            //sysprintf("***** ISR sdioIntHandler(): CRC_16 error !\n");
            // handle CRC error
        }
        else if (!(isr & SDH_INTSTS_CRC7_Msk))
        {
            extern unsigned int _sd_uR3_CMD;

            if (! _sd_uR3_CMD)
            {
                //sysprintf("***** ISR sdioIntHandler(): CRC_7 error !\n");
                // handle CRC error
            }
        }

        outpw(REG_SDH_INTSTS, SDH_INTSTS_CRCIF_Msk);      // clear interrupt flag
    }
}


void SYS_Init(void)
{
    /* enable SDH */
    outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | 0x40000000);

    outpw(REG_SDH_ECTL, 0);

    /* SD Port 0 -> PD0~7 */
    outpw(REG_SYS_GPD_MFPL, 0x66666666);

    SD_Drv = 0;

}

/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */

unsigned long get_fattime(void)
{
    unsigned long tmr;

    tmr = 0x00000;

    return tmr;
}


INT32 main()
{
    UINT8 u8Item;
    BOOL bLoop = TRUE;
    TCHAR       sd_path[] = { '0', ':', 0 };    /* SD drive started from 0 */

    sysInitializeUART();

    sysDisableCache();
    sysInvalidCache();
    sysSetMMUMappingMethod(MMU_DIRECT_MAPPING);
    sysEnableCache(CACHE_WRITE_BACK);

    SYS_Init();

    sysSetTimerReferenceClock(TIMER0, 12000000);
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);

    sysprintf("\n/*-----------------------------------------------------------------------*/");
    sysprintf("\n/*  JPEG Demo code                                                       */");
    sysprintf("\n/*-----------------------------------------------------------------------*/\n");

    /*-----------------------------------------------------------------------*/
    /*  Init SD card                                                         */
    /*-----------------------------------------------------------------------*/

    sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, SDH_IRQn, (PVOID)SDH_IRQHandler);
    /* enable CPSR I bit */
    sysSetLocalInterrupt(ENABLE_IRQ);
    sysEnableInterrupt(SDH_IRQn);

    SD_SetReferenceClock(300000);
    SD_Open_(SD_PORT0 | CardDetect_From_GPIO);
    f_chdrive(sd_path);          /* set default path */

    /* JPEG Open */
    jpegOpen();

    while (bLoop)
    {
        sysprintf("\nPlease Select Test Item\n");
        sysprintf("[*]Decode Test\n");
        sysprintf(" 0 : Panel Test ");
        g_bDecPanelTest ? sysprintf("Disable\n") : sysprintf("Enable\n");
        sysprintf("     -> Decode Downscale to QQVGA\n");
        sysprintf("     -> Decode Stride is %d\n", PANEL_WIDTH);
        sysprintf("     -> Output data size is %dx%d\n", PANEL_WIDTH, PANEL_HEIGHT);
        sysprintf(" 1 : Input Wait ");
        g_bDecIpwTest ? sysprintf("Disable\n") : sysprintf("Enable\n");
        sysprintf(" 2 : Output Wait ");
        g_bDecOpwTest ? sysprintf("Disable\n") : sysprintf("Enable\n");
        sysprintf(" 3 : Set Deocode output format\n");
        sysprintf(" 4 : Start to Decode\n");
        sysprintf("     -> Decode output format is ");

        switch (g_u32DecFormat)
        {
            case JPEG_DEC_PRIMARY_PACKET_YUV422:
                sysprintf("PACKET YUV422\n");
                break;

            case JPEG_DEC_PRIMARY_PACKET_RGB555:
                sysprintf("PACKET RGB555\n");
                break;

            case JPEG_DEC_PRIMARY_PACKET_RGB565:
                sysprintf("PACKET RGB565\n");
                break;

            case JPEG_DEC_PRIMARY_PACKET_RGB555R1:
                sysprintf("PACKET RGB555R1\n");
                break;

            case JPEG_DEC_PRIMARY_PACKET_RGB565R1:
                sysprintf("PACKET RGB565R1\n");
                break;

            case JPEG_DEC_PRIMARY_PACKET_RGB555R2:
                sysprintf("PACKET RGB555R2\n");
                break;

            case JPEG_DEC_PRIMARY_PACKET_RGB565R2:
                sysprintf("PACKET RGB565R2\n");
                break;

            case JPEG_DEC_PRIMARY_PACKET_RGB888:
                sysprintf("PACKET RGB888\n");
                break;

            case JPEG_DEC_PRIMARY_PLANAR_YUV:
                sysprintf("PLANAR format\n");
                break;
        }

        sysprintf("[*]Encode Test\n");
        sysprintf(" 5 : Upscale ");
        g_bEncUpTest ? sysprintf("Disable\n") : sysprintf("Enable\n");
        sysprintf(" 6 : Software Reserved ");
        g_bEncSwReserveTest ? sysprintf("Disable\n") : sysprintf("Enable\n");
        sysprintf(" 7 : Set Encode Width & Height\n");
        sysprintf(" 8 : Start to Encode\n");
        sysprintf("     ->  Encode Size %dx%d\n", g_u32EncWidth, g_u32EncHeight);
        sysprintf(" 9 : Exit\n>");

        u8Item = sysGetChar();

        switch (u8Item)
        {
            case '0':
                if (g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
                    sysprintf("\n<Not support Planar format Panel Test>\n");
                else
                    g_bDecPanelTest ^= 1;

                break;

            case '1':
                g_bDecIpwTest ^= 1;
                break;

            case '2':
                g_bDecOpwTest ^= 1;
                break;

            case '3':
                sysprintf("\nPlease select Decode Output format\n");
                sysprintf(" 0: PACKET YUV422\n");
                sysprintf(" 1: PACKET RGB555\n");
                sysprintf(" 2: PACKET RGB565\n");
                sysprintf(" 3: PACKET RGB888\n");
                sysprintf(" 4: PACKET RGB555R1\n");
                sysprintf(" 5: PACKET RGB565R1\n");
                sysprintf(" 6: PACKET RGB555R2\n");
                sysprintf(" 7: PACKET RGB565R2\n");
                sysprintf(" 8: PLANAR format\n");
                sysprintf(">");
                u8Item = sysGetChar();

                switch (u8Item)
                {
                    case '0':
                        g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_YUV422;
                        break;

                    case '1':
                        g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_RGB555;
                        break;

                    case '2':
                        g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_RGB565;
                        break;

                    case '3':
                        g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_RGB888;
                        break;

                    case '4':
                        g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_RGB555R1;
                        break;

                    case '5':
                        g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_RGB565R1;
                        break;

                    case '6':
                        g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_RGB555R2;
                        break;

                    case '7':
                        g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_RGB565R2;
                        break;

                    case '8':
                        if (g_bDecPanelTest)
                            sysprintf("\n<Not support Planar format Panel Test>\n");
                        else
                            g_u32DecFormat = JPEG_DEC_PRIMARY_PLANAR_YUV;

                        break;

                    default:
                        sysprintf("Write Item\n");
                        break;
                }

                break;

            case '4':
                sysprintf("\n<Decode Test>\n");

                if (g_bDecPanelTest && g_bDecOpwTest)
                {
                    sysprintf("Both Panel Test and Decode Output Functions enabled is not supported by this sample code!!\n");
                    break;
                }

                if ((g_bDecOpwTest || g_bDecPanelTest) && (g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV))
                {
                    sysprintf("Decode Output/Panel Test Functions is only supported for Packet FORMAT!!\n");
                    break;
                }

                sysprintf("   -> Panel Test ");
                g_bDecPanelTest ? sysprintf("Enabled\n") : sysprintf("Disabled\n");
                sysprintf("   -> Input Test ");
                g_bDecIpwTest ? sysprintf("Enabled\n") : sysprintf("Disabled\n");
                sysprintf("   -> Output Test ");
                g_bDecOpwTest ? sysprintf("Enabled\n") : sysprintf("Disabled\n");
                JpegDecTest();
                break;

            case '5':
                g_bEncUpTest ^= 1;
                break;

            case '6':
                g_bEncSwReserveTest ^= 1;
                break;

            case '7':
                sysprintf("\nPlease input Encode Width\n");
                g_u32EncWidth = GetData();
                sysprintf("\nPlease input Encode Height\n");
                g_u32EncHeight = GetData();
                break;

            case '8':
                sysprintf("\n<Encode Test>\n");
                sysprintf("   Upscale ");
                g_bEncUpTest ? sysprintf("Enabled\n") : sysprintf("Disabled\n");
                sysprintf("    -> Software Reserved ");
                g_bEncSwReserveTest ? sysprintf("Enabled\n") : sysprintf("Disabled\n");
                sysprintf("    -> Encode Size %dx%d\n", g_u32EncWidth, g_u32EncHeight);
                JpegEncTest();
                break;

            case '9':
                bLoop = FALSE;
                break;

            default:
                sysprintf("Write Item\n");
                break;
        }
    }

    /* JPEG Close */
    jpegClose();


    sysprintf("\n/*-----------------------------------------------------------------------*/");
    sysprintf("\n/*  JPEG Demo code End                                                   */");
    sysprintf("\n/*-----------------------------------------------------------------------*/\n");

    while (1);

}















































