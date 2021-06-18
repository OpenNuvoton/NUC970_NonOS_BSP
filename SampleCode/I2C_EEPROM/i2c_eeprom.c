/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 16/02/16 4:25p $
 * @brief    Read/write EEPROM via I2C interface
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "i2c.h"

#define I2CNUM_0	0  
#define I2CNUM_1	1 
#define RETRY		1000  /* Programming cycle may be in progress. Please refer to 24LC64 datasheet */

#define TXSIZE		512
#define ADDROFFSET  1024

int32_t main(void)
{
	uint8_t data[TXSIZE], value[TXSIZE];
	int32_t i, j, err;
	int32_t rtval;
	
	outpw(REG_CLK_HCLKEN, 0x0527);
	outpw(REG_CLK_PCLKEN0, 0);
	outpw(REG_CLK_PCLKEN1, 0);
	
	sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();
	
	/* Enable GPIO clock */
	outpw(REG_CLK_PCLKEN0, (inpw(REG_CLK_PCLKEN0) | 0x8));
	
	/* Configure multi function pins to I2C0 */
	outpw(REG_SYS_GPG_MFPL, (inpw(REG_SYS_GPG_MFPL) & ~0xff) | 0x88);
	
	/* I2C clock pin enable schmitt trigger */
	outpw(REG_GPIOG_ISEN, (inpw(REG_GPIOG_ISEN) | 0x3));
	
	/* initialize test data */
	for(i = 0 ; i < TXSIZE ; i++)
		data[i] = i + 1;
	
	i2cInit(0);	
	
	/* I2C channel 0 Test, Byte Write/Random Read */
	sysprintf("\nI2C0 Byte Write/Random Read ...\n");
	
	rtval = i2cOpen((PVOID)I2CNUM_0);
	if(rtval < 0)
	{
		sysprintf("Open I2C0 error!\n");
		goto exit_test;
	}	
	i2cIoctl(I2CNUM_0, I2C_IOC_SET_DEV_ADDRESS, 0x50, 0);  /* On 910 EV board, set 0x50 for I2C0 */ 
	i2cIoctl(I2CNUM_0, I2C_IOC_SET_SPEED, 100, 0);
	
	/* Tx porcess */
	sysprintf("Start Tx --> ");
	for(i = 0 ; i < TXSIZE ; i++)
	{
		i2cIoctl(I2CNUM_0, I2C_IOC_SET_SUB_ADDRESS, i, 2);	
		j = RETRY;	
		while(j-- > 0) 
		{
			if(i2cWrite(I2CNUM_0, &data[i], 1) == 1)
				break;
		}						
		if(j <= 0)
			sysprintf("WRITE ERROR [%d]!\n", i);
	}
	sysprintf("done\n");
	
	/* Rx porcess */	
	sysprintf("Start Rx --> ");
	memset(value, 0 , TXSIZE);
	for(i = 0 ; i < TXSIZE ; i++)
	{
		i2cIoctl(I2CNUM_0, I2C_IOC_SET_SUB_ADDRESS, i, 2);	
		j = RETRY;
		while(j-- > 0) 
		{
			if(i2cRead(I2CNUM_0, &value[i], 1) == 1)
				break;
		}
		if(j <= 0)
			sysprintf("Read ERROR [%d]!\n", i);
	}
	sysprintf("done\n");
	
	/* Compare process */
	sysprintf("Compare data ...");
	err = 0;
	for(i = 0 ; i < TXSIZE ; i++)
	{
		if(value[i] != data[i])	
		{
			sysprintf("[%d] addr = 0x%08x, val = 0x%02x (should be 0x%02x)\n", i, i + ADDROFFSET, value[i], data[i]);
			err = 1;
		}
	}	
	
	if(err)
		sysprintf("== FAIL ==\n\n");	
	else
		sysprintf(" == PASS ==\n\n");	
	
	i2cClose(I2CNUM_0);
	
exit_test:
	
	sysprintf("Test finish ...\n");	
	i2cExit();
	
	return 1;
}	
