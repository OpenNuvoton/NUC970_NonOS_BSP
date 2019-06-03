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

//extern LCDFORMATEX lcdInfo;

extern CHAR g_u8String[100];
extern UINT32 g_u32StringIndex;

UINT32 g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_RGB565;  /* Decode Output format */
PUINT8 g_pu8JpegBuffer;                                  /* The source bit stream data for decoding */
PUINT8 g_pu8DecFrameBuffer;                              /* The buffer for decoding output */

/*-----------------------------------------------------------------------*/
/*  Decode Input Wait parameter                                          */
/*-----------------------------------------------------------------------*/
#if defined ( __GNUC__ ) && !(__CC_ARM)
    __attribute__((aligned(32))) UINT8 g_au8DecInputWaitBuffer[DEC_IPW_BUFFERSIZE];
#else
    UINT8 __align(32) g_au8DecInputWaitBuffer[DEC_IPW_BUFFERSIZE]; /* Buffer for Decode Input Wait */
#endif
UINT32 g_u32IpwUsedSize  = 0;    /* Buffer Control parameter for Decode Input Wait (bitstream size put to Bitstream Buffer )*/
UINT32 g_u32BitstreamSize = 0;   /* JPEG Bitstream Size for Decode Input Wait */

/*-----------------------------------------------------------------------*/
/*  Decode Output Wait parameter                                         */
/*-----------------------------------------------------------------------*/
PUINT8 apBuffer[100];           /* Decode Output buffer pointer array */
UINT32 g_u32OpwUsedSize;        /* JPEG Bitstream Size for Decode Input Wait */
UINT32 u32MCU_Line;             /* One MCU Line data size */
UINT32 u32TotalSize;            /* Total size for JPEG Decode Ouput */
UINT32 g_u32OpwBufferIndex = 0; /* Decode output Buffer index */

/*-----------------------------------------------------------------------*/
/*  Decode Function                                                      */
/*-----------------------------------------------------------------------*/
VOID JpegDecTest(void)
{
    INT          len;
    UINT32       u32BitstreamSize;
    JPEG_INFO_T  jpegInfo;
    FRESULT      nStatus;
    UINT nWriteLen, nReadLen;

    FIL fil;       /* File object */
    FRESULT fr;

    /* Decode Output Wait */
    UINT32 u32Width, u32Height, u32Format; // starttime,endtime;

    /*******************************************************************/
    /*               Read JPEG file                                    */
    /*******************************************************************/
    sysprintf("Input Jpeg File name\n");

    GetString();

    strcpy(decodePath, g_u8String);

    fr = f_open(&fil, decodePath, FA_OPEN_EXISTING | FA_READ);

    if (fr == FR_OK)
        sysprintf("\tOpen file:[%s] \n", decodePath);
    else
    {
        sysprintf("\tFailed to open file: %s \n", decodePath);
        return;
    }

    u32BitstreamSize = f_size(&fil);

    sysprintf("\tBit stream  size for Decode is %d\n", u32BitstreamSize);

    /* Allocate the Bitstream Data Buffer for Decode Operation */
    g_pu8JpegBuffer = (PUINT8)malloc((sizeof(CHAR) * u32BitstreamSize) + 1);

    nReadLen = 0;
    nStatus = f_read(&fil, (UINT8 *)((UINT32)g_pu8JpegBuffer | 0x80000000), u32BitstreamSize, &nReadLen);

    if (nStatus != FR_OK)
        sysprintf("\tRead error!!\n");

    f_close(&fil);

    if (ParsingJPEG((UINT8 *)((UINT32)g_pu8JpegBuffer | 0x80000000), nReadLen, &u32Width, &u32Height, &u32Format, TRUE) == ERR_MODE)
    {
        sysprintf("\tNot Support the JPEG sampling\n");
        goto END_ERROR_FORMAT;
    }

    if (g_bDecOpwTest)
    {
        sysprintf("\tJPEG Width %d Height %d\n", u32Width, u32Height);

        JpedInitDecOutputWaitBuffer(u32Width, u32Height, u32Format);
    }

    /*******************************************************************/
    /*             Decode JPEG Bitstream                               */
    /*******************************************************************/

    /* JPEG Init */
    jpegInit();

    if (g_bDecIpwTest)
    {
        /*******************************************************************/
        /*            Decode Input Setting                                 */
        /*******************************************************************/
        sysprintf("\tDecode Input Test - %d KB Bitstream Buffer\n", DEC_IPW_BUFFERSIZE / 1024);
        sysprintf("\tBitstream Buffer 0 0x%x \n", (UINT32)g_au8DecInputWaitBuffer | 0x80000000);
        sysprintf("\tBitstream Buffer 1 0x%x \n", (UINT32)g_au8DecInputWaitBuffer | 0x80000000 + DEC_IPW_BUFFERSIZE / 2);

        /* Reset the Buffer control parameter for Decode Input Wait function */
        g_u32IpwUsedSize = 0;

        /* Set Bit stream Address (g_au8DecInputWaitBuffer -> 8KB Buffer) */
        jpegIoctl(JPEG_IOCTL_SET_BITSTREAM_ADDR, (UINT32) g_au8DecInputWaitBuffer, 0);

        g_u32BitstreamSize = u32BitstreamSize;

        /* Set Decode Input Wait mode (Maximum size for the buffer is 2046 KB) */
        jpegIoctl(JPEG_IOCTL_SET_DECINPUTWAIT_CALBACKFUN, (UINT32) JpegDecInputWait, DEC_IPW_BUFFERSIZE);

        if (u32BitstreamSize < DEC_IPW_BUFFERSIZE)
            len = u32BitstreamSize;
        else
            len = DEC_IPW_BUFFERSIZE;

        /* Prepare the data for first decode operation (Fill Buffer 0 & 1) */
        /* Copy bitstream from the temp buffer (g_pu8JpegBuffer) to the buffer engine used (g_au8DecInputWaitBuffer) */
        memcpy((char *)((UINT32)g_au8DecInputWaitBuffer | 0x80000000), (char *)((UINT32)g_pu8JpegBuffer | 0x80000000), len);

        /* The bitstream size put to Bitstream Buffer */
        g_u32IpwUsedSize += len;
    }
    else
    {
        /* Set Bit stream Address */
        jpegIoctl(JPEG_IOCTL_SET_BITSTREAM_ADDR, (UINT32) g_pu8JpegBuffer, 0);
    }

    if (g_bDecOpwTest)
    {
        /* Start from first buffer */
        g_u32OpwBufferIndex = 0;

        /* Reset the output data counter */
        g_u32OpwUsedSize = 0;

        /* Set Decode Output Raw data Address and Size */
        jpegIoctl(JPEG_IOCTL_SET_DECOUTPUTWAIT, (UINT32)apBuffer[g_u32OpwBufferIndex], u32MCU_Line * (g_u32OpwBufferIndex + 1) / 4);

        /* Set Decode Output Wait mode */
        jpegIoctl(JPEG_IOCTL_SET_DECOUTPUTWAIT_CALBACKFUN, (UINT32) JpegDecOutputWait, 0);
    }

    if (g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
    {
        UINT32 u32FrameBuffer;
        /* Allocate Raw Data Buffer for Decode Operation (Prepare 1MB for Planar output) */
        /* Or user needs to get image size to allocate buffer before Decode Trigger for Planar */
        g_pu8DecFrameBuffer = (PUINT8)malloc((sizeof(CHAR) * PLANAR_DEC_BUFFER_SIZE) + 100);

        if (g_pu8DecFrameBuffer == NULL)
        {
            sysprintf("\tCan't allocate the buffer for decode (size 0x%X)\n", PLANAR_DEC_BUFFER_SIZE);
            return;
        }

        u32FrameBuffer = (UINT32) g_pu8DecFrameBuffer | 0x80000000;

        sysprintf("\tThe Buffer prepared forPlanar format starts from 0x%X, Size is 0x%X\n", u32FrameBuffer, PLANAR_DEC_BUFFER_SIZE);

        /* Set Decoded Image Address (Only Can be set before Decode Trigger for Planar;The address can set any time before existing Header Decode Complete Callback function) */
        jpegIoctl(JPEG_IOCTL_SET_YADDR, u32FrameBuffer, 0);
    }

    /* Decode mode */
    jpegIoctl(JPEG_IOCTL_SET_DECODE_MODE, g_u32DecFormat, 0);

    /* Set JPEG Header Decode End Call Back Function */
    jpegIoctl(JPEG_IOCTL_SET_HEADERDECODE_CALBACKFUN, (UINT32) JpegDecHeaderComplete, 0);

    //starttime = sysGetTicks(TIMER0);

    /* Trigger JPEG decoder */
    jpegIoctl(JPEG_IOCTL_DECODE_TRIGGER, 0, 0);

    /* Wait for complete */
    if (jpegWait())
    {
        /*******************************************************************/
        /*              Prepare Output file name                           */
        /*******************************************************************/
        //endtime = sysGetTicks(TIMER0);

        g_u8String[g_u32StringIndex] = '_';
        g_u8String[g_u32StringIndex + 1] = 0x00;

        strcpy(decodePath, g_u8String);

        if (g_bDecOpwTest)
            strcat(decodePath, "OPW_");

        if (g_bDecIpwTest)
            strcat(decodePath, "IPW_");

        if (g_bDecPanelTest)
            strcat(decodePath, "Panel_");

        if (g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
            strcat(decodePath, "PLANAR_");
        else
        {
            switch (g_u32DecFormat)
            {
                case JPEG_DEC_PRIMARY_PACKET_YUV422:
                    strcat(decodePath, "PACKET_YUV422_");
                    break;

                case JPEG_DEC_PRIMARY_PACKET_RGB555:
                    strcat(decodePath, "PACKET_RGB555_");
                    break;

                case JPEG_DEC_PRIMARY_PACKET_RGB565:
                    strcat(decodePath, "PACKET_RGB565_");
                    break;

                case JPEG_DEC_PRIMARY_PACKET_RGB555R1:
                    strcat(decodePath, "PACKET_RGB555_R1_");
                    break;

                case JPEG_DEC_PRIMARY_PACKET_RGB565R1:
                    strcat(decodePath, "PACKET_RGB565_R1_");
                    break;

                case JPEG_DEC_PRIMARY_PACKET_RGB555R2:
                    strcat(decodePath, "PACKET_RGB555_R2_");
                    break;

                case JPEG_DEC_PRIMARY_PACKET_RGB565R2:
                    strcat(decodePath, "PACKET_RGB565_R2_");
                    break;

                case JPEG_DEC_PRIMARY_PACKET_RGB888:
                    strcat(decodePath, "PACKET_RGB888_");
                    break;
            }
        }

        jpegGetInfo(&jpegInfo);
        sysprintf("\tJPEG Decode Complete!!\n");
        sysprintf("\t\tJpeg YuvFormat ");

        switch (jpegInfo.yuvformat)
        {
            case JPEG_DEC_YUV420:
                sysprintf("YUV420\n");

                if (g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
                    strcat(decodePath, "YUV420_");

                break;

            case JPEG_DEC_YUV422:
                sysprintf("YUV422\n");

                if (g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
                    strcat(decodePath, "YUV422_");

                break;

            case JPEG_DEC_YUV444:
                sysprintf("YUV444\n");

                if (g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
                    strcat(decodePath, "YUV444_");

                break;

            case JPEG_DEC_YUV411:
                sysprintf("YUV411\n");

                if (g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
                    strcat(decodePath, "YUV411_");

                break;

            case JPEG_DEC_GRAY:
                sysprintf("GRAY\n");

                if (g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
                    strcat(decodePath, "GRAY_");

                break;

            case JPEG_DEC_YUV422T:
                sysprintf("YUV422T\n");

                if (g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
                    strcat(decodePath, "YUV422T_");

                break;
        }

        sysprintf("\t\tJpeg Width = %d\n", jpegInfo.jpeg_width);        /* JPEG width in Bitstream header */
        sysprintf("\t\tJpeg Height = %d\n", jpegInfo.jpeg_height);      /* JPEG width in Bitstream header */
        strcat(decodePath, "size");

        if (g_bDecPanelTest)
        {
            strcat(decodePath, intToStr(PANEL_WIDTH));
            strcat(decodePath, "x");
            strcat(decodePath, intToStr(PANEL_HEIGHT));
        }
        else
        {
            strcat(decodePath, intToStr(jpegInfo.width));
            strcat(decodePath, "x");
            strcat(decodePath, intToStr(jpegInfo.height));
        }

        strcat(decodePath, ".dat");
        sysprintf("\t\tOutput Image Width = %d\n", jpegInfo.width);   /* Decode image width */
        sysprintf("\t\tOutput Image Height = %d\n", jpegInfo.height); /* Decode image height */

        if (jpegInfo.stride)
            sysprintf("\t\tJpeg Decode Image Stride = %d\n", jpegInfo.stride);

        /*******************************************************************/
        /*               Get Output file size                              */
        /*******************************************************************/

        len = jpegInfo.image_size[0];

        if (g_bDecPanelTest)
        {
            sysprintf("\t\tOutput Raw data size from JPEG engine %d x %d\n", PANEL_WIDTH, TARGET_HEIGHT);

            if (g_u32DecFormat == JPEG_DEC_PRIMARY_PACKET_RGB888)
                len = PANEL_WIDTH * PANEL_HEIGHT * 4;
            else
                len = PANEL_WIDTH * PANEL_HEIGHT * 2;

            sysprintf("\t\tOutput Raw data size (file) %d x %d\n", PANEL_WIDTH, PANEL_HEIGHT);

            //vpostLCMInit(&lcdInfo, (UINT32*)((UINT32)g_pu8DecFrameBuffer | 0x80000000));
        }
        else
        {
            sysprintf("\t\tOuput Raw data size (file) %d\n", len);
        }
    }
    else
    {
        sysprintf("\tJPEG Decode Error!!\n");
        len = 0;
        goto END;
    }

    /*******************************************************************/
    /*                   Write to Disk                                 */
    /*******************************************************************/

    strcpy(decodePath, "dec_test.dat");
    fr = f_open(&fil, decodePath, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);

    if (fr == FR_OK)
    {
        sysprintf("\tOpen file:[%s] \n", decodePath);
    }
    else
    {
        sysprintf("\tFailed to open file: %s \n", decodePath);
        return;
    }

    nStatus = f_write(&fil, (UINT8 *)((UINT32)g_pu8DecFrameBuffer | 0x80000000), len, &nWriteLen);

    if (nStatus != FR_OK)
        sysprintf("\tWrite error!!\n");

    f_close(&fil);

END:
    free(g_pu8DecFrameBuffer);
END_ERROR_FORMAT:
    free(g_pu8JpegBuffer);

    sysprintf("\tDecode test completed!\n");

    return;
}


