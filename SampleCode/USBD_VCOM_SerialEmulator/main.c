/******************************************************************************
 * @file     main.c
 * @brief    Demonstrate how to implement a USB virtual com port device.
 * @version  V1.00
 * $Date: 15/05/11 10:06a $
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "nuc970.h"
#include "sys.h"
#include "usbd.h"
#include "vcom_serial.h"

extern void USBD_IRQHandler(void);

/*--------------------------------------------------------------------------*/
STR_VCOM_LINE_CODING gLineCoding = {115200, 0, 0, 8};   /* Baud rate : 115200    */
/* Stop bit     */
/* parity       */
/* data bits    */
uint16_t gCtrlSignal = 0;     /* BIT0: DTR(Data Terminal Ready) , BIT1: RTS(Request To Send) */

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
/* UART0 */
#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t gUsbRxBuf[64] = {0};
#else
uint8_t gUsbRxBuf[64] __attribute__((aligned(4))) = {0};
#endif

uint32_t gu32RxSize = 0;
uint32_t gu32TxSize = 0;

volatile int8_t gi8BulkInReady = 0;
volatile int8_t gi8BulkOutReady = 0;

void VCOM_TransferData(void)
{
    int32_t i;

    /* Process the Bulk out data when bulk out data is ready. */
    if (gi8BulkOutReady) {
        for (i=0; i<gu32RxSize; i++)
            USBD->EP[EPA].ep.EPDAT_BYTE = gUsbRxBuf[i];
        gi8BulkOutReady = 0; /* Clear bulk out ready flag */
        USBD->EP[EPA].EPRSPCTL = USB_EP_RSPCTL_SHORTTXEN;    // packet end
        USBD->EP[EPA].EPTXCNT = gu32RxSize;
        USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_INTKIEN_Msk);
        while(1) {
            if (gi8BulkInReady) {
                gi8BulkInReady = 0;
                break;
            }
        }
    }
}


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
	sysInitializeUART();
    sysprintf("\n");
    sysprintf("=========================\n");
    sysprintf("     NUC970 USB VCOM     \n");
    sysprintf("=========================\n");

	sysDisableCache();
	sysInvalidCache();
	sysSetMMUMappingMethod(MMU_DIRECT_MAPPING);
	sysEnableCache(CACHE_WRITE_BACK);

    sysInstallISR(HIGH_LEVEL_SENSITIVE|IRQ_LEVEL_1, USBD_IRQn, (PVOID)USBD_IRQHandler);
    /* enable CPSR I bit */
    sysSetLocalInterrupt(ENABLE_IRQ);

    USBD_Open(&gsInfo, VCOM_ClassRequest, NULL);

    /* Endpoint configuration */
    VCOM_Init();

	sysEnableInterrupt(USBD_IRQn);

    /* Start transaction */
    while(1) {
        if (USBD_IS_ATTACHED()) {
            USBD_Start();
            break;
        }
    }

    while(1) {
        VCOM_TransferData();
    }
}



/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/

