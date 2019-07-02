/**************************************************************************//**
 * @file     uvc_core.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 15/06/12 10:12a $
 * @brief    NUC970 MCU USB Host Video Class driver
 *
 * @note
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

static void  dump_parameter_block(UVC_CTRL_PARAM_T *param)
{
    UVC_DBGMSG("\n\nbmHint          = 0x%x\n", param->bmHint);
    UVC_DBGMSG("bFormatIndex    = %d\n", param->bFormatIndex);
    UVC_DBGMSG("bFrameIndex     = %d\n", param->bFrameIndex);
    UVC_DBGMSG("dwFrameInterval = %d\n", param->dwFrameInterval);
    UVC_DBGMSG("wKeyFrameRate   = %d\n", param->wKeyFrameRate);
    UVC_DBGMSG("wPFrameRate     = %d\n", param->wPFrameRate);
    UVC_DBGMSG("dwMaxVideoFrameSize      = %d\n", param->dwMaxVideoFrameSize);
    UVC_DBGMSG("dwMaxPayloadTransferSize = %d\n", param->dwMaxPayloadTransferSize);
    UVC_DBGMSG("bmFramingInfo   = 0x%x\n", param->bmFramingInfo);
    UVC_DBGMSG("bUsage          = 0x%x\n", param->bUsage);
    UVC_DBGMSG("bmSettings      = 0x%x\n", param->bmSettings);
}


/**
 *  @brief  Video Class Request - Get Video Probe Control
 *  @param[in]  vdev   UVC device
 *  @param[in]  req    Control request.
 *                     UVC_SET_CUR
 *                     UVC_GET_CUR
 *                     UVC_GET_MIN
 *                     UVC_GET_MAX
 *                     UVC_GET_RES
 *                     UVC_GET_LEN
 *                     UVC_GET_INFO
 *                     UVC_GET_DEF
 *  @param[out] param  A set of shadow parameters from the UVC device.
 *  @return   Success or failed.
 *  @retval   0        Success
 *  @retval   Otheriwse  Error occurred
 */
int usbh_uvc_probe_control(UVC_DEV_T *vdev, uint8_t req, UVC_CTRL_PARAM_T *param)
{
    uint8_t     bmRequestType;
    uint32_t    xfer_len;
    int         ret;

    if (req & 0x80)
        bmRequestType = REQ_TYPE_IN | REQ_TYPE_CLASS_DEV | REQ_TYPE_TO_IFACE;
    else
        bmRequestType = REQ_TYPE_OUT | REQ_TYPE_CLASS_DEV | REQ_TYPE_TO_IFACE;

    ret = usbh_ctrl_xfer(vdev->udev, bmRequestType, req,
                         (VS_PROBE_CONTROL << 8),    /* wValue - Control Selector (CS)    */
                         vdev->iface_stream->if_num, /* wIndex - Zero and Interface       */
                         sizeof(UVC_CTRL_PARAM_T),   /* wLength - Length of parameter block */
                         (uint8_t *)param,           /* parameter block                   */
                         &xfer_len, UVC_REQ_TIMEOUT);
    if (ret != 0)
    {
        UVC_DBGMSG("usbh_uvc_probe_control incorrect transfer length! %d %d\n", ret, xfer_len);
    }
    else if (req == UVC_GET_CUR)
    {
        dump_parameter_block(param);
    }
    return ret;
}

/**
 *  @brief  Video Class Request - Set Video Commit Control
 *  @param[in]  vdev   UVC device
 *  @param[out] param  A set of shadow parameters from the UVC device.
 *  @return   Success or failed.
 *  @retval   0        Success
 *  @retval   Otheriwse  Error occurred
 */
static int usbh_uvc_commit_control(UVC_DEV_T *vdev, UVC_CTRL_PARAM_T *param)
{
    uint8_t     bmRequestType;
    uint32_t    xfer_len;

    bmRequestType = REQ_TYPE_OUT | REQ_TYPE_CLASS_DEV | REQ_TYPE_TO_IFACE;

    return usbh_ctrl_xfer(vdev->udev, bmRequestType, UVC_SET_CUR,
                          (VS_COMMIT_CONTROL << 8),   /* wValue - Control Selector (CS)    */
                          vdev->iface_stream->if_num, /* wIndex - Zero and Interface       */
                          sizeof(UVC_CTRL_PARAM_T),   /* wLength - Length of parameter block */
                          (uint8_t *)param,           /* parameter block                   */
                          &xfer_len, UVC_REQ_TIMEOUT);
}


/*
 *  Based on the current parameter block information, select the best-fit alternative interface of
 *  UVC streaming interface.
 */
static int  usbh_uvc_select_alt_interface(UVC_DEV_T *vdev)
{
    IFACE_T      *iface;
    UVC_STRM_T   *vs = &vdev->vs;
    uint32_t     payload_size = vdev->param.dwMaxPayloadTransferSize;
    int          i, ret, best = -1;

    /*------------------------------------------------------------------------------------*/
    /*  Find the streaming interface                                                      */
    /*------------------------------------------------------------------------------------*/
    iface = vdev->udev->iface_list;
    while (iface != NULL)
    {
        if (iface->if_num == vdev->iface_stream->if_num)
            break;
        iface = iface->next;
    }
    if (iface == NULL)
    {
        UVC_DBGMSG("Can't find UVC streaming interface!\n");
        return UVC_RET_NOT_SUPPORT;
    }

    /*------------------------------------------------------------------------------------*/
    /*  Find the the best alternative interface                                           */
    /*------------------------------------------------------------------------------------*/
    if (payload_size > 3072)
        payload_size = 3072;

    /*
     *  Find the largest one of those "wMaxPacketSize <= 3072" settings
     */
    for (i = 0; i < vs->num_of_alt; i++)
    {
        if (vs->max_pktsz[i] <= 3072)
        {
            if (best == -1)
                best = i;
            else
            {
                if (vs->max_pktsz[i] > vs->max_pktsz[best])
                    best = i;
            }
        }
    }

    for (i = 0; i < vs->num_of_alt; i++)
    {
        UVC_DBGMSG("i=%d, best=%d, %d, %d\n", i, best, vs->max_pktsz[i], payload_size);
        if ((vs->max_pktsz[i] >= payload_size) && (vs->max_pktsz[i] <= 3072))
        {
            if (best == -1)
                best = i;
            else
            {
                if (vs->max_pktsz[i] < vs->max_pktsz[best])
                    best = i;
            }
        }
    }

    if (best == -1)
    {
        UVC_DBGMSG("Bandwidth - Cannot find available alternative interface! %d\n", payload_size);
        return UVC_RET_NOT_SUPPORT;
    }

    UVC_DBGMSG("Select UVC streaming interface %d alternative setting %d.\n", iface->if_num, vs->alt_no[best]);
    ret = usbh_set_interface(iface, vs->alt_no[best]);
    if (ret != 0)
    {
        UVC_DBGMSG("Fail to set UVC streaming interface! %d, %d\n", iface->if_num, vs->alt_no[best]);
        return ret;
    }

    vdev->ep_iso_in = usbh_iface_find_ep(iface, 0, EP_ADDR_DIR_IN | EP_ATTR_TT_ISO);
    if (vdev->ep_iso_in == NULL)
    {
        UVC_DBGMSG("Can't find iso-in enpoint in the selected streaming interface! %d, %d\n", iface->if_num, best);
        return UVC_RET_NOT_SUPPORT;
    }

    return 0;
}


/// @endcond HIDDEN_SYMBOLS


/**
 *  @brief  Get a video format from support list of the UVC device
 *  @param[in]  vdev    UVC device
 *  @param[in]  index   Index probe to supportd image format
 *  @param[out] format  If success, return the image format.
 *  @param[out] width   If success, return the image width.
 *  @param[out] height  If success, return the image height.
 *  @return   Success or failed.
 *  @retval   0          Success
 *  @retval   -1         Nonexistent
 *  @retval   Otheriwse  Error occurred
 */
int  usbh_get_video_format(UVC_DEV_T *vdev, int index, IMAGE_FORMAT_E *format, int *width, int *height)
{
    UVC_CTRL_T  *vc;

    if (vdev == NULL)
        return UVC_RET_DEV_NOT_FOUND;

    vc = &vdev->vc;

    if (index >= vc->num_of_frames)
        return -1;

    *format = vc->frame_format[index];
    *width  = vc->width[index];
    *height = vc->height[index];
    return 0;
}

/**
 *  @brief  Set video format
 *  @param[in]  vdev    UVC device
 *  @param[out] format  Image format
 *
 *  @param[out] width   Image width
 *  @param[out] height  Image height
 *  @return   Success or failed.
 *  @retval   0          Success
 *  @retval   Otheriwse  Error occurred
 */
int  usbh_set_video_format(UVC_DEV_T *vdev, IMAGE_FORMAT_E format, int width, int height)
{
    UVC_CTRL_T       *vc;
    UVC_CTRL_PARAM_T *param;
    int    format_index = -1, frame_index = -1;
    int    i, ret;

    if (vdev == NULL)
        return UVC_RET_DEV_NOT_FOUND;

    vc = &vdev->vc;
    param = &vdev->param;

    /*------------------------------------------------------------------------------------*/
    /*  Find video format index                                                           */
    /*------------------------------------------------------------------------------------*/
    for (i = 0; i < vc->num_of_formats; i++)
    {
        if (vc->format[i] == format)
        {
            format_index = vc->format_idx[i];
            break;
        }
    }
    if (format_index == -1)
    {
        UVC_DBGMSG("Video format 0x%x not supported!\n", format);
        return UVC_RET_NOT_SUPPORT;
    }

    /*------------------------------------------------------------------------------------*/
    /*  Find video frame index                                                            */
    /*------------------------------------------------------------------------------------*/
    for (i = 0; i < vc->num_of_frames; i++)
    {
        if (vc->frame_format[i] != format)
            continue;

        if ((vc->width[i] == width) && (vc->height[i] == height))
        {
            frame_index = vc->frame_idx[i];
            break;
        }
    }
    if (frame_index == -1)
    {
        UVC_DBGMSG("Video size %d x %d not supported!\n", width, height);
        return UVC_RET_NOT_SUPPORT;
    }

    UVC_DBGMSG("Video format found, bFormatIndex=%d, bFrameIndex=%d\n", format_index, frame_index);

    /*------------------------------------------------------------------------------------*/
    /*  Get Video Probe Control                                                           */
    /*------------------------------------------------------------------------------------*/
    ret = usbh_uvc_probe_control(vdev, UVC_GET_CUR, param);
    if (ret < 0)
    {
        UVC_DBGMSG("Get Video Probe Control failed! %d\n", ret);
        return ret;
    }

    if ((param->bFormatIndex == format_index) && (param->bFrameIndex == frame_index))
    {
        goto commit;
    }

    /*------------------------------------------------------------------------------------*/
    /*  Set Video Probe Control                                                           */
    /*------------------------------------------------------------------------------------*/

    param->bFormatIndex = format_index;
    param->bFrameIndex = frame_index;

    ret = usbh_uvc_probe_control(vdev, UVC_SET_CUR, param);
    if (ret < 0)
    {
        UVC_DBGMSG("Set Video Probe Control failed! %d\n", ret);
        return ret;
    }

    ret = usbh_uvc_probe_control(vdev, UVC_GET_CUR, param);
    if (ret < 0)
    {
        UVC_DBGMSG("Get Video Probe Control failed! %d\n", ret);
        return ret;
    }

    if ((param->bFormatIndex != format_index) && (param->bFrameIndex != frame_index))
    {
        return UVC_RET_NOT_SUPPORT;
    }

commit:
    /*------------------------------------------------------------------------------------*/
    /*  Set Video Commit Control                                                          */
    /*------------------------------------------------------------------------------------*/
    ret = usbh_uvc_commit_control(vdev, param);
    if (ret < 0)
    {
        UVC_DBGMSG("Get Video Probe Control failed! %d\n", ret);
        return ret;
    }
    return ret;
}


/// @cond HIDDEN_SYMBOLS


void  uvc_parse_streaming_data(UVC_DEV_T *vdev, uint8_t *buff, int pkt_len)
{
    UVC_STRM_T   *vs = &vdev->vs;
    int          data_len;

    if (pkt_len < 2)
        return;                             /* invalid packet                             */

    if (pkt_len < buff[0])
        return;                             /* unlikely pakcet length error               */

    data_len = pkt_len - buff[0];

    if (vs->current_frame_error)
    {
        if (buff[1] & UVC_PL_EOF)           /* error cleared only if EOF met              */
        {
            vs->current_frame_error = 0;
            vdev->img_size = 0;
        }
        return;
    }

    if (vdev->img_size == 0)                /* Start of a new image                       */
    {
        vs->current_frame_toggle = buff[1] & UVC_PL_FID;
        if (data_len > 0)
        {
            memcpy(vdev->img_buff, buff+buff[0], data_len);
            vdev->img_size = data_len;
            // sysprintf("![%d] %x %x %x %x %x\n", vdev->img_size, vdev->img_buff[0], vdev->img_buff[1], vdev->img_buff[2], vdev->img_buff[3], vdev->img_buff[4]);
            return;
        }
    }
    else
    {
        if ((buff[1] & UVC_PL_FID) != vs->current_frame_toggle)
        {
            UVC_DBGMSG("FID toggle error!\n");
            vs->current_frame_error = 1;
            return;
        }
        if (buff[1] & UVC_PL_ERR)
        {
            UVC_DBGMSG("Payload ERR bit error!\n");
            vs->current_frame_error = 1;
            return;
        }

        if ((buff[1] & UVC_PL_RES) && (buff[1] & UVC_PL_PTS))
        {
            if (vdev->func_rx && (vdev->img_size > 0))
                vdev->func_rx(vdev, vdev->img_buff, vdev->img_size);
            vdev->img_size = 0;
            return;
        }

        if (vdev->img_size + data_len > vdev->img_buff_size)
        {
            UVC_DBGMSG("Image data overrun!\n");
            vs->current_frame_error = 1;
            return;
        }

        if (data_len > 0)
        {
            memcpy(vdev->img_buff + vdev->img_size, buff+buff[0], data_len);
            vdev->img_size += data_len;
        }

        if (buff[1] & UVC_PL_EOF)
        {
            if (vdev->func_rx && (vdev->img_size > 0))
                vdev->func_rx(vdev, vdev->img_buff, vdev->img_size);

            vdev->img_size = 0;
        }
    }
}


static void iso_in_irq(UTR_T *utr)
{
    UVC_DEV_T   *vdev = (UVC_DEV_T *)utr->context;
    int         i, ret;

    /* We don't want to do anything if we are about to be removed! */
    if (!vdev || !vdev->udev)
        return;

    if (vdev->is_streaming == 0)
    {
        UVC_DBGMSG("iso_in_irq stop utr 0x%x\n", (int)utr);
        utr->status = USBH_ERR_ABORT;
        return;
    }

    // UVC_DBGMSG("SF=%d, 0x%x\n", utr->iso_sf, (int)utr);

    utr->bIsoNewSched = 0;

    for (i = 0; i < IF_PER_UTR; i++)
    {
        if (utr->iso_status[i] == 0)
        {
            uvc_parse_streaming_data(vdev, utr->iso_buff[i], utr->iso_xlen[i]);
        }
        else
        {
            // UVC_DBGMSG("Iso %d err - %d\n", i, utr->iso_status[i]);
            if ((utr->iso_status[i] == USBH_ERR_NOT_ACCESS0) || (utr->iso_status[i] == USBH_ERR_NOT_ACCESS1))
                utr->bIsoNewSched = 1;
        }
        utr->iso_xlen[i] = utr->ep->wMaxPacketSize;
    }

    /* schedule the following isochronous transfers */
    ret = usbh_iso_xfer(utr);
    if (ret < 0)
    {
        UVC_DBGMSG("usbh_iso_xfer failed!\n");
        utr->status = USBH_ERR_ABORT;
    }
}

/// @endcond HIDDEN_SYMBOLS

/**
 *  @brief  Give the image buffer where the next received image will be written to.
 *  @param[in] vdev           Video Class device
 *  @param[in] image_buff     The image buffer.
 *  @param[in] img_buff_size  Size of the image buffer.
 *  @return    None.
 */
void usbh_uvc_set_video_buffer(UVC_DEV_T *vdev, uint8_t *image_buff, int img_buff_size)
{
    vdev->img_buff = image_buff;
    vdev->img_buff_size = img_buff_size;
    vdev->img_size = 0;
}


/**
 *  @brief  Start to receive video data from UVC device.
 *  @param[in] vdev       Video Class device
 *  @param[in] func       Video in callback function.
 *  @return   Success or not.
 *  @retval    0          Success
 *  @retval    Otherwise  Failed
 */
int usbh_uvc_start_streaming(UVC_DEV_T *vdev, UVC_CB_FUNC *func)
{
    UDEV_T       *udev = vdev->udev;
    EP_INFO_T    *ep;
    UTR_T        *utr;
    int          i, j, ret;

    if ((vdev == NULL) || (func == NULL))
        return UVC_RET_INVALID;

    if (vdev->is_streaming)
        return UVC_RET_IS_STREAMING;

    /*
     *  Select the best alternative streaming interface and also determine the endpoint.
     */
    ret = usbh_uvc_select_alt_interface(vdev);
    if (ret != 0)
    {
        UVC_ERRMSG("Failed to select UVC alternative interface!\n");
        return ret;
    }
    ep = vdev->ep_iso_in;

    vdev->func_rx = func;

#ifdef UVC_DEBUG
    UVC_DBGMSG("Actived isochronous-in endpoint =>");
    usbh_dump_ep_info(ep);
#endif

    /*------------------------------------------------------------------------------------*/
    /*  Allocate isochronous in UTRs and assign transfer buffer                           */
    /*------------------------------------------------------------------------------------*/
    for (i = 0; i < UVC_UTR_PER_STREAM; i++)
    {
        if (vdev->utr_rx[i] != NULL)
        {
            vdev->utr_rx[i]->status = 0;
            continue;
        }

        utr = alloc_utr(udev);              /* allocate UTR                               */
        if (utr == NULL)
        {
            ret = USBH_ERR_MEMORY_OUT;      /* memory allocate failed                     */
            goto err_2;                     /* abort                                      */
        }
        vdev->utr_rx[i] = utr;
        utr->buff = vdev->in_buff + i * UVC_UTR_INBUF_SIZE;
        utr->data_len = UVC_UTR_INBUF_SIZE;

        for (j = 0; j < IF_PER_UTR; j++)
        {
            utr->iso_buff[j] = utr->buff + j * 3072;
            utr->iso_xlen[j] = ep->wMaxPacketSize;
        }
    }

    /*------------------------------------------------------------------------------------*/
    /*  Start UTRs                                                                        */
    /*------------------------------------------------------------------------------------*/

    vdev->utr_rx[0]->bIsoNewSched = 1;
    vdev->is_streaming = 1;

    for (i = 0; i < UVC_UTR_PER_STREAM; i++)
    {
        utr = vdev->utr_rx[i];
        utr->context = vdev;
        utr->ep = ep;
        utr->func = iso_in_irq;
        ret = usbh_iso_xfer(utr);
        if (ret < 0)
        {
            UVC_DBGMSG("Error - failed to start UTR %d isochronous-in transfer (%d)", i, ret);
            goto err_1;
        }
    }

    return UVC_RET_OK;

err_1:
    for (i = 0; i < UVC_UTR_PER_STREAM; i++)
    {
        usbh_quit_utr(vdev->utr_rx[i]);         /* quit all UTRs                          */
    }

err_2:
    for (i = 0; i < UVC_UTR_PER_STREAM; i++)    /* free all UTRs                          */
    {
        if (vdev->utr_rx[i] != NULL)
        {
            free_utr(vdev->utr_rx[i]);
            vdev->utr_rx[i] = NULL;
        }
    }

    return ret;
}


/**
 *  @brief  Pause the video straming input.
 *  @param[in] vdev       Video Class device
 *  @return   Success or not.
 *  @retval    0          Success
 *  @retval    Otherwise  Failed
 */
int usbh_uvc_stop_streaming(UVC_DEV_T *vdev)
{
    IFACE_T   *iface;
    int       ret;

    if (vdev == NULL)
        return UVC_RET_INVALID;

    if (!vdev->is_streaming)
        return UVC_RET_OK;                  /* UVC is currently not straming, do nothing  */

    vdev->is_streaming = 0;

    /*------------------------------------------------------------------------------------*/
    /*  Find the streaming interface                                                      */
    /*------------------------------------------------------------------------------------*/
    iface = vdev->udev->iface_list;
    while (iface != NULL)
    {
        if (iface->if_num == vdev->iface_stream->if_num)
            break;
        iface = iface->next;
    }
    if (iface == NULL)
    {
        UVC_DBGMSG("Can't find UVC streaming interface!\n");
        return UVC_RET_NOT_SUPPORT;
    }

    ret = usbh_set_interface(iface, 0);     /* select alternative setting (0 bandwidth)   */
    if (ret != 0)
    {
        UVC_DBGMSG("Fail to select UVC streaming interface alt. 0! %d\n", iface->if_num);
        return ret;
    }

    return 0;
}


/*@}*/ /* end of group NUC970_USBH_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC970_USBH_Library */

/*@}*/ /* end of group NUC970_Device_Driver */

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/

