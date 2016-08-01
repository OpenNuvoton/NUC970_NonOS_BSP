/**************************************************************************//**
* @file     main.c
* @version  V1.00
* $Revision: 1 $
* $Date: 16/02/18 5:54p $
* @brief    NUC970 LCD sample source file for ILI9431 MPU 80 mode
*
* @note
* Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "lcd.h"
#include "gpio.h"

#include "image_rgb565_240x320.dat"

void ILI9341_LCD_SetAddress(uint32_t x1,uint32_t x2,uint32_t y1,uint32_t y2)
{
    vpostMPUWriteAddr(0x2a);
    vpostMPUWriteData(x1>>8);
    vpostMPUWriteData(x1);
    vpostMPUWriteData(x2>>8);
    vpostMPUWriteData(x2);

    vpostMPUWriteAddr(0x2b);
    vpostMPUWriteData(y1>>8);
    vpostMPUWriteData(y1);
    vpostMPUWriteData(y2>>8);
    vpostMPUWriteData(y2);
}

void ILI9341_LCD_Init(void)
{
	/* Configure PD3 to output mode */
    GPIO_OpenBit(GPIOG, BIT3, DIR_OUTPUT, NO_PULL_UP);
	GPIO_SetBit(GPIOG, BIT3);
	
	outpw(REG_LCM_DCCS,inpw(REG_LCM_DCCS) | VPOSTB_DISP_OUT_EN); //display_out-enable

    vpostMPUWriteAddr(0xCB);
    vpostMPUWriteData(0x39);
    vpostMPUWriteData(0x2C);
    vpostMPUWriteData(0x00);
    vpostMPUWriteData(0x34);
    vpostMPUWriteData(0x02);

    vpostMPUWriteAddr(0xCF);
    vpostMPUWriteData(0x00);
    vpostMPUWriteData(0xC1);
    vpostMPUWriteData(0x30);

    vpostMPUWriteAddr(0xE8);
    vpostMPUWriteData(0x85);
    vpostMPUWriteData(0x00);
    vpostMPUWriteData(0x78);

    vpostMPUWriteAddr(0xEA);
    vpostMPUWriteData(0x00);
    vpostMPUWriteData(0x00);

    vpostMPUWriteAddr(0xED);
    vpostMPUWriteData(0x64);
    vpostMPUWriteData(0x03);
    vpostMPUWriteData(0x12);
    vpostMPUWriteData(0x81);

    vpostMPUWriteAddr(0xF7);
    vpostMPUWriteData(0x20);

    vpostMPUWriteAddr(0xC0);
    vpostMPUWriteData(0x23);

    vpostMPUWriteAddr(0xC1);
    vpostMPUWriteData(0x10);

    vpostMPUWriteAddr(0xC5);
    vpostMPUWriteData(0x3e);
    vpostMPUWriteData(0x28);

    vpostMPUWriteAddr(0xC7);
    vpostMPUWriteData(0x86);

    vpostMPUWriteAddr(0x36);
    vpostMPUWriteData(0x48);

    vpostMPUWriteAddr(0x3A);
    vpostMPUWriteData(0x55);

    vpostMPUWriteAddr(0xB1);
    vpostMPUWriteData(0x00);
    vpostMPUWriteData(0x18);

    vpostMPUWriteAddr(0xB6);
    vpostMPUWriteData(0x08);
    vpostMPUWriteData(0x82);
    vpostMPUWriteData(0x27);

    vpostMPUWriteAddr(0xF2);
    vpostMPUWriteData(0x00);

    vpostMPUWriteAddr(0x26);
    vpostMPUWriteData(0x01);

    vpostMPUWriteAddr(0xE0);
    vpostMPUWriteData(0x0F);
    vpostMPUWriteData(0x31);
    vpostMPUWriteData(0x2B);
    vpostMPUWriteData(0x0C);
    vpostMPUWriteData(0x0E);
    vpostMPUWriteData(0x08);
    vpostMPUWriteData(0x4E);
    vpostMPUWriteData(0xF1);
    vpostMPUWriteData(0x37);
    vpostMPUWriteData(0x07);
    vpostMPUWriteData(0x10);
    vpostMPUWriteData(0x03);
    vpostMPUWriteData(0x0E);
    vpostMPUWriteData(0x09);
    vpostMPUWriteData(0x00);

    vpostMPUWriteAddr(0xE1);
    vpostMPUWriteData(0x00);
    vpostMPUWriteData(0x0E);
    vpostMPUWriteData(0x14);
    vpostMPUWriteData(0x03);
    vpostMPUWriteData(0x11);
    vpostMPUWriteData(0x07);
    vpostMPUWriteData(0x31);
    vpostMPUWriteData(0xC1);
    vpostMPUWriteData(0x48);
    vpostMPUWriteData(0x08);
    vpostMPUWriteData(0x0F);
    vpostMPUWriteData(0x0C);
    vpostMPUWriteData(0x31);
    vpostMPUWriteData(0x36);
    vpostMPUWriteData(0x0F);

    vpostMPUWriteAddr(0x11);
	sysDelay(6);
	
    vpostMPUWriteAddr(0x29);    //Display on
	
	//LED ON
    GPIO_ClrBit(GPIOG, BIT3);
}

int32_t main(void)
{	
	uint8_t *u8FrameBufPtr, i;    
    
	*((volatile unsigned int *)REG_AIC_MDCR)=0xFFFFFFFF;  // disable all interrupt channel
	*((volatile unsigned int *)REG_AIC_MDCRH)=0xFFFFFFFF;  // disable all interrupt channel
	
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
    
	//DATA pin (18bit)
	//GPA0 ~ GPA7 (DATA0~7)
    outpw(REG_SYS_GPA_MFPL, 0x22222222);
	//GPA8 ~ GPA15 (DATA8~15)	
    outpw(REG_SYS_GPA_MFPH, 0x22222222);
	//GPD8~D15 (DATA16~17)
    outpw(REG_SYS_GPD_MFPH, (inpw(REG_SYS_GPD_MFPH)& ~0xFF) | 0x22);
    
	// LCD clock is selected from UPLL and divide to 20MHz
	outpw(REG_CLK_DIVCTL1, (inpw(REG_CLK_DIVCTL1) & ~0xFF1F) | 0x1018);		
    
	// Init LCD interface for ILI9341 LCD module
	vpostLCMInit(DIS_PANEL_ILI9341_MPU80);
	// Set scale to 1:1
	vpostVAScalingCtrl(1, 0, 1, 0, VA_SCALE_INTERPOLATION);
	
	// Set display color depth
    vpostSetVASrc(VA_SRC_RGB565);
	
	// Get pointer of video frame buffer
    // Note: before get pointer of frame buffer, must set display color depth first
	u8FrameBufPtr = vpostGetFrameBuffer();
    if(u8FrameBufPtr == NULL)
	{
		sysprintf("Get buffer error !!\n");
		return 0;
	}
	
	// Init ILI9341
	ILI9341_LCD_Init();
	
    /* Setup display address */
    ILI9341_LCD_SetAddress(0, 239, 0, 319);
	
	/* Memory write */
	vpostMPUWriteAddr(0x2c);
	
    // Prepare image
    memcpy((void *)u8FrameBufPtr, (void *)&video_img[0], 240*320*2);
    
	// Start video
	vpostVAStartTrigger();
 
    while(1);
}
/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/

