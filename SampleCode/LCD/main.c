/**************************************************************************//**
* @file     main.c
* @version  V1.00
* $Revision: 2 $
* $Date: 15/06/12 9:11a $
* @brief    NUC970 LCD sample source file
*
* @note
* Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "lcd.h"

#define DISPLAY_RGB565
//#define DISPLAY_RGB888

#ifdef DISPLAY_RGB888
#include "image_rgb888.dat"
#include "image_rgb888_320x240.dat"
#else
#include "image_rgb565.dat"
#include "image_rgb565_320x240.dat"
#endif
#if defined ( __GNUC__ ) && !(__CC_ARM)
__attribute__((aligned(32))) uint32_t u32CursorBuf[512];
#else
__align(32) uint32_t u32CursorBuf[512];
#endif
int32_t main(void)
{	
	uint8_t *u8FrameBufPtr, *u8OSDFrameBufPtr, i;    

	outpw(REG_CLK_HCLKEN, 0x0527);
	outpw(REG_CLK_PCLKEN0, 0);
	outpw(REG_CLK_PCLKEN1, 0);

	sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

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
	
	// Set display color depth
#ifdef DISPLAY_RGB888
	vpostSetVASrc(VA_SRC_RGB888);
#else	
    vpostSetVASrc(VA_SRC_RGB565);
#endif
	
	// Get pointer of video frame buffer
    // Note: before get pointer of frame buffer, must set display color depth first
	u8FrameBufPtr = vpostGetFrameBuffer();
    if(u8FrameBufPtr == NULL)
	{
		sysprintf("Get buffer error !!\n");
		return 0;
	}
    
	// Set OSD position and display size
    vpostOSDSetWindow(240, 120, 320, 240);
	
	// Set OSD color depth
#ifdef DISPLAY_RGB888
    vpostSetOSDSrc(OSD_SRC_RGB888);
#else
    vpostSetOSDSrc(OSD_SRC_RGB565);
#endif
	// Get pointer of OSD frame buffer 
    // Note: before get pointer of frame buffer, must set display size and display color depth first
    u8OSDFrameBufPtr = vpostGetOSDBuffer();
	if(u8OSDFrameBufPtr == NULL)
	{
		sysprintf("Get OSD buffer error !!\n");
		return 0;
	}
	
	// Set scale to 1:1
    vpostOSDScalingCtrl(1, 0, 0);
	
	// Configure overlay function of OSD to display OSD image 
    vpostOSDSetOverlay(DISPLAY_OSD, DISPLAY_OSD, 0);
	
	// Enable color key function
    vpostOSDSetColKey(0, 0, 0);
        
    // Prepare image
#ifdef DISPLAY_RGB888
	memcpy((void *)u8FrameBufPtr, (void *)&video_img[0], 800*480*4);
    memcpy((void *)u8OSDFrameBufPtr, (void *)&osd_img[0], 320*240*4);
#else
    memcpy((void *)u8FrameBufPtr, (void *)&video_img[0], 800*480*2);
    memcpy((void *)u8OSDFrameBufPtr, (void *)&osd_img[0], 320*240*2);
#endif
    
	// Prepare hardware cursor image (color bar)
    for (i=0;i<16;i++)
    {
    	u32CursorBuf[i] = 0x00;
        u32CursorBuf[i+16*1] = 0x55555555;
        u32CursorBuf[i+16*2] = 0xaaaaaaaa;
        u32CursorBuf[i+16*3] = 0xffffffff;
        u32CursorBuf[i+16*4] = 0x00;
        u32CursorBuf[i+16*5] = 0x55555555;
        u32CursorBuf[i+16*6] = 0xaaaaaaaa;
        u32CursorBuf[i+16*7] = 0xffffffff;
        u32CursorBuf[i+16*8] = 0x00;
        u32CursorBuf[i+16*9] = 0x55555555;
        u32CursorBuf[i+16*10] = 0xaaaaaaaa;
        u32CursorBuf[i+16*11] = 0xffffffff;
        u32CursorBuf[i+16*12] = 0x00;
        u32CursorBuf[i+16*13] = 0x55555555;
        u32CursorBuf[i+16*14] = 0xaaaaaaaa;
        u32CursorBuf[i+16*15] = 0xffffffff;
        u32CursorBuf[i+16*16] = 0x00;
        u32CursorBuf[i+16*17] = 0x55555555;
        u32CursorBuf[i+16*18] = 0xaaaaaaaa;
        u32CursorBuf[i+16*19] = 0xffffffff;
        u32CursorBuf[i+16*20] = 0x00;
        u32CursorBuf[i+16*21] = 0x55555555;
        u32CursorBuf[i+16*22] = 0xaaaaaaaa;
        u32CursorBuf[i+16*23] = 0xffffffff;
        u32CursorBuf[i+16*24] = 0x00;
        u32CursorBuf[i+16*25] = 0x55555555;
        u32CursorBuf[i+16*26] = 0xaaaaaaaa;
        u32CursorBuf[i+16*27] = 0xffffffff;
        u32CursorBuf[i+16*28] = 0x00;
        u32CursorBuf[i+16*29] = 0x55555555;
        u32CursorBuf[i+16*30] = 0xaaaaaaaa;
        u32CursorBuf[i+16*31] = 0xffffffff;
    }
    
	// Start video and OSD
	vpostVAStartTrigger();
	vpostOSDEnable();
    
	// Start hardware cursor
	vpostHCInit(u32CursorBuf, HC_MODE0);
    // Set hardware cursor position	
	vpostHCPosCtrl(50, 50);
    
    while(1);
}
/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/

