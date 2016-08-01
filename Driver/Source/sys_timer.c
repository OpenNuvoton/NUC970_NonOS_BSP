/**************************************************************************//**
* @file     sys_timer.c
* @version  V1.00
* $Revision: 5 $
* $Date: 15/05/18 5:48p $
* @brief    NUC970 SYS timer driver source file
*
* @note
* Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include "nuc970.h"
#include "sys.h"

/// @cond HIDDEN_SYMBOLS

/* Global variables */
PVOID  _sys_pvOldTimer0Vect, _sys_pvOldTimer1Vect;
unsigned int _sys_uTimer0ClockRate = 12000000;
UINT32 _sys_uTimer1ClockRate = 12000000;
UINT32 volatile _sys_uTimer0Count = 0, _sys_uTimer1Count = 0;
UINT32 volatile _sys_uTime0EventCount = 0, _sys_uTime1EventCount = 0;
UINT32 volatile _sys_uTimer0TickPerSecond;
BOOL   _sys_bIsSetTime0Event = FALSE, _sys_bIsSetTime1Event = FALSE;
BOOL volatile _sys_bIsTimer0Initial = FALSE;

#define SECONDS_PER_HOUR    3600
#define SECONDS_PER_DAY     86400
#define SECONDS_365_DAY     31536000
#define SECONDS_366_DAY     31622400

#define TimerEventCount     10

typedef void (*sys_pvTimeFunPtr)();   /* function pointer */

/** \brief  Structure type of Timer Event
 */
typedef struct timeEvent_t
{
    UINT32              active;     /*!< Active */
    UINT32              initTick;   /*!< init tick count */
    UINT32              curTick;    /*!< current tick count */
    sys_pvTimeFunPtr    funPtr;     /*!< callback function */
} TimeEvent_T;
TimeEvent_T tTime0Event[TimerEventCount], tTime1Event[TimerEventCount];


UINT32  volatile _sys_ReferenceTime_Clock = 0;
UINT32  volatile _sys_ReferenceTime_UTC;

static UINT32 month_seconds[] =
{
    31 * SECONDS_PER_DAY,
    28 * SECONDS_PER_DAY,
    31 * SECONDS_PER_DAY,
    30 * SECONDS_PER_DAY,
    31 * SECONDS_PER_DAY,
    30 * SECONDS_PER_DAY,
    31 * SECONDS_PER_DAY,
    31 * SECONDS_PER_DAY,
    30 * SECONDS_PER_DAY,
    31 * SECONDS_PER_DAY,
    30 * SECONDS_PER_DAY,
    31 * SECONDS_PER_DAY
};


/* Interrupt service routine */
void sysTimer0ISR()
{
    int volatile i;

    /*----- check channel 0 -----*/
    if (inpw(REG_TMR_TISR) & 0x00000001)
    {
        _sys_uTimer0Count++;
        outpw(REG_TMR_TISR, 0x01); /* clear TIF0 */
        if (_sys_uTimer0Count >= 0xfffffff)
            _sys_uTimer0Count = 0;

        if (_sys_bIsSetTime0Event)
        {
            for (i=0; i<TimerEventCount; i++)
            {
                if (tTime0Event[i].active)
                {
                    tTime0Event[i].curTick--;
                    if (tTime0Event[i].curTick == 0)
                    {
                        (*tTime0Event[i].funPtr)();
                        tTime0Event[i].curTick = tTime0Event[i].initTick;
                    }
                }
            }
        }
    }
}

void sysTimer1ISR()
{
    int volatile i;

    /*----- check channel 1 -----*/
    if (inpw(REG_TMR_TISR) & 0x00000002)
    {
        _sys_uTimer1Count++;
        outpw(REG_TMR_TISR, 0x02); /* clear TIF1 */
        if (_sys_uTimer1Count >= 0xfffffff)
            _sys_uTimer1Count = 0;

        if (_sys_bIsSetTime1Event)
        {
            for (i=0; i<TimerEventCount; i++)
            {
                if (tTime1Event[i].active)
                {
                    tTime1Event[i].curTick--;
                    if (tTime1Event[i].curTick == 0)
                    {
                        (*tTime1Event[i].funPtr)();
                        tTime1Event[i].curTick = tTime1Event[i].initTick;
                    }
                }
            }
        }
    }
}
/// @endcond HIDDEN_SYMBOLS


/* Timer library functions */

/**
 *  @brief  system Timer - get current tick count
 *
 *  @param[in]  nTimeNo  Select TMR0 or TMR1. ( \ref TIMER0 / \ref TIMER1)
 *
 *  @return   Current tick count
 */
UINT32 sysGetTicks(INT32 nTimeNo)
{
    switch (nTimeNo)
    {
        case TIMER0:
            return _sys_uTimer0Count;

        case TIMER1:
            return _sys_uTimer1Count;

        default:
            ;
    }
    return 0;
}

/**
 *  @brief  system Timer - reset current tick count
 *
 *  @param[in]  nTimeNo  Select TMR0 or TMR1. ( \ref TIMER0 / \ref TIMER1)
 *
 *  @return   0
 */
INT32 sysResetTicks(INT32 nTimeNo)
{
    switch (nTimeNo)
    {
        case TIMER0:
            _sys_uTimer0Count = 0;
            break;

        case TIMER1:
            _sys_uTimer1Count = 0;
            break;

        default:
            ;
    }
    return 0;
}

/**
 *  @brief  system Timer - update current tick count
 *
 *  @param[in]  nTimeNo  Select TMR0 or TMR1. ( \ref TIMER0 / \ref TIMER1)
 *  @param[in]  uCount   Update count
 *
 *  @return   0
 */
INT32 sysUpdateTickCount(INT32 nTimeNo, UINT32 uCount)
{
    switch (nTimeNo)
    {
        case TIMER0:
            _sys_uTimer0Count = uCount;
            break;

        case TIMER1:
            _sys_uTimer1Count = uCount;
            break;

        default:
            ;
    }
    return 0;
}

/**
 *  @brief  system Timer - set timer reference clock
 *
 *  @param[in]  nTimeNo  Select TMR0 or TMR1. ( \ref TIMER0 / \ref TIMER1)
 *  @param[in]  uClockRate   reference clock
 *
 *  @return   0
 */
INT32 sysSetTimerReferenceClock(INT32 nTimeNo, UINT32 uClockRate)
{
    switch (nTimeNo)
    {
        case TIMER0:
            _sys_uTimer0ClockRate = uClockRate;
            break;

        case TIMER1:
            _sys_uTimer1ClockRate = uClockRate;
            break;

        default:
            ;
    }
    return 0;
}

/**
 *  @brief  system Timer - start timer 
 *
 *  @param[in]  nTimeNo  Select TMR0 or TMR1. ( \ref TIMER0 / \ref TIMER1)
 *  @param[in]  uTicksPerSecond   tick count per second
 *  @param[in]  nOpMode   Operation Mode. ( \ref ONE_SHOT_MODE / \ref PERIODIC_MODE / \ref TOGGLE_MODE)
 *
 *  @return   0
 */
INT32 sysStartTimer(INT32 nTimeNo, UINT32 uTicksPerSecond, INT32 nOpMode)
{
    int volatile i;
    UINT32 _mTicr, _mTcr;

    _mTcr = 0x60000000 | (nOpMode << 27);

    switch (nTimeNo)
    {
        case TIMER0:
            outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0)| 0x100);
            _sys_bIsTimer0Initial = TRUE;
            _sys_uTimer0TickPerSecond = uTicksPerSecond;

            outpw(REG_TMR0_TCSR, 0);           /* disable timer */
            outpw(REG_TMR_TISR, 1);           /* clear for safety */

            for (i=0; i<TimerEventCount; i++)
                tTime0Event[i].active = FALSE;

            _sys_pvOldTimer0Vect = sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, TMR0_IRQn, (PVOID)sysTimer0ISR);
            sysEnableInterrupt(TMR0_IRQn);

            _sys_uTimer0Count = 0;
            _mTicr = _sys_uTimer0ClockRate / uTicksPerSecond;
            outpw(REG_TMR0_TICR, _mTicr);
            outpw(REG_TMR0_TCSR, _mTcr);
            break;

        case TIMER1:
            outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0)| 0x200);
            outpw(REG_TMR1_TCSR, 0);           /* disable timer */
            outpw(REG_TMR_TISR, 2);           /* clear for safety */

            for (i=0; i<TimerEventCount; i++)
                tTime1Event[i].active = FALSE;

            _sys_pvOldTimer1Vect = sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, TMR1_IRQn, (PVOID)sysTimer1ISR);
            sysEnableInterrupt(TMR1_IRQn);

            _sys_uTimer1Count = 0;
            _mTicr = _sys_uTimer1ClockRate / uTicksPerSecond;
            outpw(REG_TMR1_TICR, _mTicr);
            outpw(REG_TMR1_TCSR, _mTcr);
            break;

        default:
            ;
    }
    sysSetLocalInterrupt(ENABLE_IRQ);
    return 0;
}

/**
 *  @brief  system Timer - stop timer 
 *
 *  @param[in]  nTimeNo  Select TMR0 or TMR1. ( \ref TIMER0 / \ref TIMER1)
 *
 *  @return   0
 */
INT32 sysStopTimer(INT32 nTimeNo)
{
    switch (nTimeNo)
    {
        case TIMER0:
            _sys_bIsTimer0Initial = FALSE;
            sysDisableInterrupt(TMR0_IRQn);
            sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, TMR0_IRQn, _sys_pvOldTimer0Vect);

            outpw(REG_TMR0_TCSR, 0);           /* disable timer */
            outpw(REG_TMR_TISR, 1);           /* clear for safety */

            _sys_uTime0EventCount = 0;
            _sys_bIsSetTime0Event = FALSE;
            break;

        case TIMER1:
            sysDisableInterrupt(TMR1_IRQn);
            sysInstallISR(HIGH_LEVEL_SENSITIVE| IRQ_LEVEL_1, TMR1_IRQn, _sys_pvOldTimer1Vect);


            outpw(REG_TMR1_TCSR, 0);           /* disable timer */
            outpw(REG_TMR_TISR, 2);           /* clear for safety */

            _sys_uTime1EventCount = 0;
            _sys_bIsSetTime1Event = FALSE;
            break;

        default:
            ;
    }

    return 0;
}

/**
 *  @brief  system Timer - clear WDT count
 *
 *  @return   None
 */
void sysClearWatchDogTimerCount(void)
{
    UINT32 volatile _mWtcr;

    _mWtcr = inpw(REG_WDT_CTL);
    _mWtcr |= 0x01;             /* write WTR */
    outpw(REG_WDT_CTL, _mWtcr);
}

/**
 *  @brief  system Timer - clear WDT interrupt status
 *
 *  @return   None
 */
void sysClearWatchDogTimerInterruptStatus(void)
{
    UINT32 volatile _mWtcr;

    _mWtcr = inpw(REG_WDT_CTL);
    _mWtcr |= 0x08;       /* clear WTIF */
    outpw(REG_WDT_CTL, _mWtcr);
}

/**
 *  @brief  system Timer - disable WDT
 *
 *  @return   None
 */
void sysDisableWatchDogTimer(void)
{
    UINT32 volatile _mWtcr;

    _mWtcr = inpw(REG_WDT_CTL);
    _mWtcr &= 0xFFFFFF7F;
    outpw(REG_WDT_CTL, _mWtcr);
}

/**
 *  @brief  system Timer - disable WDT reset
 *
 *  @return   None
 */
void sysDisableWatchDogTimerReset(void)
{
    UINT32 volatile _mWtcr;

    _mWtcr = inpw(REG_WDT_CTL);
    _mWtcr &= 0xFFFFFFFD;
    outpw(REG_WDT_CTL, _mWtcr);
}

/**
 *  @brief  system Timer - enable WDT
 *
 *  @return   None
 */
void sysEnableWatchDogTimer(void)
{
    UINT32 volatile _mWtcr;

    _mWtcr = inpw(REG_WDT_CTL);
    _mWtcr |= 0x80;
    outpw(REG_WDT_CTL, _mWtcr);
}

/**
 *  @brief  system Timer - enable WDT reset
 *
 *  @return   None
 */
void sysEnableWatchDogTimerReset(void)
{
    UINT32 volatile _mWtcr;

    _mWtcr = inpw(REG_WDT_CTL);
    _mWtcr |= 0x02;
    outpw(REG_WDT_CTL, _mWtcr);
}

/**
 *  @brief  system Timer - install WDT interrupt handler
 *
 *  @param[in]  nIntTypeLevel   Interrupt Level. ( \ref FIQ_LEVEL_0 / \ref IRQ_LEVEL_1 / \ref IRQ_LEVEL_2 / \ref IRQ_LEVEL_3 /
 *                                                 \ref IRQ_LEVEL_4 / \ref IRQ_LEVEL_5 / \ref IRQ_LEVEL_6 / \ref IRQ_LEVEL_7 )
 *  @param[in]  pvNewISR        Interrupt handler
 *
 *  @return   old interrupt handler
 */
PVOID sysInstallWatchDogTimerISR(INT32 nIntTypeLevel, PVOID pvNewISR)
{
    PVOID _mOldVect = NULL;
    UINT32 volatile _mWtcr;

    _mWtcr = inpw(REG_WDT_CTL);
    _mWtcr |= 0x40;
    outpw(REG_WDT_CTL, _mWtcr);
    _mOldVect = sysInstallISR(nIntTypeLevel, WDT_IRQn, pvNewISR);
    sysEnableInterrupt(WDT_IRQn);
    sysSetLocalInterrupt(ENABLE_IRQ);

    return _mOldVect;
}

/**
 *  @brief  system Timer - set WDT interval
 *
 *  @param[in]  nWdtInterval  WDT interval. ( \ref ONE_HALF_SECS / \ref FIVE_SECS / \ref TEN_SECS / \ref TWENTY_SECS)
 *
 *  @return   0
 */
INT32 sysSetWatchDogTimerInterval(INT32 nWdtInterval)
{
    UINT32 volatile _mWtcr;

    _mWtcr = inpw(REG_WDT_CTL) & ~0x700;
    _mWtcr = _mWtcr | (nWdtInterval << 8);
    outpw(REG_WDT_CTL, _mWtcr);

    return 0;
}

/**
 *  @brief  system Timer - install timer event
 *
 *  @param[in]  nTimeNo  Select TMR0 or TMR1. ( \ref TIMER0 / \ref TIMER1)
 *  @param[in]  uTimeTick  tick count
 *  @param[in]  pvFun  callback function
 *
 *  @return   event number
 */
INT32 sysSetTimerEvent(INT32 nTimeNo, UINT32 uTimeTick, PVOID pvFun)
{
    int volatile i;
    int val=0;

    switch (nTimeNo)
    {
        case TIMER0:
            _sys_bIsSetTime0Event = TRUE;
            _sys_uTime0EventCount++;
            for (i=0; i<TimerEventCount; i++)
            {
                if (tTime0Event[i].active == FALSE)
                {
                    tTime0Event[i].active = TRUE;
                    tTime0Event[i].initTick = uTimeTick;
                    tTime0Event[i].curTick = uTimeTick;
                    tTime0Event[i].funPtr = (sys_pvTimeFunPtr)pvFun;
                    val = i+1;
                    break;
                }
            }
            break;

        case TIMER1:
            _sys_bIsSetTime1Event = TRUE;
            _sys_uTime1EventCount++;
            for (i=0; i<TimerEventCount; i++)
            {
                if (tTime1Event[i].active == FALSE)
                {
                    tTime1Event[i].active = TRUE;
                    tTime1Event[i].initTick = uTimeTick;
                    tTime1Event[i].curTick = uTimeTick;
                    tTime1Event[i].funPtr = (sys_pvTimeFunPtr)pvFun;
                    val = i+1;
                    break;
                }
            }
            break;

        default:
            ;
    }

    return val;
}

/**
 *  @brief  system Timer - clear specific timer event
 *
 *  @param[in]  nTimeNo  Select TMR0 or TMR1. ( \ref TIMER0 / \ref TIMER1)
 *  @param[in]  uTimeEventNo  select timer event
 *
 *  @return   None
 */
void sysClearTimerEvent(INT32 nTimeNo, UINT32 uTimeEventNo)
{
    switch (nTimeNo)
    {
        case TIMER0:
            tTime0Event[uTimeEventNo-1].active = FALSE;
            _sys_uTime0EventCount--;
            if (_sys_uTime0EventCount == 0)
                _sys_bIsSetTime0Event = FALSE;
            break;

        case TIMER1:
            tTime1Event[uTimeEventNo-1].active = FALSE;
            _sys_uTime1EventCount--;
            if (_sys_uTime1EventCount == 0)
                _sys_bIsSetTime1Event = FALSE;
            break;

        default:
            ;
    }
}

/// @cond HIDDEN_SYMBOLS

/*
 *  The default time: 2005.05.01 Sun 00:00 00:00
 *
 *  one day = 3600 * 24 = 86400
 *  one year = 86400 * 365 = 31536000
 *  year 2005 = 31536000 * 35 + 8 * 86400 = 1104451200
 */

UINT32 sysDOS_Time_To_UTC(DateTime_T ltime)
{
    int     i, leap_year_cnt;
    UINT32  utc;

    if ((ltime.mon < 1) || (ltime.mon > 12) || (ltime.day < 1) || (ltime.day > 31) ||
        (ltime.hour > 23) || (ltime.min > 59) || (ltime.sec > 59))
    {
        //_debug_msg("DOS_Time_To_UTC - illegal time!! %d-%d-%d %d:%d:%d\n", year, month, day, hour, minute, second);
        return 1;
    }

    ltime.year = ltime.year - 1970;     /* DOS is 1980 started, UTC is 1970 started */

    leap_year_cnt = (ltime.year + 1) / 4;

    utc = ltime.year * SECONDS_365_DAY + leap_year_cnt * SECONDS_PER_DAY;

    if ((ltime.year + 2) % 4 == 0)
        month_seconds[1] = 29 * SECONDS_PER_DAY;    /* leap year */
    else
        month_seconds[1] = 28 * SECONDS_PER_DAY;    /* non-leap year */

    for (i = 0; i < ltime.mon - 1; i++)
        utc += month_seconds[i];

    utc += (ltime.day - 1) * SECONDS_PER_DAY;

    utc += ltime.hour * SECONDS_PER_HOUR + ltime.min * 60 + ltime.sec;

    return utc;
}


void  sysUTC_To_DOS_Time(UINT32 utc, DateTime_T *dos_tm)
{
    int     the_year = 1970;
    int     i, seconds;

    while (1)
    {
        if (the_year % 4 == 0)
            seconds = SECONDS_366_DAY;
        else
            seconds = SECONDS_365_DAY;
        if (utc >= seconds)
        {
            utc -= seconds;
            the_year++;
        }
        else
            break;
    }

    dos_tm->year = the_year;

    if (the_year % 4 == 0)
        month_seconds[1] = 29 * SECONDS_PER_DAY;
    else
        month_seconds[1] = 28 * SECONDS_PER_DAY;

    dos_tm->mon = 1;
    for (i = 0; i < 11; i++)
    {
        if (utc >= month_seconds[i])
        {
            utc -= month_seconds[i];
            dos_tm->mon++;
        }
        else
            break;
    }

    dos_tm->day = 1 + (utc / SECONDS_PER_DAY);
    utc %= SECONDS_PER_DAY;

    dos_tm->hour = utc / SECONDS_PER_HOUR;
    utc %= SECONDS_PER_HOUR;

    dos_tm->min = utc / 60;
    dos_tm->sec = utc % 60;
}


void sysSetLocalTime(DateTime_T ltime)
{
    _sys_ReferenceTime_Clock = _sys_uTimer0Count;
    _sys_ReferenceTime_UTC = sysDOS_Time_To_UTC(ltime);
}

void sysGetCurrentTime(DateTime_T *curTime)
{
    UINT32 clock, utc_time;

    clock = _sys_uTimer0Count;
    utc_time = _sys_ReferenceTime_UTC + (clock - _sys_ReferenceTime_Clock) / _sys_uTimer0TickPerSecond;

    sysUTC_To_DOS_Time(utc_time, curTime);
}

/// @endcond HIDDEN_SYMBOLS

/**
 *  @brief  Use Timer0 to do system tick delay
 *
 *  @param[in]  uTicks  delay ticks
 *
 *  @return   None
 */
void sysDelay(UINT32 uTicks)
{
    UINT32 volatile btime;

    if(!_sys_bIsTimer0Initial)
    {
        sysStartTimer(TIMER0, 100, PERIODIC_MODE);
    }

    btime = sysGetTicks(TIMER0);
    while(1)
    {
        if((sysGetTicks(TIMER0) - btime) > uTicks)
        {
            break;
        }
    }
}

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
