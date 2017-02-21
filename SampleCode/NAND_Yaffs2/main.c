/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 15/05/28 5:19p $
 * @brief    Access a NAND flash formatted in YAFFS2 file system
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "yaffs_glue.h"

extern void nand_init(void);

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

void SYS_Init(void)
{
    /* enable FMI NAND */
    outpw(REG_CLK_HCLKEN, (inpw(REG_CLK_HCLKEN) | 0x300000));

    /* select NAND function pins */
    if (inpw(REG_SYS_PWRON) & 0x08000000)
    {
        /* Set GPI1~15 for NAND */
        outpw(REG_SYS_GPI_MFPL, 0x55555550);
        outpw(REG_SYS_GPI_MFPH, 0x55555555);
    }
    else
    {
        /* Set GPC0~14 for NAND */
        outpw(REG_SYS_GPC_MFPL, 0x55555555);
        outpw(REG_SYS_GPC_MFPH, 0x05555555);
    }
}

static char CommandLine[256];

/*----------------------------------------------*/
/* Get a line from the input                    */
/*----------------------------------------------*/
void get_line (char *buff, int len)
{
    char c;
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



int main(void)
{
    char *ptr;
    char mtpoint[] = "user";
    char buf[8];
    int volatile i;

	sysInitializeUART();
    sysprintf("\n");
    sysprintf("==========================================\n");
    sysprintf("          FMI NAND YAFFS2                 \n");
    sysprintf("==========================================\n");

	sysDisableCache();
	sysInvalidCache();
	sysSetMMUMappingMethod(MMU_DIRECT_MAPPING);
	sysEnableCache(CACHE_WRITE_BACK);

    SYS_Init();
    nand_init();
    cmd_yaffs_devconfig(mtpoint, 0, 0xb0, 0x3ff);
    cmd_yaffs_dev_ls();
	cmd_yaffs_mount(mtpoint);
    cmd_yaffs_dev_ls();
    sysprintf("\n");

    for (;;) {

        sysprintf(">");
        ptr = CommandLine;
        get_line(ptr, sizeof(CommandLine));
        switch (*ptr++) {

        case 'q' :  /* Exit program */
          	cmd_yaffs_umount(mtpoint);
            sysprintf("Program terminated!\n");
            return 0;

        case 'l' :  /* ls */
            if (*ptr++ == 's') {
                while (*ptr == ' ') ptr++;
                cmd_yaffs_ls(ptr, 1);
            }
            break;

        case 'w' :  /* wr */
            if (*ptr++ == 'r') {
                while (*ptr == ' ') ptr++;
                cmd_yaffs_write_file(ptr, 0x55, 10);    /* write 0x55 into file 10 times */
            }
            break;

        case 'r' :
            if (*ptr == 'd') {  /* rd */
                ptr++;
                while (*ptr == ' ') ptr++;
                sysprintf("Reading file %s ...\n\n", ptr);
                cmd_yaffs_read_file(ptr);
                sysprintf("\ndone.\n");
            }
            else if (*ptr == 'm') {  /* rm */
                ptr++;
                if (*ptr == 'd')
                {
                    i = 0;
                    while(*ptr != ' ')
                        buf[i++] = *ptr++;
                    ptr++;
                    sysprintf("Remove dir %s ...\n\n", ptr);
                    cmd_yaffs_rmdir(ptr);
                }
                else
                {
                    while (*ptr == ' ') ptr++;
                    sysprintf("Remove file %s ...\n\n", ptr);
                    cmd_yaffs_rm(ptr);
                }
            }
            break;

        case 'm' :  /* mkdir */
            i = 0;
            while(*ptr != ' ')
                buf[i++] = *ptr++;
            ptr++;

            if (!strcmp(buf, "kdir"))
                cmd_yaffs_mkdir(ptr);
            break;

        case '?':       /* Show usage */
            sysprintf("ls    <path>     - Show a directory. ex: ls user/test ('user' is mount point).\n");
            sysprintf("rd    <file name> - Read a file. ex: rd user/test.bin ('user' is mount point).\n");
            sysprintf("wr    <file name> - Write a file. ex: wr user/test.bin ('user' is mount point).\n");
            sysprintf("rm    <file name> - Delete a file. ex: rm user/test.bin ('user' is mount point).\n");
            sysprintf("mkdir <dir name> - Create a directory. ex: mkdir user/test ('user' is mount point).\n");
            sysprintf("rmdir <dir name> - Create a directory. ex: mkdir user/test ('user' is mount point).\n");
            sysprintf("\n");
        }
    }
}



/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
