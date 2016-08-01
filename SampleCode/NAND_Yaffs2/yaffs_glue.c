/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2007 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/******************************************************************************
 * @file     yaffs_glue.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 15/05/29 3:19p $
 * @brief    configuration for the "direct" use of yaffs
 *           This version uses the ydevconfig mechanism to set up partitions.
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <string.h>
#include "nand.h"
#include "yaffsfs.h"
#include "yaffs_packedtags2.h"
#include "yaffs_mtdif.h"
#include "yaffs_mtdif2.h"
#include "yaffs_malloc.h"
#include "yaffs_trace.h"

extern void sysprintf(char *pcStr,...);

static const char *yaffs_file_type_str(struct yaffs_stat *stat)
{
	switch (stat->st_mode & S_IFMT) {
	case S_IFREG: return "regular file";
	case S_IFDIR: return "directory";
	case S_IFLNK: return "symlink";
	default: return "unknown";
	}
}

static const char *yaffs_error_str(void)
{
	int error = yaffsfs_GetLastError();

	if (error < 0)
		error = -error;

	switch (error) {
	case EBUSY: return "Busy";
	case ENODEV: return "No such device";
	case EINVAL: return "Invalid parameter";
	case ENFILE: return "Too many open files";
	case EBADF:  return "Bad handle";
	case EACCES: return "Wrong permissions";
	case EXDEV:  return "Not on same device";
	case ENOENT: return "No such entry";
	case ENOSPC: return "Device full";
	case EROFS:  return "Read only file system";
	case ERANGE: return "Range error";
	case ENOTEMPTY: return "Not empty";
	case ENAMETOOLONG: return "Name too long";
	case ENOMEM: return "Out of memory";
	case EFAULT: return "Fault";
	case EEXIST: return "Name exists";
	case ENOTDIR: return "Not a directory";
	case EISDIR: return "Not permitted on a directory";
	case ELOOP:  return "Symlink loop";
	case 0: return "No error";
	default: return "Unknown error";
	}
}

extern nand_info_t nand_info[];

void cmd_yaffs_tracemask(unsigned set, unsigned mask)
{
	if (set)
		yaffs_trace_mask = mask;

	sysprintf("yaffs trace mask: %08x\n", yaffs_trace_mask);
}

static int yaffs_regions_overlap(int a, int b, int x, int y)
{
	return	(a <= x && x <= b) ||
		(a <= y && y <= b) ||
		(x <= a && a <= y) ||
		(x <= b && b <= y);
}

void cmd_yaffs_devconfig(char *_mp, int flash_dev,
			int start_block, int end_block)
{
	struct mtd_info *mtd = NULL;
	struct yaffs_dev *dev = NULL;
	struct yaffs_dev *chk;
	char *mp = NULL;
	struct nand_chip *chip;

// 	dev = calloc(1, sizeof(*dev));
// 	mp = (char *)strdup(_mp);
	dev = yaffs_malloc(sizeof(*dev));
    memset(dev, 0, sizeof(*dev));
    mp = yaffs_malloc(strlen(_mp));
	strcpy(mp, _mp);

	mtd = &nand_info[flash_dev];

	if (!dev || !mp) {
		/* Alloc error */
		sysprintf("Failed to allocate memory\n");
		goto err;
	}

	if (flash_dev >= 1) {
		sysprintf("Flash device invalid\n");
		goto err;
	}

	if (end_block == 0)
		end_block = mtd->size / mtd->erasesize - 1;

	if (end_block < start_block) {
		sysprintf("Bad start/end\n");
		goto err;
	}

	chip =  mtd->priv;

	/* Check for any conflicts */
	yaffs_dev_rewind();
	while (1) {
		chk = yaffs_next_dev();
		if (!chk)
			break;
		if (strcmp(chk->param.name, mp) == 0) {
			sysprintf("Mount point name already used\n");
			goto err;
		}
		if (chk->driver_context == mtd &&
			yaffs_regions_overlap(
				chk->param.start_block, chk->param.end_block,
				start_block, end_block)) {
			sysprintf("Region overlaps with partition %s\n",
				chk->param.name);
			goto err;
		}

	}

	/* Seems same, so configure */
	memset(dev, 0, sizeof(*dev));
	dev->param.name = mp;
	dev->driver_context = mtd;
	dev->param.start_block = start_block;
	dev->param.end_block = end_block;
	dev->param.chunks_per_block = mtd->erasesize / mtd->writesize;
	dev->param.total_bytes_per_chunk = mtd->writesize;
	dev->param.is_yaffs2 = 1;
	dev->param.use_nand_ecc = 1;
	dev->param.n_reserved_blocks = 5;
	if (chip->ecc.layout->oobavail <= sizeof(struct yaffs_packed_tags2))
		dev->param.inband_tags = 1;
	dev->param.n_caches = 10;
	dev->param.write_chunk_tags_fn = nandmtd2_write_chunk_tags;
	dev->param.read_chunk_tags_fn = nandmtd2_read_chunk_tags;
	dev->param.erase_fn = nandmtd_EraseBlockInNAND;
	dev->param.initialise_flash_fn = nandmtd_InitialiseNAND;
	dev->param.bad_block_fn = nandmtd2_MarkNANDBlockBad;
	dev->param.query_block_fn = nandmtd2_QueryNANDBlock;

	yaffs_add_device(dev);

	sysprintf("Configures yaffs mount %s: dev %d start block %d, end block %d %s\n",
		dev->param.name, flash_dev, start_block, end_block,
		dev->param.inband_tags ? "using inband tags" : "");
	return;

err:
	yaffs_free(dev);
	yaffs_free(mp);
}

void cmd_yaffs_dev_ls(void)
{
	struct yaffs_dev *dev;
	int flash_dev;
	int free_space;

	yaffs_dev_rewind();

	while (1) {
		dev = yaffs_next_dev();
		if (!dev)
			return;
		flash_dev =
			((unsigned) dev->driver_context - (unsigned) nand_info)/
				sizeof(nand_info[0]);
		sysprintf("%-3s %5d 0x%05x 0x%05x %s",
			dev->param.name, flash_dev,
			dev->param.start_block, dev->param.end_block,
			dev->param.inband_tags ? "using inband tags, " : "");

		free_space = yaffs_freespace(dev->param.name);
		if (free_space < 0)
			sysprintf("not mounted\n");
		else
			sysprintf("free 0x%x\n", free_space);

	}
}

void make_a_file(char *yaffsName, char bval, int sizeOfFile)
{
	int outh;
	int i;
	unsigned char buffer[100];

	outh = yaffs_open(yaffsName,
				O_CREAT | O_RDWR | O_TRUNC,
				S_IREAD | S_IWRITE);
	if (outh < 0) {
		sysprintf("Error opening file: %d. %s\n", outh, yaffs_error_str());
		return;
	}

	memset(buffer, bval, 100);

	do {
		i = sizeOfFile;
		if (i > 100)
			i = 100;
		sizeOfFile -= i;

		yaffs_write(outh, buffer, i);

	} while (sizeOfFile > 0);


	yaffs_close(outh);
}

void read_a_file(char *fn)
{
	int h;
	int i = 0;
	unsigned char b;

	h = yaffs_open(fn, O_RDWR, 0);
	if (h < 0) {
		sysprintf("File not found\n");
		return;
	}

	while (yaffs_read(h, &b, 1) > 0) {
		sysprintf("%02x ", b);
		i++;
		if (i > 32) {
			sysprintf("\n");
			i = 0;;
		}
	}
	sysprintf("\n");
	yaffs_close(h);
}

void cmd_yaffs_mount(char *mp)
{
	int retval = yaffs_mount(mp);
	if (retval < 0)
		sysprintf("Error mounting %s, return value: %d, %s\n", mp,
			yaffsfs_GetError(), yaffs_error_str());
}


void cmd_yaffs_umount(char *mp)
{
	if (yaffs_unmount(mp) == -1)
		sysprintf("Error umounting %s, return value: %d, %s\n", mp,
			yaffsfs_GetError(), yaffs_error_str());
}

void cmd_yaffs_write_file(char *yaffsName, char bval, int sizeOfFile)
{
	make_a_file(yaffsName, bval, sizeOfFile);
}


void cmd_yaffs_read_file(char *fn)
{
	read_a_file(fn);
}


void cmd_yaffs_mread_file(char *fn, char *addr)
{
	int h;
	struct yaffs_stat s;

	yaffs_stat(fn, &s);

	sysprintf("Copy %s to 0x%p... ", fn, addr);
	h = yaffs_open(fn, O_RDWR, 0);
	if (h < 0) {
		sysprintf("File not found\n");
		return;
	}

	yaffs_read(h, addr, (int)s.st_size);
	sysprintf("\t[DONE]\n");

	yaffs_close(h);
}


void cmd_yaffs_mwrite_file(char *fn, char *addr, int size)
{
	int outh;

	outh = yaffs_open(fn, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	if (outh < 0)
		sysprintf("Error opening file: %d, %s\n", outh, yaffs_error_str());

	yaffs_write(outh, addr, size);

	yaffs_close(outh);
}


void cmd_yaffs_ls(const char *mountpt, int longlist)
{
	int i;
	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat stat;
	char tempstr[255];

	d = yaffs_opendir(mountpt);

	if (!d) {
		sysprintf("opendir failed, %s\n", yaffs_error_str());
		return;
	}

	for (i = 0; (de = yaffs_readdir(d)) != NULL; i++) {
		if (longlist) {
			sprintf(tempstr, "%s/%s", mountpt, de->d_name);
			yaffs_lstat(tempstr, &stat);
			sysprintf("%-25s\t%7ld",
					de->d_name,
					(long)stat.st_size);
			sysprintf(" %5d %s\n",
					stat.st_ino,
					yaffs_file_type_str(&stat));
		} else {
			sysprintf("%s\n", de->d_name);
		}
	}

	yaffs_closedir(d);
}


void cmd_yaffs_mkdir(const char *dir)
{
	int retval = yaffs_mkdir(dir, 0);

	if (retval < 0)
		sysprintf("yaffs_mkdir returning error: %d, %s\n",
			retval, yaffs_error_str());
}

void cmd_yaffs_rmdir(const char *dir)
{
	int retval = yaffs_rmdir(dir);

	if (retval < 0)
		sysprintf("yaffs_rmdir returning error: %d, %s\n",
			retval, yaffs_error_str());
}

void cmd_yaffs_rm(const char *path)
{
	int retval = yaffs_unlink(path);

	if (retval < 0)
		sysprintf("yaffs_unlink returning error: %d, %s\n",
			retval, yaffs_error_str());
}

void cmd_yaffs_mv(const char *oldPath, const char *newPath)
{
	int retval = yaffs_rename(newPath, oldPath);

	if (retval < 0)
		sysprintf("yaffs_unlink returning error: %d, %s\n",
			retval, yaffs_error_str());
}
