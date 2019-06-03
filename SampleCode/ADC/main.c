/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/05/07 6:35p $
 * @brief    NUC970 Driver Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nuc970.h"
#include "sys.h"
#include "adc.h"
/*-----------------------------------------------------------------------------*/
INT32 TouchXYCallback(UINT32 status, UINT32 userData)
{
/*  The status content that contains Touch x-position and touch y-position.
 *  X-position = (status & 0xFFF);
 *  Y-position = ((status>>16) & 0xFFF);
 */
    return 0;
}

INT32 TouchZCallback(UINT32 status, UINT32 userData)
{
/*  The status content that contains touch pressure measure Z1 and touch pressure measure Z2.
 *  Pressure measure Z1 = (status & 0xFFF);
 *  Pressure measure Z2 = ((status>>16) & 0xFFF);
 */
    return 0;
}

volatile int pendown_complete=0;
INT32 PenDownCallback(UINT32 status, UINT32 userData)
{
    pendown_complete=1;
    return 0;
}

void touch_xy_demo()
{
  short x,y;
  int userdata=0;
  adcIoctl(T_ON,(UINT32)TouchXYCallback,userdata);
  do{
      adcIoctl(START_MST,0,0);
      if(adcReadXY(&x,&y,1))
        sysprintf("\r x = 0x%03x, y = 0x%03x ", x,y);
  }while(1);
}
/*-----------------------------------------------------------------------------*/
void touch_xyz_demo()
{
  short x,y,z1,z2;
  int userdata=0;
  adcIoctl(T_ON,(UINT32)TouchXYCallback,userdata);
  adcIoctl(Z_ON,(UINT32)TouchZCallback,userdata);
  do{
      adcIoctl(START_MST,0,0);
      if(adcReadXY(&x,&y,1))
        sysprintf("\r x = 0x%03x, y = 0x%03x ", x,y);
      if(adcReadZ(&z1,&z2,1))
       sysprintf("z1 = 0x%03x, z2 = 0x%03x ", z1,z2);
  }while(1);
}
/*-----------------------------------------------------------------------------*/
void touch_xyz_pendown_demo()
{
  short x,y,z1,z2;
  int userdata=0;
  adcIoctl(T_ON,(UINT32)TouchXYCallback,userdata);
  adcIoctl(Z_ON,(UINT32)TouchZCallback,userdata);
  adcIoctl(PEDEF_ON,(UINT32)PenDownCallback,userdata);
  do{
      pendown_complete=0;
      adcIoctl(PEPOWER_ON,0,0);
      while(!pendown_complete);
      adcIoctl(PEPOWER_OFF,0,0);
      do{
        adcIoctl(START_MST,0,0);
        if(adcReadXY(&x,&y,1))
          sysprintf("\r x = 0x%03x, y = 0x%03x ", x,y);
        if(adcReadZ(&z1,&z2,1))
         sysprintf("z1 = 0x%03x, z2 = 0x%03x ", z1,z2);
      }while(z1!=0 && z2!=0xFFF);
  }while(1);
}
/*-----------------------------------------------------------------------------*/
volatile int keypadpress_complete=0;
INT32 KeypadPressCallback(UINT32 status, UINT32 userData)
{
  keypadpress_complete=1;
  sysprintf("keypadpress\n");
  return 0;
}

volatile int keypadup_complete=0;
INT32 KeypadUpCallback(UINT32 status, UINT32 userData)
{
  keypadup_complete=1;
  sysprintf("\nkeypadup\n");
  return 0;
}

volatile int keypadconv_complete=0;
INT32 KeypadConvCallback(UINT32 status, UINT32 userData)
{
/*  The status content that contains keypad data.
 */
  keypadconv_complete=1;
  sysprintf("\rkeypad_data=0x%03x",status);
  return 0;
}

void keypad_demo()
{
  int userdata=0;
  adcIoctl(KPPOWER_ON,(UINT32)0,0); //Enable ADC Keypad power
  adcIoctl(KPPRESS_ON,(UINT32)KeypadPressCallback,userdata);
  adcIoctl(KPUP_ON,(UINT32)KeypadUpCallback,userdata);
  adcIoctl(KPCONV_ON,(UINT32)KeypadConvCallback,userdata);
  do{
    if(keypadpress_complete==1)
    {
      keypadpress_complete=0;
      adcIoctl(START_MST,0,0);
    }
  }while(1);
}
/*-----------------------------------------------------------------------------*/
volatile int battrey_complete=0;
INT32 BattreyConvCallback(UINT32 status, UINT32 userData)
{
/*  The status content that contains VBA data.
 *  To measure baterry voltage, It should be 0.5 to 5.5v.
 *  Get VBA voltage, It should be 0.125v to 1.375v.
 */
  unsigned int n;
  int d1,d2;
  int voltage=25; /* 2.5v */
  battrey_complete=1;
  n=(voltage*status*100)>>12;
  d1=n/1000;
  d2=n%1000;
  sysprintf("\rVBA_data=0x%03x,voltage=%3d.%3dv",status,d1,d2);
  return 0;
}
void battery_demo()
{
  int userdata=0;
  adcIoctl(VBPOWER_ON,(UINT32)0,0); //Enable ADC Internal Bandgap Power
  adcIoctl(VBAT_ON,(UINT32)BattreyConvCallback,userdata); //Enable Voltage Battery Conversion
  do{
      adcIoctl(START_MST,0,0);
  }while(1);
}
/*-----------------------------------------------------------------------------*/
volatile int normal_complete=0;
INT32 NormalConvCallback(UINT32 status, UINT32 userData)
{
/*  The status content that contains normal data.
 */
  normal_complete=1;
  sysprintf("\r normal data=0x%3x",status);
  return 0;
}
void normal_demo()
{
  char c;
  int val;
  sysprintf("Select channel 0:VBT(A0), 1:VHS(A1), 2:A2, 3:A3, 4:YM(A4), 5:YP(A5), 6:XM(A6), 7:XP(A7)\n");
  c=sysGetChar();
  switch(c)
  {
    case '0': val=ADC_CONF_CHSEL_VBT; break;
    case '1': val=ADC_CONF_CHSEL_VHS; break;
    case '2': val=ADC_CONF_CHSEL_A2;  break;
    case '3': val=ADC_CONF_CHSEL_A3;  break;
    case '4': val=ADC_CONF_CHSEL_YM;  break;
    case '5': val=ADC_CONF_CHSEL_YP;  break;
    case '6': val=ADC_CONF_CHSEL_XM;  break;
    case '7': val=ADC_CONF_CHSEL_XP;  break;
  }
  adcIoctl(NAC_ON,(UINT32)NormalConvCallback,0); //Enable Normal AD Conversion
  adcChangeChannel(val);
  do{
      adcIoctl(START_MST,0,0);
  }while(1);
}
/*-----------------------------------------------------------------------------*/
/*! Unlock protected register */
#define UNLOCKREG(x)  do{outpw(REG_SYS_REGWPCTL,0x59); outpw(REG_SYS_REGWPCTL,0x16); outpw(REG_SYS_REGWPCTL,0x88);}while(inpw(REG_SYS_REGWPCTL) == 0x00)
/*! Lock protected register */
#define LOCKREG(x)  do{outpw(REG_SYS_REGWPCTL,0x00);}while(0)

#if defined ( __GNUC__ ) && !(__CC_ARM)
void  __wfi(void)
{
	asm
	(
			"MCR p15, 0, r1, c7, c0, 4\n"
	);
}
#else
__asm void __wfi(void)
{
  MCR p15, 0, r1, c7, c0, 4
  BX            lr
}
#endif

void EnterPowerDown(void)
{
  UINT32 reg;
  int r0,r1,r2;
  outp32(REG_CLK_PLLSTBCNTR,0xFFFF);
  outp32(REG_SDIC_MR,  inpw(REG_SDIC_MR)|0x100); //Enable Reset DLL(bit[8]) of DDR2 

#if defined ( __GNUC__ ) && !(__CC_ARM)
  asm(
		  "mov r2, #1000\n"
		  "mov r1, #0\n"
		  "loop1:  add   r1, r1, r0\n"
		  "cmp r1, r2\n"
		  "bne loop1\n"
  );
#else
  __asm
  {/*  Delay a moment until the escape self-refresh command reached to DDR/SDRAM */
      mov r2, #1000
      mov r1, #0
      mov r0, #1
  loop1:  add   r1, r1, r0
      cmp r1, r2
      bne loop1
  }
#endif
  outp32(REG_SDIC_MR,  inpw(REG_SDIC_MR)&~0x100); //Disabe Reset DLL(bit[8]) of DDR2

#if defined ( __GNUC__ ) && !(__CC_ARM)
  asm(
		  "mov r2, #1000\n"
		  "mov r1, #0\n"
		  "loop2:  add   r1, r1, r0\n"
		  "cmp r1, r2\n"
		  "bne loop2\n"
  );
#else
  __asm
  {/*  Delay a moment until the escape self-refresh command reached to DDR/SDRAM */
      mov r2, #1000
      mov r1, #0
      mov r0, #1
  loop2:  add r1, r1, r0
      cmp r1, r2
      bne loop2
  }
#endif
  reg=inpw(REG_CLK_PMCON);   //Enable NUC970 to enter power down mode
  reg = reg & (0xFFFFFFFE); 
  outpw(REG_CLK_PMCON,reg);
  outp32(REG_SDIC_OPMCTL, (inp32(REG_SDIC_OPMCTL) & ~0x10000));         // set SDIC_OPMCTL[16] low to disable auto power down mode
  outp32(REG_SDIC_CMD, (inp32(REG_SDIC_CMD) & ~0x20));
  __wfi();
  outp32(REG_SDIC_CMD, inp32(REG_SDIC_CMD) | 0x20);
  outp32(REG_SDIC_OPMCTL, (inp32(REG_SDIC_OPMCTL) | 0x10000));         // set SDIC_OPMCTL[16] high to enable auto power down mode;  
}

volatile int touchwakeup_complete=0;
INT32 TouchWakeupCallback(UINT32 status, UINT32 userData)
{
  touchwakeup_complete=1;
  sysprintf("\nTouch wakeup\n");
  return 0;
}

volatile int keypadwakeup_complete=0;
INT32 KeypadWakeupCallback(UINT32 status, UINT32 userData)
{
  keypadwakeup_complete=1;
  sysprintf("\nKeypad wakeup\n");
  return 0;
}

void touch_xy_wakeup_demo()
{
  int userdata=0;
  adcIoctl(PEPOWER_ON,(UINT32)0,0); //Enable ADC Pen power
  adcIoctl(WKT_ON,(UINT32)TouchWakeupCallback,userdata);
  sysprintf("Enter dower down! Press touch to wake-up\n");
  while(!(*(volatile unsigned int *)(UART0_BA+0x18)& (1<<22)));
  EnterPowerDown();
  while(1);
}

void keypad_wakeup_demo()
{
  int userdata=0;
  adcIoctl(KPPOWER_ON,(UINT32)0,0); //Enable ADC Keypad power
  adcIoctl(WKP_ON,(UINT32)KeypadWakeupCallback,userdata);  
  sysprintf("Enter dower down! Press any key to wake-up\n");
  while(!(*(volatile unsigned int *)(UART0_BA+0x18)& (1<<22)));
  EnterPowerDown();
  while(1);
}
/*-----------------------------------------------------------------------------*/
void show_menu(void)
{
  sysprintf("%c[2J", 0x1B);
  sysprintf("%c[%d;1H", 0x1B, 0);
  sysprintf("****************************************************\n");
  sysprintf("* ADC demo items:                           Key:   *\n");
  sysprintf("* Touch demo - XY only                      <1>    *\n");
  sysprintf("* Touch demo - XYZ                          <2>    *\n");
  sysprintf("* Touch demo - XYZ & penwdown               <3>    *\n");
  sysprintf("* Keypad demo                               <4>    *\n");
  sysprintf("* Battery detection demo                    <5>    *\n");
  sysprintf("* Normal demo                               <6>    *\n");
  sysprintf("* Touch demo -  wakeup                      <7>    *\n");
  sysprintf("* Keypad demo - wakeup                      <8>    *\n");
  sysprintf("* Quit                                      <Q,q>  *\n");
  sysprintf("****************************************************\n");
}

int main(void)
{
    char c;

    *(volatile unsigned int *)(CLK_BA+0x18) |= (1<<16); /* Enable UART0 */
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();
    sysprintf("+-------------------------------------------------+\n");
    sysprintf("|                 ADC Sample Code                 |\n");
    sysprintf("+-------------------------------------------------+\n\n");
    adcOpen();
    while(1) {
      show_menu();
      c = sysGetChar(); 
      sysprintf("intput %c\n",c);
      switch(c)
      {
        case '1':
          touch_xy_demo();
        break;
        case '2':
          touch_xyz_demo();
        break;
        case '3':
          touch_xyz_pendown_demo();
        break;
        case '4':
          keypad_demo();
        break;
        case '5':
          battery_demo();
        break;
        case '6':
          normal_demo();
        break;
        case '7':
          touch_xy_wakeup_demo();
        break;
        case '8':
          keypad_wakeup_demo();
        break;
      }
    }
}
