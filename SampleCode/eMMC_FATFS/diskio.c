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
#include "fmi.h"
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


#ifdef __ICCARM__
#pragma data_alignment = 32
static BYTE  fatfs_win_buff_pool[_MAX_SS];       /* FATFS window buffer is cachable. Must not use it directly. */
#else
static BYTE  fatfs_win_buff_pool[_MAX_SS] __attribute__((aligned(32)));       /* FATFS window buffer is cachable. Must not use it directly. */
#endif

BYTE  *fatfs_win_buff;

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (BYTE pdrv)       /* Physical drive number (0..) */
{
    if (FMI_EMMC_GET_CARD_CAPACITY() == 0)
        return STA_NOINIT;
    return RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (BYTE pdrv)       /* Physical drive number (0..) */
{
    if (FMI_EMMC_GET_CARD_CAPACITY() == 0)
        return STA_NOINIT;
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

    outpw(REG_FMI_CTL, FMI_CTL_EMMCEN_Msk);
	//sysprintf("disk_read - drv:%d, sec:%d, cnt:%d, buff:0x%x\n", pdrv, sector, count, (UINT32)buff);
	
	if (!((UINT32)buff & 0x80000000))
	{
		/* Disk read buffer is not non-cachable buffer. Use my non-cachable to do disk read. */
		if (count * 512 > _MAX_SS)
			return RES_ERROR;
			
		fatfs_win_buff = (BYTE *)((unsigned int)fatfs_win_buff_pool | 0x80000000);
        ret = (DRESULT) eMMC_Read(fatfs_win_buff, sector, count);
		memcpy(buff, fatfs_win_buff, count * 512);
	}
	else
	{
        ret = (DRESULT) eMMC_Read(buff, sector, count);
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

    outpw(REG_FMI_CTL, FMI_CTL_EMMCEN_Msk);
	//sysprintf("disk_write - drv:%d, sec:%d, cnt:%d, buff:0x%x\n", pdrv, sector, count, (UINT32)buff);
	
	if (!((UINT32)buff & 0x80000000))
	{
		/* Disk write buffer is not non-cachable buffer. Use my non-cachable to do disk write. */
		if (count * 512 > _MAX_SS)
			return RES_ERROR;
			
		fatfs_win_buff = (BYTE *)((unsigned int)fatfs_win_buff_pool | 0x80000000);
		memcpy(fatfs_win_buff, buff, count * 512);
        ret = (DRESULT) eMMC_Write(fatfs_win_buff, sector, count);
	}
	else
	{
        ret = (DRESULT) eMMC_Write((UINT8 *)buff, sector, count);
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

    switch(cmd) {
        case CTRL_SYNC:
            break;
        case GET_SECTOR_COUNT:
            *(DWORD*)buff = eMMC.totalSectorN;
            break;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = eMMC.sectorSize;
            break;

        default:
            res = RES_PARERR;
            break;
    }
    return res;
}
