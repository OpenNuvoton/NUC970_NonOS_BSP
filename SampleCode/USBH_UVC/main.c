/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 18/03/12 10:47a $
 * @brief    This sample shows how to manage several USB HID class devices.
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "lcd.h"
#include "usbh_lib.h"
#include "usbh_uvc.h"

#include "jpegcodec.h"
#include "jpeg.h"


#define USB_PPWR_PE

//#define SELECT_MJPEG

#define SELECT_RES_WIDTH     640
#define SELECT_RES_HEIGHT    480

#define IMAGE_MAX_SIZE       (640*480*2)

#define SNAPSHOT_POST_TIME   300          /* Let snapshot image posted on OSD for 3 seconds */

static UVC_DEV_T   *g_vdev = NULL;


/*---------------------------------------------------------------------- 
 * Image buffers  
 */
#define IMAGE_BUFF_CNT       2

enum 
{
    IMAGE_BUFF_FREE,
    IMAGE_BUFF_USB,
    IMAGE_BUFF_READY,
    IMAGE_BUFF_POST
};

struct ig_buff_t
{
    uint8_t   *buff;
    int       len;
    int       state;
};   
struct ig_buff_t  _ig[IMAGE_BUFF_CNT];

__align(32) uint8_t  image_buff_pool[IMAGE_BUFF_CNT][IMAGE_MAX_SIZE];

__align(32) uint8_t  snapshot_buff_pool[IMAGE_MAX_SIZE];

int   _idx_usb = 0, _idx_post = 0;
int   _total_frame_count = 0;



/*------------------------------------------------------------------------*/

uint8_t  * g_OSD_base = 0;
uint8_t  * g_LCD_base = 0;

void delay_us(int usec)
{
    volatile int  loop = 300 * usec;
    while (loop > 0) loop--;
}

uint32_t get_ticks(void)
{
    return sysGetTicks(TIMER0);
}


void  dump_buff_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0) 
    {
        sysprintf("0x%04X  ", nIdx);
        for (i = 0; (i < 16) && (nBytes > 0); i++)
        {
            sysprintf("%02x ", pucBuff[nIdx + i]);
            nBytes--;
        }
        nIdx += 16;
        sysprintf("\n");
    }
    sysprintf("\n");
}


static void init_lcd(void)
{
    // Configure multi-function pin for LCD interface
    //GPG6 (CLK), GPG7 (HSYNC)
    outpw(REG_SYS_GPG_MFPL, (inpw(REG_SYS_GPG_MFPL)& ~0xFF000000) | 0x22000000);
    //GPG8 (VSYNC), GPG9 (DEN)
    outpw(REG_SYS_GPG_MFPH, (inpw(REG_SYS_GPG_MFPH)& ~0xFF) | 0x22);
    
    //DATA pin
    //GPA0 ~ GPA7 (DATA0~7)
    outpw(REG_SYS_GPA_MFPL, 0x22222222);
    //GPA8 ~ GPA15 (DATA8~15)   
    outpw(REG_SYS_GPA_MFPH, 0x22222222);
    //GPD8~D15 (DATA16~23)
    outpw(REG_SYS_GPD_MFPH, (inpw(REG_SYS_GPD_MFPH)& ~0xFFFFFFFF) | 0x22222222);
    
    // LCD clock is selected from UPLL and divide to 20MHz
    outpw(REG_CLK_DIVCTL1, (inpw(REG_CLK_DIVCTL1) & ~0xff1f) | 0xe18);      
    
    // Init LCD interface for E50A2V1 LCD module
    vpostLCMInit(DIS_PANEL_E50A2V1);
    // Set scale to 1:1
    vpostVAScalingCtrl(1, 0, 1, 0, VA_SCALE_INTERPOLATION);
    
    /* Select YUV422 format */
    vpostSetVASrc(VA_SRC_RGB565);   

    g_LCD_base = vpostGetFrameBuffer();
    if(g_LCD_base == NULL)
    {
        sysprintf("Get LCD buffer error !!\n");
        return;
    }

    // Set OSD position and display size
    vpostOSDSetWindow(80, 0, SELECT_RES_WIDTH, SELECT_RES_HEIGHT);
    
    vpostSetOSDSrc(OSD_SRC_YCBCR422);

    // Get pointer of OSD frame buffer 
    // Note: before get pointer of frame buffer, must set display size and display color depth first
    g_OSD_base = vpostGetOSDBuffer();
    if (g_OSD_base == NULL)
    {
        sysprintf("Get OSD buffer error !!\n");
        return;
    }
    
    // Set scale to 1:1
    vpostOSDScalingCtrl(1, 0, 0);
    
    // Configure overlay function of OSD to display OSD image 
    vpostOSDSetOverlay(DISPLAY_OSD, DISPLAY_OSD, 0);
    
    // Enable color key function
    vpostOSDSetColKey(0, 0, 0);

    // Start video and OSD
    vpostVAStartTrigger();
    vpostOSDEnable();
}

void Decode_JPEG_Image(UINT8 *image_buf, int image_len)
{
    jpegInit();
    
    jpegIoctl(JPEG_IOCTL_SET_BITSTREAM_ADDR, (UINT32)image_buf, 0); 
    
    jpegIoctl(JPEG_IOCTL_SET_DECODE_MODE, JPEG_DEC_PRIMARY_PACKET_YUV422, 0);
    
    jpegIoctl(JPEG_IOCTL_SET_YADDR, (UINT32)g_OSD_base, 0);     
    
    jpegIoctl(JPEG_IOCTL_DECODE_TRIGGER, 0, 0);  
        
    /* Wait for complete */
    if (jpegWait())
        ;  // sysprintf(".");
    else
        sysprintf("\tJPEG Decode Error!!\n");
}

int Encode_JPEG_Image(UINT8 *src_image_buf, int src_image_len, UINT8 *dst_image_buf, int *dst_image_len)
{
    JPEG_INFO_T   jpegInfo;
    
    *dst_image_len = 0;
    
    /* JPEG Init */
    jpegInit(); 
    
    /* Set Source Address */    
    jpegIoctl(JPEG_IOCTL_SET_YADDR, (UINT32)src_image_buf, 0);
            
    /* Set Source Y/U/V Stride */          
    jpegIoctl(JPEG_IOCTL_SET_YSTRIDE, SELECT_RES_WIDTH, 0);
    jpegIoctl(JPEG_IOCTL_SET_USTRIDE, SELECT_RES_WIDTH/2, 0);
    jpegIoctl(JPEG_IOCTL_SET_VSTRIDE, SELECT_RES_WIDTH/2, 0);
                                                            
    /* Set Bit stream Address */   
    jpegIoctl(JPEG_IOCTL_SET_BITSTREAM_ADDR, (UINT32)dst_image_buf, 0);

    /* Encode mode, encoding primary image, YUV 4:2:2/4:2:0 */  
    jpegIoctl(JPEG_IOCTL_SET_ENCODE_MODE, JPEG_ENC_SOURCE_PACKET, JPEG_ENC_PRIMARY_YUV422);
    
    /* Primary Encode Image Width / Height */    
    jpegIoctl(JPEG_IOCTL_SET_DIMENSION, SELECT_RES_HEIGHT, SELECT_RES_WIDTH);
       
    //Set Encode Source Image Height        
    jpegIoctl(JPEG_IOCTL_SET_SOURCE_IMAGE_HEIGHT, SELECT_RES_HEIGHT, 0);

    /* Include Quantization-Table and Huffman-Table */  
    jpegIoctl(JPEG_IOCTL_ENC_SET_HEADER_CONTROL, JPEG_ENC_PRIMARY_QTAB | JPEG_ENC_PRIMARY_HTAB, 0);
       
    /* Use the default Quantization-table 0, Quantization-table 1 */
    jpegIoctl(JPEG_IOCTL_SET_DEFAULT_QTAB, 0, 0);

    //starttime = sysGetTicks(TIMER0);  
    
    /* Trigger JPEG encoder */
    jpegIoctl(JPEG_IOCTL_ENCODE_TRIGGER, 0, 0);  
    
    /* Wait for complete */
    if (jpegWait())
    {
        //endtime = sysGetTicks(TIMER0);    
        jpegGetInfo(&jpegInfo);
        sysprintf("\nJpeg encode image Size = %d\n", jpegInfo.image_size[0]);   
        *dst_image_len = jpegInfo.image_size[0];        
        return 0;
    }
    else
    {
        sysprintf("\tJPEG Encode Error!!\n");
        *dst_image_len = 0;
        return -1;
    }
}


void  init_image_buffers(void)
{
    int   i;
    for (i = 0; i < IMAGE_BUFF_CNT; i++)
    {
        _ig[i].buff   = (uint8_t *)((uint32_t)image_buff_pool[i] | 0x80000000);
        _ig[i].len    = 0;
        _ig[i].state  = IMAGE_BUFF_FREE;
    }
    _idx_usb = 0;
    _idx_post = 0;
}


int  uvc_rx_callbak(UVC_DEV_T *vdev, uint8_t *data, int len)
{
    int  next_idx;
    
    //sysprintf("RX: %d\n", len);
    _total_frame_count++;
    
    next_idx = (_idx_usb+1) % IMAGE_BUFF_CNT;
    
    if (_ig[next_idx].state != IMAGE_BUFF_FREE)
    {
        /*
         *  Next image buffer is in used.
         *  Just drop this newly received image and reuse the same image buffer.
         */
        // sysprintf("Drop!\n");
        usbh_uvc_set_video_buffer(vdev, _ig[_idx_usb].buff, IMAGE_MAX_SIZE);
    }
    else
    {
        _ig[_idx_usb].state = IMAGE_BUFF_READY;   /* mark the current buffer as ready for decode/display */
        _ig[_idx_usb].len   = len;                /* length of this newly received image   */
        
        /* proceed to the next image buffer */
        _idx_usb = next_idx;
        _ig[_idx_usb].state = IMAGE_BUFF_USB;     /* mark the next image as used by USB    */

        /* assign the next image buffer to receive next image from USB */
        usbh_uvc_set_video_buffer(vdev, _ig[_idx_usb].buff, IMAGE_MAX_SIZE);
    }
    return 0;
}


void show_menu()
{
    sysprintf("\n\n+---------------------------------------------+\n");
    sysprintf("|  Operation menu                             |\n"); 
    sysprintf("+---------------------------------------------+\n");
    sysprintf("|  [1] Stop video streaming                   |\n");
    sysprintf("|  [2] Start video streaming                  |\n");
    sysprintf("|  [3] USB Power-down                         |\n");
    sysprintf("|  [4] Resume USB                             |\n");
    sysprintf("|  [5] Snapshot a JPEG picture                |\n");
    sysprintf("|  [6] Decode and post JPEG snapshot          |\n");
    sysprintf("+---------------------------------------------+\n\n");
    usbh_memory_used();
    sysprintf("[0x%x] [0x%x] is_streaming = %d,\n",  HSUSBH->UPSCR[0], HSUSBH->UPSCR[1], g_vdev->is_streaming);
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int32_t main(void)
{
    UVC_DEV_T       *vdev;
    IMAGE_FORMAT_E  format;
    int             i, width, height;   
    int             t_last = 0, cnt_last = 0;
    int             command, in_suspend = 0; 
    uint8_t         *snapshot_buff;
    int             snapshot_len = 0, do_sanpshot = 0, post_snapshot_time = 0;
    int             t0, ret;
    
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | 0x40000);
    outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 0x10000);
    
    snapshot_buff = (uint8_t *)((uint32_t)&snapshot_buff_pool | 0x80000000);

#ifdef USB_PPWR_PE
    // set PE.14 & PE.15 for USBH_PPWR0 & USBH_PPWR1
    outpw(REG_SYS_GPE_MFPH, (inpw(REG_SYS_GPE_MFPH) & ~0xff000000) | 0x77000000);
#else
    // set PF.10 for USBH_PPWR
    outpw(REG_SYS_GPF_MFPH, (inpw(REG_SYS_GPF_MFPH) & ~0x00000f00) | 0x00000700); 
#endif

    sysprintf("\n\n");
    sysprintf("+--------------------------------------------+\n");
    sysprintf("|                                            |\n");
    sysprintf("|    USB Host Video Class sample program     |\n");
    sysprintf("|                                            |\n");
    sysprintf("+--------------------------------------------+\n");

    /*--- init timer ---*/
    sysSetTimerReferenceClock (TIMER0, 12000000);
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);
    
    init_lcd();
    
    sysprintf("LCD buffer base is: 0x%x\n", g_LCD_base);
    sysprintf("OSD buffer base is: 0x%x\n", g_OSD_base);
    
    if (g_LCD_base != NULL)
        memset(g_LCD_base, 0, 800*480*2);   // clear LCD screen
        
    if (g_OSD_base == NULL)
    {
        sysprintf("Failed to allocate OSD buffer!\n");
        while (1);
    }

    jpegOpen();
    
    usbh_core_init();    
    usbh_uvc_init();

    while(1) 
    {
        if (usbh_pooling_hubs())       /* USB Host port detect polling and management     */
        {            
            /*
             *  Has hub port event.
             */
            vdev = usbh_uvc_get_device_list();
            if (vdev == NULL)
            {
                g_vdev = NULL;
                t_last = 0;
                cnt_last = 0;
                _total_frame_count = 0;
                sysprintf("\n[No device connected]\n\n");
                continue;
            }

            if (g_vdev == vdev)
            {
                sysprintf("\n\n\nWaiting for UVC device connected...\n");
                continue;
            }
                
            if (vdev->next != NULL)
            {
                sysprintf("Warning!! Multiple UVC device is not supported!!\n");
                while (1);
            }
            
            /*----------------------------------------------------------------------------*/
            /*  New UVC device connected.                                                 */
            /*----------------------------------------------------------------------------*/
            g_vdev = vdev;
            sysprintf("\n\n----------------------------------------------------------\n");
            sysprintf("[Video format list]\n");
            for (i = 0; ;i++)
            {
                ret = usbh_get_video_format(g_vdev, i, &format, &width, &height);
                if (ret != 0)
                    break;

                sysprintf("[%d] %s, %d x %d\n", i, (format == UVC_FORMAT_MJPEG ? "MJPEG" : "YUYV"), width, height);
            }
            sysprintf("\n\n");

#ifdef SELECT_MJPEG            
            ret = usbh_set_video_format(g_vdev, UVC_FORMAT_MJPEG, SELECT_RES_WIDTH, SELECT_RES_HEIGHT);
#else            
            ret = usbh_set_video_format(g_vdev, UVC_FORMAT_YUY2, SELECT_RES_WIDTH, SELECT_RES_HEIGHT);
#endif
            if (ret != 0)
                sysprintf("usbh_set_video_format failed! - 0x%x\n", ret);
            
            init_image_buffers();
            
            /* assign the first image buffer to receive the image from USB */
            usbh_uvc_set_video_buffer(vdev, _ig[_idx_usb].buff, IMAGE_MAX_SIZE);
            _ig[_idx_usb].state = IMAGE_BUFF_USB;
            
            ret = usbh_uvc_start_streaming(g_vdev, uvc_rx_callbak);
            if (ret != 0)
            {
                sysprintf("usbh_uvc_start_streaming failed! - %d\n", ret);
                sysprintf("Please re-connect UVC device...\n"); 
            }
            else    
                show_menu();
        }
        
        if (_ig[_idx_post].state == IMAGE_BUFF_READY)
        {
            _ig[_idx_post].state = IMAGE_BUFF_POST;

            if (do_sanpshot)
            {
#ifdef SELECT_MJPEG
                memcpy(snapshot_buff, _ig[_idx_post].buff, _ig[_idx_post].len);
                snapshot_len = _ig[_idx_post].len;
#else
                ret = Encode_JPEG_Image(_ig[_idx_post].buff, _ig[_idx_post].len, snapshot_buff, &snapshot_len);
                if (ret == 0)
                    sysprintf("Snapshot image encode done.\n");
#endif
                do_sanpshot = 0;
            }
            
            if (post_snapshot_time != 0)
            {
                if (get_ticks() - post_snapshot_time > SNAPSHOT_POST_TIME) 
                    post_snapshot_time = 0;
            }
            else
            {
#ifdef SELECT_MJPEG
                Decode_JPEG_Image(_ig[_idx_post].buff, _ig[_idx_post].len);
#else
                memcpy(g_OSD_base, _ig[_idx_post].buff, _ig[_idx_post].len);
#endif
            }
            _ig[_idx_post].state = IMAGE_BUFF_FREE;
            _idx_post = (_idx_post + 1) % IMAGE_BUFF_CNT;
        }

        if (g_vdev == NULL)
            continue;

        if (sysGetTicks(TIMER0) - t_last > 100)
        {
            cnt_last = _total_frame_count - cnt_last;
            
            sysprintf("Frame rate: %d, Total: %d        \r", (cnt_last*100)/(sysGetTicks(TIMER0) - t_last), _total_frame_count, HSUSBH->UPFLBAR, HSUSBH->UCALAR); 
            
            t_last = sysGetTicks(TIMER0);
            cnt_last = _total_frame_count;
        }        

        if (!sysIsKbHit())
            continue;
            
        command = sysGetChar();
        
        sysprintf("\n\nInput command [%c]\n", command);
        
        switch (command)
        {
            case '1':
                if (!g_vdev->is_streaming)
                    break;
                ret = usbh_uvc_stop_streaming(g_vdev);
                if (ret != 0)
                    sysprintf("\nusbh_uvc_stop_streaming failed! - %d\n", ret);
                break;

            case '2':
                if (g_vdev->is_streaming)
                    break;
                ret = usbh_uvc_start_streaming(g_vdev, uvc_rx_callbak);
                if (ret != 0)
                    sysprintf("\nusbh_uvc_start_streaming failed! - %d\n", ret);
                break;

            case '3':
                if (in_suspend)
                    break;
                usbh_uvc_stop_streaming(g_vdev);
                t0 = get_ticks();
                while (get_ticks() - t0 < 10);      
                usbh_suspend();
#ifdef USB_PPWR_PE 
                outpw(REG_GPIOE_DATAOUT, inpw(REG_GPIOE_DATAOUT) & 0x3FFF);   /* make PE.14 & PE.15 output low     */
                outpw(REG_GPIOE_DIR, inpw(REG_GPIOE_DIR) | 0xC000);           /* set PE.14 & PE.15 as output mode  */
                outpw(REG_SYS_GPE_MFPH, inpw(REG_SYS_GPE_MFPH) & 0x00FFFFFF); /* set PE.14 & PE.15 as GPIO mode    */
#else                
                outpw(REG_GPIOF_DATAOUT, inpw(REG_GPIOF_DATAOUT) & 0xFBFF);   /* make PF.10 output low             */
                outpw(REG_GPIOF_DIR, inpw(REG_GPIOF_DIR) | 0x0400);           /* set PF.10 as output mode          */
                outpw(REG_SYS_GPF_MFPH, inpw(REG_SYS_GPF_MFPH) & 0xFFFFF0FF); /* set PF.10 as GPIO mode            */
#endif
                in_suspend = 1;
                break;
            
            case '4':
                if (!in_suspend)
                    break;
                /* Release PPWR pin control back to Host Controller */
#ifdef USB_PPWR_PE 
                outpw(REG_SYS_GPE_MFPH, inpw(REG_SYS_GPE_MFPH) | 0x77000000); /* set PE.14 & PE.15 as USBH PPWR   */
#else                
                outpw(REG_SYS_GPF_MFPH, inpw(REG_SYS_GPF_MFPH) | 0x00000700); /* set PF.10 as USBH PPWR           */
#endif
                usbh_resume();
                ret = usbh_uvc_start_streaming(g_vdev, uvc_rx_callbak);
                if (ret != 0)
                    sysprintf("\nusbh_uvc_start_streaming failed! - %d\n", ret);
                    
                in_suspend = 0;
                break;

            case '5':
                if (!g_vdev->is_streaming)
                    break;
                do_sanpshot = 1;
                break;

            case '6':
                if (snapshot_len == 0)
                    break;
                Decode_JPEG_Image(snapshot_buff, snapshot_len);
                post_snapshot_time = get_ticks();
                break;

            default:
                break;
        }
        show_menu();
   }
}


/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/
