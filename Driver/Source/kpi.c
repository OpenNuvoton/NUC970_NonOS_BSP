/**************************************************************************//**
* @file     kpi.c
* @version  V1.00
* $Revision: 2 $
* $Date: 15/06/12 9:06a $
* @brief    NUC970 KPI driver source file
*
* @note
* Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include "nuc970.h"
#include "sys.h"
#include "kpi.h"

/** @addtogroup NUC970_Device_Driver NUC970 Device Driver
  @{
*/

/** @addtogroup NUC970_KPI_Driver KPI Driver
  @{
*/

/** @addtogroup NUC970_KPI_EXPORTED_FUNCTIONS KPI Exported Functions
  @{
*/

/** @addtogroup NUC970_KPI_EXPORTED_CONSTANTS KPI Exported Constants
  @{
*/

/*@}*/ /* end of group NUC970_KPI_EXPORTED_CONSTANTS */

/// @cond HIDDEN_SYMBOLS

#define KPICONF_VALUE   0x00288059 //0x003E20FA

#define NOKEY           0
#define MAX_KPI_BUFFER_SIZE     10
#define KPI_QUE_SIZE            (MAX_KPI_BUFFER_SIZE+1)

//Internal function definition
void KPI_ISR(void);
//static void KPI_Enque(UINT uValue);
static void KPI_Enque(UINT32 u32Status, UINT32 uValue);
//static UINT KPI_Deque(void);
static UINT KPI_Deque(PUCHAR pucKeyStatus, PUCHAR pucKey);
static BOOL KPI_CheckFullQue(void);
static BOOL KPI_CheckEmptyQue(void);

//Global variable
static INT volatile nKPIBufferRear = 0;  //Queue parameter
static INT volatile nKPIBufferFront = 0; //Queue parameter
static KPISTATUS_union kpiBuffer[KPI_QUE_SIZE]; // kpi buffer(queue)
static KPIKEY_union kpiKeyBuffer[KPI_QUE_SIZE]; // kpi key buffer(queue)
static BOOL bIskpiOpen=FALSE;

/// @endcond HIDDEN_SYMBOLS


/**
  * @brief   Init KPI and install KPI ISR.
  *
  * @param[in]    u8Row:   KPI row number
  * @param[in]    u8Col:   KPI colnum number
  * @param[in]    u8Press: enable key press detect function
  * @param[in]    u8Release: enable key release detect function
  *
  * @return   Successful:  Success  
  *
  */
INT KPI_Init(UINT8 u8Row, UINT8 u8Col, UINT8 u8Press, UINT8 u8Release)
{
    UINT32 uReg;
    uReg=inpw(REG_CLK_PCLKEN1)|1<<25;
    outpw(REG_CLK_PCLKEN1,uReg); // enable KPI clock
	uReg=inpw(REG_CLK_PCLKEN0)|1<<3;
	outpw(REG_CLK_PCLKEN0,uReg); // enable GPIO clock
     
    //Reset all register
    outpw(REG_KPI_CONF, 0);
    outpw(REG_KPI_3KCONF, 0);
    outpw(REG_KPI_RSTC, 0);

    //Set kpi conf default value
	if(u8Row < 2)
		u8Row = 1;
	else if (u8Row > 4)
		u8Row = 3;
	else 
		u8Row = u8Row - 1;
	
	if(u8Col < 2)
		u8Col = 1;
	else if(u8Col > 7)
		u8Col = 0;
	else
		u8Col = u8Col - 1;
	
	uReg = (KPICONF_VALUE | (u8Row << 28) | (u8Col << 24) | (u8Press << 1) | (u8Release << 2));
    outpw(REG_KPI_CONF, uReg);

	sysInstallISR(IRQ_LEVEL_1, KPI_IRQn, (PVOID)KPI_ISR);
	sysSetLocalInterrupt(ENABLE_IRQ);
	
	return Successful;
}

/**
  * @brief   Open KPI function.
  *
  * @return   Successful:  Success  \n
  *           kpiBusy:  KPI function already open
  *
  */
INT KPI_Open(void)
{
    if(bIskpiOpen==TRUE)
    {
        return kpiBusy;
    }
    sysEnableInterrupt(KPI_IRQn);
    bIskpiOpen=TRUE;
    nKPIBufferFront = nKPIBufferRear = 0;
 
    return Successful;
}

/**
  * @brief   Close KPI function.
  *
  * @return   Successful:  Success   \n
  *           kpiNotOpen:  KPI function not open
  */
INT KPI_Close(void)
{
    if(bIskpiOpen==FALSE)
    {
        return kpiNotOpen;
    }
    sysDisableInterrupt(KPI_IRQn);
    bIskpiOpen=FALSE;
    return Successful;
}


/**
  * @brief   Read KPI key Status.
  *
  * @param[in]    pucKeyStatus:   The point of struct KPISTATUS_union
  * @param[in]    pucKey:         The point of struct KPIKEY_union
  * @param[in]    uReadMode:      BLOCK_MODE / NONBLOCK_MODE
  *
  * @return   Successful:        Success   \n
  *           kpiReadModeError:  read mode error  \n
  *           kpiNoKey:          queue is empty, no key press
  */
INT KPI_Read(PUCHAR pucKeyStatus, PUCHAR pucKey, UINT uReadMode)
{
    if(bIskpiOpen==FALSE)
    {
        return kpiNotOpen;
    }
    switch(uReadMode)
    {
        case BLOCK_MODE:
        {
            while(KPI_CheckEmptyQue() == TRUE);
			//((KPIKEY_union *)pucKey)->value=KPI_Deque();
			KPI_Deque(pucKeyStatus, pucKey);
            break;
        }
        case NONBLOCK_MODE:
        {
			//((KPIKEY_union *)pucKey)->value=KPI_Deque();
            KPI_Deque(pucKeyStatus, pucKey);
            break;
        }
        default:
        {
            return kpiReadModeError;
            //break;
        }
    }
    if(((KPISTATUS_union *)pucKey)->value == NOKEY)
    {
        return kpiNoKey;
    }
    return Successful;
}


/**
  * @brief    The ioctl function of KPI device library.
  *
  * @param[in]    uCommand:     Ioctl command which indicates different operation
  * @param[in]    uIndication:  Parameter for future usage
  * @param[in]    uValue:      The new value which should be writed to register
  *
  * @return   Successful:    Success    \n
  *           kpiNotOpen:    KPI not open  \n
  *           kpiInvalidIoctlCommand: Ioctl command error
  */
INT KPI_Ioctl(UINT uCommand, UINT uIndication, UINT uValue)
{
    //UINT32 nRegisterValue=0;
    if(bIskpiOpen==FALSE)
    {
        return kpiNotOpen;
    }
    switch(uCommand)
    {
        case SET_KPICONF:
        {
            outpw(REG_KPI_CONF, uValue);
            break;
        }
        case SET_KPI3KCONF:
        {
            outpw(REG_KPI_3KCONF, uValue);
            break;
        }
        case SET_KPIRSTC:
        {
            outpw(REG_KPI_RSTC, uValue);
            break;
        }
		case CLEAN_KPIKPE:
		{
			outpw(REG_KPI_KPE, uValue);
			break;
		}
		case CLEAN_KPIKRE:
		{
			outpw(REG_KPI_KRE, uValue);
			break;
		}
		case SET_PRESCALDIV:
		{
			outpw(REG_KPI_PRESCALDIV, uValue);
			break;
		}
        case CLEAN_KPI_BUFFER:
        {
            nKPIBufferFront = nKPIBufferRear = 0;
            break;
        }
        case CONTINUOUS_MODE:
        {
			UINT32 volatile btime;
			UINT32 uReg;
			
			uReg = inpw(REG_KPI_CONF);
			
    		// Set KPI prescale = 0
		    outpw(REG_KPI_CONF, inpw(REG_KPI_CONF)&0xFFFF00FF);

	    	if(uValue == 0)
	    	{
	    		uValue = 1; // Delay at least  10 ms (should be 20us for KPI scan timing)
	    	}

	    	// Delay for KPI scan
	    	//sysDelay( uValue);
			sysStartTimer(TIMER0, 1, PERIODIC_MODE);
		    btime = sysGetTicks(TIMER0);
		    while (1)
		    {
		        if ((sysGetTicks(TIMER0) - btime) >uValue)
		        {
		            break;
		        }
		    }
	    	// Restore prescale value
		    outpw(REG_KPI_CONF, uReg);

        	break;
        }
        default:
        {
            return kpiInvalidIoctlCommand;
            //break;
        }
    }
    return Successful; 
}

/*@}*/ /* end of group NUC970_KPI_EXPORTED_FUNCTIONS */

/// @cond HIDDEN_SYMBOLS

/**
  * @brief    The interrupt service routines of KPI.
  *
  * @param[in]    None
  *
  * @return   None
  */
void KPI_ISR(void)
{
	UINT32 u32KPI_Status;
	u32KPI_Status = inpw(REG_KPI_STATUS);
	
	if(u32KPI_Status & (1 << 4)) // PKEY_INT
	{
		KPI_Enque(u32KPI_Status, ((UINT32)inpw(REG_KPI_KPE)) );
		outpw(REG_KPI_KPE, inpw(REG_KPI_KPE) );
	}
	else if(u32KPI_Status & (1 << 3)) // RKEY_INT
	{
		KPI_Enque(u32KPI_Status, ((UINT32)inpw(REG_KPI_KRE)) );
		outpw(REG_KPI_KRE, inpw(REG_KPI_KRE) );
	}
	else if(u32KPI_Status & (1 << 1)) // RST_3KEY
	{
		KPI_Enque(u32KPI_Status, ((UINT32)inpw(REG_KPI_KPE)) );
		outpw(REG_KPI_STATUS, (1 << 1) ); // clear status
	}
	
	#if 0
  	KPI_Enque((UINT)inpw(REG_KPISTATUS));
	#ifdef DEBUG_MODE
	sysprintf("ISR\n");
    #endif
	#endif
}


static void KPI_Enque(UINT32 u32Status, UINT32 uValue)
{
    if(KPI_CheckFullQue() == FALSE)
    {
		kpiBuffer[nKPIBufferRear].value=u32Status;
		kpiKeyBuffer[nKPIBufferRear].value=uValue;
		nKPIBufferRear=(nKPIBufferRear+1)%KPI_QUE_SIZE;
    }
}


static UINT KPI_Deque(PUCHAR pucKeyStatus, PUCHAR pucKey)
{
    //UINT uValue;
    if(KPI_CheckEmptyQue() == FALSE)
    {
		((KPIKEY_union *)pucKey)->value = kpiKeyBuffer[nKPIBufferFront].value;
		((KPISTATUS_union *)pucKeyStatus)->value = kpiBuffer[nKPIBufferFront].value;
		
		#if 0
		uValue=kpiBuffer[nKPIBufferFront].value;
		#endif
		nKPIBufferFront=(nKPIBufferFront+1)%KPI_QUE_SIZE;
       //return uValue;
		return 1;
    }
	
	((KPIKEY_union *)pucKey)->value = 0;
	((KPISTATUS_union *)pucKeyStatus)->value = 0;
	
    return NOKEY; //queue is empty
}


static BOOL KPI_CheckFullQue(void)
{
    if((nKPIBufferRear+1)%KPI_QUE_SIZE == nKPIBufferFront)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


static BOOL KPI_CheckEmptyQue(void)
{
    if(nKPIBufferFront == nKPIBufferRear)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
/// @endcond HIDDEN_SYMBOLS



/*@}*/ /* end of group NUC970_KPI_Driver */

/*@}*/ /* end of group NUC970_Device_Driver */

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/


