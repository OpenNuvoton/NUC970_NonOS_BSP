/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved.  *
 *                                                              *
 ****************************************************************/

#define DEC_IPW_BUFFERSIZE    8192 /* Buffer size for Decode Input Wait */

/* ParsingJPEG Error Code */
#define ERR_MODE       -1
#define ERR_FORMAT     -2
#define ERR_SUCCESS     0
#define ERR_ONLY_NORMAL 1

#define ENC_WIDTH   640  /* Encode Width    */
#define ENC_HEIGHT  480  /* Encode Height */

/* Jpeg Output File Name Function parameter */
extern CHAR decodePath[];

/* Test Control Flag */
extern BOOL g_bDecPanelTest, g_bDecIpwTest, g_bDecOpwTest, g_bEncUpTest, g_bEncSwReserveTest;

extern UINT32 g_u32EncWidth, g_u32EncHeight;

extern UINT32 g_u32DecFormat;

extern PUINT8 g_pu8JpegBuffer; /* The source bit stream data for decoding */
extern PUINT8 g_pu8DecFrameBuffer; /* The buffer for decoding output */

extern PUINT8 g_pu8DecFrameBuffer; /* The buffer for decoding output */
#if defined ( __GNUC__ ) && !(__CC_ARM)
    __attribute__((aligned(32))) extern UINT8 g_au8BitstreamBuffer[];
#else
    extern UINT8 __align(32) g_au8BitstreamBuffer[];    /* The buffer for encoding output */
#endif

/*-----------------------------------------------------------------------*/
/*  Decode Input Wait parameter                                          */
/*-----------------------------------------------------------------------*/
#if defined ( __GNUC__ ) && !(__CC_ARM)
    __attribute__((aligned(32))) extern UINT8 g_au8DecInputWaitBuffer[];
#else
    extern UINT8 __align(32) g_au8DecInputWaitBuffer[]; /* Buffer for Decode Input Wait */
#endif
extern UINT32 g_u32IpwUsedSize;
extern UINT32 g_u32BitstreamSize;

/*-----------------------------------------------------------------------*/
/*  Decode Output Wait parameter                                         */
/*-----------------------------------------------------------------------*/
extern PUINT8 apBuffer[];           /* Decode Output buffer pointer array */
extern UINT32 g_u32OpwUsedSize;     /* JPEG Bitstream Size for Decode Input Wait */
extern UINT32 u32MCU_Line;          /* One MCU Line data size */
extern UINT32 u32TotalSize;         /* Total size for JPEG Decode Ouput */
extern UINT32 g_u32OpwBufferIndex;  /* Decode output Buffer index */

#define PLANAR_DEC_BUFFER_SIZE  0x100000 /* Decode Output Buffer size for Planar */

#define PANEL_WIDTH   320    /* PANEL Width (Raw data output width for Panel Test) */
#define PANEL_HEIGHT  240    /* PANEL Height (Raw data output height for Panel Test) */

#define TARGET_WIDTH  160    /* JPEG decode output width for __PANEL_TEST__ */
#define TARGET_HEIGHT 120    /* JPEG decode output height for __PANEL_TEST__ */

VOID JpegDecTest(void);
VOID JpegEncTest(void);

/* Jpeg Output File Name Function */
CHAR *intToStr(UINT32 u32quotient);
/*  Header Decode Complete Callback function */
BOOL JpegDecHeaderComplete(void);
/* Decode Input Wait Callback function */
BOOL JpegDecInputWait(UINT32 u32Address, UINT32 u32Size);
/* Decode Output Wait Callback function */
BOOL JpegDecOutputWait(UINT32 u32Address, UINT32 u32Size);

VOID JpedInitDecOutputWaitBuffer(UINT32 u32Width, UINT32 u32Height, UINT32 u32jpegFormat);

INT32 ParsingJPEG(PUINT8 JPEG_Buffer, UINT32 Length, PUINT32 pu32Width, PUINT32 pu32Height, PUINT32 u32Format, BOOL bPrimary);

UINT32 GetData(void);
VOID GetString(void);
