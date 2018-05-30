/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/06/10 12:01p $
 * @brief    NUC970 Driver Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
 #include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "nuc970.h"
#include "sys.h"
#include "uart.h"


/* debug information */
//#define UARTMSG  /* ..... don't define it */

#define uartprintf		sysprintf
#define uartgetchar		sysGetChar


#define TXSIZE		0x2800  /* 10Kbyte */
#define RXSIZE		0x2800  /* 10Kbyte */

/* LIN CheckSum Method */
#define MODE_CLASSIC    2
#define MODE_ENHANCED   1

static UINT8 RX_Test[500];

static UINT8 TX_Test[] = "1111111111";

static UINT8 RS485_Test[50];

static 	UART_T param;

volatile int32_t g_i32pointer;
uint8_t g_u8SendData[12] ;

//------------------------- Program -------------------------//
#ifdef UARTMSG
static void UART_ShowErrInfo(INT errno) 
{
	UINT32 err = 0;
	
	switch(errno)
	{
		case 0:
			uartprintf("\nSuccess\n");	
			break;		
		case UART_ENOTTY:
			uartprintf("\nCommand not support\n");	
			break;						
		case UART_ENODEV:	
			uartprintf("\nUART channel number out of range\n");	
			break;			
		case UART_EIO:		
			uartprintf("\nRead or Write error\n");	
			break;
	
		default:
			uartprintf("\nReturn value [%d]\n\n", errno);	return;
	}	

	/* Get more detail error information */
	uartIoctl(param.ucUartNo, UART_IOC_GETERRNO, (UINT32) &err, 0);
	uartprintf("UART driver error number [%d]\n", err);
}	
#endif

void uartExample (void)
{
	int ch, retval, len, reccnt = 0, i, err = 0;
	
	uartprintf("\n\nMake sure the H/W configuration of different UART channel!\n");

	/* configure UART */
	param.uFreq = 12000000;
	param.uBaudRate = 115200; 
	param.ucUartNo = UART1;
	param.ucDataBits = DATA_BITS_8;
	param.ucStopBits = STOP_BITS_1;
	param.ucParity = PARITY_NONE;
	param.ucRxTriggerLevel = UART_FCR_RFITL_1BYTE;	
	retval = uartOpen(&param); 
	if(retval != 0) 
	{
		uartprintf("Open UART error!\n");
		return;
	}
	
	/* set TX interrupt mode */
	retval = uartIoctl(param.ucUartNo, UART_IOC_SETTXMODE, UARTINTMODE, 0);
	if (retval != 0) 
	{
		uartprintf("Set TX interrupt mode fail!\n");
		return;	
	}
	
	/* set RX interrupt mode */
	retval = uartIoctl(param.ucUartNo, UART_IOC_SETRXMODE, UARTINTMODE, 0);
	if (retval != 0) 
	{
		uartprintf("Set RX interrupt mode fail!\n");
		return;	
	}
	
	retval = uartIoctl(param.ucUartNo, UART_IOC_ENABLEHWFLOWCONTROL, 0, 0);
	if (retval != 0) 
	{
		uartprintf("Set H/W flow control fail!\n");
		return;	
	}
	
	while(1)
	{
		uartprintf("\n Select TX/RX test [0/1]\n");
		ch = uartgetchar();
		if ( (ch == '1') || (ch == '0') ) 
			break;	
	}
		
	if (ch == '0')	
	{
		/* TX test */
		uartprintf("\n Any key start to TX test\n");
		ch = uartgetchar();			
		len = strlen((PINT8) TX_Test);
		while(1)
		{
			retval = uartWrite(param.ucUartNo, TX_Test, len);
			if(retval < 0)
			{
                /* buffer full case */                				
#ifdef UARTMSG                
				UART_ShowErrInfo(retval);  /* ..... it affects the TX process ? */
#endif				
            }
			else //if (retval == len)
			{
				reccnt += retval;

#ifndef UARTMSG 				
				uartprintf(" TX[%d]\r", reccnt);  /* if printf this message, the TX speed will slow */
#endif				
				
				if (reccnt >= TXSIZE)
				{
					uartprintf("\n\n TX test complete [%d] bytes transmitted\n", reccnt);	
					break;	
				}
			}
		}		
	}
	else
	{
		/* RX test */
		uartprintf("\n Any key start to RX test ...\n");
		uartprintf("    Should receive [%d] bytes\n", RXSIZE);
		ch = uartgetchar();		
		while(1)
		{
			/* 
				Don't printf any message that occur data lost. The RX buffer full will be occurred. 
			*/
			retval = uartRead(param.ucUartNo, RX_Test, 500);
			if(retval <= 500) 
			{
				reccnt += retval;
				if (retval != 0)
				{
					for (i = 0; i < retval; i++)
					{
						if (RX_Test[i] != 0x31)	
							uartprintf("%d[%x]\n", i + 1, RX_Test[i]);
					}	
				}
				
				if (reccnt >= RXSIZE)
				{
					uartprintf("\n RX test complete [%d] bytes received\n", reccnt);	
					break;	
				}	
			}
			else
				err++;
		}		
	}
	
	return;	
}

void IrDAExample (void)
{
	int ch, retval, len, reccnt = 0, i, err = 0;
	
	uartprintf("\n\nMake sure the H/W configuration of different UART channel!\n");

	/* configure UART */
	param.uFreq = 12000000;
	param.uBaudRate = 57600; //115200; 
	param.ucUartNo = UART1;
	param.ucDataBits = DATA_BITS_8;
	param.ucStopBits = STOP_BITS_1;
	param.ucParity = PARITY_NONE;
	param.ucRxTriggerLevel = UART_FCR_RFITL_1BYTE;	
	retval = uartOpen(&param); 
	if(retval != 0) 
	{
		uartprintf("Open UART error!\n");
		return;
	}
	
	/* set TX interrupt mode */
	retval = uartIoctl(param.ucUartNo, UART_IOC_SETTXMODE, UARTINTMODE, 0);
	if (retval != 0) 
	{
		uartprintf("Set TX interrupt mode fail!\n");
		return;	
	}
	
	/* set RX interrupt mode */
	retval = uartIoctl(param.ucUartNo, UART_IOC_SETRXMODE, UARTINTMODE, 0);
	if (retval != 0) 
	{
		uartprintf("Set RX interrupt mode fail!\n");
		return;	
	}
	
	while(1)
	{
		uartprintf("\n Select IrDA TX/RX test [0/1]\n");
		ch = uartgetchar();
		if ( (ch == '1') || (ch == '0') ) 
			break;	
	}
		
	if (ch == '0')	
	{
		/* TX test */
		
		retval = uartIoctl(param.ucUartNo, UART_IOC_PERFORMIrDA, ENABLEIrDA, IrDA_TX);
		if (retval != 0) 
		{
			uartprintf("Set IrDA Mode fail!\n");
			return;	
		}
		
		uartprintf("\n Any key start to TX test\n");
		ch = uartgetchar();			
		len = strlen((PINT8) TX_Test);
		while(1)
		{
			retval = uartWrite(param.ucUartNo, TX_Test, len);
			if(retval < 0)
			{
                /* buffer full case */                				
#ifdef UARTMSG                
				UART_ShowErrInfo(retval);  /* ..... it affects the TX process ? */
#endif				
            }
			else //if (retval == len)
			{
				reccnt += retval;

#ifndef UARTMSG 				
				uartprintf(" TX[%d]\r", reccnt);  /* if printf this message, the TX speed will slow */
#endif				
				
				if (reccnt >= TXSIZE)
				{
					uartprintf("\n\n TX test complete [%d] bytes transmitted\n", reccnt);	
					break;	
				}
			}
		}		
	}
	else
	{
		/* RX test */
		retval = uartIoctl(param.ucUartNo, UART_IOC_PERFORMIrDA, ENABLEIrDA, IrDA_RX);
		if (retval != 0) 
		{
			uartprintf("Set IrDA Mode fail!\n");
			return;	
		}
	
		uartprintf("\n Any key start to RX test ...\n");
		uartprintf("    Should receive [%d] bytes\n", RXSIZE);
		ch = uartgetchar();		
		while(1)
		{
			/* 
				Don't printf any message that occur data lost. The RX buffer full will be occurred. 
			*/
			retval = uartRead(param.ucUartNo, RX_Test, 500);
			if(retval <= RXSIZE) 
			{
				
				if (retval != 0)
				{
					for (i = 0; i < retval; i++)
					{
						if (RX_Test[i] != 0x31)	
							uartprintf("%d[%x]\n", i, RX_Test[i]);
					}	
				}
				
				reccnt += retval;
				
				if (reccnt >= RXSIZE)
				{
					uartprintf("\n RX test complete [%d] bytes received\n", reccnt);	
					break;	
				}	
			}
			else
				err++;
		}		
	}
	
	return;	
}



void RS485_Tx_Test()
{
	int retval, cnt = 0;
	UINT8 uAddress = 0xC0;
	UINT32 i;
	
	/* set RS485 mode */
	retval = uartIoctl(param.ucUartNo, UART_IOC_SET_RS485_MODE, UART_ALT_CSR_RS485_AUD_Msk, uAddress);
	if (retval != 0) 
	{
		uartprintf("Set RS485 Mode mode fail!\n");
		return;	
	}
	
	RS485_Test[0] = uAddress;
	for(i = 1; i < 50; i++)
	{
		RS485_Test[i] = i;
	}
	
	/* Send RS485 address */
	retval = uartIoctl(param.ucUartNo, UART_IOC_SEND_RS485_ADDRESS, RS485_Test[0], 0);
	if (retval != 0) 
	{
		uartprintf("Set RS485 Mode mode fail!\n");
		return;	
	}
	
	while(1)
	{
		retval = uartWrite(param.ucUartNo, &RS485_Test[cnt], (50-1-cnt));
		cnt += retval;
		if(cnt >= 49)
		{
			uartprintf("\n TX test complete [%d] bytes send\n", (cnt+1));
			break;
		}
	}

}

void RS485_Rx_Test()
{
	int retval, reccnt = 0;
	UINT32 uAddress = 0xC0;
	UINT32 uSelectNMMMode = 0;
	UINT32 i;
	
	/* set RS485 mode */
	if(uSelectNMMMode == 1)
	{
		/* Set RX_DIS enable before set RS485-NMM mode */
        retval = uartIoctl(param.ucUartNo, UART_IOC_SET_RS485_RXOFF, 1, 0);
		
		// Set NMM Mode
		retval = uartIoctl(param.ucUartNo, UART_IOC_SET_RS485_MODE, 
						  (UART_ALT_CSR_RS485_NMM_Msk|UART_ALT_CSR_RS485_ADD_EN_Msk|UART_ALT_CSR_RS485_AUD_Msk), uAddress);
	}
	else
	{
		// Set AAD Mode
		retval = uartIoctl(param.ucUartNo, UART_IOC_SET_RS485_MODE, 
	                      (UART_ALT_CSR_RS485_AAD_Msk|UART_ALT_CSR_RS485_ADD_EN_Msk|UART_ALT_CSR_RS485_AUD_Msk), uAddress);
		
	}
	if (retval != 0) 
	{
		uartprintf("Set RS485 Mode mode fail!\n");
		return;	
	}
	
	/* set RX interrupt mode */
	retval = uartIoctl(param.ucUartNo, UART_IOC_SETRXMODE, UARTINTMODE, 0);
	if (retval != 0) 
	{
		uartprintf("Set RX interrupt mode fail!\n");
		return;	
	}
	
	while(1)
	{
		retval = uartRead(param.ucUartNo, &RX_Test[reccnt], (50-reccnt));
		if(retval <= 50) 
		{
			reccnt += retval;

			if (reccnt >= 50)
			{
				for (i = 1; i < 50; i++)
				{
					if (RX_Test[i] != i)	
						uartprintf("%d[%x]\n", i + 1, RX_Test[i]);
				}
				
				uartprintf("\n RX test complete [%d] bytes received\n", reccnt);	
				break;	
			}	
		}
	}
}

void RS485Example (void)
{
	int ch, retval;
	
	/* configure UART */
	param.uFreq = 12000000;
	param.uBaudRate = 115200; 
	param.ucUartNo = UART1;
	param.ucDataBits = DATA_BITS_8;
	param.ucStopBits = STOP_BITS_1;
	param.ucParity = PARITY_NONE;
	param.ucRxTriggerLevel = UART_FCR_RFITL_1BYTE;	
	retval = uartOpen(&param); 
	if(retval != 0) 
	{
		uartprintf("Open UART error!\n");
		return;
	}
	
	uartprintf("\n Select Test Tx or Rx: ...\n");
	uartprintf("  1. Tx \n");
	uartprintf("  2. Rx \n\n");
	ch = uartgetchar();
	if((ch != '1') && (ch != '2'))
	{
		uartprintf("Please Select again !!\n");
		return;
	}
	
	/* set TX interrupt mode */
	retval = uartIoctl(param.ucUartNo, UART_IOC_SETTXMODE, UARTINTMODE, 0);
	if (retval != 0) 
	{
		uartprintf("Set TX interrupt mode fail!\n");
		return;	
	}
		
	/* set interrupt */
	retval = uartIoctl(param.ucUartNo, UART_IOC_SETINTERRUPT, UARTINTMODE, (UART_IER_RLS_IEN_Msk|UART_IER_RTO_IEN_Msk));
	if (retval != 0) 
	{
		uartprintf("Set RX interrupt mode fail!\n");
		return;	
	}
	
	if(ch == '1')
	{
		RS485_Tx_Test();
	}
	else if(ch == '2')
	{
		RS485_Rx_Test();
	}
	
}

UINT8 GetParityValue(UINT32 u32id)
{
	/* Compute Parity Value  */
    UINT32 u32Res = 0, ID[6], p_Bit[2] , mask = 0;

    for(mask = 0; mask < 6; mask++)
        ID[mask] = (u32id & (1 << mask)) >> mask;

    p_Bit[0] = (ID[0] + ID[1] + ID[2] + ID[4]) % 2;
    p_Bit[1] = (!((ID[1] + ID[3] + ID[4] + ID[5]) % 2));

    u32Res = u32id + (p_Bit[0] << 6) + (p_Bit[1] << 7);
    return u32Res;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Compute CheckSum Value , MODE_CLASSIC:(Not Include ID)    MODE_ENHANCED:(Include ID)                    */
/*---------------------------------------------------------------------------------------------------------*/
uint32_t GetCheckSumValue(uint8_t *pu8Buf, uint32_t u32ModeSel)
{
	/* Compute CheckSum Value , MODE_CLASSIC:(Not Include ID)    MODE_ENHANCED:(Include ID)  */
    uint32_t i, CheckSum = 0;

    for(i = u32ModeSel; i <= 9; i++)
    {
        CheckSum += pu8Buf[i];
        if(CheckSum >= 256)
            CheckSum -= 255;
    }
    return (255 - CheckSum);
}

void LIN_SendHeader(UINT32 uLINID)
{
	UINT32 retval;
	UINT32 cnt = 0;
	
    g_i32pointer = 0 ;

    g_u8SendData[g_i32pointer++] = 0x55 ;                   // SYNC Field
    g_u8SendData[g_i32pointer++] = GetParityValue(uLINID);   // ID+Parity Field
	
	while(1)
	{
		retval = uartWrite(param.ucUartNo, &g_u8SendData[cnt], (2-cnt));
		cnt += retval;
		if(cnt >= 2)
		{
			break;
		}
	}
}

void LIN_SendResponse(INT32 checkSumOption, UINT32 *pu32TxBuf)
{
    int32_t i32;
	UINT32 retval;
	UINT32 cnt = 0;

    for(i32 = 0; i32 < 8; i32++)
        g_u8SendData[g_i32pointer++] = pu32TxBuf[i32] ;

    g_u8SendData[g_i32pointer++] = GetCheckSumValue(g_u8SendData, checkSumOption) ; //CheckSum Field

	while(1)
	{
		retval = uartWrite(param.ucUartNo, &g_u8SendData[cnt+2], (9-cnt));
		cnt += retval;
		if(cnt >= 2)
		{
			break;
		}
	}
}

void LINExample(void)
{
	int retval;
	UINT32 uBreakLength = 11;  // break field length is 12 bits 
	UINT32 uID = 0x35;
	static UINT32 testPattern[8] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
	
	/* configure UART */
	param.uFreq = 12000000;
	param.uBaudRate = 9600; 
	param.ucUartNo = UART1;
	param.ucDataBits = DATA_BITS_8;
	param.ucStopBits = STOP_BITS_1;
	param.ucParity = PARITY_NONE;
	param.ucRxTriggerLevel = UART_FCR_RFITL_1BYTE;	
	retval = uartOpen(&param); 
	if(retval != 0) 
	{
		uartprintf("Open UART error!\n");
		return;
	}
	
	/* set TX interrupt mode */
	retval = uartIoctl(param.ucUartNo, UART_IOC_SETTXMODE, UARTINTMODE, 0);
	if (retval != 0) 
	{
		uartprintf("Set TX interrupt mode fail!\n");
		return;	
	}
		
	/* set interrupt */
	retval = uartIoctl(param.ucUartNo, UART_IOC_SETINTERRUPT, UARTINTMODE, (UART_IER_RLS_IEN_Msk|UART_IER_RTO_IEN_Msk));
	if (retval != 0) 
	{
		uartprintf("Set RX interrupt mode fail!\n");
		return;	
	}
	
	/* Set LIN operation mode, Tx mode and break field length is 12 bits */
	retval = uartIoctl(param.ucUartNo, UART_IOC_SET_LIN_MODE, UART_ALT_CSR_LIN_TX_EN_Msk, uBreakLength);
	if (retval != 0) 
	{
		uartprintf("Set LIN mode fail!\n");
		return;	
	}
	
	/* Send ID=0x35 Header and Response TestPatten */
    LIN_SendHeader(uID);
    LIN_SendResponse(MODE_CLASSIC, &testPattern[0]);
}

int main (void)
{
	int ch;
	
	sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();
	
	outpw(REG_SYS_GPE_MFPL, (inpw(REG_SYS_GPE_MFPL) & 0xff0000ff) | (0x9999 << 8));// GPE2, 3, 4, 5 //TX, RX, RTS, CTS 
	
	while(1)
	{
		uartprintf("\n Select Example: ...\n");
		uartprintf("  1. UART Example \n");
		uartprintf("  2. RS485 Example \n");
		uartprintf("  3. LIN Example \n\n");
		uartprintf("  4. IrDA Example \n\n");
		ch = uartgetchar();
	
		if (ch == '1')
	uartExample();
		else if(ch == '2')
			RS485Example();
		else if(ch == '3')
			LINExample();
		else if(ch == '4')
			IrDAExample();
	}
	
	//while(1);
	//return 0;
}

