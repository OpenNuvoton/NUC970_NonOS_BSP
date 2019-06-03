/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/08/26 11:56a $
 * @brief    This sample shows how to access USB mass stroage disk.
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
#include "ff.h"
#include "diskio.h"


#define BUFF_SIZE       (256*1024)

DWORD acc_size;                         /* Work register for fs command */
WORD acc_files, acc_dirs;
FILINFO Finfo;

char Line[256];                         /* Console input buffer */
#if _USE_LFN
char Lfname[512];
#endif

BYTE Buff_Pool[BUFF_SIZE] __attribute__((aligned(32)));       /* Working buffer */

BYTE  *Buff;


uint32_t   g_hid_buff_pool[1024] __attribute__((aligned(32)));

HID_DEV_T   *g_hid_list[CONFIG_HID_MAX_DEV];


void delay_us(int usec)
{
#ifdef IS_FPGA
    volatile int  loop = 12 * usec;
#else
    volatile int  loop = 300 * usec;
#endif
    while (loop > 0) loop--;
}

uint32_t get_ticks(void)
{
    return sysGetTicks(TIMER0);
}

void timer_init()
{
    sysprintf("timer_init() To do...\n");
}

uint32_t get_timer_value()
{
    sysprintf("get_timer_value() To do...\n");
    return 1;
}


void  dump_buff_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        sysprintf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            sysprintf("%02x ", pucBuff[nIdx + i]);
        sysprintf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                sysprintf("%c", pucBuff[nIdx + i]);
            else
                sysprintf(".");
            nBytes--;
        }
        nIdx += 16;
        sysprintf("\n");
    }
    sysprintf("\n");
}


/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */

unsigned long get_fattime (void)
{
    unsigned long tmr;

    tmr=0x00000;

    return tmr;
}


int  is_a_new_hid_device(HID_DEV_T *hdev)
{
    int    i;
    for (i = 0; i < CONFIG_HID_MAX_DEV; i++)
    {
        if ((g_hid_list[i] != NULL) && (g_hid_list[i] == hdev) &&
                (g_hid_list[i]->uid == hdev->uid))
            return 0;
    }
    return 1;
}

void update_hid_device_list(HID_DEV_T *hdev)
{
    int  i = 0;
    memset(g_hid_list, 0, sizeof(g_hid_list));
    while ((i < CONFIG_HID_MAX_DEV) && (hdev != NULL))
    {
        g_hid_list[i++] = hdev;
        hdev = hdev->next;
    }
}

void  int_read_callback(HID_DEV_T *hdev, uint16_t ep_addr, int status, uint8_t *rdata, uint32_t data_len)
{
    /*
     *  USB host HID driver notify user the transfer status via <status> parameter. If the
     *  If <status> is 0, the USB transfer is fine. If <status> is not zero, this interrupt in
     *  transfer failed and HID driver will stop this pipe. It can be caused by USB transfer error
     *  or device disconnected.
     */
    if (status < 0)
    {
        sysprintf("Interrupt in transfer failed! status: %d\n", status);
        return;
    }
    //sysprintf("Device [0x%x,0x%x] ep 0x%x, %d bytes received =>\n",
    //       hdev->idVendor, hdev->idProduct, ep_addr, data_len);
    //dump_buff_hex(rdata, data_len);
    sysprintf(".");
}

#ifdef HAVE_INT_OUT
void  int_write_callback(HID_DEV_T *hdev, uint16_t ep_addr, int staus, uint8_t *wbuff, uint32_t *data_len)
{
    int   max_len = *data_len;

    sysprintf("Device [0x%x,0x%x] ep 0x%x, ask user to fill data buffer and length.\n",
              hdev->idVendor, hdev->idProduct, ep_addr);

    memset(wbuff, 0, max_len);         /* Fill data to be sent via interrupt out pipe     */

    *data_len = max_len;               /* Tell HID driver transfer length of this time    */
}
#endif


int  init_hid_device(HID_DEV_T *hdev)
{
    uint8_t   *data_buff;
    int       ret;

    data_buff = (uint8_t *)((uint32_t)g_hid_buff_pool | 0x80000000);

    sysprintf("\n\n==================================\n");
    sysprintf("  Init HID device : 0x%x\n", (int)hdev);
    sysprintf("  VID: 0x%x, PID: 0x%x\n\n", hdev->idVendor, hdev->idProduct);

    ret = usbh_hid_get_report_descriptor(hdev, data_buff, 1024);
    if (ret > 0)
    {
        //sysprintf("\nDump report descriptor =>\n");
        //dump_buff_hex(data_buff, ret);
    }

    sysprintf("\nUSBH_HidStartIntReadPipe...\n");
    ret = usbh_hid_start_int_read(hdev, 0, int_read_callback);
    if (ret != HID_RET_OK)
        sysprintf("usbh_hid_start_int_read failed! %d\n", ret);
    else
        sysprintf("Interrupt in transfer started...\n");

#ifdef HAVE_INT_OUT
    ret = usbh_hid_start_int_write(hdev, 0, int_write_callback);
    if ((ret != HID_RET_OK) && (ret != HID_RET_EP_USED))
        sysprintf("usbh_hid_start_int_write failed!\n");
    else
        sysprintf("Interrupt out transfer started...\n");
#endif

    return 0;
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int32_t main(void)
{
    HID_DEV_T   *hdev;
    TCHAR       usb_path[] = { '3', ':', 0 };    /* USB drive started from 3 */
    int         sector;
    FRESULT     res;

    Buff = (BYTE *)((UINT32)&Buff_Pool[0] | 0x80000000);     /* use non-cache buffer */

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | 0x40000);
    outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 0x10000);

    // set PE.14 & PE.15 for USBH_PPWR0 & USBH_PPWR1
    outpw(REG_SYS_GPE_MFPH, (inpw(REG_SYS_GPE_MFPH) & ~0xff000000) | 0x77000000);

    sysprintf("\n\n");
    sysprintf("+---------------------------------------------------+\n");
    sysprintf("|                                                   |\n");
    sysprintf("|     USB Host Mass Storage + HID sample program    |\n");
    sysprintf("|                                                   |\n");
    sysprintf("+---------------------------------------------------+\n");

    /*--- init timer ---*/
    sysSetTimerReferenceClock (TIMER0, 12000000);
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);

    usbh_core_init();
    usbh_umas_init();
    usbh_hid_init();
    memset(g_hid_list, 0, sizeof(g_hid_list));

    f_chdrive(usb_path);          /* set default path */

    sysprintf("EHCI: 0x%x, 0x%x; OHCI: 0x%x, 0x%x\n", inpw(0xB0005064), inpw(0xB0005068), inpw(0xB0007054), inpw(0xB0007058));

    sector = 0;
    for (;;)
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

        if ((sector % 50) == 0)
            sysprintf("Read sector %d\n", sector);

        res = (FRESULT)disk_read(3, Buff, sector++, 256);
        if (res)
        {
            sysprintf("disk_read sector %d failed! rc=%d\n", sector, (WORD)res);
            break;
        }

        if (sector >= 1000000)
            sector = 0;
    }
    while (1);
}


/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/
