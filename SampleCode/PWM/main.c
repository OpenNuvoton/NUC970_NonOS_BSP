/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/05/08 7:09p $
 * @brief    NUC970 PWM Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "nuc970.h"
#include "sys.h"
#include "pwm.h"


void show_menu(void);
INT PWM_Timer(INT timer_num);
INT PWM_TimerDZ(INT dz_num);


int main (void)
{
    INT item;

    // Disable all interrupts.
    outpw(REG_AIC_MDCR, 0xFFFFFFFE);
    outpw(REG_AIC_MDCRH, 0x3FFFFFFF);
    
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    outpw(REG_CLK_PCLKEN1, inpw(REG_CLK_PCLKEN1) | (1 << 27)); // Enable PWM engine clock

    while(1) {
        show_menu();
        item = sysGetChar();// Get user key
        switch(item)
        {
            case '0':    PWM_Timer(PWM_TIMER0);        break;
            case '1':    PWM_Timer(PWM_TIMER1);        break;
            case '2':    PWM_Timer(PWM_TIMER2);        break;
            case '3':    PWM_Timer(PWM_TIMER3);        break;
            case '4':    PWM_TimerDZ(PWM_TIMER0);    break;
            case '5':    PWM_TimerDZ(PWM_TIMER2);    break;
            default:                                break;
        }
    }

}

void show_menu(void)
{
    sysprintf("\n");
    sysprintf("+------------------------------------------------------+\n");
    sysprintf("| Nuvoton PWM Demo Code                                |\n");
    sysprintf("+------------------------------------------------------+\n");
    sysprintf("| PWM Timer0 test                                - [0] |\n");
    sysprintf("| PWM Timer1 test                                - [1] |\n");
    sysprintf("| PWM Timer2 test                                - [2] |\n");
    sysprintf("| PWM Timer3 test                                - [3] |\n");
    sysprintf("| PWM Dead zone 0 test                           - [4] |\n");
    sysprintf("| PWM Dead zone 1 test                           - [5] |\n");
    sysprintf("+------------------------------------------------------+\n");
    sysprintf("Please Select :\n");
}

INT PWM_Timer(INT timer_num)
{
    typePWMVALUE pwmvalue;
    typePWMSTATUS PWMSTATUS;
    INT nLoop=0;
    INT nStatus=0;
    INT nInterruptInterval=0;
    PWMSTATUS.PDR=0;
    PWMSTATUS.InterruptFlag=FALSE;

    pwmInit();
    pwmOpen(timer_num);

    // Change PWM Timer setting
    pwmIoctl(timer_num, SET_CSR, 0, CSRD16);
    pwmIoctl(timer_num, SET_CP, 0, 249);
    pwmIoctl(timer_num, SET_DZI, 0, 0);
    pwmIoctl(timer_num, SET_INVERTER, 0, PWM_INVOFF);
    pwmIoctl(timer_num, SET_MODE, 0, PWM_TOGGLE);
    pwmIoctl(timer_num, DISABLE_DZ_GENERATOR, 0, 0);
    if(timer_num == PWM_TIMER0)
        pwmIoctl(timer_num, ENABLE_PWMGPIOOUTPUT, PWM_TIMER0, PWM0_GPA12);
    else if(timer_num == PWM_TIMER1)
        pwmIoctl(timer_num, ENABLE_PWMGPIOOUTPUT, PWM_TIMER1, PWM1_GPA13);
    else if(timer_num == PWM_TIMER2)
        pwmIoctl(timer_num, ENABLE_PWMGPIOOUTPUT, PWM_TIMER2, PWM2_GPA14);
    else
        pwmIoctl(timer_num, ENABLE_PWMGPIOOUTPUT, PWM_TIMER3, PWM3_GPA15);

    pwmvalue.field.cnr=59999;
    pwmvalue.field.cmr=4999;
    pwmWrite(timer_num, (PUCHAR)(&pwmvalue), sizeof(pwmvalue));

    sysprintf("PWM Timer%d one shot mode test\nPWM. Timer interrupt will occure soon.", timer_num);

    //Start PWM Timer
    pwmIoctl(timer_num, START_PWMTIMER, 0, 0);

    while(1)
    {
        nLoop++;
        if(nLoop%100000 == 0)
        {
            sysprintf(".");
        }
        nStatus=pwmRead(timer_num, (PUCHAR)&PWMSTATUS, sizeof(PWMSTATUS));
        if(nStatus != Successful)
        {
            sysprintf("PWM read error, ERR CODE:%d",nStatus);
            pwmClose(timer_num);
            return Fail;
        }
        if(PWMSTATUS.InterruptFlag==TRUE)
        {
            sysprintf("\n\nPWM Timer interrupt occurred!\n\n");
            break;
        }
    }

     // Change PWM Timer setting
    pwmIoctl(timer_num, SET_CSR, 0, CSRD16);
    pwmIoctl(timer_num, SET_CP, 0, 255);
    pwmIoctl(timer_num, SET_DZI, 0, 0);
    pwmIoctl(timer_num, SET_INVERTER, 0, PWM_INVOFF);
    pwmIoctl(timer_num, SET_MODE, 0, PWM_TOGGLE);
    pwmIoctl(timer_num, DISABLE_DZ_GENERATOR, 0, 0);
    //pwmIoctl(timer_num, ENABLE_PWMGPIOOUTPUT, 0, 0);


    nInterruptInterval=30000;
    pwmvalue.field.cnr=nInterruptInterval;
    pwmvalue.field.cmr=4999;
    pwmWrite(timer_num, (PUCHAR)(&pwmvalue), sizeof(pwmvalue));

    sysprintf("PWM Timer%d toggle mode test\nPWM Timer interrupt interval will decrease gradually\n", timer_num);

    //Start PWM Timer
    pwmIoctl(timer_num, START_PWMTIMER, 0, 0);
    nLoop=0;
    while(1)
    {
        nStatus=pwmRead(timer_num, (PUCHAR)&PWMSTATUS, sizeof(PWMSTATUS));
        if(nStatus != Successful)
        {
            sysprintf("PWM read error, ERR CODE:%d",nStatus);
            pwmClose(timer_num);
            return Fail;
        }
        if(PWMSTATUS.InterruptFlag==TRUE)
        {
            sysprintf("PWM Timer interrupt [%d], CNR:%d\n",nLoop,nInterruptInterval);
            nInterruptInterval/=2;
            pwmvalue.field.cnr=nInterruptInterval;
            pwmvalue.field.cmr=4999;
            pwmWrite(timer_num, (PUCHAR)(&pwmvalue), sizeof(pwmvalue));
            nLoop++;
            if(nLoop==10)
            {
                break;
            }
        }
    }
    pwmIoctl(timer_num, STOP_PWMTIMER, 0, 0);
    pwmClose(timer_num);
    sysprintf("\nPWM Timer %d test finish\nPress any key to continue....", timer_num);
    sysGetChar();
    return Successful;
}

INT PWM_TimerDZ(INT timer_num)
{
    typePWMSTATUS PWMSTATUS0;
    typePWMSTATUS PWMSTATUS1;

    PWMSTATUS0.PDR=0;
    PWMSTATUS0.InterruptFlag=FALSE;
    PWMSTATUS1.PDR=0;
    PWMSTATUS1.InterruptFlag=FALSE;

    pwmInit();
    pwmOpen(timer_num);
    pwmOpen(timer_num+1);
    if(timer_num == PWM_TIMER0) {
        pwmIoctl(timer_num, ENABLE_PWMGPIOOUTPUT, PWM_TIMER0, PWM0_GPA12);
        pwmIoctl(timer_num + 1, ENABLE_PWMGPIOOUTPUT, PWM_TIMER1, PWM1_GPA13);
    } else {
        pwmIoctl(timer_num, ENABLE_PWMGPIOOUTPUT, PWM_TIMER2, PWM2_GPA14);
        pwmIoctl(timer_num + 1, ENABLE_PWMGPIOOUTPUT, PWM_TIMER3, PWM3_GPA15);
    }
    
    pwmIoctl(timer_num, SET_DZI, 0, 0x50);
    pwmIoctl(timer_num, ENABLE_DZ_GENERATOR, 0, 0);

    pwmIoctl(timer_num, START_PWMTIMER, 0, 0);
    pwmIoctl(timer_num+1, START_PWMTIMER, 0, 0);

    sysprintf("Hit any key to quit\n");
    sysGetChar();
    pwmIoctl(timer_num, STOP_PWMTIMER, 0, 0);
    pwmIoctl(timer_num+1, STOP_PWMTIMER, 0, 0);
    pwmClose(timer_num);
    pwmClose(timer_num+1);

    return Successful;
}
