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

/*
 * yaffscfg.c  The configuration for the "direct" use of yaffs.
 *
 * This is set up for u-boot.
 *
 * This version now uses the ydevconfig mechanism to set up partitions.
 */

#include "malloc.h"

extern void sysprintf(char *pcStr,...);
unsigned yaffs_trace_mask = 0; /* Disable logging */

/***************************************/

#define YAFFS_MEM_ALLOC_MAGIC     0x41090908    /* magic number in leading block */
#define YAFFS_MEMORY_POOL_SIZE   (4*1024*1024)
#define YAFFS_MEM_BLOCK_SIZE      512

typedef struct YAFFS_mhdr
{
	unsigned int  flag;  /* 0:free, 1:allocated, 0x3:first block */
	unsigned int  bcnt;  /* if allocated, the block count of allocated memory block */
	unsigned int  magic;
	unsigned int  reserved;
}  YAFFS_MHDR_T;

__align(YAFFS_MEM_BLOCK_SIZE) unsigned char  _YAFFSMemoryPool[YAFFS_MEMORY_POOL_SIZE];

static YAFFS_MHDR_T  *_pCurrent;
static unsigned int  _FreeMemorySize;
unsigned int  _AllocatedMemorySize;   
unsigned int  *_YAFFS_pCurrent = (unsigned int *)&_pCurrent; 

static unsigned int  _MemoryPoolBase, _MemoryPoolEnd;  

void  YAFFS_InitializeMemoryPool(void)
{
	_MemoryPoolBase = (unsigned int)&_YAFFSMemoryPool[0] | 0x80000000;

	_MemoryPoolEnd = _MemoryPoolBase + YAFFS_MEMORY_POOL_SIZE;
	_FreeMemorySize = _MemoryPoolEnd - _MemoryPoolBase;
	_AllocatedMemorySize = 0;
	_pCurrent = (YAFFS_MHDR_T *)_MemoryPoolBase;
	memset((char *)_MemoryPoolBase, 0, _FreeMemorySize);
}

/***************************************/
void *yaffs_malloc(size_t size)
{
// 	return malloc(size);
	YAFFS_MHDR_T  *pPrimitivePos = _pCurrent;
	YAFFS_MHDR_T  *pFound;
	int   found_size=-1;
	int   i, block_count;
	int   wrap = 0;

	if (size >= _FreeMemorySize)
	{
		sysprintf("yaffs_malloc - want=%d, free=%d\n", size, _FreeMemorySize);
		return NULL;
	}

	if ((unsigned int)_pCurrent >= _MemoryPoolEnd)
	   _pCurrent = (YAFFS_MHDR_T *)_MemoryPoolBase;   /* wrapped */

	do 
	{
		if (_pCurrent->flag)          /* is not a free block */
		{
			if (_pCurrent->magic != YAFFS_MEM_ALLOC_MAGIC)
			{
				sysprintf("\nyaffs_malloc - incorrect magic number! C:%x F:%x, wanted:%d, Base:0x%x, End:0x%x\n", 
                        (unsigned int)_pCurrent, _FreeMemorySize, size, (unsigned int)_MemoryPoolBase, (unsigned int)_MemoryPoolEnd);
				return NULL;
			}

			if (_pCurrent->flag == 0x3)
				_pCurrent = (YAFFS_MHDR_T *)((unsigned int)_pCurrent + _pCurrent->bcnt * YAFFS_MEM_BLOCK_SIZE);
			else
			{
				sysprintf("USB_malloc warning - not the first block!\n");
				_pCurrent = (YAFFS_MHDR_T *)((unsigned int)_pCurrent + YAFFS_MEM_BLOCK_SIZE);
			}
			
			if ((unsigned int)_pCurrent > _MemoryPoolEnd)
				sysprintf("yaffs_malloc - exceed limit!!\n");

			if ((unsigned int)_pCurrent == _MemoryPoolEnd)
			{
				sysprintf("yaffs_malloc - warp!!\n");
				wrap = 1;
				_pCurrent = (YAFFS_MHDR_T *)_MemoryPoolBase;   /* wrapped */
			}
			
			found_size = -1;          /* reset the accumlator */
		}
		else                         /* is a free block */
		{
			if (found_size == -1)     /* the leading block */
			{
				pFound = _pCurrent;
				block_count = 1;
			   
				found_size = YAFFS_MEM_BLOCK_SIZE - sizeof(YAFFS_MHDR_T);
			}
			else                      /* not the leading block */
			{
				found_size += YAFFS_MEM_BLOCK_SIZE;
				block_count++;
			}
			   
			if (found_size >= size)
			{
				pFound->bcnt = block_count;
				pFound->magic = YAFFS_MEM_ALLOC_MAGIC;
				_FreeMemorySize -= block_count * YAFFS_MEM_BLOCK_SIZE;
				_AllocatedMemorySize += block_count * YAFFS_MEM_BLOCK_SIZE;
				_pCurrent = pFound;
				for (i=0; i<block_count; i++)
				{
					_pCurrent->flag = 1;     /* allocate block */
					_pCurrent = (YAFFS_MHDR_T *)((unsigned int)_pCurrent + YAFFS_MEM_BLOCK_SIZE);
				} 
				pFound->flag = 0x3;
				
				return (void *)((unsigned int)pFound + sizeof(YAFFS_MHDR_T));
			}
			 
			/* advance to the next block */
			_pCurrent = (YAFFS_MHDR_T *)((unsigned int)_pCurrent + YAFFS_MEM_BLOCK_SIZE);
			if ((unsigned int)_pCurrent >= _MemoryPoolEnd)
			{
				wrap = 1;
				_pCurrent = (YAFFS_MHDR_T *)_MemoryPoolBase;   /* wrapped */
				found_size = -1;     /* reset accumlator */
			}
		}
	} while ((wrap == 0) || (_pCurrent < pPrimitivePos));
	   
	sysprintf("yaffs_malloc - No free memory!\n");
	return NULL;
}

void yaffs_free(void *ptr)
{
// 	free(ptr);
	YAFFS_MHDR_T  *pMblk;
	unsigned int  addr = (unsigned int)ptr;
	int     i, count;

	if ((addr < _MemoryPoolBase) || (addr >= _MemoryPoolEnd))
	{
		if (addr)
			free(ptr);
		return;
	}

	/* get the leading block address */
	if (addr % YAFFS_MEM_BLOCK_SIZE == 0)
		addr -= YAFFS_MEM_BLOCK_SIZE;
	else
		addr -= sizeof(YAFFS_MHDR_T);
		
	if (addr % YAFFS_MEM_BLOCK_SIZE != 0)
	{
		sysprintf("yaffs_free fatal error on address: %x!!\n", (unsigned int)ptr);
		return;
	}
	
	pMblk = (YAFFS_MHDR_T *)addr;
	if (pMblk->flag == 0)
	{
		sysprintf("yaffs_free(), warning - try to free a free block: %x\n", (unsigned int)ptr);
		return;
	}
	if (pMblk->magic != YAFFS_MEM_ALLOC_MAGIC)
	{
		sysprintf("yaffs_free(), warning - try to free an unknow block at address:%x.\n", addr);
		return;
	}

	count = pMblk->bcnt;
	for (i = 0; i < count; i++)
	{
		pMblk->flag = 0;     /* release block */
		pMblk = (YAFFS_MHDR_T *)((unsigned int)pMblk + YAFFS_MEM_BLOCK_SIZE);
	}

	_FreeMemorySize += count * YAFFS_MEM_BLOCK_SIZE;
	_AllocatedMemorySize -= count * YAFFS_MEM_BLOCK_SIZE;
	return;
}

/*======================================================================*/
static int yaffs_errno;

void yaffs_bug_fn(const char *fn, int n)
{
	sysprintf("yaffs bug at %s:%d\n", fn, n);
}

void yaffsfs_SetError(int err)
{
	yaffs_errno = err;
}

int yaffsfs_GetLastError(void)
{
	return yaffs_errno;
}


int yaffsfs_GetError(void)
{
	return yaffs_errno;
}

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

__u32 yaffsfs_CurrentTime(void)
{
	return 0;
}


void yaffsfs_LocalInitialisation(void)
{
	/* No locking used */
}




