/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/05/11 10:06a $
 * @brief    Simulate an USB mouse and draws circle on the screen
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NUC970.h"
#include "sys.h"
#include "usbd.h"
#include "hid_mousekeyboard.h"

extern void USBD_IRQHandler(void);

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
	sysInitializeUART();
    sysprintf("\n");
    sysprintf("========================================\n");
    sysprintf("     NUC970 USB HID Mouse/ Keyboard     \n");
    sysprintf("========================================\n");

	sysDisableCache();
	sysInvalidCache();
	sysSetMMUMappingMethod(MMU_DIRECT_MAPPING);
	sysEnableCache(CACHE_WRITE_BACK);

    sysInstallISR(HIGH_LEVEL_SENSITIVE|IRQ_LEVEL_1, USBD_IRQn, (PVOID)USBD_IRQHandler);
    /* enable CPSR I bit */
    sysSetLocalInterrupt(ENABLE_IRQ);

    USBD_Open(&gsInfo, HID_ClassRequest, NULL);
    USBD_SetVendorRequest(HID_VendorRequest);

    /* Endpoint configuration */
    HID_Init();

	sysEnableInterrupt(USBD_IRQn);
    /* Start transaction */
    USBD_Start();

    while(1) {
        HID_UpdateMouseData();
        HID_UpdateKeyboardData();
    }
}



/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/

