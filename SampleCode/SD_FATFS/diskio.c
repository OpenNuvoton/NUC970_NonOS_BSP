/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nuc970.h"
#include "sdh.h"
#include "ff.h"
#include "diskio.h"


#define SD0_DRIVE		0        /* for SD0          */
#define SD1_DRIVE		1        /* for SD1          */
#define EMMC_DRIVE		2        /* for eMMC/NAND    */
#define USBH_DRIVE_0    3        /* USB Mass Storage */
#define USBH_DRIVE_1    4        /* USB Mass Storage */
#define USBH_DRIVE_2    5        /* USB Mass Storage */
#define USBH_DRIVE_3    6        /* USB Mass Storage */
#define USBH_DRIVE_4    7        /* USB Mass Storage */


#define DISK_BUFFER_SIZE    (32*1024)
#ifdef __ICCARM__
#pragma data_alignment = 32
static BYTE  fatfs_win_buff_pool[DISK_BUFFER_SIZE];       /* FATFS window buffer is cachable. Must not use it directly. */
#else
static BYTE  fatfs_win_buff_pool[DISK_BUFFER_SIZE] __attribute__((aligned(32)));       /* FATFS window buffer is cachable. Must not use it directly. */
#endif

BYTE  *fatfs_win_buff;

/* Definitions of physical drive number for each media */

#define DRV_SD0     0
#define DRV_SD1     1


/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (BYTE pdrv)       /* Physical drive number (0..) */
{

    switch (pdrv) {
    case DRV_SD0 :
        if (SD_GET_CARD_CAPACITY(SD_PORT0) == 0)
            return STA_NOINIT;
        break;

    case DRV_SD1 :
        if (SD_GET_CARD_CAPACITY(SD_PORT1) == 0)
            return STA_NOINIT;
        break;
    }
    return RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (BYTE pdrv)       /* Physical drive number (0..) */
{

    switch (pdrv) {
    case DRV_SD0 :
        if (SD_GET_CARD_CAPACITY(SD_PORT0) == 0)
            return STA_NOINIT;
        break;

    case DRV_SD1 :
        if (SD_GET_CARD_CAPACITY(SD_PORT1) == 0)
            return STA_NOINIT;
        break;
    }
    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,      /* Physical drive number (0..) */
    BYTE *buff,     /* Data buffer to store read data */
    DWORD sector,   /* Sector address (LBA) */
    UINT count      /* Number of sectors to read (1..128) */
)
{
	DRESULT   ret;

    outpw(REG_SDH_GCTL, SDH_GCTL_SDEN_Msk);
	//sysprintf("disk_read - drv:%d, sec:%d, cnt:%d, buff:0x%x\n", pdrv, sector, count, (UINT32)buff);
	
	if (!((UINT32)buff & 0x80000000))
	{
		/* Disk read buffer is not non-cachable buffer. Use my non-cachable to do disk read. */
		if (count * 512 > DISK_BUFFER_SIZE)
			return RES_ERROR;
			
		fatfs_win_buff = (BYTE *)((unsigned int)fatfs_win_buff_pool | 0x80000000);
        if (pdrv == DRV_SD0)
            ret = (DRESULT) SD_Read(SD_PORT0, fatfs_win_buff, sector, count);
        else if (pdrv == DRV_SD1)
            ret = (DRESULT) SD_Read(SD_PORT1, fatfs_win_buff, sector, count);
        else
			return RES_ERROR;
		memcpy(buff, fatfs_win_buff, count * 512);
	}
	else
	{
        if (pdrv == DRV_SD0)
            ret = (DRESULT) SD_Read(SD_PORT0, buff, sector, count);
        else if (pdrv == DRV_SD1)
            ret = (DRESULT) SD_Read(SD_PORT1, buff, sector, count);
        else
			return RES_ERROR;
	}
	return ret;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
    BYTE pdrv,          /* Physical drive number (0..) */
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Sector address (LBA) */
    UINT count          /* Number of sectors to write (1..128) */
)
{
	DRESULT   ret;

    outpw(REG_SDH_GCTL, SDH_GCTL_SDEN_Msk);
	//sysprintf("disk_write - drv:%d, sec:%d, cnt:%d, buff:0x%x\n", pdrv, sector, count, (UINT32)buff);
	
	if (!((UINT32)buff & 0x80000000))
	{
		/* Disk write buffer is not non-cachable buffer. Use my non-cachable to do disk write. */
		if (count * 512 > DISK_BUFFER_SIZE)
			return RES_ERROR;
			
		fatfs_win_buff = (BYTE *)((unsigned int)fatfs_win_buff_pool | 0x80000000);
		memcpy(fatfs_win_buff, buff, count * 512);
        if (pdrv == DRV_SD0)
            ret = (DRESULT) SD_Write(SD_PORT0, fatfs_win_buff, sector, count);
        else if (pdrv == DRV_SD1)
            ret = (DRESULT) SD_Write(SD_PORT1, fatfs_win_buff, sector, count);
        else
			return RES_ERROR;
	}
	else
	{
        if (pdrv == DRV_SD0)
            ret = (DRESULT) SD_Write(SD_PORT0, (UINT8 *)buff, sector, count);
        else if (pdrv == DRV_SD1)
            ret = (DRESULT) SD_Write(SD_PORT1, (UINT8 *)buff, sector, count);
        else
			return RES_ERROR;
	}
	return ret;
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,      /* Physical drive number (0..) */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{

    DRESULT res = RES_OK;

    switch (pdrv) {
    case DRV_SD0 :
        switch(cmd) {
        case CTRL_SYNC:
            break;
        case GET_SECTOR_COUNT:
            *(DWORD*)buff = SD0.totalSectorN;
            break;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = SD0.sectorSize;
            break;

        default:
            res = RES_PARERR;
            break;
        }
        break;

    case DRV_SD1 :
        switch(cmd) {
        case CTRL_SYNC:
            break;
        case GET_SECTOR_COUNT:
            *(DWORD*)buff = SD1.totalSectorN;
            break;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = SD1.sectorSize;
            break;

        default:
            res = RES_PARERR;
            break;
        }
        break;

    default:
        res = RES_PARERR;
        break;

    }
    return res;
}
