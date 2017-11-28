/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 15/06/10 2:04p $
 * @brief    This sample shows how to use USB Host Audio Class driver.
 *           The test device is a Game Audio (UAC+HID composite device).
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "usbh_lib.h"
#include "usbh_hid.h"
#include "usbh_uac.h"


#define AUDIO_IN_BUFSIZ             8192

uint8_t   au_in_buff[AUDIO_IN_BUFSIZ] __attribute__((aligned(32)));

uint32_t  g_buff_pool[1024] __attribute__((aligned(32)));


HID_DEV_T   *g_hid_list[CONFIG_HID_MAX_DEV];

static volatile int  au_in_cnt, au_out_cnt;

static uint16_t  vol_max, vol_min, vol_res, vol_cur;


void delay_us(int usec)
{
	volatile int  loop = 300 * usec;
	while (loop > 0) loop--;
}

uint32_t get_ticks(void)
{
    return sysGetTicks(TIMER0);
}

void  dump_buff_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0) {
        sysprintf("0x%04X  ", nIdx);
        for (i = 0; (i < 16) && (nBytes > 0); i++) {
            sysprintf("%02x ", pucBuff[nIdx + i]);
            nBytes--;
        }
        nIdx += 16;
        sysprintf("\n");
    }
    sysprintf("\n");
}

int  is_a_new_hid_device(HID_DEV_T *hdev)
{
    int    i;
    for (i = 0; i < CONFIG_HID_MAX_DEV; i++) {
        if ((g_hid_list[i] != NULL) && (g_hid_list[i] == hdev) &&
                (g_hid_list[i]->uid == hdev->uid))
            return 0;
    }
    return 1;
}

void update_hid_device_list(HID_DEV_T *hdev)
{
    int  i = 0;
    memset(g_hid_list, 0, sizeof(g_hid_list));
    while ((i < CONFIG_HID_MAX_DEV) && (hdev != NULL)) {
        g_hid_list[i++] = hdev;
        hdev = hdev->next;
    }
}

void  int_read_callback(HID_DEV_T *hdev, uint16_t ep_addr, int status, uint8_t *rdata, uint32_t data_len)
{
    /*
     *  USB host HID driver notify user the transfer status via <status> parameter. If the
     *  If <status> is 0, the USB transfer is fine. If <status> is not zero, this interrupt in
     *  transfer failed and HID driver will stop this pipe. It can be caused by USB transfer error
     *  or device disconnected.
     */
    if (status < 0) {
        sysprintf("Interrupt in transfer failed! status: %d\n", status);
        return;
    }
    sysprintf("Device [0x%x,0x%x] ep 0x%x, %d bytes received =>\n",
           hdev->idVendor, hdev->idProduct, ep_addr, data_len);
    dump_buff_hex(rdata, data_len);
}

int  init_hid_device(HID_DEV_T *hdev)
{
    uint8_t   *data_buff;
    int       ret;

    data_buff = (uint8_t *)((uint32_t)g_buff_pool);

    sysprintf("\n\n==================================\n");
    sysprintf("  Init HID device : 0x%x\n", (int)hdev);
    sysprintf("  VID: 0x%x, PID: 0x%x\n\n", hdev->idVendor, hdev->idProduct);

    ret = usbh_hid_get_report_descriptor(hdev, data_buff, 1024);
    if (ret > 0) {
        sysprintf("\nDump report descriptor =>\n");
        dump_buff_hex(data_buff, ret);
    }

    sysprintf("\nUSBH_HidStartIntReadPipe...\n");
    ret = usbh_hid_start_int_read(hdev, 0, int_read_callback);
    if (ret != HID_RET_OK)
        sysprintf("usbh_hid_start_int_read failed! %d\n", ret);
    else
        sysprintf("Interrupt in transfer started...\n");

    return 0;
}

/**
 *  @brief  Audio-in data callback function.
 *          UAC driver notify user that audio-in data has been moved into user audio-in buffer,
 *          which is provided by user application via UAC_InstallIsoInCbFun().
 *  @param[in] dev    Audio Class device
 *  @param[in] data   Available audio-in data, which is located in user audio-in buffer.
 *  @param[in] len    Length of available audio-in data started from <data>.
 *  @return   UAC driver does not check this return value.
 */
int audio_in_callback(UAC_DEV_T *dev, uint8_t *data, int len)
{
    au_in_cnt += len;
    //sysprintf("I %x,%x\n", (int)data & 0xffff, len);   // UART send too many will cause ISO transfer time overrun

    // Add your code here to get audio-in data ...
    // For example, memcpy(audio_record_buffer, data, len);
    // . . .

    return 0;
}


/**
 *  @brief  Audio-out data callback function.
 *          UAC driver requests user to move audio-out data into the specified address. The audio-out
 *          data will then be send to UAC device via isochronous-out pipe.
 *  @param[in] dev    Audio Class device
 *  @param[in] data   Application should move audio-out data into this buffer.
 *  @param[in] len    Maximum length of audio-out data can be moved.
 *  @return   Actual length of audio data moved.
 */
int audio_out_callback(UAC_DEV_T *dev, uint8_t *data, int len)
{
    au_out_cnt += len;
    //sysprintf("O %x,%x\n", (int)data & 0xffff, len);   // UART send too many will cause ISO transfer time overrun

    // Add your code here to put audio-out data ...
    // For example, memcpy(data, playback_buffer, actual_len);
    //              return actual_len;
    // . . .

    return 192;   // for 48000 stereo Hz
}


void  uac_control_example(UAC_DEV_T *uac_dev)
{
    uint16_t   val16;
    uint32_t   srate[4];
    uint8_t    val8;
    uint8_t    data[8];
    int        i, ret;
    uint32_t   val32;

    vol_max = vol_min = vol_res = 0;

    sysprintf("\nGet channel information ===>\n");

    /*-------------------------------------------------------------*/
    /*  Get channel number information                             */
    /*-------------------------------------------------------------*/
    ret = usbh_uac_get_channel_number(uac_dev, UAC_SPEAKER);
    if(ret < 0)
        sysprintf("    Failed to get speaker's channel number.\n");
    else
        sysprintf("    Speaker: %d\n", ret);

    ret = usbh_uac_get_channel_number(uac_dev, UAC_MICROPHONE);
    if (ret < 0)
        sysprintf("    Failed to get microphone's channel number.\n");
    else {
        sysprintf("    Microphone: %d\n", ret);
    }

    sysprintf("\nGet subframe bit resolution ===>\n");

    /*-------------------------------------------------------------*/
    /*  Get audio subframe bit resolution information              */
    /*-------------------------------------------------------------*/
    ret = usbh_uac_get_bit_resolution(uac_dev, UAC_SPEAKER, &val8);
    if(ret < 0)
        sysprintf("    Failed to get speaker's bit resoltion.\n");
    else {
        sysprintf("    Speaker audio subframe size: %d bytes\n", val8);
        sysprintf("    Speaker subframe bit resolution: %d\n", ret);
    }

    ret = usbh_uac_get_bit_resolution(uac_dev, UAC_MICROPHONE, &val8);
    if(ret < 0)
        sysprintf("    Failed to get microphone's bit resoltion.\n");
    else {
        sysprintf("    Microphone audio subframe size: %d bytes\n", val8);
        sysprintf("    Microphone subframe bit resolution: %d\n", ret);
    }

    sysprintf("\nGet sampling rate list ===>\n");

    /*-------------------------------------------------------------*/
    /*  Get audio subframe bit resolution information              */
    /*-------------------------------------------------------------*/
    ret = usbh_uac_get_sampling_rate(uac_dev, UAC_SPEAKER, (uint32_t *)&srate[0], 4, &val8);
    if(ret < 0)
        sysprintf("    Failed to get speaker's sampling rate.\n");
    else {
        if(val8 == 0)
            sysprintf("    Speaker sampling rate range: %d ~ %d Hz\n", srate[0], srate[1]);
        else {
            for(i = 0; i < val8; i++)
                sysprintf("    Speaker sampling rate: %d\n", srate[i]);
        }
    }

    ret = usbh_uac_get_sampling_rate(uac_dev, UAC_MICROPHONE, (uint32_t *)&srate[0], 4, &val8);
    if(ret < 0)
        sysprintf("    Failed to get microphone's sampling rate.\n");
    else {
        if(val8 == 0)
            sysprintf("    Microphone sampling rate range: %d ~ %d Hz\n", srate[0], srate[1]);
        else {
            for(i = 0; i < val8; i++)
                sysprintf("    Microphone sampling rate: %d\n", srate[i]);
        }
    }

    sysprintf("\nSpeaker mute control ===>\n");

    /*-------------------------------------------------------------*/
    /*  Get current mute value of UAC device's speaker.            */
    /*-------------------------------------------------------------*/
    if (usbh_uac_mute_control(uac_dev, UAC_SPEAKER, UAC_GET_CUR, UAC_CH_MASTER, data) == UAC_RET_OK) {
        sysprintf("    Speaker mute state is %d.\n", data[0]);
    } else
        sysprintf("    Failed to get speaker mute state!\n");

    sysprintf("\nSpeaker L(F) volume control ===>\n");

#if 0
    /*--------------------------------------------------------------------------*/
    /*  Get current volume value of UAC device's speaker left channel.          */
    /*--------------------------------------------------------------------------*/
    if (usbh_uac_vol_control(uac_dev, UAC_SPEAKER, UAC_GET_CUR, UAC_CH_LEFT_FRONT, &val16) == UAC_RET_OK)
        sysprintf("    Speaker L(F) volume is 0x%x.\n", val16);
    else
        sysprintf("    Failed to get seaker L(F) volume!\n");

    /*--------------------------------------------------------------------------*/
    /*  Get minimum volume value of UAC device's speaker left channel.          */
    /*--------------------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_SPEAKER, UAC_GET_MIN, UAC_CH_LEFT_FRONT, &val16) == UAC_RET_OK)
        sysprintf("    Speaker L(F) minimum volume is 0x%x.\n", val16);
    else
        sysprintf("    Failed to get speaker L(F) minimum volume!\n");

    /*--------------------------------------------------------------------------*/
    /*  Get maximum volume value of UAC device's speaker left channel.          */
    /*--------------------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_SPEAKER, UAC_GET_MAX, UAC_CH_LEFT_FRONT, &val16) == UAC_RET_OK)
        sysprintf("    Speaker L(F) maximum volume is 0x%x.\n", val16);
    else
        sysprintf("    Failed to get speaker L(F) maximum volume!\n");

    /*--------------------------------------------------------------------------*/
    /*  Get volume resolution of UAC device's speaker left channel.             */
    /*--------------------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_SPEAKER, UAC_GET_RES, UAC_CH_LEFT_FRONT, &val16) == UAC_RET_OK)
        sysprintf("    Speaker L(F) volume resolution is 0x%x.\n", val16);
    else
        sysprintf("    Failed to get speaker L(F) volume resolution!\n");

    sysprintf("\nSpeaker R(F) volume control ===>\n");

    /*--------------------------------------------------------------------------*/
    /*  Get current volume value of UAC device's speaker right channel.         */
    /*--------------------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_SPEAKER, UAC_GET_CUR, UAC_CH_RIGHT_FRONT, &val16) == UAC_RET_OK)
        sysprintf("    Speaker R(F) volume is 0x%x.\n", val16);
    else
        sysprintf("    Failed to get speaker R(F) volume!\n");

    /*--------------------------------------------------------------------------*/
    /*  Get minimum volume value of UAC device's speaker right channel.         */
    /*--------------------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_SPEAKER, UAC_GET_MIN, UAC_CH_RIGHT_FRONT, &val16) == UAC_RET_OK)
        sysprintf("    Speaker R(F) minimum volume is 0x%x.\n", val16);
    else
        sysprintf("    Failed to get speaker R(F) minimum volume!\n");

    /*--------------------------------------------------------------------------*/
    /*  Get maximum volume value of UAC device's speaker right channel.         */
    /*--------------------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_SPEAKER, UAC_GET_MAX, UAC_CH_RIGHT_FRONT, &val16) == UAC_RET_OK)
        sysprintf("    Speaker R(F) maximum volume is 0x%x.\n", val16);
    else
        sysprintf("    Failed to get speaker R(F) maximum volume!\n");

    /*--------------------------------------------------------------------------*/
    /*  Get volume resolution of UAC device's speaker right channel.            */
    /*--------------------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_SPEAKER, UAC_GET_RES, UAC_CH_RIGHT_FRONT, &val16) == UAC_RET_OK)
        sysprintf("    Speaker R(F) volume resolution is 0x%x.\n", val16);
    else
        sysprintf("    Failed to get speaker R(F) volume resolution!\n");
#endif

    sysprintf("\nSpeaker master volume control ===>\n");

    /*--------------------------------------------------------------------------*/
    /*  Get minimum volume value of UAC device's speaker master channel.        */
    /*--------------------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_SPEAKER, UAC_GET_MIN, UAC_CH_MASTER, &val16) == UAC_RET_OK)
        sysprintf("    Speaker minimum master volume is 0x%x.\n", val16);
    else
        sysprintf("    Failed to get speaker master minimum volume!\n");

    /*--------------------------------------------------------------------------*/
    /*  Get maximum volume value of UAC device's speaker master channel.        */
    /*--------------------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_SPEAKER, UAC_GET_MAX, UAC_CH_MASTER, &val16) == UAC_RET_OK)
        sysprintf("    Speaker maximum master volume is 0x%x.\n", val16);
    else
        sysprintf("    Failed to get speaker maximum master volume!\n");

    /*--------------------------------------------------------------------------*/
    /*  Get volume resolution of UAC device's speaker master channel.           */
    /*--------------------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_SPEAKER, UAC_GET_RES, UAC_CH_MASTER, &val16) == UAC_RET_OK)
        sysprintf("    Speaker master volume resolution is 0x%x.\n", val16);
    else
        sysprintf("    Failed to get speaker master volume resolution!\n");

    /*--------------------------------------------------------------------------*/
    /*  Get current volume value of UAC device's speaker master channel.        */
    /*--------------------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_SPEAKER, UAC_GET_CUR, UAC_CH_MASTER, &val16) == UAC_RET_OK)
        sysprintf("    Speaker master volume is 0x%x.\n", val16);
    else
        sysprintf("    Failed to get speaker master volume!\n");

#if 0
    sysprintf("\nMixer master volume control ===>\n");

    /*-------------------------------------------------------------*/
    /*  Get current mute value of UAC device's microphone.         */
    /*-------------------------------------------------------------*/
    sysprintf("\nMicrophone mute control ===>\n");
    if(usbh_uac_mute_control(uac_dev, UAC_MICROPHONE, UAC_GET_CUR, UAC_CH_MASTER, data) == UAC_RET_OK)
        sysprintf("    Microphone mute state is %d.\n", data[0]);
    else
        sysprintf("    Failed to get microphone mute state!\n");
#endif

    sysprintf("\nMicrophone volume control ===>\n");

    /*-------------------------------------------------------------*/
    /*  Get current volume value of UAC device's microphone.       */
    /*-------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_MICROPHONE, UAC_GET_CUR, UAC_CH_MASTER, &vol_cur) == UAC_RET_OK)
        sysprintf("    Microphone current volume is 0x%x.\n", vol_cur);
    else
        sysprintf("    Failed to get microphone current volume!\n");

    /*-------------------------------------------------------------*/
    /*  Get minimum volume value of UAC device's microphone.       */
    /*-------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_MICROPHONE, UAC_GET_MIN, UAC_CH_MASTER, &vol_min) == UAC_RET_OK)
        sysprintf("    Microphone minimum volume is 0x%x.\n", vol_min);
    else
        sysprintf("    Failed to get microphone minimum volume!\n");

    /*-------------------------------------------------------------*/
    /*  Get maximum volume value of UAC device's microphone.       */
    /*-------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_MICROPHONE, UAC_GET_MAX, UAC_CH_MASTER, &vol_max) == UAC_RET_OK)
        sysprintf("    Microphone maximum volume is 0x%x.\n", vol_max);
    else
        sysprintf("    Failed to get microphone maximum volume!\n");

    /*-------------------------------------------------------------*/
    /*  Get resolution of UAC device's microphone volume value.    */
    /*-------------------------------------------------------------*/
    if(usbh_uac_vol_control(uac_dev, UAC_MICROPHONE, UAC_GET_RES, UAC_CH_MASTER, &vol_res) == UAC_RET_OK)
        sysprintf("    Microphone volume resolution is 0x%x.\n", vol_res);
    else
        sysprintf("    Failed to get microphone volume resolution!\n");

#if 0
    /*-------------------------------------------------------------*/
    /*  Get current auto-gain setting of UAC device's microphone.  */
    /*-------------------------------------------------------------*/
    sysprintf("\nMicrophone automatic gain control ===>\n");
    if(UAC_AutoGainControl(uac_dev, UAC_MICROPHONE, UAC_GET_CUR, UAC_CH_MASTER, data) == UAC_RET_OK)
        sysprintf("    Microphone auto gain is %s.\n", data[0] ? "ON" : "OFF");
    else
        sysprintf("    Failed to get microphone auto-gain state!\n");
#endif

    sysprintf("\nSampling rate control ===>\n");

    /*-------------------------------------------------------------*/
    /*  Get current sampling rate value of UAC device's speaker.   */
    /*-------------------------------------------------------------*/
    if (usbh_uac_sampling_rate_control(uac_dev, UAC_SPEAKER, UAC_GET_CUR, &val32) == UAC_RET_OK)
        sysprintf("    Speaker's current sampling rate is %d.\n", val32);
    else
        sysprintf("    Failed to get speaker's current sampling rate!\n");

    /*-------------------------------------------------------------*/
    /*  Set new sampling rate value of UAC device's speaker.       */
    /*-------------------------------------------------------------*/
    val32 = 48000;
    if (usbh_uac_sampling_rate_control(uac_dev, UAC_SPEAKER, UAC_SET_CUR, &val32) != UAC_RET_OK)
        sysprintf("    Failed to set Speaker's current sampling rate %d.\n", val32);

    if(usbh_uac_sampling_rate_control(uac_dev, UAC_SPEAKER, UAC_GET_CUR, &val32) == UAC_RET_OK)
        sysprintf("    Speaker's current sampling rate is %d.\n", val32);
    else
        sysprintf("    Failed to get speaker's current sampling rate!\n");

    /*-------------------------------------------------------------*/
    /*  Get current sampling rate value of UAC device's microphone.*/
    /*-------------------------------------------------------------*/
    if(usbh_uac_sampling_rate_control(uac_dev, UAC_MICROPHONE, UAC_GET_CUR, &val32) == UAC_RET_OK)
        sysprintf("    Microphone's current sampling rate is %d.\n", val32);
    else
        sysprintf("    Failed to get microphone's current sampling rate!\n");

    /*-------------------------------------------------------------*/
    /*  Set new sampling rate value of UAC device's microphone.    */
    /*-------------------------------------------------------------*/
    val32 = 48000;
    if (usbh_uac_sampling_rate_control(uac_dev, UAC_MICROPHONE, UAC_SET_CUR, &val32) != UAC_RET_OK)
        sysprintf("    Failed to set microphone's current sampling rate!\n");

    if (usbh_uac_sampling_rate_control(uac_dev, UAC_MICROPHONE, UAC_GET_CUR, &val32) == UAC_RET_OK)
        sysprintf("    Microphone's current sampling rate is %d.\n", val32);
    else
        sysprintf("    Failed to get microphone's current sampling rate!\n");
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int32_t main(void)
{
    UAC_DEV_T    *uac_dev;
    HID_DEV_T    *hdev, *hdev_list;
    int          ch;
    uint16_t     val16;
	
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

	outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | 0x40000);
	outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 0x10000);

	// set PE.14 & PE.15 for USBH_PPWR0 & USBH_PPWR1
	outpw(REG_SYS_GPE_MFPH, (inpw(REG_SYS_GPE_MFPH) & ~0xff000000) | 0x77000000);

    sysprintf("\n\n");
    sysprintf("+--------------------------------------------+\n");
    sysprintf("|                                            |\n");
    sysprintf("|     USB Host UAC class sample program      |\n");
    sysprintf("|                                            |\n");
    sysprintf("+--------------------------------------------+\n");

	/*--- init timer ---*/
	sysSetTimerReferenceClock (TIMER0, 15000000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);
	
    usbh_core_init();
    usbh_uac_init();
    usbh_hid_init();
    usbh_memory_used();

    while(1) {
        if (usbh_pooling_hubs()) {            /* USB Host port detect polling and management */
            /*
             *  Has hub port event.
             */

            uac_dev = usbh_uac_get_device_list();
            if (uac_dev == NULL)
                continue;

            if (uac_dev != NULL) {                /* should be newly connected UAC device        */
                usbh_uac_open(uac_dev);

                uac_control_example(uac_dev);

                usbh_uac_start_audio_out(uac_dev, audio_out_callback);

                usbh_uac_start_audio_in(uac_dev, audio_in_callback);
            }

            hdev_list = usbh_hid_get_device_list();
            hdev = hdev_list;
            while (hdev != NULL) {
                if (is_a_new_hid_device(hdev)) {
                    init_hid_device(hdev);
                }
                hdev = hdev->next;
            }
            update_hid_device_list(hdev_list);
        }

        if (uac_dev == NULL) {
            au_in_cnt = 0;
            au_out_cnt = 0;

            if (!sysIsKbHit()) {
                ch = sysGetChar();
                usbh_memory_used();
            }

            continue;
        }

        if (!sysIsKbHit()) {
            ch = sysGetChar();

            if ((ch == '+') && (vol_cur + vol_res <= vol_max)) {
                sysprintf("+");
                val16 = vol_cur+vol_res;
                if (usbh_uac_vol_control(uac_dev, UAC_MICROPHONE, UAC_SET_CUR, UAC_CH_MASTER, &val16) == UAC_RET_OK) {
                    sysprintf("    Microphone set volume 0x%x success.\n", val16);
                    vol_cur = val16;
                } else
                    sysprintf("    Failed to set microphone volume 0x%x!\n", val16);
            } else if ((ch == '-') && (vol_cur - vol_res >= vol_min)) {
                sysprintf("-");
                val16 = vol_cur-vol_res;
                if (usbh_uac_vol_control(uac_dev, UAC_MICROPHONE, UAC_SET_CUR, UAC_CH_MASTER, &val16) == UAC_RET_OK) {
                    sysprintf("    Microphone set volume 0x%x success.\n", val16);
                    vol_cur = val16;
                } else
                    sysprintf("    Failed to set microphone volume 0x%x!\n", val16);
            } else if ((ch == '0') && (vol_cur - vol_res >= vol_min)) {
                if (usbh_uac_vol_control(uac_dev, UAC_MICROPHONE, UAC_GET_CUR, UAC_CH_MASTER, &vol_cur) == UAC_RET_OK)
                    sysprintf("    Microphone current volume is 0x%x.\n", vol_cur);
                else
                    sysprintf("    Failed to get microphone current volume!\n");
            } else {
                sysprintf("IN: %d, OUT: %d\n", au_in_cnt, au_out_cnt);
                usbh_memory_used();
            }

        }  /* end of sysIsKbHit() */
    }
}


/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
