/**************************************************************************//**
 * @file     spi_flash.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 15/05/07 4:46p $
 * @brief    Read/write Flash via SPI interface
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "spi.h"

#define TEST_NUMBER 1   /* page numbers */
#define TEST_LENGTH 256 /* length */

#define SPI_FLASH_PORT  SPI2

uint8_t SrcArray[TEST_LENGTH];
uint8_t DestArray[TEST_LENGTH];

void SpiFlash_ChipErase(void)
{
    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x06, Write enable
    spiWrite(0, 0, 0x06);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
  
    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);

    //////////////////////////////////////////

    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0xC7, Chip Erase
    spiWrite(0, 0, 0xC7);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);
}

uint8_t SpiFlash_ReadStatusReg(void)
{
    uint8_t u8Status;
    
    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x05, Read status register
    spiWrite(0, 0, 0x05);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    // read status
     spiWrite(0, 0, 0x00);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    u8Status = spiRead(0, 0);
    
    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);
    
    return u8Status;
}

void SpiFlash_WaitReady(void)
{
    uint8_t ReturnValue;

    do {
        ReturnValue = SpiFlash_ReadStatusReg();
        ReturnValue = ReturnValue & 1;
    } while(ReturnValue!=0); // check the BUSY bit
}

void SpiFlash_NormalPageProgram(uint32_t StartAddress, uint8_t *u8DataBuffer)
{
    uint32_t i = 0;

    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x06, Write enable
    spiWrite(0, 0, 0x06);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);


    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x02, Page program
    spiWrite(0, 0, 0x02);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    // send 24-bit start address
    spiWrite(0, 0, (StartAddress>>16) & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    
    spiWrite(0, 0, (StartAddress>>8) & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    
    spiWrite(0, 0, StartAddress & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    
    // write data
    for(i=0;i<255;i++)
    {
        spiWrite(0, 0, u8DataBuffer[i]);
        spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
        while(spiGetBusyStatus(0));
    }
    
    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);
}

void SpiFlash_NormalRead(uint32_t StartAddress, uint8_t *u8DataBuffer)
{
    uint32_t i;

    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x03, Read data
    spiWrite(0, 0, 0x03);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    // send 24-bit start address
   spiWrite(0, 0, (StartAddress>>16) & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    
    spiWrite(0, 0, (StartAddress>>8) & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    
    spiWrite(0, 0, StartAddress & 0xFF);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));

    // read data
    for(i=0; i<256; i++) {
        spiWrite(0, 0, 0x00);
        spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
        while(spiGetBusyStatus(0));
        u8DataBuffer[i] = spiRead(0, 0);  
    }
}

uint16_t SpiFlash_ReadMidDid(void)
{
    uint8_t u8RxData[2];

    // /CS: active
    spiIoctl(0, SPI_IOC_ENABLE_SS, SPI_SS_SS0, 0);

    // send Command: 0x90, Read Manufacturer/Device ID
    spiWrite(0, 0, 0x90);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    
    // send 24-bit '0', dummy
    spiWrite(0, 0, 0x00);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    
    spiWrite(0, 0, 0x00);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    
    spiWrite(0, 0, 0x00);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
 
    // receive 16-bit
    spiWrite(0, 0, 0x00);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    u8RxData[0] = spiRead(0, 0);
    
    spiWrite(0, 0, 0x00);
    spiIoctl(0, SPI_IOC_TRIGGER, 0, 0);
    while(spiGetBusyStatus(0));
    u8RxData[1] = spiRead(0, 0);
        
    // /CS: de-active
    spiIoctl(0, SPI_IOC_DISABLE_SS, SPI_SS_SS0, 0);

    return ( (u8RxData[0]<<8) | u8RxData[1] );
}

int32_t main(void)
{	
	uint16_t u16ID;
    uint32_t u32ByteCount, u32FlashAddress, u32PageNumber;
    uint32_t nError = 0;
    
	*((volatile unsigned int *)REG_AIC_MDCR)=0xFFFFFFFF;  // disable all interrupt channel
	*((volatile unsigned int *)REG_AIC_MDCRH)=0xFFFFFFFF;  // disable all interrupt channel
	
	outpw(REG_CLK_HCLKEN, 0x0527);
	outpw(REG_CLK_PCLKEN0, 0);
	outpw(REG_CLK_PCLKEN1, 0);
	
	sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();
	
	/* Configure multi function pins to SPI0 */
	outpw(REG_SYS_GPB_MFPL, (inpw(REG_SYS_GPB_MFPL) & ~0xff000000) | 0xBB000000);
    outpw(REG_SYS_GPB_MFPH, (inpw(REG_SYS_GPB_MFPH) & ~0xff) | 0xBB);
	
    spiInit(0);
	spiOpen(0);
    
    // set spi interface speed to 2MHz
    spiIoctl(0, SPI_IOC_SET_SPEED, 2000000, 0);
    // set transfer length to 8-bit
    spiIoctl(0, SPI_IOC_SET_TX_BITLEN, 8, 0);
    // set transfer mode to mode-0
    spiIoctl(0, SPI_IOC_SET_MODE, 0, 0);
	
    // check flash id
    if((u16ID = SpiFlash_ReadMidDid()) != 0xEF17) {
        sysprintf("Wrong ID, 0x%x\n", u16ID);
        return -1;
    } else
        sysprintf("Flash found: W25Q128BV ...\n");
    
     sysprintf("Erase chip ...");

    /* Erase SPI flash */
    SpiFlash_ChipErase();

    /* Wait ready */
    SpiFlash_WaitReady();

    sysprintf("[OK]\n");

    /* init source data buffer */
    for(u32ByteCount=0; u32ByteCount<TEST_LENGTH; u32ByteCount++) {
        SrcArray[u32ByteCount] = u32ByteCount;
    }

    sysprintf("Start to normal write data to Flash ...");
    /* Program SPI flash */
    u32FlashAddress = 0;
    for(u32PageNumber=0; u32PageNumber<TEST_NUMBER; u32PageNumber++) {
        /* page program */
        SpiFlash_NormalPageProgram(u32FlashAddress, SrcArray);
        SpiFlash_WaitReady();
        u32FlashAddress += 0x100;
    }

    sysprintf("[OK]\n");

    /* clear destination data buffer */
    for(u32ByteCount=0; u32ByteCount<TEST_LENGTH; u32ByteCount++) {
        DestArray[u32ByteCount] = 0;
    }

    sysprintf("Normal Read & Compare ...");

    /* Read SPI flash */
    u32FlashAddress = 0;
    for(u32PageNumber=0; u32PageNumber<TEST_NUMBER; u32PageNumber++) {
        /* page read */
        SpiFlash_NormalRead(u32FlashAddress, DestArray);
        u32FlashAddress += 0x100;

        for(u32ByteCount=0; u32ByteCount<TEST_LENGTH; u32ByteCount++) {
            if(DestArray[u32ByteCount] != SrcArray[u32ByteCount])
                nError ++;
        }
    }

    if(nError == 0)
        sysprintf("[OK]\n");
    else
        sysprintf("[FAIL]\n");
    
	return 1;
}	
