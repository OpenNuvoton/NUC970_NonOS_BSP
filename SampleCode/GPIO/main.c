/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/04/30 4:30p $
 * @brief    NUC970 Driver Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "nuc970.h"
#include "sys.h"
#include "gpio.h"

/*-----------------------------------------------------------------------------*/
volatile int eint_complete=0;
INT32 EINT0Callback(UINT32 status, UINT32 userData)
{
    /* To do */
    eint_complete=1;

    /********/
    GPIO_ClrISRBit(GPIOF,BIT11);
    return 0;
}

volatile int gpio_complete=0;
INT32 GPIOECallback(UINT32 status, UINT32 userData)
{
    /* To do */
    if(status & BIT3)
      gpio_complete=1;
    /********/
    GPIO_ClrISRBit(GPIOE,status);
    return 0;
}

int main(void)
{
    int32_t i32Err;

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    sysprintf("+-------------------------------------------------+\n");
    sysprintf("|                 GPIO Sample Code                |\n");
    sysprintf("+-------------------------------------------------+\n\n");

    /* Configure Port C to input mode and pull-up */
    GPIO_Open(GPIOC, DIR_INPUT, PULL_UP);
   
    /* Set Port C output data to 0xFFF */
    GPIO_Set(GPIOC, 0xFFF);

    /* Set Port C output data to 0x000 */
    GPIO_Clr(GPIOC, 0xFFF);

    /* Configure Port C to default value */
    GPIO_Close(GPIOC);

    i32Err = 0;
    sysprintf("GPIO PD.3(output mode) connect to PD.4(input mode) ......");

    /* Configure PD3 to output mode */
    GPIO_OpenBit(GPIOD, BIT3, DIR_OUTPUT, NO_PULL_UP);

    /* Configure PD4 to output mode */
    GPIO_OpenBit(GPIOD, BIT4, DIR_INPUT, NO_PULL_UP);

    /* Use Pin Data Input/Output Control to pull specified I/O or get I/O pin status */
    /* Pull PD.3 to High and check PD.4 status */
    GPIO_SetBit(GPIOD, BIT3);

    if(GPIO_ReadBit(GPIOD,BIT4)==0)  
      i32Err = 1;

    /* Pull PD.3 to Low and check PD.4 status */
    GPIO_ClrBit(GPIOD, BIT3);

    if(GPIO_ReadBit(GPIOD,BIT4)==1)  
      i32Err = 1;

    if(i32Err)
    {
        sysprintf("  [FAIL].\n");
    }
    else
    {
        sysprintf("  [OK].\n");
    }

    /* Configure PD3 to default value */
    GPIO_CloseBit(GPIOD, BIT3);

    /* Configure PD4 to default value */
    GPIO_CloseBit(GPIOD, BIT3);

    /* Set MFP_GPF11 to EINT0 */
    outpw(REG_SYS_GPF_MFPH,(inpw(REG_SYS_GPF_MFPH) & ~(0xF<<12)) | (0xF<<12));

    /* Configure PF11 to input mode and pull-up */
    GPIO_OpenBit(GPIOF, BIT11, DIR_INPUT, PULL_UP);

    /* Confingure PF11 to rising-edge trigger */
    GPIO_EnableTriggerType(GPIOF, BIT11,RISING);

    /* Enable external 0 interrupt */
    GPIO_EnableEINT(NIRQ0, (GPIO_CALLBACK)EINT0Callback, 0);

    /* waiting for external 0 interrupt */
    sysprintf("waiting for PF11 rsing-edge trigger...");
    while(!eint_complete);

    /* Disable PF11 trigger type */
    GPIO_DisableTriggerType(GPIOF, BIT11);

    /* Enable external 0 interrupt */
    GPIO_DisableEINT(NIRQ0);

    sysprintf("  [OK].\n");

    /* Configure PF11 to default value */
    GPIO_CloseBit(GPIOF, BIT11);


    /* Configure PE3 to output mode */
    GPIO_OpenBit(GPIOE, BIT3, DIR_INPUT, NO_PULL_UP);

    /* Confingure PE3 to falling-edge trigger */
    GPIO_EnableTriggerType(GPIOE, BIT3,FALLING);

    /* Enable GPIOE interrupt */
    GPIO_EnableInt(GPIOE, (GPIO_CALLBACK)GPIOECallback, 0);

    /* waiting for external 0 interrupt */
    sysprintf("waiting for PE3 falling-edge trigger...");
    while(!gpio_complete);

    /* Disable PE3 to trigger type */
    GPIO_DisableTriggerType(GPIOE, BIT3);

    /* Disable GPIOE interrupt */
    GPIO_DisableInt(GPIOE);

    /* Configure PE0 to default value */
    GPIO_CloseBit(GPIOE, BIT3);

    sysprintf("  [OK].\n");

    while(1);
}
