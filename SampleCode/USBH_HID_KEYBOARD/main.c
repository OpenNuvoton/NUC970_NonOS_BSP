/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 15/05/18 4:03p $
 * @brief    This sample shows how to manage USB keyboard devices.
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "usbh_lib.h"
#include "usbh_hid.h"

__align(32) uint32_t   g_buff_pool[1024];

extern int  kbd_parse_report(HID_DEV_T *hdev, uint8_t *buf, int len);

static HID_DEV_T  *hdev_ToDo = NULL;
static uint8_t    data_ToDo[8];


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
    while (nBytes > 0) {
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


void  int_read_callback(HID_DEV_T *hdev, uint16_t ep_addr, int status, uint8_t *rdata, uint32_t data_len)
{
	/*
	 *  This callback is in interrupt context.
	 *  Copy the device and data and then handle it somewhere not in interrupt context.
	 */
	//dump_buff_hex(rdata, data_len);
	hdev_ToDo = hdev;
	memcpy(data_ToDo, rdata, sizeof(data_ToDo));
}


int  init_hid_device(HID_DEV_T *hdev)
{
	uint8_t   *data_buff;
	int       ret;
	
	data_buff = (uint8_t *)((uint32_t)g_buff_pool | 0x80000000);   // get non-cachable buffer address

	sysprintf("\n\n==================================\n");
	sysprintf("  Init HID device : 0x%x\n", (int)hdev);
	sysprintf("  VID: 0x%x, PID: 0x%x\n\n", hdev->idVendor, hdev->idProduct);

    ret = usbh_hid_get_report_descriptor(hdev, data_buff, 1024);
    if (ret > 0)
    {
    	sysprintf("\nDump report descriptor =>\n");
    	dump_buff_hex(data_buff, ret);
    }
    
    sysprintf("\nUSBH_HidStartIntReadPipe...\n");
    ret = usbh_hid_start_int_read(hdev, 0, int_read_callback);
    if (ret != HID_RET_OK)
    	sysprintf("usbh_hid_start_int_read failed!\n");
    else
    	sysprintf("Interrupt in transfer started...\n");

    return 0;
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int32_t main(void)
{
	HID_DEV_T    *hdev;
	
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
	sysSetTimerReferenceClock (TIMER0, 15000000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);
	
	usbh_core_init();    
	usbh_hid_init();
	usbh_pooling_hubs();
	
    while (1)
    {
        if (usbh_pooling_hubs())             /* USB Host port detect polling and management */
        {
        	sysprintf("\n Has hub events.\n");
        	hdev = usbh_hid_get_device_list();
        	if (hdev == NULL)
            	continue;
            	
            while (hdev != NULL)
            {
            	init_hid_device(hdev);

				if (hdev != NULL)            	
            		hdev = hdev->next;
        	}
        }

        if (hdev_ToDo != NULL)
        {
			kbd_parse_report(hdev_ToDo, data_ToDo, 8);
			hdev_ToDo = NULL;
		}
    }
}


/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
