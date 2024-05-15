/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 03/03/19 4:02p $
 * @brief    Use USB Host core driver and CDC driver. This sample demonstrates how
 *           to connect a CDC class VCOM device.
 *
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "usbh_lib.h"
#include "usbh_cdc.h"


#define MAX_VCOM_PORT      8

uint32_t  g_buff_pool[1024] __attribute__((aligned(32)));

char Line[64];             /* Console input buffer */

typedef struct
{
    CDC_DEV_T  *cdev;
    LINE_CODING_T  line_code;
    int    checked;
}  VCOM_PORT_T;

VCOM_PORT_T   vcom_dev[MAX_VCOM_PORT];


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


void  vcom_status_callback(CDC_DEV_T *cdev, uint8_t *rdata, int data_len)
{
    int  i, slot;

    slot = (int)cdev->client;

    sysprintf("[VCOM%d STS] ", slot);
    for(i = 0; i < data_len; i++)
        sysprintf("0x%02x ", rdata[i]);
    sysprintf("\n");
}

void  vcom_rx_callback(CDC_DEV_T *cdev, uint8_t *rdata, int data_len)
{
    int   i, slot;

    slot = (int)cdev->client;
    //sysprintf("[%d][RX %d] ", cdev->iface_cdc->if_num, data_len);
    sysprintf("[RX][VCOM%d]: ", slot);
    for (i = 0; i < data_len; i++)
    {
        //sysprintf("0x%02x ", rdata[i]);
        sysprintf("%c", rdata[i]);
    }
    sysprintf("\n");
}

void show_line_coding(LINE_CODING_T *lc)
{
    sysprintf("[CDC device line coding]\n");
    sysprintf("====================================\n");
    sysprintf("Baud rate:  %d bps\n", lc->baud);
    sysprintf("Parity:     ");
    switch (lc->parity)
    {
    case 0:
        sysprintf("None\n");
        break;
    case 1:
        sysprintf("Odd\n");
        break;
    case 2:
        sysprintf("Even\n");
        break;
    case 3:
        sysprintf("Mark\n");
        break;
    case 4:
        sysprintf("Space\n");
        break;
    default:
        sysprintf("Invalid!\n");
        break;
    }
    sysprintf("Data Bits:  ");
    switch (lc->data_bits)
    {
    case 5 :
    case 6 :
    case 7 :
    case 8 :
    case 16:
        sysprintf("%d\n", lc->data_bits);
        break;
    default:
        sysprintf("Invalid!\n");
        break;
    }
    sysprintf("Stop Bits:  %s\n\n", (lc->stop_bits == 0) ? "1" : ((lc->stop_bits == 1) ? "1.5" : "2"));
}

int  init_cdc_device(CDC_DEV_T *cdev, int slot)
{
    int     ret;
    LINE_CODING_T  *line_code;

    sysprintf("\n\n===  VCOM%d  ===============================\n", slot);
    sysprintf("  Init CDC device : 0x%x\n", (int)cdev);
    sysprintf("  VID: 0x%x, PID: 0x%x, interface: %d\n\n", cdev->udev->descriptor.idVendor, cdev->udev->descriptor.idProduct, cdev->iface_cdc->if_num);

    line_code = &(vcom_dev[slot].line_code);

    ret = usbh_cdc_get_line_coding(cdev, line_code);
    if (ret < 0)
    {
        sysprintf("Get Line Coding command failed: %d\n", ret);
    }
    else
        show_line_coding(line_code);

    line_code->baud = 115200;
    line_code->parity = 0;
    line_code->data_bits = 8;
    line_code->stop_bits = 0;

    ret = usbh_cdc_set_line_coding(cdev, line_code);
    if (ret < 0)
    {
        sysprintf("Set Line Coding command failed: %d\n", ret);
    }

    ret = usbh_cdc_get_line_coding(cdev, line_code);
    if (ret < 0)
    {
        sysprintf("Get Line Coding command failed: %d\n", ret);
    }
    else
    {
        sysprintf("New line coding =>\n");
        show_line_coding(line_code);
    }

    usbh_cdc_set_control_line_state(cdev, 1, 1);

    sysprintf("usbh_cdc_start_polling_status...\n");
    usbh_cdc_start_polling_status(cdev, vcom_status_callback);

    sysprintf("usbh_cdc_start_to_receive_data...\n");
    usbh_cdc_start_to_receive_data(cdev, vcom_rx_callback);

    return 0;
}

void update_vcom_device()
{
    int    i, free_slot;
    CDC_DEV_T   *cdev;

    for (i = 0; i < MAX_VCOM_PORT; i++)
        vcom_dev[i].checked = 0;

    cdev = usbh_cdc_get_device_list();
    while (cdev != NULL)
    {
        free_slot = -1;
        for (i = MAX_VCOM_PORT-1; i >= 0; i--)
        {
            if (vcom_dev[i].cdev == NULL)
                free_slot = i;

            if ((vcom_dev[i].cdev == cdev) && (i == (int)cdev->client))
            {
                vcom_dev[i].checked = 1;
                break;
            }
        }

        sysprintf("free_slot %d\n", free_slot);

        if (i < 0)      /* not found in VCOM device list, add it */
        {
            if (free_slot == -1)
            {
                sysprintf("No free VCOM device slots!\n");
                goto next_cdev;
            }

            i = free_slot;
            vcom_dev[i].cdev = cdev;
            cdev->client = (void *)i;
            init_cdc_device(cdev, i);
            vcom_dev[i].checked = 1;
        }

next_cdev:
        cdev = cdev->next;
    }

    for (i = 0; i < MAX_VCOM_PORT; i++)
    {
        if ((vcom_dev[i].cdev != NULL) && (vcom_dev[i].checked == 0))
        {
            vcom_dev[i].cdev = NULL;
        }
    }
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int32_t main(void)
{
    CDC_DEV_T   *cdev;
    int         i, ret;
    char        *message;

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | 0x40000);
    outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 0x10000);

    // set PE.14 & PE.15 for USBH_PPWR0 & USBH_PPWR1
    outpw(REG_SYS_GPE_MFPH, (inpw(REG_SYS_GPE_MFPH) & ~0xff000000) | 0x77000000);

    message = (char *)((uint32_t)g_buff_pool | 0x80000000);   // get non-cachable buffer address

    sysprintf("\n\n");
    sysprintf("+--------------------------------------------+\n");
    sysprintf("|                                            |\n");
    sysprintf("|     USB Host VCOM sample program           |\n");
    sysprintf("|                                            |\n");
    sysprintf("+--------------------------------------------+\n");

    /*--- init timer ---*/
    sysSetTimerReferenceClock (TIMER0, 15000000);
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);

    for (i = 0; i < MAX_VCOM_PORT; i++)
        vcom_dev[i].cdev = NULL;

    usbh_core_init();
    usbh_cdc_init();
    usbh_memory_used();

    while(1)
    {
        if (usbh_pooling_hubs())             /* USB Host port detect polling and management */
        {
            usbh_memory_used();              /* print out USB memory allocating information */

            if (usbh_cdc_get_device_list() == NULL)
            {
                /* There's no any VCOM device connected. */
                memset(vcom_dev, 0, sizeof(vcom_dev));
                continue;
            }

            update_vcom_device();
        }

        for (i = 0; i < MAX_VCOM_PORT; i++)
        {
            cdev = vcom_dev[i].cdev;
            if (cdev == NULL)
                continue;

            if (!cdev->rx_busy)
            {
                usbh_cdc_start_to_receive_data(cdev, vcom_rx_callback);
            }
        }

        /*
         *  Check user input and send data to CDC device immediately. For loopback test only.
         */
        if (sysIsKbHit())
        {
            sysGetChar();
            for (i = 0; i < MAX_VCOM_PORT; i++)
            {
                cdev = vcom_dev[i].cdev;
                if (cdev == NULL)
                    continue;

                memset(message, 0, 64);
                sprintf(message, "To VCOM%d (VID:0x%x, PID:0x%x, interface %d).\n",
                        i, cdev->udev->descriptor.idVendor, cdev->udev->descriptor.idProduct, cdev->iface_cdc->if_num);

                ret = usbh_cdc_send_data(cdev, (uint8_t *)message, 64);
                if (ret != 0)
                    sysprintf("\n!! Send data failed, 0x%x!\n", ret);
            }

        }
    }
}


/*** (C) COPYRIGHT 2019 Nuvoton Technology Corp. ***/
