#include <stdio.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"
#include "rtc.h"

extern VOID RTC_Releative_AlarmISR(void);
extern VOID RTC_AlarmISR(void);
extern BOOL volatile g_bAlarm;
BOOL volatile g_bPowerKeyPress = FALSE;

#define HW_POWER_OFF	0
#define SW_POWER_OFF	1
#define SYSTEM_CLOCK	12000000

#if defined ( __GNUC__ ) && !(__CC_ARM)
void __wfi(void)
{
    asm
    (
        "MCR p15, 0, r1, c7, c0, 4 \n"
        "BX  lr \n"
    );
}
#else
__asm void __wfi()
{
    MCR p15, 0, r1, c7, c0, 4
    BX       lr
}
#endif

void Entry_PowerDown(void){
    int i,r0,r1,r2;
    outp32(REG_CLK_PLLSTBCNTR,0xFFFF);
    outp32(REG_SDIC_MR,  inpw(REG_SDIC_MR)|0x100); //Enable Reset DLL(bit[8]) of DDR2
#if defined ( __GNUC__ ) && !(__CC_ARM)
    asm
    (
        "mov r2, #1000  \n"
        "mov r1, #0  \n"
        "mov r0, #1  \n"
        "loop1: "
        "add   r1, r1, r0 \n"
        "cmp r1, r2 \n"
        "bne loop1 \n"
    );
#else
    __asm
    {/*  Delay a moment until the escape self-refresh command reached to DDR/SDRAM */
        mov r2, #1000
        mov r1, #0
        mov r0, #1
    loop1:  add   r1, r1, r0
        cmp r1, r2
        bne loop1
    }
#endif
    outp32(REG_SDIC_MR,  inpw(REG_SDIC_MR)&~0x100); //Disabe Reset DLL(bit[8]) of DDR2
#if defined ( __GNUC__ ) && !(__CC_ARM)
    asm
    (
        "mov r2, #1000 \n"
        "mov r1, #0 \n"
        "mov r0, #1 \n"
        "loop2: \n"
        "add r1, r1, r0 \n"
        "cmp r1, r2 \n"
        "bne loop2 \n"
    );
#else
    __asm
    {/*  Delay a moment until the escape self-refresh command reached to DDR/SDRAM */
        mov r2, #1000
        mov r1, #0
        mov r0, #1
    loop2:  add r1, r1, r0
        cmp r1, r2
        bne loop2
    }
#endif
    i = *(volatile unsigned int *)(0xB0000200);
    i = i & (0xFFFFFFFE);
    *(volatile unsigned int *)(0xB0000200)=i;
    outp32(REG_SDIC_OPMCTL, (inp32(REG_SDIC_OPMCTL) & ~0x10000)); // set SDIC_OPMCTL[16] low to disable auto power down mode
    outp32(REG_SDIC_CMD, (inp32(REG_SDIC_CMD) & ~0x20));
    __wfi();
    outp32(REG_SDIC_CMD, inp32(REG_SDIC_CMD) | 0x20);
    outp32(REG_SDIC_OPMCTL, (inp32(REG_SDIC_OPMCTL) | 0x10000)); // set SDIC_OPMCTL[16] high to enable auto power down mode;
}

void Smpl_RTC_Powerdown_Wakeup_Relative(void)
{
    RTC_TIME_DATA_T sCurTime;

    sysprintf("\n2. RTC Powerdown Wakeup Test (Wakeup after 10 seconds)\n");

    g_bAlarm = FALSE;

    /* Get the currnet time */
    RTC_Read(RTC_CURRENT_TIME, &sCurTime);

    sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);

    /* Enable RTC Tick Interrupt and install tick call back function */
    RTC_Ioctl(0,RTC_IOC_SET_RELEATIVE_ALARM, 10, (UINT32)RTC_Releative_AlarmISR);

    // Unlock Register
    outpw(0xB00001FC, 0x59);
    outpw(0xB00001FC, 0x16);
    outpw(0xB00001FC, 0x88);
    while(!(inpw(0xB00001FC) & 0x1));

    outpw(REG_SYS_WKUPSER , (1 << 24) ); // wakeup source select RTC

    //enter power down mode
    Entry_PowerDown();

    sysprintf("   Wake up!!!\n");

    while(!g_bAlarm);

    /* Get the currnet time */
    RTC_Read(RTC_CURRENT_TIME, &sCurTime);

    sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);

    RTC_Ioctl(0,RTC_IOC_DISABLE_INT,RTC_RELATIVE_ALARM_INT,0);
}

void Smpl_RTC_Powerdown_Wakeup(void)
{
    unsigned int i;
    RTC_TIME_DATA_T sCurTime;

    sysprintf("\n2. RTC Powerdown Wakeup Test (Wakeup after 10 seconds)\n");

    g_bAlarm = FALSE;

    /* Get the currnet time */
    RTC_Read(RTC_CURRENT_TIME, &sCurTime);

    /* Set Alarm call back function */
    sCurTime.pfnAlarmCallBack = RTC_AlarmISR;

    /* Disable Alarm Mask */
    sCurTime.u32AlarmMaskSecond = 0;
    sCurTime.u32AlarmMaskMinute = 0;
    sCurTime.u32AlarmMaskHour = 0;
    //sCurTime.u32AlarmMaskDay = 0;
    //sCurTime.u32AlarmMaskMonth = 0;
    //sCurTime.u32AlarmMaskYear = 0;
    //sCurTime.u32AlarmMaskDayOfWeek = 0;
    sCurTime.u32AlarmMaskSecond = 0;

    sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);

    /* The alarm time setting */
    sCurTime.u32cSecond = sCurTime.u32cSecond + 10; 

    if(sCurTime.u32cSecond >= 60)
    {
        sCurTime.u32cSecond = sCurTime.u32cSecond - 60;

        sCurTime.u32cMinute++;
        if(sCurTime.u32cMinute >= 60)
        {
            sCurTime.u32cMinute = sCurTime.u32cMinute - 60;

            sCurTime.u32cHour++;
            if(sCurTime.u32cHour >= 24)
            {
                sCurTime.u32cHour = 0;
                sCurTime.u32cDay++;
            }
        }
    }
    /* Set the alarm time (Install the call back function and enable the alarm interrupt)*/
    RTC_Write(RTC_ALARM_TIME,&sCurTime);

    // Unlock Register
    outpw(0xB00001FC, 0x59);
    outpw(0xB00001FC, 0x16);
    outpw(0xB00001FC, 0x88);
    while(!(inpw(0xB00001FC) & 0x1));

    outpw(REG_SYS_WKUPSER , (1 << 24) ); // wakeup source select RTC

    //enter power down mode
    i = *(volatile unsigned int *)(0xB0000200);
    i = i & (0xFFFFFFFE);
    *(volatile unsigned int *)(0xB0000200)=i;

    __wfi();

    sysprintf("   Wake up!!!\n");

    while(!g_bAlarm);

    sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);

    RTC_Ioctl(0,RTC_IOC_DISABLE_INT,RTC_ALARM_INT,0);
}	

void PowerKeyPress(void)
{
    sysprintf("\nPower Key Press!!\n");
    g_bPowerKeyPress = TRUE;
    return;
}


void Smpl_RTC_PowerOff_Control(UINT32 u32Mode)
{
    UINT32 u32PowerKeyStatus;
    INT32 volatile i;
    //UINT32 u32ExtFreq;
    //u32ExtFreq = sysGetClock();
    //sysSetTimerReferenceClock (TIMER0, u32ExtFreq);
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);

    g_bPowerKeyPress = FALSE;

    sysprintf("Turn on H/W power off function\n");

    /* Press Power Key during 6 sec to Power off */
    RTC_Ioctl(0, RTC_IOC_SET_POWER_OFF_PERIOD, 6, 0);

    /* Install the callback function for Power Key Press */
    RTC_Ioctl(0, RTC_IOC_SET_PSWI_CALLBACK, (UINT32)PowerKeyPress, 0);

    /* Enable Hardware Power off */
    RTC_Ioctl(0, RTC_IOC_ENABLE_HW_POWEROFF, 0, 0);

    /* Wait Key Press */
    sysprintf("Wait for Key Press\n");

    while(!g_bPowerKeyPress);

    /* Query Power Key 3 SEC (query a time per 1 sec)*/
    sysprintf("Press Key 3 seconds (Power off procedure starts after 3 seconds)\n");
    for(i = 0 ; i< 3;i++)
    {
        /* Delay 1 second */
        sysDelay(100);

        /* Query Power Key Status */
        RTC_Ioctl(0, RTC_IOC_GET_POWERKEY_STATUS, (UINT32)&u32PowerKeyStatus, 0);

        if(u32PowerKeyStatus)
        {
            sysprintf("	Power Key Release\n");
            sysprintf("	Power Off Flow Stop\n");
            return;
        }
        else
        sysprintf("	Power Key Press\n");
    }

    /* S/W Power off sequence */
    sysprintf("S/W Power off sequence start (1 second)..\n");

    /* Use time to simulate the S/W Power off sequence (1 sec) */
    sysDelay(100);
    
    if(u32Mode == SW_POWER_OFF) 
    {
        sysprintf("Power off sequence Complete -> Power Off ");

        /* Power Off - S/W can call the API to power off any time he wnats */
        RTC_Ioctl(0, RTC_IOC_SET_POWER_OFF, 0, 0);
    }
    else
    {
        sysprintf("S/W Crash!!\n");

        sysprintf("Wait for HW Power off");
    }

    i = 0;
    while(1)
    {
        if((i%50000) == 0)
        sysprintf(".");
        i++;
    }
}
