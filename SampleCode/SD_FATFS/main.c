/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 15/06/12 10:03a $
 * @brief    Access a SD card formatted in FAT file system
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "sdh.h"
#include "ff.h"
#include "diskio.h"


#define BUFF_SIZE		(64*1024)

static UINT blen = BUFF_SIZE;
DWORD acc_size;             			/* Work register for fs command */
WORD acc_files, acc_dirs;
FILINFO Finfo;

char Line[256];             			/* Console input buffer */
#if _USE_LFN
char Lfname[512];
#endif

__align(32) BYTE Buff_Pool[BUFF_SIZE] ;       /* Working buffer */

BYTE  *Buff;

void timer_init()
{
	sysprintf("timer_init() To do...\n");
}

uint32_t get_timer_value()
{
	sysprintf("get_timer_value() To do...\n");
	return 1;
}
BYTE SD_Drv; // select SD0

void  dump_buff_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0) {
        sysprintf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            sysprintf("%02x ", pucBuff[nIdx + i]);
        sysprintf("  ");
        for (i = 0; i < 16; i++) {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                sysprintf("%c", pucBuff[nIdx + i]);
            else
                sysprintf(".");
            nBytes--;
        }
        nIdx += 16;
        sysprintf("\n");
    }
    sysprintf("\n");
}


/*--------------------------------------------------------------------------*/
/* Monitor                                                                  */

/*----------------------------------------------*/
/* Get a value of the string                    */
/*----------------------------------------------*/
/*  "123 -5   0x3ff 0b1111 0377  w "
        ^                           1st call returns 123 and next ptr
           ^                        2nd call returns -5 and next ptr
                   ^                3rd call returns 1023 and next ptr
                          ^         4th call returns 15 and next ptr
                               ^    5th call returns 255 and next ptr
                                  ^ 6th call fails and returns 0
*/

int xatoi (         /* 0:Failed, 1:Successful */
    TCHAR **str,    /* Pointer to pointer to the string */
    long *res       /* Pointer to a variable to store the value */
)
{
    unsigned long val;
    unsigned char r, s = 0;
    TCHAR c;


    *res = 0;
    while ((c = **str) == ' ') (*str)++;    /* Skip leading spaces */

    if (c == '-') {     /* negative? */
        s = 1;
        c = *(++(*str));
    }

    if (c == '0') {
        c = *(++(*str));
        switch (c) {
        case 'x':       /* hexadecimal */
            r = 16;
            c = *(++(*str));
            break;
        case 'b':       /* binary */
            r = 2;
            c = *(++(*str));
            break;
        default:
            if (c <= ' ') return 1; /* single zero */
            if (c < '0' || c > '9') return 0;   /* invalid char */
            r = 8;      /* octal */
        }
    } else {
        if (c < '0' || c > '9') return 0;   /* EOL or invalid char */
        r = 10;         /* decimal */
    }

    val = 0;
    while (c > ' ') {
        if (c >= 'a') c -= 0x20;
        c -= '0';
        if (c >= 17) {
            c -= 7;
            if (c <= 9) return 0;   /* invalid char */
        }
        if (c >= r) return 0;       /* invalid char for current radix */
        val = val * r + c;
        c = *(++(*str));
    }
    if (s) val = 0 - val;           /* apply sign if needed */

    *res = val;
    return 1;
}


/*----------------------------------------------*/
/* Dump a block of byte array                   */

void put_dump (
    const unsigned char* buff,  /* Pointer to the byte array to be dumped */
    unsigned long addr,         /* Heading address value */
    int cnt                     /* Number of bytes to be dumped */
)
{
    int i;


    sysprintf("%08x ", addr);

    for (i = 0; i < cnt; i++)
        sysprintf(" %02x", buff[i]);

    sysprintf(" ");
    for (i = 0; i < cnt; i++)
        sysPutChar((TCHAR)((buff[i] >= ' ' && buff[i] <= '~') ? buff[i] : '.'));

    sysprintf("\n");
}


/*--------------------------------------------------------------------------*/
/* Monitor                                                                  */
/*--------------------------------------------------------------------------*/

static
FRESULT scan_files (
    char* path      /* Pointer to the path name working buffer */
)
{
    DIR dirs;
    FRESULT res;
    BYTE i;
    char *fn;


    if ((res = f_opendir(&dirs, path)) == FR_OK) {
        i = strlen(path);
        while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
            if (_FS_RPATH && Finfo.fname[0] == '.') continue;
#if _USE_LFN
            fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname;
#else
            fn = Finfo.fname;
#endif
            if (Finfo.fattrib & AM_DIR) {
                acc_dirs++;
                *(path+i) = '/';
                strcpy(path+i+1, fn);
                res = scan_files(path);
                *(path+i) = '\0';
                if (res != FR_OK) break;
            } else {
                /*              sysprintf("%s/%s\n", path, fn); */
                acc_files++;
                acc_size += Finfo.fsize;
            }
        }
    }

    return res;
}



void put_rc (FRESULT rc)
{
    const TCHAR *p =
        _T("OK\0DISK_ERR\0INT_ERR\0NOT_READY\0NO_FILE\0NO_PATH\0INVALID_NAME\0")
        _T("DENIED\0EXIST\0INVALID_OBJECT\0WRITE_PROTECTED\0INVALID_DRIVE\0")
        _T("NOT_ENABLED\0NO_FILE_SYSTEM\0MKFS_ABORTED\0TIMEOUT\0LOCKED\0")
        _T("NOT_ENOUGH_CORE\0TOO_MANY_OPEN_FILES\0");
    //FRESULT i;
    uint32_t i;
    for (i = 0; (i != (UINT)rc) && *p; i++) {
        while(*p++) ;
    }
    sysprintf(_T("rc=%d FR_%s\n"), (UINT)rc, p);
}

/*----------------------------------------------*/
/* Get a line from the input                    */
/*----------------------------------------------*/

void get_line (char *buff, int len)
{
    TCHAR c;
    int idx = 0;



    for (;;) {
        c = sysGetChar();
        sysPutChar(c);
        if (c == '\r') break;
        if ((c == '\b') && idx) idx--;
        if ((c >= ' ') && (idx < len - 1)) buff[idx++] = c;
    }
    buff[idx] = 0;

    sysPutChar('\n');

}


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
    	    for (i=0; i<0x500;i++);    // delay to make sure got updated value from REG_SDISR.
            isr = inpw(REG_SDH_INTSTS);
        }

#ifdef _USE_DAT3_DETECT_
        if (!(isr & SDH_INTSTS_CDSTS0_Msk)) {
            SD0.IsCardInsert = FALSE;
            sysprintf("\nCard Remove!\n");
            SD_Close_Disk(0);
        } else {
            SD_Open_Disk(SD_PORT0 | CardDetect_From_DAT3);
        }
#else
        if (isr & SDH_INTSTS_CDSTS0_Msk) {
            SD0.IsCardInsert = FALSE;   // SDISR_CD_Card = 1 means card remove for GPIO mode
            sysprintf("\nCard Remove!\n");
            SD_Close_Disk(0);
        } else {
            SD_Open_Disk(SD_PORT0 | CardDetect_From_GPIO);
        }
#endif

        outpw(REG_SDH_INTSTS, SDH_INTSTS_CDIF0_Msk);
    }

    if (isr & SDH_INTSTS_CDIF1_Msk) { // port 1 card detect
        //----- SD interrupt status
        // it is work to delay 50 times for SD_CLK = 200KHz
        {
    	    volatile int i;         // delay 30 fail, 50 OK
    	    for (i=0; i<0x500;i++);    // delay to make sure got updated value from REG_SDISR.
            isr = inpw(REG_SDH_INTSTS);
        }

#ifdef _USE_DAT3_DETECT_
        if (!(isr & SDH_INTSTS_CDSTS1_Msk)) {
            SD0.IsCardInsert = FALSE;
            sysprintf("\nCard Remove!\n");
            SD_Close_Disk(1);
        } else {
            SD_Open_Disk(SD_PORT1 | CardDetect_From_DAT3);
        }
#else
        if (isr & SDH_INTSTS_CDSTS1_Msk) {
            SD0.IsCardInsert = FALSE;   // SDISR_CD_Card = 1 means card remove for GPIO mode
            sysprintf("\nCard Remove!\n");
            SD_Close_Disk(1);
        } else {
            SD_Open_Disk(SD_PORT1 | CardDetect_From_GPIO);
        }
#endif

        outpw(REG_SDH_INTSTS, SDH_INTSTS_CDIF1_Msk);
    }

    // CRC error interrupt
    if (isr & SDH_INTSTS_CRCIF_Msk) {
        if (!(isr & SDH_INTSTS_CRC16_Msk)) {
            //sysprintf("***** ISR sdioIntHandler(): CRC_16 error !\n");
            // handle CRC error
        } else if (!(isr & SDH_INTSTS_CRC7_Msk)) {
            extern unsigned int _sd_uR3_CMD;
            if (! _sd_uR3_CMD) {
                //sysprintf("***** ISR sdioIntHandler(): CRC_7 error !\n");
                // handle CRC error
            }
        }
        outpw(REG_SDH_INTSTS, SDH_INTSTS_CRCIF_Msk);      // clear interrupt flag
    }
}

void SYS_Init(void)
{
    /* enable SDH */
    outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | 0x40000000);

    /* select multi-function-pin */
    /* SD Port 0 -> PD0~7 */
    outpw(REG_SYS_GPD_MFPL, 0x66666666);
    SD_Drv = 0;

#if 0 // port 1
    /* initial SD1 pin -> PE2~9 */
    outpw(REG_SYS_GPE_MFPL, inpw(REG_SYS_GPE_MFPL) & ~0xffffff00 | 0x66666600);
    outpw(REG_SYS_GPE_MFPH, inpw(REG_SYS_GPE_MFPH) & ~0x000000ff | 0x00000066);

    /* initial SD1 pin -> PH6~13 */
    outpw(REG_SYS_GPH_MFPL, inpw(REG_SYS_GPH_MFPL) & ~0xff000000 | 0x66000000);
    outpw(REG_SYS_GPH_MFPH, inpw(REG_SYS_GPH_MFPH) & ~0x00ffffff | 0x00666666);

    /* initial SD1 pin -> PI5~10, 12~13 */
    outpw(REG_SYS_GPI_MFPL, inpw(REG_SYS_GPI_MFPL) & ~0xfff00000 | 0x44400000);
    outpw(REG_SYS_GPI_MFPH, inpw(REG_SYS_GPI_MFPH) & ~0x00ff0fff | 0x00440444);
    SD_Drv = 1;
#endif
    
}


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


static FIL file1, file2;        /* File objects */

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int32_t main(void)
{
    char 		*ptr, *ptr2;
    long 		p1, p2, p3;
    BYTE 		*buf;
    FATFS 		*fs;              /* Pointer to file system object */
    TCHAR		sd_path[] = { '0', ':', 0 };    /* SD drive started from 0 */
    FRESULT 	res;

    DIR dir;                /* Directory object */
    UINT s1, s2, cnt;
    static const BYTE ft[] = {0, 12, 16, 32};
    DWORD ofs = 0, sect = 0;

	sysInitializeUART();
    sysprintf("\n");
    sysprintf("===============================================\n");
    sysprintf("          SDH/SD Card Testing                  \n");
    sysprintf("===============================================\n");

	sysDisableCache();
	sysInvalidCache();
	sysSetMMUMappingMethod(MMU_DIRECT_MAPPING);
	sysEnableCache(CACHE_WRITE_BACK);

	Buff = (BYTE *)((UINT32)&Buff_Pool[0] | 0x80000000);   /* use non-cache buffer */

    SYS_Init();

    sysInstallISR(HIGH_LEVEL_SENSITIVE|IRQ_LEVEL_1, SDH_IRQn, (PVOID)SDH_IRQHandler);
    /* enable CPSR I bit */
    sysSetLocalInterrupt(ENABLE_IRQ);
	sysEnableInterrupt(SDH_IRQn);

	/*--- init timer ---*/
	sysSetTimerReferenceClock (TIMER0, 12000000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);
	
    sysprintf("\n\nNUC970 SD FATFS TEST!\n");
    SD_SetReferenceClock(300000);
    SD_Open_Disk(SD_PORT0 | CardDetect_From_GPIO);
	f_chdrive(sd_path);          /* set default path */

    for (;;) {
        if(!(SD_CardDetection(SD_Drv)))
            continue;

        sysprintf(_T(">"));
        ptr = Line;
        get_line(ptr, sizeof(Line));
        switch (*ptr++) {

        case 'q' :  /* Exit program */
            return 0;

        case '0' :
        case '1' :
        	ptr--;
        	*(ptr+1) = ':';
        	*(ptr+2) = 0;
        	put_rc(f_chdrive((TCHAR *)ptr));
        	break;

        case 'd' :
            switch (*ptr++) {
            case 'd' :  /* dd [<lba>] - Dump sector */
                if (!xatoi(&ptr, &p2)) p2 = sect;
                res = (FRESULT)disk_read(SD_Drv, Buff, p2, 1);
                if (res) {
                    sysprintf("rc=%d\n", (WORD)res);
                    break;
                }
                sect = p2 + 1;
                sysprintf("Sector:%d\n", p2);
                for (buf=(unsigned char*)Buff, ofs = 0; ofs < 0x200; buf+=16, ofs+=16)
                    put_dump(buf, ofs, 16);
                break;

            }
            break;

        case 'b' :
            switch (*ptr++) {
            case 'd' :  /* bd <addr> - Dump R/W buffer */
                if (!xatoi(&ptr, &p1)) break;
                for (ptr=(char*)&Buff[p1], ofs = p1, cnt = 32; cnt; cnt--, ptr+=16, ofs+=16)
                    put_dump((BYTE*)ptr, ofs, 16);
                break;

            case 'e' :  /* be <addr> [<data>] ... - Edit R/W buffer */
                if (!xatoi(&ptr, &p1)) break;
                if (xatoi(&ptr, &p2)) {
                    do {
                        Buff[p1++] = (BYTE)p2;
                    } while (xatoi(&ptr, &p2));
                    break;
                }
                for (;;) {
                    sysprintf("%04X %02X-", (WORD)p1, Buff[p1]);
                    get_line(Line, sizeof(Line));
                    ptr = Line;
                    if (*ptr == '.') break;
                    if (*ptr < ' ') {
                        p1++;
                        continue;
                    }
                    if (xatoi(&ptr, &p2))
                        Buff[p1++] = (BYTE)p2;
                    else
                        sysprintf("???\n");
                }
                break;

            case 'r' :  /* br <sector> [<n>] - Read disk into R/W buffer */
                if (!xatoi(&ptr, &p2)) break;
                if (!xatoi(&ptr, &p3)) p3 = 1;
                sysprintf("rc=%d\n", disk_read(SD_Drv, Buff, p2, p3));
                break;

            case 'w' :  /* bw <sector> [<n>] - Write R/W buffer into disk */
                if (!xatoi(&ptr, &p2)) break;
                if (!xatoi(&ptr, &p3)) p3 = 1;
                sysprintf("rc=%d\n", disk_write(SD_Drv, Buff, p2, p3));
                break;

            case 'f' :  /* bf <n> - Fill working buffer */
                if (!xatoi(&ptr, &p1)) break;
                memset(Buff, (int)p1, BUFF_SIZE);
                break;

            }
            break;



        case 'f' :
            switch (*ptr++) {

            case 's' :  /* fs - Show logical drive status */
                res = f_getfree("", (DWORD*)&p2, &fs);
                if (res) {
                    put_rc(res);
                    break;
                }
                sysprintf("FAT type = FAT%d\nBytes/Cluster = %d\nNumber of FATs = %d\n"
                       "Root DIR entries = %d\nSectors/FAT = %d\nNumber of clusters = %d\n"
                       "FAT start (lba) = %d\nDIR start (lba,clustor) = %d\nData start (lba) = %d\n\n...",
                       ft[fs->fs_type & 3], fs->csize * 512UL, fs->n_fats,
                       fs->n_rootdir, fs->fsize, fs->n_fatent - 2,
                       fs->fatbase, fs->dirbase, fs->database
                      );
                acc_size = acc_files = acc_dirs = 0;
#if _USE_LFN
                Finfo.lfname = Lfname;
                Finfo.lfsize = sizeof(Lfname);
#endif
                res = scan_files(ptr);
                if (res) {
                    put_rc(res);
                    break;
                }
                sysprintf("\r%d files, %d bytes.\n%d folders.\n"
                       "%d KB total disk space.\n%d KB available.\n",
                       acc_files, acc_size, acc_dirs,
                       (fs->n_fatent - 2) * (fs->csize / 2), p2 * (fs->csize / 2)
                      );
                break;
            case 'l' :  /* fl [<path>] - Directory listing */
                while (*ptr == ' ') ptr++;
                res = f_opendir(&dir, ptr);
                if (res) {
                    put_rc(res);
                    break;
                }
                p1 = s1 = s2 = 0;
                for(;;) {
                    res = f_readdir(&dir, &Finfo);
                    if ((res != FR_OK) || !Finfo.fname[0]) break;
                    if (Finfo.fattrib & AM_DIR) {
                        s2++;
                    } else {
                        s1++;
                        p1 += Finfo.fsize;
                    }
                    sysprintf("%c%c%c%c%c %d/%02d/%02d %02d:%02d    %9d  %s",
                           (Finfo.fattrib & AM_DIR) ? 'D' : '-',
                           (Finfo.fattrib & AM_RDO) ? 'R' : '-',
                           (Finfo.fattrib & AM_HID) ? 'H' : '-',
                           (Finfo.fattrib & AM_SYS) ? 'S' : '-',
                           (Finfo.fattrib & AM_ARC) ? 'A' : '-',
                           (Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
                           (Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63, Finfo.fsize, Finfo.fname);
#if _USE_LFN
                    for (p2 = strlen(Finfo.fname); p2 < 14; p2++)
                        sysprintf(" ");
                    sysprintf("%s\n", Lfname);
#else
                    sysprintf("\n");
#endif
                }
                sysprintf("%4d File(s),%10d bytes total\n%4d Dir(s)", s1, p1, s2);
                if (f_getfree(ptr, (DWORD*)&p1, &fs) == FR_OK)
                    sysprintf(", %10d bytes free\n", p1 * fs->csize * 512);
                break;


            case 'o' :  /* fo <mode> <file> - Open a file */
                if (!xatoi(&ptr, &p1)) break;
                while (*ptr == ' ') ptr++;
                put_rc(f_open(&file1, ptr, (BYTE)p1));
                break;

            case 'c' :  /* fc - Close a file */
                put_rc(f_close(&file1));
                break;

            case 'e' :  /* fe - Seek file pointer */
                if (!xatoi(&ptr, &p1)) break;
                res = f_lseek(&file1, p1);
                put_rc(res);
                if (res == FR_OK)
                    sysprintf("fptr=%d(0x%lX)\n", file1.fptr, file1.fptr);
                break;

            case 'd' :  /* fd <len> - read and dump file from current fp */
                if (!xatoi(&ptr, &p1)) break;
                ofs = file1.fptr;
                while (p1) {
                    if ((UINT)p1 >= 16) {
                        cnt = 16;
                        p1 -= 16;
                    } else                {
                        cnt = p1;
                        p1 = 0;
                    }
                    res = f_read(&file1, Buff, cnt, &cnt);
                    if (res != FR_OK) {
                        put_rc(res);
                        break;
                    }
                    if (!cnt) break;
                    put_dump(Buff, ofs, cnt);
                    ofs += 16;
                }
                break;

            case 'r' :  /* fr <len> - read file */
                if (!xatoi(&ptr, &p1)) break;
                p2 = 0;
                timer_init();
                while (p1) {
                    if ((UINT)p1 >= blen) {
                        cnt = blen;
                        p1 -= blen;
                    } else {
                        cnt = p1;
                        p1 = 0;
                    }
                    res = f_read(&file1, Buff, cnt, &s2);
                    if (res != FR_OK) {
                        put_rc(res);
                        break;
                    }
                    p2 += s2;
                    if (cnt != s2) break;
                }
                p1 = get_timer_value()/1000;
                if (p1)
                    sysprintf("%d bytes read with %d kB/sec.\n", p2, ((p2 * 100) / p1)/1024);
                break;

            case 'w' :  /* fw <len> <val> - write file */
                if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
                memset(Buff, (BYTE)p2, blen);
                p2 = 0;
                timer_init();
                while (p1) {
                    if ((UINT)p1 >= blen) {
                        cnt = blen;
                        p1 -= blen;
                    } else {
                        cnt = p1;
                        p1 = 0;
                    }
                    res = f_write(&file1, Buff, cnt, &s2);
                    if (res != FR_OK) {
                        put_rc(res);
                        break;
                    }
                    p2 += s2;
                    if (cnt != s2) break;
                }
                p1 = get_timer_value()/1000;
                if (p1)
                    sysprintf("%d bytes written with %d kB/sec.\n", p2, ((p2 * 100) / p1)/1024);
                break;

            case 'n' :  /* fn <old_name> <new_name> - Change file/dir name */
                while (*ptr == ' ') ptr++;
                ptr2 = strchr(ptr, ' ');
                if (!ptr2) break;
                *ptr2++ = 0;
                while (*ptr2 == ' ') ptr2++;
                put_rc(f_rename(ptr, ptr2));
                break;

            case 'u' :  /* fu <name> - Unlink a file or dir */
                while (*ptr == ' ') ptr++;
                put_rc(f_unlink(ptr));
                break;

            case 'v' :  /* fv - Truncate file */
                put_rc(f_truncate(&file1));
                break;

            case 'k' :  /* fk <name> - Create a directory */
                while (*ptr == ' ') ptr++;
                put_rc(f_mkdir(ptr));
                break;

            case 'a' :  /* fa <atrr> <mask> <name> - Change file/dir attribute */
                if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
                while (*ptr == ' ') ptr++;
                put_rc(f_chmod(ptr, p1, p2));
                break;

            case 't' :  /* ft <year> <month> <day> <hour> <min> <sec> <name> - Change timestamp */
                if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
                Finfo.fdate = (WORD)(((p1 - 1980) << 9) | ((p2 & 15) << 5) | (p3 & 31));
                if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
                Finfo.ftime = (WORD)(((p1 & 31) << 11) | ((p1 & 63) << 5) | ((p1 >> 1) & 31));
                put_rc(f_utime(ptr, &Finfo));
                break;

            case 'x' : /* fx <src_name> <dst_name> - Copy file */
                while (*ptr == ' ') ptr++;
                ptr2 = strchr(ptr, ' ');
                if (!ptr2) break;
                *ptr2++ = 0;
                while (*ptr2 == ' ') ptr2++;
                sysprintf("Opening \"%s\"", ptr);
                res = f_open(&file1, ptr, FA_OPEN_EXISTING | FA_READ);
                sysprintf("\n");
                if (res) {
                    put_rc(res);
                    break;
                }
                sysprintf("Creating \"%s\"", ptr2);
                res = f_open(&file2, ptr2, FA_CREATE_ALWAYS | FA_WRITE);
                sysPutChar('\n');
                if (res) {
                    put_rc(res);
                    f_close(&file1);
                    break;
                }
                sysprintf("Copying...");
                p1 = 0;
                for (;;) {
                    res = f_read(&file1, Buff, BUFF_SIZE, &s1);
                    if (res || s1 == 0) break;   /* error or eof */
                    res = f_write(&file2, Buff, s1, &s2);
                    p1 += s2;
                    if (res || s2 < s1) break;   /* error or disk full */
                }
                sysprintf("\n%d bytes copied.\n", p1);
                f_close(&file1);
                f_close(&file2);
                break;
#if _FS_RPATH
            case 'g' :  /* fg <path> - Change current directory */
                while (*ptr == ' ') ptr++;
                put_rc(f_chdir(ptr));
                break;

            case 'j' :  /* fj <drive#> - Change current drive */
                while (*ptr == ' ') ptr++;
				dump_buff_hex((uint8_t *)&p1, 16);
				put_rc(f_chdrive((TCHAR *)ptr));
                break;
#endif
#if _USE_MKFS
            case 'm' :  /* fm <partition rule> <sect/clust> - Create file system */
                if (!xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
                sysprintf("The memory card will be formatted. Are you sure? (Y/n)=");
                get_line(ptr, sizeof(Line));
                if (*ptr == 'Y')
                    put_rc(f_mkfs(0, (BYTE)p2, (WORD)p3));
                break;
#endif
            case 'z' :  /* fz [<rw size>] - Change R/W length for fr/fw/fx command */
                if (xatoi(&ptr, &p1) && p1 >= 1 && (size_t)p1 <= BUFF_SIZE)
                    blen = p1;
                sysprintf("blen=%d\n", blen);
                break;
            }
            break;
        case '?':       /* Show usage */
            sysprintf(
                _T("n: - Change default drive (SD drive is 0~1)\n")
                _T("dd [<lba>] - Dump sector\n")
                //_T("ds <pd#> - Show disk status\n")
                _T("\n")
                _T("bd <ofs> - Dump working buffer\n")
                _T("be <ofs> [<data>] ... - Edit working buffer\n")
                _T("br <pd#> <sect> [<num>] - Read disk into working buffer\n")
                _T("bw <pd#> <sect> [<num>] - Write working buffer into disk\n")
                _T("bf <val> - Fill working buffer\n")
                _T("\n")
                _T("fs - Show volume status\n")
                _T("fl [<path>] - Show a directory\n")
                _T("fo <mode> <file> - Open a file\n")
                _T("fc - Close the file\n")
                _T("fe <ofs> - Move fp in normal seek\n")
                //_T("fE <ofs> - Move fp in fast seek or Create link table\n")
                _T("fd <len> - Read and dump the file\n")
                _T("fr <len> - Read the file\n")
                _T("fw <len> <val> - Write to the file\n")
                _T("fn <object name> <new name> - Rename an object\n")
                _T("fu <object name> - Unlink an object\n")
                _T("fv - Truncate the file at current fp\n")
                _T("fk <dir name> - Create a directory\n")
                _T("fa <atrr> <mask> <object name> - Change object attribute\n")
                _T("ft <year> <month> <day> <hour> <min> <sec> <object name> - Change timestamp of an object\n")
                _T("fx <src file> <dst file> - Copy a file\n")
                _T("fg <path> - Change current directory\n")
                _T("fj <ld#> - Change current drive. For example: <fj 4:>\n")
                _T("fm <ld#> <rule> <cluster size> - Create file system\n")
                _T("\n")
            );
            break;
        }
    }

}



/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
