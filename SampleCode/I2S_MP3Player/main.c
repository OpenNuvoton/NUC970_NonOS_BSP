/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * @brief    MP3 player sample plays MP3 files stored on SD memory card
 *
 * @copyright (C) 2024 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"
#include "i2s.h"
#include "i2c.h"
#include "config.h"
#include "sdh.h"
#include "ff.h"
#include "diskio.h"

FATFS FatFs[_VOLUMES];               /* File system object for logical drive */

uint8_t bAudioPlaying = 0;
extern uint32_t volatile sd_init_ok;
extern volatile uint8_t aPCMBuffer_Full[2];
volatile uint8_t u8PCMBuffer_Playing=0;

/***********************************************/
/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */

unsigned long get_fattime (void)
{
    unsigned long tmr;

    tmr=0x00000;

    return tmr;
}

unsigned int volatile gCardInit = 0;
void SDH_IRQHandler(void)
{
    unsigned int volatile isr;

    // FMI data abort interrupt
    if (inpw(REG_SDH_GINTSTS) & SDH_GINTSTS_DTAIF_Msk) {
        /* ResetAllEngine() */
        outpw(REG_SDH_GCTL, inpw(REG_SDH_GCTL) | SDH_GCTL_GCTLRST_Msk);
        outpw(REG_SDH_GINTSTS, SDH_GINTSTS_DTAIF_Msk);
    }

	//----- SD interrupt status
	isr = inpw(REG_SDH_INTSTS);
	if (isr & SDH_INTSTS_BLKDIF_Msk)		// block down
	{
		_sd_SDDataReady = TRUE;
		outpw(REG_SDH_INTSTS, SDH_INTSTS_BLKDIF_Msk);
	}

    if (isr & SDH_INTSTS_CDIF0_Msk) { // port 0 card detect
        //----- SD interrupt status
        // it is work to delay 50 times for SD_CLK = 200KHz
        {
    	    volatile int i;         // delay 30 fail, 50 OK
    	    for (i=0; i<0x500;i++){}    // delay to make sure got updated value from REG_SDISR.
            isr = inpw(REG_SDH_INTSTS);
        }

        if (isr & SDH_INTSTS_CDSTS0_Msk) {
            SD0.IsCardInsert = FALSE;   // SDISR_CD_Card = 1 means card remove for GPIO mode
            gCardInit = 0;
            sysprintf("\nCard Remove!\n");
            SD_Close_Disk(0);
        } else {
            gCardInit = 1;
        }
        outpw(REG_SDH_INTSTS, SDH_INTSTS_CDIF0_Msk);
    }

    if (isr & SDH_INTSTS_CDIF1_Msk) { // port 1 card detect
        //----- SD interrupt status
        // it is work to delay 50 times for SD_CLK = 200KHz
        {
    	    volatile int i;         // delay 30 fail, 50 OK
    	    for (i=0; i<0x500;i++){}    // delay to make sure got updated value from REG_SDISR.
            isr = inpw(REG_SDH_INTSTS);
        }

        if (isr & SDH_INTSTS_CDSTS1_Msk) {
            SD0.IsCardInsert = FALSE;   // SDISR_CD_Card = 1 means card remove for GPIO mode
            sysprintf("\nCard Remove!\n");
            SD_Close_Disk(1);
        } else {
            SD_Open_Disk(SD_PORT1 | CardDetect_From_GPIO);
        }
        outpw(REG_SDH_INTSTS, SDH_INTSTS_CDIF1_Msk);
    }

    // CRC error interrupt
    if (isr & SDH_INTSTS_CRCIF_Msk) {
        if (!(isr & SDH_INTSTS_CRC16_Msk)) {
            // handle CRC error
        } else if (!(isr & SDH_INTSTS_CRC7_Msk)) {
            extern unsigned int _sd_uR3_CMD;
            if (! _sd_uR3_CMD) {
                // handle CRC error
            }
        }
        outpw(REG_SDH_INTSTS, SDH_INTSTS_CRCIF_Msk);      // clear interrupt flag
    }
}

void Delay(int count)
{
	volatile uint32_t i;
	for (i = 0; i < count ; i++);
}

void play_callback(void)
{
    if (aPCMBuffer_Full[u8PCMBuffer_Playing^1] != 1)
        sysprintf("underflow!!\n");
    aPCMBuffer_Full[u8PCMBuffer_Playing] = 0;       //set empty flag
    u8PCMBuffer_Playing ^= 1;
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Write 9-bit data to 7-bit address register of NAU8822 with I2C0                                        */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_WriteNAU8822(uint8_t u8addr, uint16_t u16data)
{
    uint8_t TxData[2];
    
retry:    
    TxData[0] = (uint8_t)((u8addr << 1) | (u16data >> 8));
    TxData[1] = (uint8_t)(u16data & 0x00FF);
    
    i2cIoctl(0, I2C_IOC_SET_SUB_ADDRESS, TxData[0], 0);
    if(i2cWrite(0, &TxData[0], 2) != 2)
        goto retry;
}

/*---------------------------------------------------------------------------------------------------------*/
/*  NAU8822 Settings with I2C interface                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
void NAU8822_Setup()
{
    sysprintf("\nConfigure NAU8822 ...");

    I2C_WriteNAU8822(0,  0x000);   /* Reset all registers */
    Delay(0x200);

    //input source is MIC
    I2C_WriteNAU8822(1,  0x03F);
    I2C_WriteNAU8822(2,  0x1BF);   /* Enable L/R Headphone, ADC Mix/Boost, ADC */
    I2C_WriteNAU8822(3,  0x07F);   /* Enable L/R main mixer, DAC */
    I2C_WriteNAU8822(4,  0x010);   /* 16-bit word length, I2S format, Stereo */
    I2C_WriteNAU8822(5,  0x000);   /* Companding control and loop back mode (all disable) */
    I2C_WriteNAU8822(10, 0x008);   /* DAC soft mute is disabled, DAC oversampling rate is 128x */
    I2C_WriteNAU8822(14, 0x108);   /* ADC HP filter is disabled, ADC oversampling rate is 128x */
    I2C_WriteNAU8822(15, 0x1EF);   /* ADC left digital volume control */
    I2C_WriteNAU8822(16, 0x1EF);   /* ADC right digital volume control */
    I2C_WriteNAU8822(44, 0x033);   /* LMICN/LMICP is connected to PGA */
    I2C_WriteNAU8822(50, 0x001);   /* Left DAC connected to LMIX */
    I2C_WriteNAU8822(51, 0x001);   /* Right DAC connected to RMIX */

    sysprintf("[OK]\n");
}

void SYS_Init(void)
{
    /* enable SDH */
    outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | 0x40000000);
    /* select multi-function-pin */
    /* SD Port 0 -> PD0~7 */
    outpw(REG_SYS_GPD_MFPL, 0x66666666);

	/* Configure multi function pins to I2S */
    outpw(REG_SYS_GPG_MFPH, (inpw(REG_SYS_GPG_MFPH) & ~0x0FFFFF00) | 0x08888800);
    /* Configure multi function pins to I2C0 */
	outpw(REG_SYS_GPG_MFPL, (inpw(REG_SYS_GPG_MFPL) & ~0xffff) | 0x88);
}

extern void MP3Player(void);

int32_t main(void)
{
    TCHAR sd_path[] = { '0', ':', 0 };    /* SD drive started from 0 */

	outpw(REG_CLK_HCLKEN, 0x0527);
	outpw(REG_CLK_PCLKEN0, 0);
	outpw(REG_CLK_PCLKEN1, 0);
	
	sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    sysprintf("\n");
    sysprintf("+-----------------------------------------------------------------------+\n");
    sysprintf("|                   MP3 Player Sample with NAU8822 Codec                |\n");
    sysprintf("+-----------------------------------------------------------------------+\n");
    sysprintf(" Please put MP3 files on SD card \n");

    SYS_Init();

    sysInstallISR(HIGH_LEVEL_SENSITIVE|IRQ_LEVEL_1, SDH_IRQn, (PVOID)SDH_IRQHandler);
    /* enable CPSR I bit */
    sysSetLocalInterrupt(ENABLE_IRQ);
	sysEnableInterrupt(SDH_IRQn);

	/*--- init timer ---*/
	sysSetTimerReferenceClock (TIMER0, 12000000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);
    SD_SetReferenceClock(300000);

    SD_Open_Disk(SD_PORT0 | CardDetect_From_GPIO);
    f_chdrive(sd_path);          /* set default path */

	// Initialize I2S interface
    i2sInit();
    if(i2sOpen() != 0)
        return 0;

    // Select I2S function
    i2sIoctl(I2S_SELECT_BLOCK, I2S_BLOCK_I2S, 0);
    // Select 16-bit data width
    i2sIoctl(I2S_SELECT_BIT, I2S_BIT_WIDTH_16, 0);
    
    // Set DMA interrupt selection to half of DMA buffer
    i2sIoctl(I2S_SET_PLAY_DMA_INT_SEL, I2S_DMA_INT_HALF, 0);
    
    // Set to stereo 
    i2sIoctl(I2S_SET_CHANNEL, I2S_PLAY, I2S_CHANNEL_P_I2S_TWO);
    
    // Select I2S format
    i2sIoctl(I2S_SET_I2S_FORMAT, I2S_FORMAT_I2S, 0);

    // Set as master
    i2sIoctl(I2S_SET_MODE, I2S_MODE_MASTER, 0);

    // Set play and record call-back functions
    i2sIoctl(I2S_SET_I2S_CALLBACKFUN, I2S_PLAY, (uint32_t)&play_callback);

    // Initialize I2C-0 interface
    i2cInit(0);
    if(i2cOpen(0) != 0)
        return 0;
    
    // Set slave address is 0x1a
    i2cIoctl(0, I2C_IOC_SET_DEV_ADDRESS, 0x1A, 0);
	// I2C interface speed is 100KHz
    i2cIoctl(0, I2C_IOC_SET_SPEED, 100, 0);
    
    // Configure NAU8822 audio codec
    NAU8822_Setup();

    //while(1)
    {
        /* play mp3 */
        if (SD0.IsCardInsert == TRUE)
            MP3Player();
    }
}	

/* config play sampling rate */
void i2sConfigSampleRate(unsigned int u32SampleRate)
{
    sysprintf("Configure Sampling Rate to %d\n", u32SampleRate);
    if((u32SampleRate % 8) == 0)
    {
        //12.288MHz ==> APLL=98.4MHz / 4 = 24.6MHz
        //APLL is 98.4MHz
        outpw(REG_CLK_APLLCON, 0xC0008028);
        // Select APLL as I2S source and divider is (3+1)
        outpw(REG_CLK_DIVCTL1, (inpw(REG_CLK_DIVCTL1) & ~0x001f0000) | (0x2 << 19) | (0x3 << 24));
        // Set data width is 16-bit, stereo
        i2sSetSampleRate(24600000, u32SampleRate, 16, 2);
    }
    else
    {
        //11.2896Mhz ==> APLL=90.4 MHz / 8 = 11.2875 MHz
        //APLL is 90.4 MHz
        outpw(REG_CLK_APLLCON, 0xC0008170);
        // Select APLL as I2S source and divider is (7+1)
        outpw(REG_CLK_DIVCTL1, (inpw(REG_CLK_DIVCTL1) & ~0x001f0000) | (0x2 << 19) | (0x7 << 24));
        // Set data width is 16-bit, stereo
        i2sSetSampleRate(11287500, u32SampleRate, 16, 2);
    }
}
