/*
 * (C) Copyright 2005
 * 2N Telekomunikace, a.s. <www.2n.cz>
 * Ladislav Michl <michl@2n.cz>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "common.h"
#include "nand.h"
#include "errno.h"
#include "yaffs_malloc.h"

extern void sysprintf(char* pcStr,...);

#ifndef CONFIG_SYS_NAND_BASE_LIST
#define CONFIG_SYS_NAND_BASE_LIST { CONFIG_SYS_NAND_BASE }
#endif

#define CONFIG_SYS_MAX_NAND_DEVICE  1

int nand_curr_device = -1;

nand_info_t nand_info[CONFIG_SYS_MAX_NAND_DEVICE];

static struct nand_chip nand_chip[CONFIG_SYS_MAX_NAND_DEVICE];

static char dev_name[CONFIG_SYS_MAX_NAND_DEVICE][8];

static unsigned long total_nand_size; /* in kiB */

/* Register an initialized NAND mtd device with the U-Boot NAND command. */
int nand_register(int devnum)
{
	struct mtd_info *mtd;

	if (devnum >= CONFIG_SYS_MAX_NAND_DEVICE)
		return -EINVAL;

	mtd = &nand_info[devnum];

	sprintf(dev_name[devnum], "nand%d", devnum);
	mtd->name = dev_name[devnum];

	/*
	 * Add MTD device so that we can reference it later
	 * via the mtdcore infrastructure (e.g. ubi).
	 */
	add_mtd_device(mtd);

	total_nand_size += mtd->size / 1024;

	if (nand_curr_device == -1)
		nand_curr_device = devnum;

    sysprintf("%s\n", mtd->name);

	return 0;
}

static void nand_init_chip(int i)
{
	struct mtd_info *mtd = &nand_info[i];
	struct nand_chip *nand = &nand_chip[i];
	int maxchips = CONFIG_SYS_NAND_MAX_CHIPS;

	if (maxchips < 1)
		maxchips = 1;

	mtd->priv = nand;
	nand->IO_ADDR_R = nand->IO_ADDR_W = (void  __iomem *)0xB000D8B8;

	if (board_nand_init(nand))
		return;

	if (nand_scan(mtd, maxchips))
		return;

	nand_register(i);
}

void nand_init(void)
{
	int i;

    YAFFS_InitializeMemoryPool();
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_init_chip(i);

	sysprintf("%lu MiB\n", total_nand_size / 1024);

}
