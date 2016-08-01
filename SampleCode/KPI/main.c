/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/05/22 2:58p $
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
#include "kpi.h"

void KPI_Test(void);
static void KPI_Item(void);
static void KPI_Readkey_block(void);
static void KPI_Readkey_nonblock(void);
static void KPI_BufferTest(void);
static void KPI_LPWakeTest(void);
static void KPI_ThreeKeyTest(void);
static void KPI_ShowKeyMap(void);


__asm void __wfi()
{
	MCR p15, 0, r1, c7, c0, 4			  
	BX			  lr
      
}


INT main(void)
{
  	CHAR cUserSelect;
                                             
    /* UART setting */
	sysInitializeUART();

	// Enable GPIO clock
	outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | (1 << 3));
	
	// Set GPH are input mode
	outpw(REG_GPIOH_DIR, (inpw(REG_GPIOH_DIR) & 0xffff000f));

	//set GPIO pull up
	outpw(REG_GPIOH_PDEN, (inpw(REG_GPIOH_PDEN) & 0xffff000f));
	outpw(REG_GPIOH_PUEN, (inpw(REG_GPIOH_PUEN) & 0xffff000f) | (0xfff0));

	// Set GPH nulti-function pin
	outpw(REG_SYS_GPH_MFPH, (inpw(REG_SYS_GPH_MFPH) & 0x00000000) | (0x44444444));
	outpw(REG_SYS_GPH_MFPL, (inpw(REG_SYS_GPH_MFPL) & 0x0000ffff) | (0x4444 << 16));

	KPI_Init(4, 8, 1, 0);
	
    do
    {
        KPI_Item();                // Show menu
        
        cUserSelect = sysGetChar();// Get user select
        
        switch(cUserSelect)
        {
            case '1':   KPI_LPWakeTest();           break;
            case '2':   KPI_Readkey_block();        break;
            case '3':   KPI_Readkey_nonblock();     break;
            case '4':   KPI_BufferTest();           break;
            case '5':   KPI_ThreeKeyTest();         break;
            default:    break;
        }
    }while(cUserSelect != 27);     // Do until user press ESC
    
    sysprintf("Leave program");
    return 0;
}

static void KPI_Item(void)
{
    sysprintf("\n\n\n");
    sysprintf("+------------------------------------------------------+\n");
    sysprintf("| Winbond KPI Diagnostic Code                          |\n");
    sysprintf("+------------------------------------------------------+\n");
    sysprintf("| Power down wake up test                        - [1] |\n");
    sysprintf("| KPI block mode test                            - [2] |\n");
    sysprintf("| KPI non-block mode test                        - [3] |\n");
    sysprintf("| KPI buffer test                                - [4] |\n");
    sysprintf("| KPI three key test                             - [5] |\n");
    sysprintf("| Quit                                         - [ESC] |\n");
    sysprintf("+------------------------------------------------------+\n");
    sysprintf("Please Select : \n");
}

static void KPI_ThreeKeyTest(void)
{
    INT nStatus;
    CHAR cItem;
    KPISTATUS_union kpistatus;
	KPIKEY_union kpikey;
    KPI3KCONF_union kpi3kconf;
    kpikey.value = 0;
	kpistatus.value = 0;
	kpi3kconf.value = 0;

	KPI_Open();
    KPI_ShowKeyMap();
    sysprintf("Setup KPI3KCONF-ENRST\nReset?(y/n)");
    cItem = sysGetChar();
    switch(cItem)
    {
        case 'y':    
        case 'Y':    
			EN3KYRST = EN3KYRST_ENABLE;    
			break;
        case 'n':    
        case 'N':    
			EN3KYRST = EN3KYRST_DISABLE;   
			break;
        default:     
			EN3KYRST = EN3KYRST_DISABLE;   
			break;
    }

    K30R = 1;
    K30C = 0;
    K31R = 1;
    K31C = 1;
    K32R = 1;
    K32C = 2;

    nStatus = KPI_Ioctl(SET_KPI3KCONF, 0, kpi3kconf.value);
    if(nStatus == kpiInvalidIoctlCommand)
    {
        sysprintf("KPI ioctl command error!");
        return;
    }
    sysprintf("keypad three key test.\n");
    sysprintf("Three key setup:\n  Key1 R:%2d C:%2d\n  Key2 R:%2d C:%2d\n  Key3 R:%2d C:%2d\n",
               K30R, K30C, K31R, K31C, K32R, K32C);
    if( (cItem == 'y') || (cItem == 'Y') )
    {
        sysprintf("  Reset: yes\n");
    }
    else
    {
        sysprintf("  Reset: no\n");
    }
	
    sysprintf("Wait for three key press... \n");
	
    while(1)
    {
        if( KPI_Read((PUCHAR)&kpistatus, (PUCHAR)&kpikey, BLOCK_MODE) == 0 )
		{
			if(KEY_INT)
			{
				sysprintf("Not Three key interrupt ! \n\n");
				//break;
			}
			if(RST_3KEY)
			{
				sysprintf("Three key test OK!!\n\n");
			}
		}
    }
	
    //KPI_Close();
    //sysprintf("\n\nTest finish\n");
}

static void KPI_LPWakeTest(void)
{
    unsigned int i;
    INT nStatus;
    KPISTATUS_union kpistatus;
	KPIKEY_union kpikey;
    KPICONF_union kpiconf;
    
    kpistatus.value = 0;
	kpikey.value = 0;
	kpiconf.value = 0;
		
	KPI_Open();
   
    KPI_ShowKeyMap();
 
	WAKEUP = WAKEUP_ENABLE;
	ODEN = ODEN_ENABLE;
	PKINTEN = PKINTEN_ENABLE;
	INPU = INPU_ENABLE;
	INTEN = INTEN_ENABLE;
	DB_EN = DB_EN_ENABLE;
	ENKP = ENKP_ENABLE;
	
    nStatus = KPI_Ioctl(SET_KPICONF, 0, kpiconf.value);
   
    if(nStatus == kpiInvalidIoctlCommand)
    {
        sysprintf("KPI ioctl command error!");
        return;
    }

    sysprintf("KPI power down wake up test\n");

    sysprintf("Press any key to enter power down mode....");
    sysGetChar();
    KPI_Ioctl(CLEAN_KPI_BUFFER, 0, 0);
    sysprintf("\n");
	
	// Unlock Register
	outpw(0xB00001FC, 0x59);
	outpw(0xB00001FC, 0x16);
	outpw(0xB00001FC, 0x88);
	while(!(inpw(0xB00001FC) & 0x1));
	
	outpw(REG_SYS_WKUPSER , (1 << 27) ); // wakeup source select KPI
	
	//enter power down mode
	i = *(volatile unsigned int *)(0xB0000200);
	i = i & (0xFFFFFFFE);
	*(volatile unsigned int *)(0xB0000200)=i; 
	
	__wfi(); 
	
    KPI_Read((PUCHAR)&kpistatus, (PUCHAR)&kpikey, BLOCK_MODE);
    
    if(PDWAKE==1)
    {
        sysprintf("Wake up success.\n");

        if(KEY_INT)
        {
            sysprintf("\nKey: 0x%x\n", kpikey.value);
        }
    }
	
    KPI_Close();
    sysprintf("\n Power down Test finish!\n Press any key to continue....");
    sysGetChar();
}

static void KPI_Readkey_block(void)
{
    INT nStatus;
	INT8 cUserSelect;
    KPISTATUS_union kpistatus;
	KPIKEY_union kpikey;
	
	kpistatus.value = 0;
    kpikey.value = 0;
	
	KPI_Open();
	
    KPI_ShowKeyMap();
    sysprintf("KPI block mode test\n");

    sysprintf("Please press any KPI key\n\n");
    while(1)
    {
		
		cUserSelect = sysGetChar();
		
		if((cUserSelect == 'E') || (cUserSelect == 'e'))
		{
			break;
		}
		
        nStatus = KPI_Read((PUCHAR)&kpistatus, (PUCHAR)&kpikey, BLOCK_MODE);
		
        if(nStatus != 0)
        {
            sysprintf("KPI read function error!\nError code:%x\n", nStatus);
            return;
        }
		
        if(KEY_INT)
        {
            sysprintf("Key: 0x%x \n", kpikey.value);
        }
    }
    KPI_Close();
    sysprintf("\n\nKPI block mode test finish!\n");
}

static void KPI_Readkey_nonblock(void)
{
    INT nStatus;
    KPISTATUS_union kpistatus;
	KPIKEY_union kpikey;
    INT nLoop;
    
	kpistatus.value = 0;
    kpikey.value = 0;

	KPI_Open();
	
    KPI_ShowKeyMap();
    sysprintf("KPI non block mode test\n");
    sysprintf("Please press any KPI key before time out\n");
    for(nLoop = 0; nLoop < 2000000; nLoop++)
    {
        if(nLoop%100000 == 0)
        {
            sysprintf(".");
        }
        nStatus = KPI_Read((PUCHAR)&kpistatus, (PUCHAR)&kpikey, NONBLOCK_MODE);
		
        if(nStatus == kpiReadModeError)
        {
            sysprintf("KPI read function read mode error!");
            return;
        }
        if(nStatus == kpiNoKey)
        {
            continue;
        }
		
        if(KEY_INT)
        {
            sysprintf("\nKey: 0x%x\n", kpikey.value);
            break;
        }
    }
    KPI_Close();
    sysprintf("\n\nKPI non block mode test finish!\nPress any key to continue....");
    sysGetChar();
}

static void KPI_BufferTest(void)
{
    KPISTATUS_union kpistatus;
	KPIKEY_union kpikey;
	INT nBuffercount = 0;

	kpistatus.value = 0;
    kpikey.value = 0;
	
	KPI_Open();
	
    KPI_ShowKeyMap();
    sysprintf("\nKPI buffer test\nThe KPI buffer size is ten\nPlease press any KPI key then press any key in PC\n");
    sysGetChar();
	
    while(KPI_Read((PUCHAR)&kpistatus, (PUCHAR)&kpikey, NONBLOCK_MODE) == 0)
    {
        if(KEY_INT)
        {
            nBuffercount++;
            sysprintf("[Buffer %2d] Key : 0x%x\n", nBuffercount, kpikey.value);
        }
    }
    KPI_Close();
    sysprintf("KPI buffer test finish!\nPress any key to continue....");
    sysGetChar();
}

static void KPI_ShowKeyMap(void)
{
    sysprintf("Key may:[ROM,COL]\n");
	sysprintf("[0,0]~[0,7], [1,0]~[1,7], [2,0]~[2,7], [3,0]~[3,7] \n\n");
}


