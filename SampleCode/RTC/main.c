/***************************************************************************
 *                                                                         *
 * Copyright (c) 2008 Nuvoton Technolog. All rights reserved.              *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include "nuc970.h"
#include "sys.h"
#include "rtc.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
UINT32 volatile g_u32TICK = 0;
BOOL volatile g_bAlarm = FALSE;
VOID Smpl_RTC_Powerdown_Wakeup(void);
VOID Smpl_RTC_Powerdown_Wakeup_Relative(void);
VOID Smpl_RTC_PowerOff_Control(UINT32 u32Mode);

#define HW_POWER_OFF	0
#define SW_POWER_OFF	1

/*---------------------------------------------------------------------------------------------------------*/
/* RTC Tick Callback function                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
VOID RTC_TickISR(void)
{
	RTC_TIME_DATA_T sCurTime;
	
	/* Get the currnet time */
	RTC_Read(RTC_CURRENT_TIME, &sCurTime);		
	
	sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);				
	RTC_EnableClock(TRUE);
	g_u32TICK++;
}

/*---------------------------------------------------------------------------------------------------------*/
/* RTC Alarm Callback function                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
VOID RTC_AlarmISR(void)
{
	
	sysprintf("   Abolute Alarm!!\n");
	
	g_bAlarm = TRUE; 
}

VOID RTC_Releative_AlarmISR(void)
{
	
	sysprintf("   Relative Alarm!!\n");
	
	g_bAlarm = TRUE; 
}

int main()
{
	//UINT32 u32Item, u32ExtFreq;
	UINT32 u32Item;
	RTC_TIME_DATA_T sInitTime;
	//UINT32 u32ClockSource;
		
	sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

	RTC_EnableClock(TRUE);
	
	/* RTC Initialize */
	RTC_Init();	
	
	/* Time Setting */
	sInitTime.u32Year = 2015;
	sInitTime.u32cMonth = 5;	
	sInitTime.u32cDay = 25;
	sInitTime.u32cHour = 13;
	sInitTime.u32cMinute = 30;
	sInitTime.u32cSecond = 0;
	sInitTime.u32cDayOfWeek = RTC_TUESDAY;
	sInitTime.u8cClockDisplay = RTC_CLOCK_24;			

	/* Initialization the RTC timer */
	if(RTC_Open(&sInitTime) !=E_RTC_SUCCESS)
		sysprintf("Open Fail!!\n");
			
	sysSetLocalInterrupt(ENABLE_IRQ);	
	
	sysprintf("Do RTC Calibration \n");	
	RTC_Ioctl(0,RTC_IOC_SET_FREQUENCY, 0,0);	
	
	do
	{    	
		sysprintf("=======================================\n");
		sysprintf("             RTC Demo Code	          \n"); 	
		sysprintf("=======================================\n");	
		sysprintf("      (Clock from External 32K Hz)     \n");  			
		sysprintf("=======================================\n");	
		sysprintf("Select Demo Item:\n");
		sysprintf("0. Time Display Test\n");
		sysprintf("1. Alarm Test (Absolute)\n");
		sysprintf("2. Alarm Test (Relative)\n");
		sysprintf("3. Power down Wakeup Test (Absolute)\n");
		sysprintf("4. Power down Wakeup Test (Relative)\n");
		sysprintf("5. S/W Force to Power Off Test\n");
		sysprintf("6. S/W Power Off (Normal Case) Control Flow Test\n");
		sysprintf("7. H/W Power Off (System Crash) Control Flow Test\n");		
		sysprintf("8. RTC Alarm Mask Test for Hour\n");	
		sysprintf("9. RTC Frequency Compensation\n");
		sysprintf("Q. Exit\n");
		u32Item = sysGetChar();			
		
		switch(u32Item)
		{
			case '0':
			{
				RTC_TICK_T sTick;
				
				sysprintf("\n0. RTC Time Display Test (Exit after 5 seconds)\n");
					
				/* Set Tick property */
				sTick.ucMode = RTC_TICK_1_SEC;
				sTick.pfnTickCallBack = RTC_TickISR;		
				
				/* Set Tick setting */
				RTC_Ioctl(0,RTC_IOC_SET_TICK_MODE, (UINT32)&sTick,0);

				/* Enable RTC Tick Interrupt and install tick call back function */
				RTC_Ioctl(0,RTC_IOC_ENABLE_INT, (UINT32)RTC_TICK_INT,0);	
				
				g_u32TICK = 0;
				
				RTC_EnableClock(TRUE);
				
				while(g_u32TICK < 5);
				
				RTC_EnableClock(FALSE);

				/* Disable RTC Tick Interrupt */
				RTC_Ioctl(0,RTC_IOC_DISABLE_INT, (UINT32)RTC_TICK_INT,0);							
				break;
			}
			case '1':
			{
				RTC_TIME_DATA_T sCurTime;	
				
				sysprintf("\n1. RTC Alarm Test (Absolute)-> Alarm after 10 seconds\n");
					
				g_bAlarm = FALSE;	
				
				/* Get the currnet time */
				RTC_Read(RTC_CURRENT_TIME, &sCurTime);
				
				/* Set Alarm call back function */
				sCurTime.pfnAlarmCallBack = RTC_AlarmISR;
				
				/* Disable Alarm Mask */
				sCurTime.u32AlarmMaskSecond = 0;	
				sCurTime.u32AlarmMaskMinute = 0;	
				sCurTime.u32AlarmMaskHour = 0;	
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
				
				RTC_EnableClock(TRUE);
						
				while(!g_bAlarm);
			
				/* Get the currnet time */
				RTC_Read(RTC_CURRENT_TIME, &sCurTime);	
				
				sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);
				
				RTC_Ioctl(0,RTC_IOC_DISABLE_INT,RTC_ALARM_INT,0);
							
				break;				
			}
			case '2':
			{
				RTC_TIME_DATA_T sCurTime;	
				
				sysprintf("\n2. RTC Alarm Test (Relative) -> Alarm after 10 seconds\n");
					
				g_bAlarm = FALSE;	
				
				/* Get the currnet time */
				RTC_Read(RTC_CURRENT_TIME, &sCurTime);
				
				sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);								
				
				/* Enable RTC Relative Alarm Interrupt and install call back function */
				RTC_Ioctl(0,RTC_IOC_SET_RELEATIVE_ALARM, 10, (UINT32)RTC_Releative_AlarmISR);	
	
				RTC_EnableClock(TRUE);
	
				while(!g_bAlarm);			
				
				/* Get the currnet time */
				RTC_Read(RTC_CURRENT_TIME, &sCurTime);
	
				sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);
				
				RTC_Ioctl(0,RTC_IOC_DISABLE_INT,RTC_RELATIVE_ALARM_INT,0);
								
				break;				
			}			
			case '3':
				sysprintf("\n3. Power down Wakeup Test (Absolute)\n");
		
				Smpl_RTC_Powerdown_Wakeup();	
				break;
			case '4':
				sysprintf("\n4. Power down Wakeup Test (Relative)\n");
		
				Smpl_RTC_Powerdown_Wakeup_Relative();	
				break;	
			case '5':			
				sysprintf("\n5. S/W Force to Power Off Test\n");	
				sysprintf("   Force to Power Off imediatedly\n");							
				RTC_Ioctl(0, RTC_IOC_SET_POWER_OFF, 0, 0);
				while(1);
//				break;										
			case '6':
				sysprintf("\n6. S/W Power Off (Normal Case) Control Flow Test\n");
				Smpl_RTC_PowerOff_Control(SW_POWER_OFF);
				break;
			case '7':
				sysprintf("\n7. H/W Power Off (System Crash!) Control Flow Test\n");
				Smpl_RTC_PowerOff_Control(HW_POWER_OFF);
				break;
			case '8':
			{			
				int i;				
				RTC_TIME_DATA_T sCurTime,sAlarmTime;	
				
				sysprintf("\n8. RTC Alarm Mask Test for Hour\n");
				
				/* Time Setting */
				sCurTime.u32Year = 2015;
				sCurTime.u32cMonth = 1;	
				sCurTime.u32cDay = 8;
				sCurTime.u32cHour = 10;
				sCurTime.u32cMinute = 13;
				sCurTime.u32cSecond = 0;
				sCurTime.u32cDayOfWeek = RTC_FRIDAY;
				sCurTime.u8cClockDisplay = RTC_CLOCK_24;				
			 	RTC_Write(RTC_CURRENT_TIME,&sCurTime);		
				
				/* Alarm Setting */
				sAlarmTime.u32Year = 2015;
				sAlarmTime.u32cMonth = 1;	
				sAlarmTime.u32cDay = 8;
				sAlarmTime.u32cHour = 10;
				sAlarmTime.u32cMinute = 13;
				sAlarmTime.u32cSecond = 5;
				sAlarmTime.u32cDayOfWeek = RTC_FRIDAY;
				sAlarmTime.u8cClockDisplay = RTC_CLOCK_24;		

				/* Set Alarm call back function */
				sAlarmTime.pfnAlarmCallBack = RTC_AlarmISR;		
								
				/* Enable Alarm Mask for Day */					
				sAlarmTime.u32AlarmMaskSecond = 0;	
				sAlarmTime.u32AlarmMaskMinute = 0;	
				sAlarmTime.u32AlarmMaskHour = 1;	
				
				/* Set the alarm time (Install the call back function and enable the alarm interrupt)*/
				RTC_Write(RTC_ALARM_TIME,&sAlarmTime);				
		
				sysprintf("Alarm should occur when %d/%02d/%02d xx:%02d:%02d\n",sAlarmTime.u32Year,sAlarmTime.u32cMonth,sAlarmTime.u32cDay,sAlarmTime.u32cMinute,sAlarmTime.u32cSecond);			
		
				sCurTime.u32cHour = 1;
		
				for(i=0;i<5;i++)
				{		
					sysprintf("Test %d\n",i); 
												
					g_bAlarm = FALSE;		
					
					sCurTime.u32Year = 2015;
					sCurTime.u32cMonth = 1;		
					sCurTime.u32cDay = 8;
					sCurTime.u32cHour++;
					sCurTime.u32cMinute = 13;
					sCurTime.u32cSecond = 0;
					sCurTime.u32cDayOfWeek = RTC_FRIDAY;
					sCurTime.u8cClockDisplay = RTC_CLOCK_24;	
					RTC_Write(RTC_CURRENT_TIME,&sCurTime);	
					
					sysprintf("   Change Current Time To:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);
					
					while(!g_bAlarm);		
					
					/* Get the currnet time */
					RTC_Read(RTC_CURRENT_TIME, &sCurTime);					

					sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);
				}	
				
				RTC_Ioctl(0,RTC_IOC_DISABLE_INT,RTC_ALARM_INT,0);
						
				break;
			}				
			case '9':
			{
				sysprintf("Set RTC Frequency Compensation\n");	
				RTC_Ioctl(0,RTC_IOC_SET_FREQUENCY, 0,0);
				break;	
			}							
			default:
				sysprintf("Wrong Item\n");
				break;			
		}
		
	}while((u32Item!= 'q') || (u32Item!= 'Q'));	
			
	
	/* Disable RTC  */		
	RTC_Close();
	
	/* Disable I & F bit */ 
	sysSetLocalInterrupt(DISABLE_IRQ);		


}  
