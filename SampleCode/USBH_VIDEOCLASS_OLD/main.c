/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 15/05/21 10:47a $
 * @brief    This sample shows how to manage several USB HID class devices.
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "lcd.h"
#include "usbh_lib.h"


uint8_t  * g_OSD_base = 0;
uint8_t  * g_LCD_base = 0;

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
    vpostOSDSetWindow(80, 0, 640, 480);    // 640x480 image 
    
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



/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int32_t main(void)
{
	unsigned char	*image_buf;
	int 			image_len; 
	int    			handle, image_rcv_count;
	int             t_last = 0, cnt_last = 0;
	int             status;
	
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

	outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | 0x40000);
	outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 0x10000);

	// set PE.14 & PE.15 for USBH_PPWR0 & USBH_PPWR1
	outpw(REG_SYS_GPE_MFPH, (inpw(REG_SYS_GPE_MFPH) & ~0xff000000) | 0x77000000);

    sysprintf("\n\n");
    sysprintf("+--------------------------------------------+\n");
    sysprintf("|                                            |\n");
    sysprintf("|     USB Host HID class sample program      |\n");
    sysprintf("|                                            |\n");
    sysprintf("+--------------------------------------------+\n");

	/*--- init timer ---*/
	sysSetTimerReferenceClock (TIMER0, 12000000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);
	
	init_lcd();
	
	sysprintf("LCD buffer base is: 0x%x\n", g_LCD_base);
	sysprintf("OSD buffer base is: 0x%x\n", g_OSD_base);
	
	if (g_LCD_base != NULL)
		memset(g_LCD_base, 0, 800*480*2);	// clear LCD screen
		
	if (g_OSD_base == NULL)
	{
		sysprintf("Failed to allocate OSD buffer!\n");
		while (1);
	}
	
	usbh_core_init();    
	usbh_uvc_init();
	usbh_pooling_hubs();
	
    while (1)
    {
        if (usbh_pooling_hubs())             /* USB Host port detect polling and management */
        {
        	sysprintf("\n Has hub events.\n");
        	if (usbh_uvc_is_device_found())
        	{
        		status = usbh_uvc_open(UVC_FORMAT_YUYV);
        		if (status != UVC_OK)
        		{
        			sysprintf("Failed to select YUV image format!\n");
        			continue;
        		}	
        		
        		status = usbh_uvc_set_format(640, 480, UVC_FORMAT_YUYV);
        		if (status != UVC_OK)
        		{
        			sysprintf("Failed to set 320x240 YUV image format!\n");
        			continue;
        		}	
        		
        		status = usbh_uvc_set_frame_rate(30);
        		if (status != UVC_OK)
        		{
        			sysprintf("Failed to set frame rate!\n");
        			continue;
        		}	
        		
        		usbh_uvc_start_video_streaming();
        	}
        	else
        	{
        		t_last = 0;
        		cnt_last = 0;
        		image_rcv_count = 0;
    		}
        }
        
		if (usbh_uvc_query_image(&image_buf, &image_len, &handle) == 0)
		{
			image_rcv_count++;
			//sysprintf("IMG - len:%d\n", image_len);
			memcpy(g_OSD_base, image_buf, image_len);
			usbh_uvc_release_image(handle);
		}

		if (sysGetTicks(TIMER0) - t_last > 100)
		{
			cnt_last = image_rcv_count - cnt_last;
			
			sysprintf("Frame rate: %d, Total: %d\n", (cnt_last*100)/(sysGetTicks(TIMER0) - t_last), image_rcv_count); 
			
			t_last = sysGetTicks(TIMER0);
			cnt_last = image_rcv_count;
		}        
    }
}


/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
