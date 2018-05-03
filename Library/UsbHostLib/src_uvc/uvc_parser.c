/**************************************************************************//**
 * @file     uvc_parser.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 15/06/10 2:03p $
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


/// @cond HIDDEN_SYMBOLS


/**
 *  @brief  Parse and get Video Control interface information from descriptors.
 *  @param[in]  vdev    UVC device
 *  @param[in]  iface   Video Control interface
 *  @return   Success or failed.
 *  @retval   0        Success
 *  @retval   Otheriwse  Error occurred
 */
int uvc_parse_control_interface(UVC_DEV_T *vdev, IFACE_T *iface)
{
    DESC_CONF_T    *config;
    DESC_IF_T      *ifd;
    DESC_VC_HDR_T  *ifd_vc_hdr;
    uint8_t        *bptr;
    int            size;

    UVC_DBGMSG("UVC parsing video control interface %d...\n", iface->if_num);

    vdev->iface_ctrl = iface;

    bptr = vdev->udev->cfd_buff;
    config = (DESC_CONF_T *)bptr;

    /* step over configuration descritpor */
    bptr += config->bLength;
    size = config->wTotalLength - config->bLength;

    /*------------------------------------------------------------------------------------*/
    /*  Find this Standard Video Control Interface Descriptor                             */
    /*------------------------------------------------------------------------------------*/
    while (size >= sizeof(DESC_IF_T))
    {
        ifd = (DESC_IF_T *)bptr;

        if ((ifd->bDescriptorType == USB_DT_INTERFACE) && (ifd->bInterfaceNumber == iface->if_num))
            break;

        if (ifd->bLength == 0)
            return UVC_RET_PARSER;          /* prevent infinit loop                       */

        bptr += ifd->bLength;
        size -= ifd->bLength;
    }

    if (size < sizeof(DESC_IF_T))
    {
        /* cannot find the Standard VC descriptor     */
        UVC_ERRMSG("UVC_RET_PARSER! - Can't find the Standard Video Control Interface!\n");
        return UVC_RET_PARSER;
    }

    vdev->version = ifd->bInterfaceProtocol;

    bptr += ifd->bLength;
    size -= ifd->bLength;

    /*------------------------------------------------------------------------------------*/
    /*  Parsing Video Control interface VC header descriptor                              */
    /*------------------------------------------------------------------------------------*/
    ifd_vc_hdr = (DESC_VC_HDR_T *)bptr;

    if ((ifd_vc_hdr->bDescriptorType != UVC_CS_INTERFACE) || (ifd_vc_hdr->bDescriptorSubType != VC_HEADER))
    {
        UVC_ERRMSG("UVC VC_HEADER not found!! 0x%x 0x%x\n", ifd_vc_hdr->bDescriptorType, ifd_vc_hdr->bDescriptorSubType);
        return UVC_RET_PARSER;
    }

    if (ifd_vc_hdr->bInCollection != 1)
    {
        UVC_ERRMSG("UVC - number of streaming interface is not 1! Not supported!! %d\n", ifd_vc_hdr->bInCollection);
        return UVC_RET_DRV_NOT_SUPPORTED;
    }

    UVC_DBGMSG("VC Interface VC_HEADER\n");
    UVC_DBGMSG("    bcdUVC: 0%x\n", ifd_vc_hdr->bcdUVC);
    UVC_DBGMSG("    baInterfaceNr: 0%x\n", ifd_vc_hdr->baInterfaceNr);

    // ifnum_strm = ifd_vc_hdr->baInterfaceNr;
    bptr += ifd_vc_hdr->bLength;
    size = ifd_vc_hdr->wTotalLength - ifd_vc_hdr->bLength;

    /*------------------------------------------------------------------------------------*/
    /*  Walk though Video Control interface descriptor group                              */
    /*------------------------------------------------------------------------------------*/
    while (size >= sizeof(DESC_HDR_T))
    {
#ifdef UVC_DEBUG
        DESC_VC_IT_T  *ifd_it;
        DESC_VC_OT_T  *ifd_ot;
        DESC_VC_SU_T  *ifd_su;
        DESC_VC_PU_T  *ifd_pu;
        DESC_VC_EU_T  *ifd_eu;
        DESC_VC_ECU_T *ifd_ecu;
        int           i;
#endif

        ifd = (DESC_IF_T *)bptr;

        //UVC_DBGMSG("Parse VC - [%d] [0x%x] [0x%x]\n", ((CS_HDR_T *)bptr)->bLength, ((CS_HDR_T *)bptr)->bDescriptorType, ((CS_HDR_T *)bptr)->bDescriptorSubtype);

        if (ifd->bDescriptorType == USB_DT_ENDPOINT)
            break;

        if (ifd->bDescriptorType != UVC_CS_INTERFACE)
        {
            UVC_DBGMSG("UVC VC descriptor parsing length error!! %d\n", size);
            return UVC_RET_PARSER;
        }

#ifdef UVC_DEBUG
        ifd_it = (DESC_VC_IT_T *)ifd;

        switch (ifd_it->bDescriptorSubType)
        {
        case VC_INPUT_TERMINAL:
            ifd_it = (DESC_VC_IT_T *)ifd;
            UVC_DBGMSG("VC Interface VC_INPUT_TERMINAL\n");
            UVC_DBGMSG("    bTerminalID:    0x%x\n", ifd_it->bTerminalID);
            UVC_DBGMSG("    wTerminalType:  0x%x\n", ifd_it->wTerminalType);
            UVC_DBGMSG("    bAssocTerminal: 0x%x\n", ifd_it->bAssocTerminal);
            break;

        case VC_OUTPUT_TERMINAL:
            ifd_ot = (DESC_VC_OT_T *)ifd;
            UVC_DBGMSG("VC Interface VC_OUTPUT_TERMINAL\n");
            UVC_DBGMSG("    bTerminalID:    0x%x\n", ifd_ot->bTerminalID);
            UVC_DBGMSG("    wTerminalType:  0x%x\n", ifd_ot->wTerminalType);
            UVC_DBGMSG("    bAssocTerminal: 0x%x\n", ifd_ot->bAssocTerminal);
            UVC_DBGMSG("    bSourceID:      0x%x\n", ifd_ot->bSourceID);
            if (ifd_ot->wTerminalType == TT_STREAMING)
            {
                UVC_DBGMSG("UVC USB OT found.\n");
            }
            break;

        case VC_SELECTOR_UNIT:
            ifd_su = (DESC_VC_SU_T *)ifd;
            UVC_DBGMSG("VC Interface VC_SELECTOR_UNIT\n");
            UVC_DBGMSG("    bUnitID:    0x%x\n", ifd_su->bUnitID);
            UVC_DBGMSG("    bNrInPins:  0x%x\n", ifd_su->bNrInPins);
            for (i = 0; i < ifd_su->bNrInPins; i++)
                UVC_DBGMSG("    bSourceID:  0x%x\n", ifd_su->bSourceID[i]);
            break;

        case VC_PROCESSING_UNIT:
            ifd_pu = (DESC_VC_PU_T *)ifd;
            UVC_DBGMSG("VC Interface VC_PROCESSING_UNIT\n");
            UVC_DBGMSG("    bUnitID:          0x%x\n", ifd_pu->bUnitID);
            UVC_DBGMSG("    wMaxMultiplier:   0x%x\n", ifd_pu->wMaxMultiplier);
            UVC_DBGMSG("    bControlSize:     0x%x\n", ifd_pu->bControlSize);
            UVC_DBGMSG("    bmControls:       0x%02x%02x%02x\n", ifd_pu->bmControls[2], ifd_pu->bmControls[1], ifd_pu->bmControls[0]);
            UVC_DBGMSG("    bmVideoStandards: 0x%x\n", ifd_pu->bmVideoStandards);
            break;

        case VC_EXTENSION_UNIT:
            ifd_eu = (DESC_VC_EU_T *)ifd;
            UVC_DBGMSG("VC Interface VC_EXTENSION_UNIT\n");
            UVC_DBGMSG("    bUnitID:      0x%x\n", ifd_eu->bUnitID);
            UVC_DBGMSG("    bNumControls: 0x%x\n", ifd_eu->bNumControls);
            UVC_DBGMSG("    bNrInPins:    0x%x\n", ifd_eu->bNrInPins);
            break;

        case VC_ENCODING_UNIT:
            ifd_ecu = (DESC_VC_ECU_T *)ifd;
            UVC_DBGMSG("VC Interface VC_ENCODING_UNIT\n");
            UVC_DBGMSG("    bUnitID:   0x%x\n", ifd_ecu->bUnitID);
            UVC_DBGMSG("    bSourceID: 0x%x\n", ifd_ecu->bSourceID);
            UVC_DBGMSG("    iEncoding: 0x%x\n", ifd_ecu->iEncoding);
            break;
        }
#endif

        if (ifd->bLength == 0)
            return UVC_RET_PARSER;          /* prevent infinite loop                      */

        bptr += ifd->bLength;
        size -= ifd->bLength;
    }

    /*------------------------------------------------------------------------------------*/
    /*  Parsing Endpoint descriptors of Video Control interface                           */
    /*------------------------------------------------------------------------------------*/
    while (size > sizeof(DESC_HDR_T))
    {
        DESC_EP_T   *epd;

        epd = (DESC_EP_T *)bptr;

        if (epd->bDescriptorType != USB_DT_ENDPOINT)
        {
            UVC_DBGMSG("UVC VC interface endpoint parsing error, remain %d bytes!\n", size);
            break;
        }

        if ((epd->bDescriptorType == USB_DT_ENDPOINT) && (epd->bmAttributes == 0x03))
        {
            //vdev->ep_sts = usbh_iface_find_ep(iface, epd->bEndpointAddress, 0);
            //if (vdev->ep_sts == NULL)
            //  UVC_DBGMSG("UVC find interrupt in endpoint failed!\n");
            //else
            //    UVC_DBGMSG("UVC interrupt in endpoint 0x%x foudn.\n", epd->bEndpointAddress);
        }

        if (ifd->bLength == 0)
            return UVC_RET_PARSER;          /* prevent infinite loop                      */

        bptr += epd->bLength;
        size -= epd->bLength;

        if (epd->bDescriptorType != USB_DT_ENDPOINT)
        {
            UVC_DBGMSG("UVC VC interface endpoint parsing error, remain %d bytes!\n", size);
            break;
        }
    }

    if (size != 0)
    {
        UVC_DBGMSG("Warning - VC interface parsing problem, remain %d bytes!\n", size);
    }

    return 0;
}


/**
 *  @brief  Parse and get Video Streaming interface information from descriptors.
 *  @param[in]  vdev    UVC device
 *  @param[in]  iface   Video Streaming interface
 *  @return   Success or failed.
 *  @retval   0        Success
 *  @retval   Otheriwse  Error occurred
 */
int uvc_parse_streaming_interface(UVC_DEV_T *vdev, IFACE_T *iface)
{
    DESC_CONF_T    *config;
    DESC_IF_T      *ifd;
    DESC_EP_T      *epd;
    DESC_VSI_HDR_T *ifd_vs_hdr;
    UVC_CTRL_T     *vc = &vdev->vc;
    UVC_STRM_T     *vs = &vdev->vs;
    uint8_t        *bptr;
    int            i, idx, size;

    UVC_DBGMSG("UVC parsing video streaming interface %d...\n", iface->if_num);

    vdev->iface_stream = iface;

    bptr = vdev->udev->cfd_buff;
    config = (DESC_CONF_T *)bptr;

    /* step over configuration descritpor */
    bptr += config->bLength;
    size = config->wTotalLength - config->bLength;

    /*------------------------------------------------------------------------------------*/
    /*  Find the starting of Standard Video Streaming Interface Descriptor                */
    /*------------------------------------------------------------------------------------*/
    while (size >= sizeof(DESC_IF_T))
    {
        ifd = (DESC_IF_T *)bptr;

        if ((ifd->bDescriptorType == USB_DT_INTERFACE) && (ifd->bInterfaceNumber == iface->if_num) &&
                (ifd->bInterfaceClass == USB_CLASS_VIDEO) && (ifd->bInterfaceSubClass == UVC_SC_VIDEOSTREAMING))
            break;

        if (ifd->bLength == 0)
            return UVC_RET_PARSER;          /* prevent infinite loop                      */

        bptr += ifd->bLength;
        size -= ifd->bLength;
    }

    if (size < sizeof(DESC_IF_T))
    {
        UVC_ERRMSG("UVC_RET_PARSER! - Can't find the Standard Video Streaming Interface!\n");
        return UVC_RET_PARSER;
    }

    bptr += ifd->bLength;
    size -= ifd->bLength;

    /*------------------------------------------------------------------------------------*/
    /*  Parsing Video Streaming interface alternative setting 0                           */
    /*------------------------------------------------------------------------------------*/
    while (size >= sizeof(DESC_IF_T))
    {
        DESC_VSU_FORMAT_T   *vsu_format;
        DESC_VSU_FRAME_T    *vsu_frame;
        DESC_MJPG_FORMAT_T  *mjpg_format;
        DESC_MJPG_FRAME_T   *mjpg_frame;

        ifd = (DESC_IF_T *)bptr;

        // sysprintf("[%d] 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", size, bptr[0], bptr[1], bptr[2], bptr[3], bptr[4], bptr[5], bptr[6], bptr[7]);

        if (ifd->bDescriptorType != UVC_CS_INTERFACE)
            break;

        ifd_vs_hdr = (DESC_VSI_HDR_T *)bptr;

        switch (ifd_vs_hdr->bDescriptorSubType)
        {
        case VS_INPUT_HEADER:
            UVC_DBGMSG("VS Interface VS_INPUT_HEADER\n");
            UVC_DBGMSG("    bNumFormats:      0x%x\n", ifd_vs_hdr->bNumFormats);
            UVC_DBGMSG("    bEndpointAddress: 0x%x\n", ifd_vs_hdr->bEndpointAddress);
            UVC_DBGMSG("    bTerminalLink:    0x%x\n", ifd_vs_hdr->bTerminalLink);
            break;

        case VS_FORMAT_UNCOMPRESSED:
            vsu_format = (DESC_VSU_FORMAT_T *)ifd_vs_hdr;
            UVC_DBGMSG("VS Interface VS_FORMAT_UNCOMPRESSED\n");
            UVC_DBGMSG("    bFormatIndex:         0x%x\n", vsu_format->bFormatIndex);
            UVC_DBGMSG("    bNumFrameDescriptors: 0x%x\n", vsu_format->bNumFrameDescriptors);
            UVC_DBGMSG("    guidFormat:           0x%08x%08x%08x%08x\n", vsu_format->guidFormat[0], vsu_format->guidFormat[1], vsu_format->guidFormat[2], vsu_format->guidFormat[3]);
            UVC_DBGMSG("    bBitsPerPixel         0x%x\n", vsu_format->bBitsPerPixel);
            UVC_DBGMSG("    bDefaultFrameIndex:   0x%x\n", vsu_format->bDefaultFrameIndex);
            UVC_DBGMSG("    bAspectRatioX:        0x%x\n", vsu_format->bAspectRatioX);
            UVC_DBGMSG("    bAspectRatioY:        0x%x\n", vsu_format->bAspectRatioY);

            idx = vc->num_of_formats;
            vc->format_idx[idx] = vsu_format->bFormatIndex;

            if (vsu_format->guidFormat[0] == 0x32595559)
            {
                vc->format[idx] = UVC_FORMAT_YUY2;
            }
            else if (vsu_format->guidFormat[0] == 0x3231564E)
            {
                vc->format[idx] = UVC_FORMAT_NV12;
            }
            else if (vsu_format->guidFormat[0] == 0x3032344D)
            {
                vc->format[idx] = UVC_FORMAT_M420;
            }
            else if (vsu_format->guidFormat[0] == 0x30323449)
            {
                vc->format[idx] = UVC_FORMAT_I420;
            }
            vc->num_of_formats++;

            /*------------------------------------------------------------------------*/
            /*  Parsing all frame descriptors under this format descriptor            */
            /*------------------------------------------------------------------------*/
            bptr += vsu_format->bLength;
            size -= vsu_format->bLength;

            for (i = 0; (i < vsu_format->bNumFrameDescriptors) && (vc->num_of_frames < UVC_MAX_FRAME); i++)
            {
                vsu_frame = (DESC_VSU_FRAME_T *)bptr;

                if ((vsu_frame->bDescriptorType != UVC_CS_INTERFACE) ||
                        (vsu_frame->bDescriptorSubType != VS_FRAME_UNCOMPRESSED))
                {
                    UVC_DBGMSG("Frame parsing error!!\n");
                    return UVC_RET_PARSER;
                }

                UVC_DBGMSG("VS Interface VS_FRAME_UNCOMPRESSED\n");
                UVC_DBGMSG("    bFormatIndex: 0x%x\n", vsu_frame->bFrameIndex);
                UVC_DBGMSG("    wWidth:       0x%x\n", vsu_frame->wWidth);
                UVC_DBGMSG("    wHeight       0x%x\n", vsu_frame->wHeight);
                UVC_DBGMSG("    dwMinBitRate: 0x%x\n", vsu_frame->dwMinBitRate);
                UVC_DBGMSG("    dwMaxBitRate: 0x%x\n", vsu_frame->dwMaxBitRate);
                UVC_DBGMSG("    dwDefaultFrameInterval:  0x%x\n", vsu_frame->dwDefaultFrameInterval);

                idx = vc->num_of_frames;
                vc->frame_idx[idx] = vsu_frame->bFrameIndex;
                vc->frame_format[idx] = vc->format[vc->num_of_formats-1];
                vc->width[idx] = vsu_frame->wWidth;
                vc->height[idx] = vsu_frame->wHeight;
                vc->num_of_frames++;

                bptr += vsu_frame->bLength;
                size -= vsu_frame->bLength;
            }
            ifd = (DESC_IF_T *)bptr;  /* must */
            break;

#if 0
        case VS_FRAME_UNCOMPRESSED:
            vsu_frame = (DESC_VSU_FRAME_T *)ifd_vs_hdr;
            UVC_DBGMSG("VS Interface VS_FRAME_UNCOMPRESSED\n");
            UVC_DBGMSG("    bFormatIndex: 0x%x\n", vsu_frame->bFrameIndex);
            UVC_DBGMSG("    wWidth:       0x%x\n", vsu_frame->wWidth);
            UVC_DBGMSG("    wHeight       0x%x\n", vsu_frame->wHeight);
            UVC_DBGMSG("    dwMinBitRate: 0x%x\n", vsu_frame->dwMinBitRate);
            UVC_DBGMSG("    dwMaxBitRate: 0x%x\n", vsu_frame->dwMaxBitRate);
            UVC_DBGMSG("    dwDefaultFrameInterval:  0x%x\n", vsu_frame->dwDefaultFrameInterval);
            break;
#endif

        case VS_FORMAT_MJPEG:
            mjpg_format = (DESC_MJPG_FORMAT_T *)ifd_vs_hdr;
            UVC_DBGMSG("VS Interface VS_FORMAT_MJPEG\n");
            UVC_DBGMSG("    bFormatIndex:         0x%x\n", mjpg_format->bFormatIndex);
            UVC_DBGMSG("    bNumFrameDescriptors: 0x%x\n", mjpg_format->bNumFrameDescriptors);
            UVC_DBGMSG("    bAspectRatioX:        0x%x\n", mjpg_format->bAspectRatioX);
            UVC_DBGMSG("    bAspectRatioY:        0x%x\n", mjpg_format->bAspectRatioY);

            idx = vc->num_of_formats;
            vc->format_idx[idx] = mjpg_format->bFormatIndex;
            vc->format[idx] = UVC_FORMAT_MJPEG;
            vc->num_of_formats++;

            /*------------------------------------------------------------------------*/
            /*  Parsing all frame descriptors under this format descriptor            */
            /*------------------------------------------------------------------------*/
            bptr += mjpg_format->bLength;
            size -= mjpg_format->bLength;

            for (i = 0; (i < mjpg_format->bNumFrameDescriptors) && (vc->num_of_frames < UVC_MAX_FRAME); i++)
            {
                mjpg_frame = (DESC_MJPG_FRAME_T *)bptr;

                if ((mjpg_frame->bDescriptorType != UVC_CS_INTERFACE) ||
                        (mjpg_frame->bDescriptorSubType != VS_FRAME_MJPEG))
                {
                    UVC_DBGMSG("Frame parsing error!!\n");
                    return UVC_RET_PARSER;
                }

                UVC_DBGMSG("VS Interface VS_FRAME_MJPEG\n");
                UVC_DBGMSG("    bFormatIndex: 0x%x\n", mjpg_frame->bFrameIndex);
                UVC_DBGMSG("    wWidth:       0x%x\n", mjpg_frame->wWidth);
                UVC_DBGMSG("    wHeight       0x%x\n", mjpg_frame->wHeight);
                UVC_DBGMSG("    dwMinBitRate: 0x%x\n", mjpg_frame->dwMinBitRate);
                UVC_DBGMSG("    dwMaxBitRate: 0x%x\n", mjpg_frame->dwMaxBitRate);
                UVC_DBGMSG("    dwDefaultFrameInterval:  0x%x\n", mjpg_frame->dwDefaultFrameInterval);

                idx = vc->num_of_frames;
                vc->frame_idx[idx] = mjpg_frame->bFrameIndex;
                vc->frame_format[idx] = vc->format[vc->num_of_formats-1];
                vc->width[idx] = mjpg_frame->wWidth;
                vc->height[idx] = mjpg_frame->wHeight;
                vc->num_of_frames++;

                bptr += mjpg_frame->bLength;
                size -= mjpg_frame->bLength;
            }
            ifd = (DESC_IF_T *)bptr;  /* must */
            break;

#if 0
        case VS_FRAME_MJPEG:
            mjpg_frame = (DESC_MJPG_FRAME_T *)ifd_vs_hdr;
            UVC_DBGMSG("VS Interface VS_FRAME_MJPEG\n");
            UVC_DBGMSG("    bFormatIndex: 0x%x\n", mjpg_frame->bFrameIndex);
            UVC_DBGMSG("    wWidth:       0x%x\n", mjpg_frame->wWidth);
            UVC_DBGMSG("    wHeight       0x%x\n", mjpg_frame->wHeight);
            UVC_DBGMSG("    dwMinBitRate: 0x%x\n", mjpg_frame->dwMinBitRate);
            UVC_DBGMSG("    dwMaxBitRate: 0x%x\n", mjpg_frame->dwMaxBitRate);
            UVC_DBGMSG("    dwDefaultFrameInterval:  0x%x\n", mjpg_frame->dwDefaultFrameInterval);
            break;
#endif
        case VS_OUTPUT_HEADER:
        case VS_UNDEFINED:
        case VC_INPUT_TERMINAL:
        case VS_FORMAT_MPEG2TS:
        case VS_FORMAT_DV:
        case VS_COLORFORMAT:
        case VS_FORMAT_FRAME_BASED:
        case VS_FRAME_FRAME_BASED:
        case VS_FORMAT_STREAM_BASED:
        case VS_FORMAT_H264:
        case VS_FRAME_H264:
        case VS_FORMAT_H264_SIMULCAST:
        case VS_FORMAT_VP8:
        case VS_FRAME_VP8:
        case VS_FORMAT_VP8_SIMULCAST:
            UVC_DBGMSG("Unsupported VS class interface descriptor 0x%x! Skip.\n", ifd_vs_hdr->bDescriptorSubType);
            break;

        default:
            UVC_ERRMSG("Parsing error! Unknown VS class interface descriptor 0x%x!\n", ifd_vs_hdr->bDescriptorSubType);
            return UVC_RET_PARSER;
        }

        if (ifd->bLength == 0)
            return UVC_RET_PARSER;          /* prevent infinite loop                      */

        bptr += ifd->bLength;
        size -= ifd->bLength;
    }

#if 0
    /*------------------------------------------------------------------------------------*/
    /*  Find the next alternative interface                                               */
    /*------------------------------------------------------------------------------------*/
    while (size >= sizeof(DESC_IF_T))
    {
        ifd = (DESC_IF_T *)bptr;

        if ((ifd->bDescriptorType == USB_DT_INTERFACE) && (ifd->bInterfaceNumber == ifnum) &&
                (ifd->bAlternateSetting != 0))
            break;                              /* done, not alternative setting 0        */

        if (ifd->bLength == 0)
            return UVC_RET_PARSER;          /* prevent infinite loop                      */

        bptr += ifd->bLength;
        size -= ifd->bLength;
    }

    if (size < sizeof(DESC_IF_T))
    {
        UVC_ERRMSG("UVC_RET_PARSER! - Can't find any ALT of VS interface!\n");
        return UVC_RET_PARSER;
    }
#endif

    /*------------------------------------------------------------------------------------*/
    /*  Find all alternative interface of Standard Video Streaming Interface              */
    /*------------------------------------------------------------------------------------*/
    while (size >= sizeof(DESC_IF_T))
    {
        int   pksz;

        ifd = (DESC_IF_T *)bptr;

        if ((ifd->bDescriptorType != USB_DT_INTERFACE) || (ifd->bInterfaceNumber != iface->if_num) ||
                (ifd->bInterfaceClass != USB_CLASS_VIDEO) || (ifd->bInterfaceSubClass != UVC_SC_VIDEOSTREAMING))
            break;                          /* done, the following content not belong to this interface */

        if (size < 8)
        {
            /* Parsing error                              */
            UVC_ERRMSG("UVC_RET_PARSER! - VS descriptor length!\n");
            return UVC_RET_PARSER;
        }

        UVC_DBGMSG("Parsing VS if %d, alt %d...\n", iface->if_num, ifd->bAlternateSetting);

        if (ifd->bNumEndpoints != 1)
        {
            UVC_DBGMSG("  [!] bNumEndpoints is not 1!\n");
        }

        if (ifd->bLength == 0)
            return UVC_RET_PARSER;          /* prevent infinite loop                      */

        bptr += ifd->bLength;
        size -= ifd->bLength;

        epd = (DESC_EP_T *)bptr;

        if (epd->bDescriptorType != USB_DT_ENDPOINT)
        {
            UVC_DBGMSG("  [!] Endpoint descriptor not found!\n");
            continue;                       /* endpoint descriptor not found!             */
        }

        UVC_DBGMSG("  Endpoint wMaxPacketSize = %d\n", epd->wMaxPacketSize);

        vs->alt_no[vs->num_of_alt] = ifd->bAlternateSetting;
        pksz = epd->wMaxPacketSize;
        pksz = (pksz & 0x07ff) * (1 + ((pksz >> 11) & 3));
        vs->max_pktsz[vs->num_of_alt] = pksz;
        vs->num_of_alt++;

        if (epd->bLength == 0)
            return UVC_RET_PARSER;      /* prevent infinite loop                      */

        bptr += epd->bLength;
        size -= epd->bLength;
    }

    UVC_DBGMSG("\n\n----------------------------------------------------------\n");
    UVC_DBGMSG("[Video Streaming interface parsing result dump]\n");
    for (i = 0; i < vs->num_of_alt; i++)
    {
        UVC_DBGMSG("  Alt %d, wMaxPacketSize = %d\n", vs->alt_no[i], vs->max_pktsz[i]);
    }
    return 0;
}


/// @endcond HIDDEN_SYMBOLS

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/

