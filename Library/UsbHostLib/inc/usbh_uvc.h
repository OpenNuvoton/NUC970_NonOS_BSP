#ifndef __INCLUDED_UVC_H__
#define __INCLUDED_UVC_H__

#include "usb.h"

/// @cond HIDDEN_SYMBOLS

/*--------------------------------------------------------------------------------------*/
/*  Capability settings                                                                 */
/*--------------------------------------------------------------------------------------*/

#define UVC_MAX_DEVICE          1           /* Allowed maximum number of UVC device connected  */
#define UVC_MAX_STREAM          1           /* Allowed maximum number of streaming interface per UVC device   */

#define UVC_MAX_ALT_IF          12          /* Maximum number of alternative interface per UVC streaming interface supported */
#define UVC_MAX_FORMAT          8           /* Maximum number of video stream format supported in an UVC device.    */
#define UVC_MAX_FRAME           24          /* Maximum number of video stream frame supported in an UVC device.    */

#define UVC_UTR_PER_STREAM      4
// #define IF_PER_UTR           8           /* defined in usb.h                        */
#define UVC_UTR_INBUF_SIZE      (IF_PER_UTR * 3072)

#define UVC_REQ_TIMEOUT         50          /*!< UAC control request timeout value in tick (10ms unit)     */


/*--------------------------------------------------------------------------------------*/
/*  Debug settings                                                                      */
/*--------------------------------------------------------------------------------------*/
//#define UVC_DEBUG

#define UVC_ERRMSG      sysprintf
#ifdef UVC_DEBUG
#define UVC_DBGMSG      sysprintf
#else
#define UVC_DBGMSG(...)
#endif


/* Interface Class Codes (defined in usb.h) */
//#define USB_CLASS_VIDEO       0x0E

/* Video Interface Subclass Codes */
#define UVC_SC_UNDEFINED            0x00    /* Undefined                                 */
#define UVC_SC_VIDEOCONTROL         0x01    /* Video control interface                   */
#define UVC_SC_VIDEOSTREAMING       0x02    /* Video streaming                           */
#define UVC_SC_VIDEO_IF_COLLECT     0x03    /* Video interface collection                */

/* Video Interface Protocol Codes */
#define UVC_PC_PROT_UNDEFINED       0x00    /* Undefined                                 */
#define UVC_PC_PROT_15              0x01    /* Video Class version 1.5                   */

/* UVC Class-specific descritpor types */
#define UVC_CS_UNDEFINED            0x20
#define UVC_CS_DEVICE               0x21
#define UVC_CS_CONFIGURATION        0x22
#define UVC_CS_STRING               0x23
#define UVC_CS_INTERFACE            0x24
#define UVC_CS_ENDPOINT             0x25

/*
 * Video Class-Specific VC Interface Descriptor Subtypes
 */
#define VC_DESCRIPTOR_UNDEFINED     0x00
#define VC_HEADER                   0x01
#define VC_INPUT_TERMINAL           0x02
#define VC_OUTPUT_TERMINAL          0x03
#define VC_SELECTOR_UNIT            0x04
#define VC_PROCESSING_UNIT          0x05
#define VC_EXTENSION_UNIT           0x06
#define VC_ENCODING_UNIT            0x07

/*
 * Video Class-Specific VS Interface Descriptor Subtypes
 */
#define VS_UNDEFINED                0x00
#define VS_INPUT_HEADER             0x01
#define VC_INPUT_TERMINAL           0x02
#define VS_OUTPUT_HEADER            0x03
#define VS_FORMAT_UNCOMPRESSED      0x04
#define VS_FRAME_UNCOMPRESSED       0x05
#define VS_FORMAT_MJPEG             0x06
#define VS_FRAME_MJPEG              0x07
#define VS_FORMAT_MPEG2TS           0x0A
#define VS_FORMAT_DV                0x0C
#define VS_COLORFORMAT              0x0D
#define VS_FORMAT_FRAME_BASED       0x10
#define VS_FRAME_FRAME_BASED        0x11
#define VS_FORMAT_STREAM_BASED      0x12
#define VS_FORMAT_H264              0x13
#define VS_FRAME_H264               0x14
#define VS_FORMAT_H264_SIMULCAST    0x15
#define VS_FORMAT_VP8               0x16
#define VS_FRAME_VP8                0x17
#define VS_FORMAT_VP8_SIMULCAST     0x18

/*
 * Video Class-Specific Endpoint Descriptor Subtypes
 */
#define UVC_EP_UNDEFINED            0x00
#define UVC_EP_GENERAL              0x01
#define UVC_EP_ENDPOINT             0x02
#define UVC_EP_INTERRUPT            0x03


/*
 * Video Class-Specific Request Codes
 */
#define UVC_RC_UNDEFINED            0x00
#define UVC_SET_CUR                 0x01
#define UVC_SET_CUR_ALL             0x11
#define UVC_GET_CUR                 0x81
#define UVC_GET_MIN                 0x82
#define UVC_GET_MAX                 0x83
#define UVC_GET_RES                 0x84
#define UVC_GET_LEN                 0x85
#define UVC_GET_INFO                0x86
#define UVC_GET_DEF                 0x87
#define UVC_GET_CUR_ALL             0x91
#define UVC_GET_MIN_ALL             0x92
#define UVC_GET_MAX_ALL             0x93
#define UVC_GET_RES_ALL             0x94
#define UVC_GET_DEF_ALL             0x97


/*
 * VideoControl Interface Control Selectors
 */
#define VC_CONTROL_UNDEFINED               0x00
#define VC_VIDEO_POWER_MODE_CONTROL        0x01
#define VC_REQUEST_ERROR_CODE_CONTROL      0x02
#define VC_RESEVED                         0x03


/*
 * Terminal Control Selectors
 */
#define TE_CONTROL_UNDEFINED               0x00


/*
 * Selector Unit Control Selectors
 */
#define SU_CONTROL_UNDEFINED               0x00
#define SU_INPUT_SELECT_CONTROL            0x01


/*
 * Camera Terminal Control Selectors
 */
#define SU_CONTROL_UNDEFINED               0x00
#define CT_SCANNING_MODE_CONTROL           0x01
#define CT_AE_MODE_CONTROL                 0x02
#define CT_AE_PRIORITY_CONTROL             0x03
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL  0x04
#define CT_EXPOSURE_TIME_RELATIVE_CONTROL  0x05
#define CT_FOCUS_ABSOLUTE_CONTROL          0x06
#define CT_FOCUS_RELATIVE_CONTROL          0x07
#define CT_FOCUS_AUTO_CONTROL              0x08
#define CT_IRIS_ABSOLUTE_CONTROL           0x09
#define CT_IRIS_RELATIVE_CONTROL           0x0A
#define CT_ZOOM_ABSOLUTE_CONTROL           0x0B
#define CT_ZOOM_RELATIVE_CONTROL           0x0C
#define CT_PANTILT_ABSOLUTE_CONTROL        0x0D
#define CT_PANTILT_RELATIVE_CONTROL        0x0E
#define CT_ROLL_ABSOLUTE_CONTROL           0x0F
#define CT_ROLL_RELATIVE_CONTROL           0x10
#define CT_PRIVACY_CONTROL                 0x11
#define CT_FOCUS_SIMPLE_CONTROL            0x12
#define CT_WINDOW_CONTROL                  0x13
#define CT_REGION_OF_INTEREST_CONTROL      0x14


/*
 * Processing Unit Control Selectors
 */
#define PU_CONTROL_UNDEFINED                       0x00
#define PU_BACKLIGHT_COMPENSATION_CONTROL          0x01
#define PU_BRIGHTNESS_CONTROL                      0x02
#define PU_CONTRAST_CONTROL                        0x03
#define PU_GAIN_CONTROL                            0x04
#define PU_POWER_LINE_FREQUENCY_CONTROL            0x05
#define PU_HUE_CONTROL                             0x06
#define PU_SATURATION_CONTROL                      0x07
#define PU_SHARPNESS_CONTROL                       0x08
#define PU_GAMMA_CONTROL                           0x09
#define PU_WHITE_BALANCE_TEMPERATURE_CONTROL       0x0A
#define PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL  0x0B
#define PU_WHITE_BALANCE_COMPONENT_CONTROL         0x0C
#define PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL    0x0D
#define PU_DIGITAL_MULTIPLIER_CONTROL              0x0E
#define PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL        0x0F
#define PU_HUE_AUTO_CONTROL                        0x10
#define PU_ANALOG_VIDEO_STANDARD_CONTROL           0x11
#define PU_ANALOG_LOCK_STATUS_CONTROL              0x12
#define PU_CONTRAST_AUTO_CONTROL                   0x13


/*
 * Encoding Unit Control Selectors
 */
#define EU_CONTROL_UNDEFINED               0x00
#define EU_SELECT_LAYER_CONTROL            0x01
#define EU_PROFILE_TOOLSET_CONTROL         0x02
#define EU_VIDEO_RESOLUTION_CONTROL        0x03
#define EU_MIN_FRAME_INTERVAL_CONTROL      0x04
#define EU_SLICE_MODE_CONTROL              0x05
#define EU_RATE_CONTROL_MODE_CONTROL       0x06
#define EU_AVERAGE_BITRATE_CONTROL         0x07
#define EU_CPB_SIZE_CONTROL                0x08
#define EU_PEAK_BIT_RATE_CONTROL           0x09
#define EU_QUANTIZATION_PARAMS_CONTROL     0x0A
#define EU_SYNC_REF_FRAME_CONTROL          0x0B
#define EU_LTR_BUFFER_                     0x0C
#define EU_LTR_PICTURE_CONTROL             0x0D
#define EU_LTR_VALIDATION_CONTROL          0x0E
#define EU_LEVEL_IDC_LIMIT_CONTROL         0x0F
#define EU_SEI_PAYLOADTYPE_CONTROL         0x10
#define EU_QP_RANGE_CONTROL                0x11
#define EU_PRIORITY_CONTROL                0x12
#define EU_START_OR_STOP_LAYER_CONTROL     0x13
#define EU_ERROR_RESILIENCY_CONTROL        0x14


/*
 * Extension Unit Control Selectors
 */
#define XU_CONTROL_UNDEFINED               0x00


/*
 * VideoStreaming Interface Control Selectors
 */
#define VS_CONTROL_UNDEFINED               0x00
#define VS_PROBE_CONTROL                   0x01
#define VS_COMMIT_CONTROL                  0x02
#define VS_STILL_PROBE_CONTROL             0x03
#define VS_STILL_COMMIT_CONTROL            0x04
#define VS_STILL_IMAGE_TRIGGER_CONTROL     0x05
#define VS_STREAM_ERROR_CODE_CONTROL       0x06
#define VS_GENERATE_KEY_FRAME_CONTROL      0x07
#define VS_UPDATE_FRAME_SEGMENT_CONTROL    0x08
#define VS_SYNCH_DELAY_CONTROL             0x09


/*
 * USB Terminal Types
 */
#define TT_VENDOR_SPECIFIC                 0x0100
#define TT_STREAMING                       0x0101


/*
 * Input Terminal Types
 */
#define ITT_VENDOR_SPECIFIC                0x0200
#define ITT_CAMERA                         0x0201
#define ITT_MEDIA_TRANSPORT_INPUT          0x0202


/*
 * Output Terminal Types
 */
#define OTT_VENDOR_SPECIFIC                0x0300
#define OTT_DISPLAY                        0x0301
#define OTT_MEDIA_TRANSPORT_OUTPUT         0x0302


/*
 * External Terminal Types
 */
#define EXTERNAL_ VENDOR_SPECIFIC          0x0400
#define COMPOSITE_CONNECTOR                0x0401
#define SVIDEO_CONNECTOR                   0x0402
#define COMPONENT_CONNECTOR                0x0403



/*-----------------------------------------------------------------------------------
 *  Video Class-specific interface descriptor header
 */

/*  Standard Video Interface Collection IAD */
typedef struct __attribute__((__packed__)) desc_vc_iad_t
{
    uint8_t  bLength;                  /* 0x08: Size of this descriptor, in bytes        */
    uint8_t  bDescriptorType;          /* 0x0B: INTERFACE ASSOCIATION Descriptor         */
    uint8_t  bFirstInterface;          /* Interface number of the VideoControl interface */
    uint8_t  bInterfaceCount;          /* Number of contiguous Video interfaces          */
    uint8_t  bFunctionClass;           /* 0x0E: CC_VIDEO                                 */
    uint8_t  bFunctionSubClass;        /* 0x03: SC_VIDEO_INTERFACE_COLLECTION            */
    uint8_t  bFunctionProtocol;        /* 0x00: Not used.                                */
    uint8_t  iFunction;                /* Index to string descriptor                     */
} DESC_VC_IAD_T;


/* VC_HEADER */
typedef struct __attribute__((__packed__)) desc_vc_hdr_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x24: CS_INTERFACE                             */
    uint8_t  bDescriptorSubType;       /* 0x01: VC_HEADER                                */
    uint16_t bcdUVC;                   /* Video Device Class Specification release number in binary-coded decimal */
    uint16_t wTotalLength;             /* Total number of bytes returned for the class-specific VideoControl interface descriptor. Includes the combined length of this descriptor header and all Unit and Terminal descriptors. */
    uint32_t dwClockFrequency;         /* Use of this field has been deprecated.         */
    uint8_t  bInCollection;            /* The number of VideoStreaming interfaces        */
    uint8_t  baInterfaceNr;            /* Interface number of VideoStreaming interface   */
} DESC_VC_HDR_T;


/* VC_INPUT_TERMINAL */
typedef struct __attribute__((__packed__)) desc_vc_it_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x24: CS_INTERFACE                             */
    uint8_t  bDescriptorSubType;       /* 0x02: VC_INPUT_TERMINAL                        */
    uint8_t  bTerminalID;              /* Unique terminal ID                             */
    uint16_t wTerminalType;            /* Terminal types                                 */
    uint8_t  bAssocTerminal;           /* ID of the Output Terminal to which this Input Terminal is associated, or zero (0) if no such association exists.    */
    uint8_t  iTerminal;                /* Index of a string descriptor                   */
} DESC_VC_IT_T;


/* VC_OUTPUT_TERMINAL */
typedef struct __attribute__((__packed__)) desc_vc_ot_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x24: CS_INTERFACE                             */
    uint8_t  bDescriptorSubType;       /* 0x03: VC_OUTPUT_TERMINAL                       */
    uint8_t  bTerminalID;              /* Unique terminal ID                             */
    uint16_t wTerminalType;            /* Terminal types                                 */
    uint8_t  bAssocTerminal;           /* Constant, identifying the Input Terminal to which this Output Terminal is associated, or zero (0) if no such association exists.  */
    uint8_t  bSourceID;                /* ID of the Unit or Terminal to which this Terminal is connected.  */
    uint8_t  iTerminal;                /* Index of a string descriptor                   */
} DESC_VC_OT_T;


/* VC_SELECTOR_UNIT */
typedef struct __attribute__((__packed__)) desc_vc_su_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x24: CS_INTERFACE                             */
    uint8_t  bDescriptorSubType;       /* 0x04: VC_SELECTOR_UNIT                         */
    uint8_t  bUnitID;                  /* Unique terminal ID. This value is used in all requests to address this Unit. */
    uint8_t  bNrInPins;                /* Number of Input Pins of this Unit              */
    uint8_t  bSourceID[4];             /* ID of the Unit or Terminal Input Pins          */
} DESC_VC_SU_T;


/* VC_PROCESSING_UNIT */
typedef struct __attribute__((__packed__)) desc_vc_pu_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x24: CS_INTERFACE                             */
    uint8_t  bDescriptorSubType;       /* 0x05: VC_PROCESSING_UNIT                       */
    uint8_t  bUnitID;                  /* Unique terminal ID. This value is used in all requests to address this Unit. */
    uint8_t  bSourceID;                /* ID of the Unit or Terminal to which this Unit is connected.   */
    uint16_t wMaxMultiplier;           /* If the Digital Multiplier control is supported, this field indicates the maximum digital magnification, multiplied by 100. For example, for a device that supports 1-4.5X digital zoom (a multiplier of 4.5), this field would be set to 450. If the Digital Multiplier control is not supported, this field shall be set to 0.   */
    uint8_t  bControlSize;             /* Size of the bmControls field, in bytes: 3      */
    uint8_t  bmControls[3];            /* A bit set to 1 indicates that the mentioned Control is supported for the video stream.   */
    /* D0: Brightness                                 */
    /* D1: Contrast                                   */
    /* D2: Hue                                        */
    /* D3: Saturation                                 */
    /* D4: Sharpness                                  */
    /* D5: Gamma                                      */
    /* D6: White Balance Temperature                  */
    /* D7: White Balance Component                    */
    /* D8: Backlight Compensation                     */
    /* D9: Gain                                       */
    /* D10: Power Line Frequency                      */
    /* D11: Hue, Auto                                 */
    /* D12: White Balance Temperature, Auto           */
    /* D13: White Balance Component, Auto             */
    /* D14: Digital Multiplier                        */
    /* D15: Digital Multiplier Limit                  */
    /* D16: Analog Video Standard                     */
    /* D17: Analog Video Lock Status                  */
    /* D18: Contrast, Auto                            */
    /* D19 ¡V D23: Reserved. Set to zero.              */
    uint8_t  iProcessing;              /* Index of a string descriptor                   */
    uint8_t  bmVideoStandards;         /* A bitmap of all analog video standards supported by the Processing Unit.  */
    /* D0: None                                       */
    /* D1: NTSC ¡V 525/60                              */
    /* D2: PAL ¡V 625/50                               */
    /* D3: SECAM ¡V 625/50                             */
    /* D4: NTSC ¡V 625/50                              */
    /* D5: PAL ¡V 525/60                               */
    /* D6: White Balance Temperature                  */
    /* D6-D7: Reserved. Set to zero.                  */
} DESC_VC_PU_T;


/* VC_EXTENSION_UNIT */
typedef struct __attribute__((__packed__)) desc_vc_eu_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x24: CS_INTERFACE                             */
    uint8_t  bDescriptorSubType;       /* 0x04: VC_EXTENSION_UNIT                        */
    uint8_t  bUnitID;                  /* Unique terminal ID. This value is used in all requests to address this Unit. */
    uint32_t guidExtensionCode[4];     /* Vendor-specific code identifying theExtension Unit  */
    uint8_t  bNumControls;             /* Number of controls in this extension unit      */
    uint8_t  bNrInPins;                /* Number of Input Pins of this Unit              */
} DESC_VC_EU_T;


/* VC_ENCODING_UNIT */
typedef struct __attribute__((__packed__)) desc_vc_ecu_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x24: CS_INTERFACE                             */
    uint8_t  bDescriptorSubType;       /* 0x04: VC_ENCODING_UNIT                         */
    uint8_t  bUnitID;                  /* Unique terminal ID. This value is used in all requests to address this Unit. */
    uint8_t  bSourceID;                /* ID of the Unit or Terminal to which this Unit is connected.   */
    uint8_t  iEncoding;                /* Index of a string descriptor                   */
    uint8_t  bControlSize;             /* Size, in bytes, of the bmControls and bmControlsRuntime fields: The value must be 3.  */
    uint8_t  bmControls[3];            /* A bit set to 1 indicates that the specified control is supported for initialization */
    uint8_t  bmControlsRuntime[3];     /* A bit set to 1 indicates that the mentioned control is supported during runtime. */
} DESC_VC_ECU_T;


/* Class-specific VC Interrupt Endpoint Descriptor */
typedef struct __attribute__((__packed__)) desc_vc_epi_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x25: CS_ENDPOINT                              */
    uint8_t  bDescriptorSubType;       /* 0x03: EP_INTERRUPT                             */
    uint16_t wMaxTransferSize;         /* Maximum interrupt structure size this endpoint is capable of sending. */
} DESC_VC_EPI_T;


/* VS_INPUT_HEADER */
typedef struct __attribute__((__packed__)) desc_vsi_hdr_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x24: CS_INTERFACE                             */
    uint8_t  bDescriptorSubType;       /* 0x01: VS_INPUT_HEADER                          */
    uint8_t  bNumFormats;              /* Number of video payload Format descriptors     */
    uint16_t wTotalLength;             /* Total number of bytes returned for the class-specific VideoStreaming interface descriptors including this header descriptor.  */
    uint8_t  bEndpointAddress;         /* The address of the isochronous or bulk endpoint used for video data. */
    uint8_t  bmInfo;                   /* Indicates the capabilities of this VideoStreaming interface   */
    uint8_t  bTerminalLink;            /* The terminal ID of the Output Terminal to which the video endpoint of this interface is connected.  */
    uint8_t  bStillCaptureMethod;      /* Method of still image capture supported        */
    uint8_t  bTriggerSupport;          /* Specifies if hardware triggering is supported through this interface  */
    uint8_t  bTriggerUsage;            /* Specifies how the host software shall respond to a hardware trigger interrupt event from this interface.  */
    uint8_t  bControlSize;             /* Size of each bmaControls(x) field, in bytes.   */
} DESC_VSI_HDR_T;


/*-----------------------------------------------------------------------------------
 *  Video Class-specific format and frame descriptors
 */

/* VS_FORMAT_UNCOMPRESSED */
typedef struct __attribute__((__packed__)) desc_vsu_format_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x24: CS_INTERFACE                             */
    uint8_t  bDescriptorSubType;       /* 0x04: VS_FORMAT_UNCOMPRESSED                   */
    uint8_t  bFormatIndex;             /* Index of this format descriptor                */
    uint8_t  bNumFrameDescriptors;     /* Number of frame descriptors following that correspond to this format  */
    uint32_t guidFormat[4];            /* Globally Unique Identifier used to identify stream-encoding format */
    uint8_t  bBitsPerPixel;            /* Number of bits per pixel used to specify color in the decoded video frame  */
    uint8_t  bDefaultFrameIndex;       /* Optimum Frame Index (used to select resolution) for this stream.  */
    uint8_t  bAspectRatioX;            /* The X dimension of the picture aspect ratio.   */
    uint8_t  bAspectRatioY;            /* The Y dimension of the picture aspect ratio.   */
    uint8_t  bmInterlaceFlags;         /* Specifies interlace information.               */
    uint8_t  bCopyProtect;             /* Specifies whether duplication of the video stream is restricted   */
} DESC_VSU_FORMAT_T;


/* VS_FRAME_UNCOMPRESSED */
typedef struct __attribute__((__packed__)) desc_vsu_frame_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x24: CS_INTERFACE                             */
    uint8_t  bDescriptorSubType;       /* 0x05: VS_FRAME_UNCOMPRESSED                    */
    uint8_t  bFrameIndex;              /* Index of this frame descriptor                 */
    uint8_t  bmCapabilities;
    uint16_t wWidth;                   /* Width of decoded bitmap frame in pixels        */
    uint16_t wHeight;                  /* Height of decoded bitmap frame in pixels       */
    uint32_t dwMinBitRate;             /* Specifies the minimum bit rate at the longest frame interval in units of bps at which the data can be transmitted. */
    uint32_t dwMaxBitRate;             /* Specifies the maximum bit rate at the shortest frame interval in units of bps at which the data can be transmitted. */
    uint32_t dwMaxVideoFrameBufferSize;/* Use of this field has been deprecated.         */
    uint32_t dwDefaultFrameInterval;   /* Specifies the frame interval the device would like to indicate for use as a default.   */
    uint8_t  bFrameIntervalType;       /* Indicates how the frame interval can be programmed */
    uint32_t dwMinFrameInterval;       /* Shortest frame interval supported (at highest frame rate), in 100 ns units. */
    uint32_t dwMaxFrameInterval;       /* Longest frame interval supported (at lowest frame rate), in 100 ns units.  */
    uint32_t dwFrameIntervalStep;      /* Indicates granularity of frame interval range, in 100 ns units. */
} DESC_VSU_FRAME_T;


/* VS_FORMAT_MJPEG */
typedef struct __attribute__((__packed__)) desc_mpjg_format_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x24: CS_INTERFACE                             */
    uint8_t  bDescriptorSubType;       /* 0x06: VS_FORMAT_MJPEG                          */
    uint8_t  bFormatIndex;             /* Index of this format descriptor                */
    uint8_t  bNumFrameDescriptors;     /* Number of frame descriptors following that correspond to this format  */
    uint8_t  bmFlags;                  /* Specifies characteristics of this format.      */
    uint8_t  bDefaultFrameIndex;       /* Optimum Frame Index (used to select resolution) for this stream.  */
    uint8_t  bAspectRatioX;            /* The X dimension of the picture aspect ratio.   */
    uint8_t  bAspectRatioY;            /* The Y dimension of the picture aspect ratio.   */
    uint8_t  bmInterlaceFlags;         /* Specifies interlace information.               */
    uint8_t  bCopyProtect;             /* Specifies whether duplication of the video stream is restricted   */
} DESC_MJPG_FORMAT_T;


/* VS_FRAME_MJPEG */
typedef struct __attribute__((__packed__)) desc_mjpg_frame_t
{
    uint8_t  bLength;                  /* Size of this descriptor, in bytes              */
    uint8_t  bDescriptorType;          /* 0x24: CS_INTERFACE                             */
    uint8_t  bDescriptorSubType;       /* 0x07: VS_FRAME_MJPEG                           */
    uint8_t  bFrameIndex;              /* Index of this frame descriptor                 */
    uint8_t  bmCapabilities;
    uint16_t wWidth;                   /* Width of decoded bitmap frame in pixels        */
    uint16_t wHeight;                  /* Height of decoded bitmap frame in pixels       */
    uint32_t dwMinBitRate;             /* Specifies the minimum bit rate at the longest frame interval in units of bps at which the data can be transmitted. */
    uint32_t dwMaxBitRate;             /* Specifies the maximum bit rate at the shortest frame interval in units of bps at which the data can be transmitted. */
    uint32_t dwMaxVideoFrameBufferSize;/* Use of this field has been deprecated.         */
    uint32_t dwDefaultFrameInterval;   /* Specifies the frame interval the device would like to indicate for use as a default.   */
    uint8_t  bFrameIntervalType;       /* Indicates how the frame interval can be programmed */
    uint32_t dwMinFrameInterval;       /* Shortest frame interval supported (at highest frame rate), in 100 ns units. */
    uint32_t dwMaxFrameInterval;       /* Longest frame interval supported (at lowest frame rate), in 100 ns units.  */
    uint32_t dwFrameIntervalStep;      /* Indicates granularity of frame interval range, in 100 ns units. */
} DESC_MJPG_FRAME_T;



struct uvc_dev_t;

/// @endcond HIDDEN_SYMBOLS


/** @addtogroup USBH_EXPORTED_STRUCTURES USB Host Exported Structures
  @{
*/

typedef struct uvc_ctrl_t
{
    uint8_t           ep_num_sts;           /* interrupt in endpoint address              */
    uint8_t           num_of_formats;       /* total number of video formats              */
    uint8_t           format_idx[UVC_MAX_FORMAT];  /* format index                        */
    IMAGE_FORMAT_E    format[UVC_MAX_FORMAT];      /* video format                        */
    uint8_t           num_of_frames;        /* total number of video frame types          */
    uint8_t           frame_idx[UVC_MAX_FRAME];    /* frame index                         */
    IMAGE_FORMAT_E    frame_format[UVC_MAX_FRAME]; /* frame format                        */
    uint16_t          width[UVC_MAX_FRAME]; /* frame width                                */
    uint16_t          height[UVC_MAX_FRAME];/* frame height                               */
}   UVC_CTRL_T;


/*
 *  Payload Header
 */
#define UVC_PL_EOH    0x80
#define UVC_PL_ERR    0x40
#define UVC_PL_STI    0x20
#define UVC_PL_RES    0x10
#define UVC_PL_SCR    0x08
#define UVC_PL_PTS    0x04
#define UVC_PL_EOF    0x02
#define UVC_PL_FID    0x01


typedef struct uvc_strm_t
{
    uint8_t           ep_addr;              /* endpoint address                           */
    uint8_t           num_of_alt;           /* total number of alternative interface      */
    uint8_t           alt_no[UVC_MAX_ALT_IF];
    uint16_t          max_pktsz[UVC_MAX_ALT_IF];
    uint8_t           current_frame_error;  /* indicate error detected in the current frame while parsing the video stream */
    uint8_t           current_frame_toggle; /* indicate the toggle bit of current frame while parsing the video stream */
}   UVC_STRM_T;


typedef struct __attribute__((__packed__)) uvc_ctrl_param_t
{
    uint16_t          bmHint;               /* Bitfield control indicating to the function what fields shall be kept fixed */
    uint8_t           bFormatIndex;         /* Video format index from a Format descriptor for this video interface. */
    uint8_t           bFrameIndex;          /* Video frame index from a Frame descriptor. */
    uint32_t          dwFrameInterval;      /* Frame interval in 100 ns units.            */
    uint16_t          wKeyFrameRate;        /* Key frame rate in key-frame per video-frame units.  */
    uint16_t          wPFrameRate;          /* P Frame rate in PFrame/key frame units.    */
    uint16_t          wCompQuality;         /* Compression quality control in abstract units 1 (lowest) to 10000 (highest).   */
    uint16_t          wCompWindowSize;      /* Window size for average bit rate control.  */
    uint16_t          wDelay;               /* Internal video streaming interface latency in ms from video data capture to presentation on the USB.  */
    uint32_t          dwMaxVideoFrameSize;  /* Maximum video frame or codec-specific segment size in bytes.  */
    uint32_t          dwMaxPayloadTransferSize;  /* Specifies the maximum number of bytes that the device can transmit or receive in a single payload transfer. This field must be supported. */
    uint32_t          dwClockFrequency;     /* The device clock frequency in Hz for the specified format. */
    uint8_t           bmFramingInfo;        /* This control indicates to the function whether payload transfers will contain out-of-band framing information in the Video Payload Header. */
    uint8_t           bPreferedVersion;     /* The preferred payload format version supported by the host or device for the specified bFormatIndex value */
    uint8_t           bMinVersion;          /* The minimum payload format version supported by the device for the specified bFormatIndex value. */
    uint8_t           bMaxVersion;          /* The maximum payload format version supported by the device for the specified bFormatIndex value. */
    uint8_t           bUsage;               /* This bitmap enables features reported by the bmUsages field of the Video Frame Descriptor. */
    uint8_t           bBitDepthLuma;        /* Represents bit_depth_luma_minus8 + 8, which must be the same as bit_depth_chroma_minus8 + 8. */
    uint8_t           bmSettings;           /* A bitmap of flags that is used to discover and control specific features of a temporally encoded video stream. */
    uint8_t           bMaxNumberOfRefFramesPlus1;  /* Host indicates the maximum number of frames stored for use as references. */
    uint16_t          bmRateControlModes;   /* This field contains 4 subfields, each of which is a 4 bit number. */
    uint8_t           bmLayoutPerStream[8]; /* This field contains 4 subfields, each of which is a 2 byte number. */
}   UVC_CTRL_PARAM_T;


/*
 * USB-specific UVC device struct
 */
typedef struct uvc_dev_t
{
    UDEV_T            *udev;
    char              version;        /* USB Video Class version. 0x00: 1.1; 0x01: 1.5      */
    UVC_CTRL_T        vc;             /* Video control interface stuff                      */
    UVC_STRM_T        vs;             /* Video streaming interface stuff                    */
    IFACE_T           *iface_ctrl;    /* Video control interface                            */
    IFACE_T           *iface_stream;  /* Video streaming interface                          */
    UVC_CTRL_PARAM_T  param;          /* Video control parameter block                      */
    uint8_t           is_streaming;   /* Video is currently streaming or not                */
    EP_INFO_T         *ep_iso_in;     /* Isochronous in endpoint                            */
    uint8_t           *in_buff;       /* Isochronous streaming in buffer                    */
    UTR_T             *utr_rx[UVC_UTR_PER_STREAM];    /* Isochronous-in UTRs                */
    uint8_t           *img_buff;      /* Image buffer provided by user                      */
    int               img_buff_size;  /* Size of the image buffer provided by user          */
    int               img_size;       /* Size of the image data stored in img_buff          */
    UVC_CB_FUNC       *func_rx;       /* user callback function for receiving images        */
    struct uvc_dev_t  *next;
}   UVC_DEV_T;


#endif /* __INCLUDED_UVC_H__ */

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/

