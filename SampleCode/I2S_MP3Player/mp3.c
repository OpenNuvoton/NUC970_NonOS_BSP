/*
 * libmad - MPEG audio decoder library
 * Copyright (C) 2000-2004 Underbit Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: minimad.c,v 1.4 2004/01/23 09:41:32 rob Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"

#include "config.h"
#include "diskio.h"
#include "ff.h"
#include "mad.h"
#include "i2s.h"

#define MP3_FILE    "0:\\test.mp3"

#if defined _FS_RPATH
    #undef _FS_RPATH
    #define _FS_RPATH 0
#endif

/*
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */


struct mad_stream   Stream;
struct mad_frame    Frame;
struct mad_synth    Synth;

FIL             mp3FileObject;
FILINFO         Finfo;
size_t          ReadSize;
size_t          Remaining;
size_t          ReturnSize;
FSIZE_t         id3v2Size = 0;
FSIZE_t         id3v1Size = 128;

// I2S PCM buffer x2
signed int aPCMBuffer[2][PCM_BUFFER_SIZE];
// File IO buffer for MP3 library
unsigned char MadInputBuffer[FILE_IO_BUFFER_SIZE+MAD_BUFFER_GUARD];
// buffer full flag x2
volatile uint8_t aPCMBuffer_Full[2]= {0,0};
// audio information structure
struct AudioInfoObject audioInfo;
extern volatile uint8_t u8PCMBuffer_Playing;

/**
 * MP3 frame can be attached with either ID3v1 or v2, or both
 * [ID3v2][Frame][Frame]...[Frame][ID3v1]
 * ID3v2 : ['ID3' + ...], total 10 bytes header + tag frame(M bytes)
 * ID3v1 : ['TAG' + ...], total 128 bytes
 * Frame : [4 bytes header + body(N bytes)]
 */
FRESULT MP3_id3v2_offload(void)
{
    FRESULT res;

    f_lseek(&mp3FileObject, 0);
    res = f_read(&mp3FileObject, (char *)(&MadInputBuffer[0]), 10, &ReturnSize);

    if (res == FR_OK)
    {
        // Check header exist
        if ((ReturnSize >= 10) && !memcmp(MadInputBuffer, "ID3", 3) && (!(MadInputBuffer[5] & 15) ||
            (MadInputBuffer[6] & 0x80) || (MadInputBuffer[7] & 0x80) || (MadInputBuffer[8] & 0x80) || (MadInputBuffer[9] & 0x80)))
        {
            id3v2Size = (((MadInputBuffer[6] & 0x7f) << 21) | ((MadInputBuffer[7] & 0x7f) << 14) |
                            ((MadInputBuffer[8] & 0x7f) << 7) | (MadInputBuffer[9] & 0x7f)) + 10;
            f_lseek(&mp3FileObject, id3v2Size);
        }
    }

    return res;
}

FSIZE_t MP3_id3v1_offload(FSIZE_t filesize)
{
    FRESULT res;

    f_lseek(&mp3FileObject, audioInfo.playFileSize - id3v1Size);
    res = f_read(&mp3FileObject, (char *)(&MadInputBuffer[0]), id3v1Size, &ReturnSize);

    if(res == FR_OK)
    {
        // Check header exist
        if(ReturnSize >= id3v1Size && !memcmp(MadInputBuffer, "TAG", 3))
            filesize -= id3v1Size;
        else
            id3v1Size = 0;
    }
    else
        filesize = 0;

    return filesize;
}

// Parse MP3 header and get some informations
void MP3_ParseHeaderInfo(uint8_t *pFileName)
{
    FRESULT res;
    uint32_t fptr = 0;

    res = f_open(&mp3FileObject, (void *)pFileName, FA_OPEN_EXISTING | FA_READ);
    if (res == FR_OK)
    {
        sysprintf("file is opened!!\r\n");
        f_stat((void *)pFileName, &Finfo);
        audioInfo.playFileSize = Finfo.fsize;

        MP3_id3v2_offload();

        while(1)
        {
            res = f_read(&mp3FileObject, (char *)(&MadInputBuffer[0]), FILE_IO_BUFFER_SIZE, &ReturnSize);

            //parsing MP3 header
            mp3CountV1L3Headers((unsigned char *)(&MadInputBuffer[0]), ReturnSize);
            if (audioInfo.mp3SampleRate != 0)
                // Got the header and sampling rate
                break;

            // ID3 may too long, try to parse following data
            // but only forward file point to half of buffer to prevent the header is
            // just right at the boundry of buffer
            fptr += FILE_IO_BUFFER_SIZE/2;
            if (fptr >= audioInfo.playFileSize)
                // Fail to find header
                break;

            f_lseek(&mp3FileObject, fptr);
        }
    }
    else
    {
        //sysprintf("Open File Error\r\n");
        return;
    }
    MP3_id3v1_offload(audioInfo.playFileSize);

    f_close(&mp3FileObject);

    sysprintf("====[MP3 Info]======\r\n");
    sysprintf("FileSize = %d\r\n", audioInfo.playFileSize);
    sysprintf("SampleRate = %d\r\n", audioInfo.mp3SampleRate);
    sysprintf("BitRate = %d\r\n", audioInfo.mp3BitRate);
    sysprintf("Channel = %d\r\n", audioInfo.mp3Channel);
    sysprintf("PlayTime = %d\r\n", audioInfo.mp3PlayTime);
    sysprintf("=====================\r\n");
}

// Enable I2S TX with PDMA function
void StartPlay(void)
{
    sysprintf("Start playing ... \n");
    // Set DMA buffer address
    i2sIoctl(I2S_SET_DMA_ADDRESS, I2S_PLAY, (uint32_t)&aPCMBuffer[0][0]);
    // Set DMA buffer length (byte count)
    i2sIoctl(I2S_SET_DMA_LENGTH, I2S_PLAY, PCM_BUFFER_SIZE*8);

    i2sIoctl(I2S_SET_PLAY, I2S_START_PLAY, 0);

    // enable sound output
    //PI3 = 0;
    audioInfo.mp3Playing = 1;
}

// Disable I2S TX with PDMA function
void StopPlay(void)
{
    i2sIoctl(I2S_SET_PLAY, I2S_STOP_PLAY, 0);

    audioInfo.mp3Playing = 0;
    sysprintf("Stop ...\n");
}

// MP3 decode player
void MP3Player(void)
{
    FRESULT res;
    uint8_t *ReadStart;
    uint8_t *GuardPtr;
    volatile uint8_t u8PCMBufferTargetIdx = 0;
    volatile uint32_t pcmbuf_idx, i;
    volatile unsigned int Mp3FileOffset=0;
    uint16_t sampleL, sampleR;

    pcmbuf_idx = 0;
    u8PCMBuffer_Playing = 0;
    memset((void *)&audioInfo, 0, sizeof(audioInfo));
    memset((void *)MadInputBuffer, 0, sizeof(MadInputBuffer));
    memset((void *)aPCMBuffer, 0, sizeof(aPCMBuffer));
    memset((void *)aPCMBuffer_Full, 0, sizeof(aPCMBuffer_Full));

    /* Parse MP3 header */
    MP3_ParseHeaderInfo((uint8_t *)MP3_FILE);

    /* First the structures used by libmad must be initialized. */
    mad_stream_init(&Stream);
    mad_frame_init(&Frame);
    mad_synth_init(&Synth);

    /* Open MP3 file */
    res = f_open(&mp3FileObject, MP3_FILE, FA_OPEN_EXISTING | FA_READ);
    if (res != FR_OK)
    {
        //sysprintf("Open file error \r\n");
        return;
    }

    mp3FileObject.fsize -= id3v1Size;
    f_lseek(&mp3FileObject, id3v2Size);

    /* Configure to specific sample rate */
    i2sConfigSampleRate(audioInfo.mp3SampleRate);

    while(1)
    {
        if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
        {
            if(Stream.next_frame != NULL)
            {
                /* Get the remaining frame */
                Remaining = Stream.bufend - Stream.next_frame;
                memmove(MadInputBuffer, Stream.next_frame, Remaining);
                ReadStart = MadInputBuffer + Remaining;
                ReadSize = FILE_IO_BUFFER_SIZE - Remaining;
            }
            else
            {
                ReadSize = FILE_IO_BUFFER_SIZE,
                ReadStart = MadInputBuffer,
                Remaining = 0;
            }

            /* read the file */
            res = f_read(&mp3FileObject, ReadStart, ReadSize, &ReturnSize);
            if(res != FR_OK)
            {
                sysprintf("Stop !(%x)\n\r", res);
                goto stop;
            }

            if(f_eof(&mp3FileObject))
            {
                if(ReturnSize == 0)
                    goto stop;
            }

            /* if the file is over */
            if (ReadSize > ReturnSize)
            {
                GuardPtr=ReadStart+ReadSize;
                memset(GuardPtr,0,MAD_BUFFER_GUARD);
                ReadSize+=MAD_BUFFER_GUARD;
            }

            Mp3FileOffset = Mp3FileOffset + ReturnSize;
            /* Pipe the new buffer content to libmad's stream decoder
             * facility.
            */
            mad_stream_buffer(&Stream,MadInputBuffer,ReadSize+Remaining);
            Stream.error=(enum  mad_error)0;
        }

        /* decode a frame from the mp3 stream data */
        if(mad_frame_decode(&Frame,&Stream))
        {
            if(MAD_RECOVERABLE(Stream.error))
            {
                /*if(Stream.error!=MAD_ERROR_LOSTSYNC ||
                   Stream.this_frame!=GuardPtr)
                {
                }*/
                continue;
            }
            else
            {
                /* the current frame is not full, need to read the remaining part */
                if(Stream.error==MAD_ERROR_BUFLEN)
                {
                    continue;
                }
                else
                {
                    sysprintf("Something error!!\n");

                    /* play the next file */
                    audioInfo.mp3FileEndFlag = 1;
                    goto stop;
                }
            }
        }

        /* Once decoded the frame is synthesized to PCM samples. No errors
        * are reported by mad_synth_frame();
        */
        mad_synth_frame(&Synth,&Frame);

        //
        // decode finished, try to copy pcm data to audio buffer
        //

        if(audioInfo.mp3Playing)
        {
            //if next buffer is still full (playing), wait until it's empty
            if(aPCMBuffer_Full[u8PCMBufferTargetIdx] == 1)
                while(aPCMBuffer_Full[u8PCMBufferTargetIdx]);
        }
        else
        {
            if((aPCMBuffer_Full[0] == 1) && (aPCMBuffer_Full[1] == 1))         //all buffers are full, wait
            {
                StartPlay();
            }
        }

        for(i=0; i<(int)Synth.pcm.length; i++)
        {
            /* Get the left/right samples */
            sampleL = Synth.pcm.samples[0][i];
            sampleR = Synth.pcm.samples[1][i];

            /* Fill PCM data to I2S buffer */
            aPCMBuffer[u8PCMBufferTargetIdx][pcmbuf_idx++] = sampleR | (sampleL << 16);

            /* Need change buffer ? */
            if(pcmbuf_idx == PCM_BUFFER_SIZE)
            {
                aPCMBuffer_Full[u8PCMBufferTargetIdx] = 1;      //set full flag
                u8PCMBufferTargetIdx ^= 1;

                pcmbuf_idx = 0;
                //sysprintf("change to ==>%d ..\n", u8PCMBufferTargetIdx);
                /* if next buffer is still full (playing), wait until it's empty */
                if((aPCMBuffer_Full[u8PCMBufferTargetIdx] == 1) && (audioInfo.mp3Playing))
                    while(aPCMBuffer_Full[u8PCMBufferTargetIdx]);
            }
        }
    }

stop:

    sysprintf("Exit MP3\r\n");

    mad_synth_finish(&Synth);
    mad_frame_finish(&Frame);
    mad_stream_finish(&Stream);

    f_close(&mp3FileObject);
    StopPlay();
}

