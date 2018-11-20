/**************************************************************************//**
 * @file     uvc_driver.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 15/06/12 10:12a $
 * @brief    NUC970 MCU USB Host Video Class driver
 *
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "nuc970.h"

#include "usb.h"
#include "usbh_lib.h"
#include "usbh_uvc.h"


/** @addtogroup NUC970_Library NUC970 Library
  @{
*/

/** @addtogroup NUC970_USBH_Library USB Host Library
  @{
*/

/** @addtogroup NUC970_USBH_EXPORTED_FUNCTIONS USB Host Exported Functions
  @{
*/

/// @cond HIDDEN_SYMBOLS


extern int uvc_parse_control_interface(UVC_DEV_T *vdev, IFACE_T *iface);
extern int uvc_parse_streaming_interface(UVC_DEV_T *vdev, IFACE_T *iface);

extern int usbh_uvc_probe_control(UVC_DEV_T *vdev, uint8_t req, UVC_CTRL_PARAM_T *param);


__align(32)  uint8_t  g_uvc_buff_pool[UVC_MAX_DEVICE][UVC_UTR_PER_STREAM * UVC_UTR_INBUF_SIZE];
static uint8_t g_uvc_buff_used[UVC_MAX_DEVICE];

static UVC_DEV_T *g_vdev_list = NULL;

static UVC_DEV_T *alloc_uvc_device(UDEV_T *udev)
{
    UVC_DEV_T  *vdev;
    int        i;

    /*
     *  Search UVC device list check if this device already allocated.
     */
    vdev = g_vdev_list;
    while (vdev != NULL)
    {
        if (vdev->udev == udev)
            return vdev;
        vdev = vdev->next;
    }

    vdev = (UVC_DEV_T *)usbh_alloc_mem(sizeof(UVC_DEV_T));
    if (vdev == NULL)
        return NULL;

    memset((char *)vdev, 0, sizeof(UVC_DEV_T));

    for (i = 0; i < UVC_MAX_DEVICE; i++)
    {
        if (g_uvc_buff_used[i] == 0)
        {
            vdev->in_buff = (uint8_t *)((uint32_t)&g_uvc_buff_pool[i][0] | 0x80000000);
            g_uvc_buff_used[i] = 1;
            break;
        }
    }
    if (vdev->in_buff == NULL)
    {
        UVC_DBGMSG("Failed to allocate UVC iso-in buffer!\n");
        usbh_free_mem(vdev, sizeof(UVC_DEV_T));
        return NULL;
    }

    return vdev;
}

void  free_uvc_device(UVC_DEV_T *vdev)
{
    int   i;

    if (vdev->in_buff != NULL)
    {
        vdev->in_buff = (uint8_t *)((uint32_t)vdev->in_buff & ~0x80000000);
        for (i = 0; i < UVC_MAX_DEVICE; i++)
        {
            if (vdev->in_buff == &g_uvc_buff_pool[i][0])
            {
                UVC_DBGMSG("Free UVC iso-in buffer 0x%x.\n", (int)vdev->in_buff);
                g_uvc_buff_used[i] = 0;
            }
        }
    }
    usbh_free_mem(vdev, sizeof(UVC_DEV_T));
}

static void add_device_to_list(UVC_DEV_T *vdev)
{
    UVC_DEV_T  *v;

    v = g_vdev_list;                        /* Search UVC device list. If this device     */
    while (v != NULL)                       /* found in list, do nothing.                 */
    {
        if (v == vdev)
            return;
        v = v->next;
    }

    if (g_vdev_list == NULL)                /* Add the UVC device into list.              */
    {
        vdev->next = NULL;
        g_vdev_list = vdev;
    }
    else
    {
        vdev->next = g_vdev_list;
        g_vdev_list = vdev;
    }
}

static void remove_device_from_list(UVC_DEV_T *vdev)
{
    UVC_DEV_T  *p;

    if (g_vdev_list == vdev)
    {
        g_vdev_list = g_vdev_list->next;
        return;
    }

    p = g_vdev_list;
    while (p != NULL)
    {
        if (p->next == vdev)
        {
            p->next = vdev->next;
            return;
        }
        p = p->next;
    }
    UVC_DBGMSG("Warning! remove_device_from_list 0x%x not found!\n", (int)vdev);
}


static int  uvc_probe(IFACE_T *iface)
{
    UDEV_T         *udev = iface->udev;
    ALT_IFACE_T    *aif = iface->aif;
    DESC_IF_T      *ifd;
    DESC_VC_IAD_T  *iad;
    UVC_DEV_T      *vdev;
    int            ret;

    ifd = aif->ifd;

    /* Is this interface UVC class? */
    if (ifd->bInterfaceClass != USB_CLASS_VIDEO)
        return USBH_ERR_NOT_MATCHED;

    if (ifd->bInterfaceSubClass == UVC_SC_VIDEO_IF_COLLECT)
    {
        iad = (DESC_VC_IAD_T *)ifd;
        if (iad->bInterfaceCount != 2)
        {
            UVC_DBGMSG("Warning UVC IAD - interface count is not 2!\n");
        }
        return USBH_ERR_NOT_MATCHED;   /* Do not add to device working interface list.   */
    }
    else if (ifd->bInterfaceSubClass == UVC_SC_VIDEOCONTROL)
    {
        UVC_DBGMSG("uvc_probe - device (vid=0x%x, pid=0x%x), control interface %d.\n",
                   udev->descriptor.idVendor, udev->descriptor.idProduct, ifd->bInterfaceNumber);

        vdev = alloc_uvc_device(udev);
        if (vdev == NULL)
            return USBH_ERR_NOT_FOUND;

        vdev->udev = udev;
        iface->context = (void *)vdev;

        ret = uvc_parse_control_interface(vdev, iface);
        if (ret != 0)
        {
            UVC_DBGMSG("Parsing UVC control desceiptor failed! 0x%x\n", ret);
            remove_device_from_list(vdev);
            free_uvc_device(vdev);
            return -1;
        }
    }
    else if (ifd->bInterfaceSubClass == UVC_SC_VIDEOSTREAMING)
    {
        UVC_DBGMSG("uvc_probe - device (vid=0x%x, pid=0x%x), streaming interface %d.\n",
                   udev->descriptor.idVendor, udev->descriptor.idProduct, ifd->bInterfaceNumber);

        vdev = alloc_uvc_device(udev);
        if (vdev == NULL)
            return USBH_ERR_NOT_FOUND;

        vdev->udev = udev;
        iface->context = (void *)vdev;

        ret = uvc_parse_streaming_interface(vdev, iface);
        if (ret != 0)
        {
            UVC_DBGMSG("Parsing UVC control desceiptor failed! 0x%x\n", ret);
            remove_device_from_list(vdev);
            free_uvc_device(vdev);
            return -1;
        }
    }
    else
    {
        UVC_DBGMSG("Unsupported Video Class interface sub-class 0x%x!\n", ifd->bInterfaceSubClass);
        return -1;
    }

    add_device_to_list(vdev);

    if ((vdev->iface_ctrl != NULL) && (vdev->iface_stream != NULL))
    {
        ret = usbh_uvc_probe_control(vdev, UVC_GET_CUR, &vdev->param);
        if (ret < 0)
        {
            UVC_DBGMSG("Get Video Probe Control failed! %d\n", ret);
            remove_device_from_list(vdev);
            free_uvc_device(vdev);
            return ret;
        }
    }

    return 0;
}


static void  uvc_disconnect(IFACE_T *iface)
{
    UVC_DEV_T   *vdev;
    int         i;

    UVC_DBGMSG("UVC device interface %d disconnected!\n", iface->if_num);
    vdev = (UVC_DEV_T *)(iface->context);

    if (vdev == NULL)
        return;                                     /* should have been disconnected.     */

    vdev->is_streaming = 0;                         /* inhibit isochronous transfer       */

    if (iface == vdev->iface_ctrl)
    {
        vdev->iface_ctrl = NULL;
    }

    if (iface == vdev->iface_stream)
    {
        for (i = 0; i < UVC_UTR_PER_STREAM; i++)
        {
            usbh_quit_utr(vdev->utr_rx[i]);          /* quit all UTRs                      */
        }

        for (i = 0; i < UVC_UTR_PER_STREAM; i++)    /* free all UTRs                      */
        {
            if (vdev->utr_rx[i] != NULL)
            {
                free_utr(vdev->utr_rx[i]);
                vdev->utr_rx[i] = NULL;
            }
        }
        vdev->iface_stream = NULL;
    }

    if ((vdev->iface_ctrl == NULL) && (vdev->iface_stream == NULL))
    {
        remove_device_from_list(vdev);
        free_uvc_device(vdev);
    }
}


static UDEV_DRV_T  uvc_driver =
{
    uvc_probe,
    uvc_disconnect,
    NULL,
    NULL,
};


/// @endcond /* HIDDEN_SYMBOLS */


/**
  * @brief    Init USB Host CDC driver.
  * @return   None
  */
void usbh_uvc_init(void)
{
    memset(g_uvc_buff_used, 0, sizeof(g_uvc_buff_used));
    g_vdev_list = NULL;
    usbh_register_driver(&uvc_driver);
}


/**
 *  @brief   Get a list of currently connected USB Hid devices.
 *  @return  List of CDC devices.
 *  @retval  NULL       There's no CDC device found.
 *  @retval  Otherwise  A list of connected CDC devices.
 *
 *  The CDC devices are chained by the "next" member of UVC_DEV_T.
 */
UVC_DEV_T * usbh_uvc_get_device_list(void)
{
    return g_vdev_list;
}


/*@}*/ /* end of group NUC970_USBH_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC970_USBH_Library */

/*@}*/ /* end of group NUC970_Device_Driver */

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/

