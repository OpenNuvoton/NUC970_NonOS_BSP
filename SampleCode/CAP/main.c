/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 16/04/01 4:14p $
 * @brief    NUC970 Driver Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "stdio.h"
#include "string.h"
#include "nuc970.h"
#include "sys.h"
#include "cap.h"
#include "lcd.h"

extern int InitNT99141_VGA(void);
extern int InitNT99050_VGA(void);
extern int InitHM1055_VGA(void);
extern CAPDEV_T CAP;
CAPDEV_T* pCAP;

#define OPT_LCD_WIDTH 800
#define OPT_LCD_HEIGHT 480

#define OPT_INFMT_HEIGHT 480
#define OPT_INFMT_WIDTH  640

#define OPT_PACKET_STRIDE 800
#define OPT_PACKET_WIDTH  640
#define OPT_PACKET_HEIGHT 480

#define OPT_PLANAR_STRIDE 640
#define OPT_PLANAR_WIDTH  640
#define OPT_PLANAR_HEIGHT 480

#define OPT_CROP_HEIGHT 640
#define OPT_CROP_WIDTH 480

#if defined ( __GNUC__ ) && !(__CC_ARM)
//__attribute__((aligned(4))) UINT8 u8PacketFrameBuffer[OPT_PACKET_WIDTH*OPT_PACKET_HEIGHT*2];		//Keep 640*480*2 RGB565 frame buffer
__attribute__((aligned(256))) UINT8 u8PlanarFrameBuffer[OPT_PLANAR_WIDTH*OPT_PLANAR_HEIGHT*2];		//Keep 640x480*2 PlanarYUV422 frame buffer
#else
 //__align(4) UINT8 u8PacketFrameBuffer[OPT_PACKET_WIDTH*OPT_PACKET_HEIGHT*2];		//Keep 640*480*2 RGB565 frame buffer
 __align(256) UINT8 u8PlanarFrameBuffer[OPT_PLANAR_WIDTH*OPT_PLANAR_HEIGHT*2];		//Keep 640x480*2 PlanarYUV422 frame buffer
#endif
/*------------------------------------------------------------------------------------------*/
/* To run CAP_InterruptHandler, when CAP frame end interrupt                                */
/*------------------------------------------------------------------------------------------*/
volatile uint32_t u32FramePass = 0;
void CAP_InterruptHandler(void)
{
    u32FramePass++;
}
#if 1
void VGA_Demo(unsigned int u8PacketFrameBuffer)
{
  uint32_t u32Frame;

  PFN_CAP_CALLBACK pfnOldCallback;

  //Enable frame end interrupt
  pCAP->EnableInt(eCAP_VINTF);
  
  //Frame End interrupt
  pCAP->InstallCallback(eCAP_VINTF, (PFN_CAP_CALLBACK)CAP_InterruptHandler,&pfnOldCallback);

  //Configure packet frame buffer to use CAP_PKTBA0 register.
  pCAP->SetPacketFrameBufferControl(0);

  //Set data format and order
  pCAP->SetDataFormatAndOrder(eCAP_IN_YUYV,eCAP_IN_YUV422,eCAP_OUT_RGB565);

  //Set cropping window start address
  pCAP->SetCropWinStartAddr(0,0);

  //standard CCIR656 mode
  pCAP->SetStandardCCIR656(FALSE);

  //Set sensor polarity
  pCAP->SetSensorPolarity(FALSE,FALSE,TRUE);

  //Set Cropping window size
  pCAP->SetCropWinSize(OPT_PACKET_HEIGHT,OPT_PACKET_WIDTH);

  //Set Packet/ Planar Stride.
  pCAP->SetStride(OPT_PACKET_STRIDE, OPT_PLANAR_STRIDE);

  //Packet buffer address.
  pCAP->SetBaseStartAddress(eCAP_PACKET,(E_CAP_BUFFER)0,(UINT32)u8PacketFrameBuffer);

  //Planar buffer Y addrress
  pCAP->SetBaseStartAddress(eCAP_PLANAR,(E_CAP_BUFFER)0,(UINT32)u8PlanarFrameBuffer); 

  //Planar buffer U addrress
  pCAP->SetBaseStartAddress(eCAP_PLANAR,(E_CAP_BUFFER)1,(UINT32)u8PlanarFrameBuffer+OPT_PLANAR_WIDTH*OPT_PLANAR_HEIGHT); 

  //Planar buffer V addrress
  pCAP->SetBaseStartAddress(eCAP_PLANAR,(E_CAP_BUFFER)2,(UINT32)u8PlanarFrameBuffer+OPT_PLANAR_WIDTH*OPT_PLANAR_HEIGHT+OPT_PLANAR_WIDTH*OPT_PLANAR_HEIGHT/2);

  //Planar YUV422/420/macro
  pCAP->SetPlanarFormat(eCAP_PLANAR_YUV422);

  //Enable Packet/Planar pipe
  pCAP->SetPipeEnable(TRUE,eCAP_BOTH_PIPE_ENABLE);

  sysSetLocalInterrupt(ENABLE_IRQ);

  u32Frame=u32FramePass;
  while(1) {
      if(u32Frame!=u32FramePass) {
          u32Frame=u32FramePass;
          sysprintf("Get frame %3d\n",u32Frame);
      }
  }
}
#endif
/*-----------------------------------------------------------------------------*/
uint8_t* LCD_RGB565_Init(int h,int w)
{
  uint8_t *u8FrameBufPtr;
  /* GPG6 (CLK), GPG7 (HSYNC) */
  outpw(REG_SYS_GPG_MFPL, (inpw(REG_SYS_GPG_MFPL)& ~0xFF000000) | 0x22000000);
  /* GPG8 (VSYNC), GPG9 (DEN) */
  outpw(REG_SYS_GPG_MFPH, (inpw(REG_SYS_GPG_MFPH)& ~0xFF) | 0x22);

  /* DATA pin */
  /* GPA0 ~ GPA7 (DATA0~7) */
  outpw(REG_SYS_GPA_MFPL, 0x22222222);
  /* GPA8 ~ GPA15 (DATA8~15) */
  outpw(REG_SYS_GPA_MFPH, 0x22222222);
  /* GPD8~D15 (DATA16~23) */
  outpw(REG_SYS_GPD_MFPH, (inpw(REG_SYS_GPD_MFPH)& ~0xFFFFFFFF) | 0x22222222);
    
  outpw(REG_CLK_APLLCON, 0xc0004018);
  outpw(REG_CLK_DIVCTL1, (inpw(REG_CLK_DIVCTL1) & ~0xff1f) | 0xe18);		//APLL 20MHz output

  vpostLCMInit(DIS_PANEL_E50A2V1);
  vpostVAScalingCtrl(1,0,1,0,VA_SCALE_INTERPOLATION);

  vpostSetVASrc(VA_SRC_RGB565);

  u8FrameBufPtr = vpostGetFrameBuffer();

  memset(u8FrameBufPtr,0x00,w*h*2);

  vpostVAStartTrigger(); 
    
  return u8FrameBufPtr;
}


int main(void)
{
  uint8_t *u8FrameBufPtr;
  UINT32 u32Item;

  *(volatile unsigned int *)(CLK_BA+0x18) |= (1<<16); /* Enable UART0 */
  sysDisableCache();
  sysFlushCache(I_D_CACHE);
  sysEnableCache(CACHE_WRITE_BACK);
  sysInitializeUART();
  
  u8FrameBufPtr=LCD_RGB565_Init(OPT_LCD_HEIGHT,OPT_LCD_WIDTH);

  pCAP = &CAP;
  pCAP->Init(TRUE, (E_CAP_SNR_SRC)eCAP_SNR_UPLL, 24000);

  sysprintf("\n======================================================\n");
  sysprintf("Please use LCD                                        \n");
  sysprintf("======================================================\n");
  do
  {
    sysprintf("======================================================\n");
    sysprintf(" CAP library demo code                                \n"); 
    sysprintf(" [1] NT99141 VGA                                      \n");
    sysprintf(" [2] NT99050 VGA                                      \n");
    sysprintf(" [3] HM1055 VGA                                       \n");
    sysprintf("======================================================\n");
    u32Item = sysGetChar();
    switch(u32Item)
    {
      case '1':
        pCAP->Open(36000);
        InitNT99141_VGA();
        break;
      case '2':
        pCAP->Open(36000);
        InitNT99050_VGA();
        break;
      case '3':
        pCAP->Open(36000);
        InitHM1055_VGA();
        break;
      default:
        break;
    }
    VGA_Demo((unsigned int)u8FrameBufPtr);
  }while((u32Item!= 'q') || (u32Item!= 'Q'));	
  return 0;
}
