/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 15/05/18 4:02p $
 * @brief    This sample shows how to manage several USB HID class devices.
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


void  int_read_callback(HID_DEV_T *hdev, uint16_t ep_addr, uint8_t *rdata, uint32_t data_len)
{
    sysprintf("Device [0x%x,0x%x] ep 0x%x, %d bytes received =>\n", 
              hdev->idVendor, hdev->idProduct, ep_addr, data_len);
    dump_buff_hex(rdata, data_len);
}

#if 0
static uint8_t  _write_data_buff[4];

void  int_write_callback(HID_DEV_T *hdev, uint16_t ep_addr, uint8_t **wbuff, uint32_t *buff_size)
{
    sysprintf("INT-out pipe request to write data.\n");

    *wbuff = &_write_data_buff[0];
    *buff_size = 4;
}
#endif

int  init_hid_device(HID_DEV_T *hdev)
{
	uint8_t   *data_buff;
	int       i, ret;
	
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

	/*
	 *  Example: GET_PROTOCOL request. 
	 */
	ret = usbh_hid_get_protocol(hdev, data_buff);
	sysprintf("[GET_PROTOCOL] ret = %d, protocol = %d\n", ret, data_buff[0]);

	/*
	 *  Example: SET_PROTOCOL request. 
	 */
	ret = usbh_hid_set_protocol(hdev, data_buff[0]);
	sysprintf("[SET_PROTOCOL] ret = %d, protocol = %d\n", ret, data_buff[0]);

	/*
	 *  Example: GET_REPORT request on report ID 0x1, report type FEATURE. 
	 */
	ret = usbh_hid_get_report(hdev, RT_FEATURE, 0x1, data_buff, 64);
	if (ret > 0)
	{
		sysprintf("[GET_REPORT] Data => ");
		for (i = 0; i < ret; i++)
		    sysprintf("%02x ", data_buff[i]);
		sysprintf("\n");
	}
    
    sysprintf("\nUSBH_HidStartIntReadPipe...\n");
    ret = usbh_hid_start_int_read(hdev, 0, int_read_callback);
    if ((ret != HID_RET_OK) && (ret != HID_RET_EP_USED))
    	sysprintf("usbh_hid_start_int_read failed!\n");
    else
    	sysprintf("Interrupt in transfer started...\n");

    //if (usbh_hid_start_int_write(hdev, 0, int_write_callback) == HID_RET_OK) 
    //{
    //	sysprintf("Interrupt out transfer started...\n");
    //}
    
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
    }
}


/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
