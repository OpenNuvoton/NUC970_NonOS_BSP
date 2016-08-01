#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"

#include "sdh.h"
#include "ff.h"
#include "diskio.h"
#include "jpegcodec.h"
#include "jpeg.h"
#include "jpegSample.h"
          
extern CHAR g_u8String[100];          
extern UINT32 g_u32StringIndex;           
          
PUINT8 g_pu8EncFrameBuffer;								/* Source image data for encoding */ 

UINT32 g_u32EncWidth = 640, g_u32EncHeight = 480;       /* Encode Width & Height */   	

UINT8 __align(32) g_au8BitstreamBuffer[0x100000];		/* The buffer for encoding output */
                  
VOID JpegEncTest (void)
{
	FRESULT		nStatus; 
	UINT    nWriteLen, nReadLen;
	//INT		hFile;
	CHAR	encodePath[50];
	//CHAR	suFileName1[100];	
	UINT16 	u16Width = ENC_WIDTH, u16Height = ENC_HEIGHT;
	JPEG_INFO_T jpegInfo;
	UINT len;
	//UINT32 starttime,endtime;
	
	FIL fil;       /* File object */
	FRESULT fr;
		
	u16Width = g_u32EncWidth;
	u16Height = g_u32EncHeight;
	
	sysprintf ("\nStart JPEG Encode Testing...\n");		
/*******************************************************************/	
/* 							Read Raw Data file					   */
/*******************************************************************/			
	sysprintf("Input Source file name\n");
	
	GetString();
		
	strcpy(encodePath, g_u8String);
			
	fr = f_open(&fil, encodePath, FA_OPEN_EXISTING | FA_READ);
	
	if(fr == FR_OK)
		sysprintf("\tOpen file:[%s] \n", decodePath);
    else
	{
        sysprintf("\tFailed to open file: %s \n", decodePath);
		 return;
    }
		
	len = f_size(&fil);
	
	sysprintf("\tRaw data size for Encode is %d\n", len);
	
	/* Allocate the Raw Data Buffer for Encode Operation */	
	g_pu8EncFrameBuffer = (PUINT8)malloc(sizeof(CHAR) * len);
	
	sysprintf("\tJPEG Bitstream is located 0x%X\n", (UINT32)g_au8BitstreamBuffer);
	
	nStatus = f_read(&fil, (UINT8 *)((UINT32)g_pu8EncFrameBuffer | 0x80000000), len, &nReadLen);
		
	if(nStatus != FR_OK)
		sysprintf("\tRead error!!\n");
	
	f_close(&fil);	
	    
	    
/*******************************************************************/	
/* 						Encode JPEG Bitstream					   */
/*******************************************************************/	    

	/* JPEG Init */
	jpegInit();	
	
	/* Set Source Address */	
	jpegIoctl(JPEG_IOCTL_SET_YADDR, (UINT32)g_pu8EncFrameBuffer, 0);
			
	/* Set Source Y/U/V Stride */	       
    jpegIoctl(JPEG_IOCTL_SET_YSTRIDE, u16Width, 0);
    jpegIoctl(JPEG_IOCTL_SET_USTRIDE, u16Width/2, 0);
    jpegIoctl(JPEG_IOCTL_SET_VSTRIDE, u16Width/2, 0);
     														
    /* Set Bit stream Address */   
    jpegIoctl(JPEG_IOCTL_SET_BITSTREAM_ADDR, (UINT32)g_au8BitstreamBuffer, 0);

	/* Encode mode, encoding primary image, YUV 4:2:2/4:2:0 */	
    jpegIoctl(JPEG_IOCTL_SET_ENCODE_MODE, JPEG_ENC_SOURCE_PACKET, JPEG_ENC_PRIMARY_YUV422);
    
    /* Primary Encode Image Width / Height */    
    jpegIoctl(JPEG_IOCTL_SET_DIMENSION, u16Height, u16Width);
       
	if(g_bEncSwReserveTest)
	{  
		/* Reserve memory space for user application 
		   # Reserve memory space Start address is Bit stream Address + 6 and driver will add the app marker (FF E0 xx xx)for user automatically. 
		   # User can set the data before or after trigger JPEG (Engine will not write the reserved memory space).	
		   # The size parameter is the actual size that can used by user and it must be multiple of 2 but not be multiple of 4 (Max is 65530). 	   
		   # User can get the marker length (reserved length + 2) from byte 4 and byte 5 in the bitstream. 
		   		Byte 0 & 1 :  FF D8
				Byte 2 & 3 :  FF E0
				Byte 4 & 5 :  [High-byte of Marker Length][Low-byte of Marker Length]
		   		Byte 6 ~ (Length + 4)  :  [(Marker Length - 2)-byte Data] for user application
		   	   
		   	 Ex : jpegIoctl(JPEG_IOCTL_ENC_RESERVED_FOR_SOFTWARE, 1024,0); 
		          FF D8 FF E0 04 02 [1024 bytes]	   	   
		*/
		jpegIoctl(JPEG_IOCTL_ENC_RESERVED_FOR_SOFTWARE, 1024,0);     
	}
	      
	if(g_bEncUpTest)
	{       
	    /* Primary Encode Image Width / Height */        
		/* Encode upscale 2x */
	    jpegIoctl(JPEG_IOCTL_SET_ENCODE_UPSCALE, u16Height * 2, u16Width * 2);       
    }
           
    //Set Encode Source Image Height        
    jpegIoctl(JPEG_IOCTL_SET_SOURCE_IMAGE_HEIGHT, u16Height, 0);

	/* Include Quantization-Table and Huffman-Table */	
    jpegIoctl(JPEG_IOCTL_ENC_SET_HEADER_CONTROL, JPEG_ENC_PRIMARY_QTAB | JPEG_ENC_PRIMARY_HTAB, 0);
       
	/* Use the default Quantization-table 0, Quantization-table 1 */
	jpegIoctl(JPEG_IOCTL_SET_DEFAULT_QTAB, 0, 0);

	//starttime = sysGetTicks(TIMER0);	
	
	/* Trigger JPEG encoder */
    jpegIoctl(JPEG_IOCTL_ENCODE_TRIGGER, 0, 0);  
    
    /* Wait for complete */
	if(jpegWait())
	{
		//endtime = sysGetTicks(TIMER0);	
		jpegGetInfo(&jpegInfo);
		sysprintf("\tJPEG Encode Complete!!\n");
		sysprintf("\tJpeg Image Size = %d\n",jpegInfo.image_size[0]);	
		len = jpegInfo.image_size[0];		
    }
    else
    {
    	sysprintf("\tJPEG Encode Error!!\n");
    	len = 0;
    	return;
    }
       
/*******************************************************************/	
/* 					   	  Write to Disk	    	    			   */
/*******************************************************************/	
	g_u8String[g_u32StringIndex] = 0x00;
	
	strcpy(encodePath, "enc_test");
	
	strcat(encodePath, ".jpg");
	
	fr = f_open(&fil, encodePath, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
	
	if(fr == FR_OK)
		sysprintf("\tOpen file:[%s] \n", encodePath);
    else
	{
        sysprintf("\tFailed to open file: %s \n", encodePath);
		 return;
    }
	
	nStatus = f_write(&fil, (UINT8 *)((UINT32)g_au8BitstreamBuffer | 0x80000000), len, &nWriteLen);
	
	if(nStatus != FR_OK)
		sysprintf("\tRead error!!\n");
	
    f_close(&fil);	
    
    free(g_pu8EncFrameBuffer);
    
	sysprintf ("Encode test completed!\n"); 	
	
	
	return;   
}
