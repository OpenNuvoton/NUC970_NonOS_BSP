/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 15/06/11 2:44p $
 * @brief    NUC970 Driver Sample Code
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "nuc970.h"
#include "sys.h"
#include "can.h"

#define PLLCON_SETTING      SYSCLK_PLLCON_50MHz_XTAL
#define PLL_CLOCK           50000000

//extern char GetChar(void);

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
STR_CANMSG_T rrMsg;

void CAN_ShowMsg(STR_CANMSG_T* Msg);

/*---------------------------------------------------------------------------------------------------------*/
/* ISR to handle CAN interrupt event                                                            */
/*---------------------------------------------------------------------------------------------------------*/
void CAN_MsgInterrupt(UINT32 uCAN, uint32_t u32IIDR)
{
	sysprintf("Msg-%d INT and Callback \n", (u32IIDR-1));
    CAN_Receive(uCAN, (u32IIDR-1),&rrMsg);
    CAN_ShowMsg(&rrMsg);
}


/**
  * @brief  CAN0_IRQ Handler.
  * @param  None.
  * @return None.
  */
void CAN0_IRQHandler(void)
{
    uint32_t u8IIDRstatus;

    u8IIDRstatus = inpw(REG_CAN0_IIDR);

    if(u8IIDRstatus == 0x00008000) {      /* Check Status Interrupt Flag (Error status Int and Status change Int) */
        /**************************/
        /* Status Change interrupt*/
        /**************************/
        if(inpw(REG_CAN0_STATUS) & CAN_STATUS_RXOK_Msk) {
            outpw(REG_CAN0_STATUS, inpw(REG_CAN0_STATUS) & ~CAN_STATUS_RXOK_Msk);  /* Clear Rx Ok status*/

            sysprintf("RX OK INT\n") ;
        }

        if(inpw(REG_CAN0_STATUS) & CAN_STATUS_TXOK_Msk) {  
            outpw(REG_CAN0_STATUS, inpw(REG_CAN0_STATUS) & ~CAN_STATUS_TXOK_Msk);  /* Clear Tx Ok status*/

            sysprintf("TX OK INT\n") ;
        }

        /**************************/
        /* Error Status interrupt */
        /**************************/
        if(inpw(REG_CAN0_STATUS) & CAN_STATUS_EWARN_Msk) {
            sysprintf("EWARN INT\n") ;
        }

        if(inpw(REG_CAN0_STATUS) & CAN_STATUS_BOFF_Msk) {
            sysprintf("BOFF INT\n") ;
        }
    } else if (u8IIDRstatus!=0) {
        sysprintf("=> Interrupt Pointer = %d\n", (u8IIDRstatus-1));

        CAN_MsgInterrupt(CAN0, u8IIDRstatus);

        CAN_CLR_INT_PENDING_BIT(CAN0, (u8IIDRstatus-1));      /* Clear Interrupt Pending */

    }else if(inpw(REG_CAN0_WU_STATUS) == 1) {
        sysprintf("Wake up\n");
               
        outpw(REG_CAN0_WU_STATUS, 0x0);  /* Write '0' to clear */
    }

}

/**
  * @brief  CAN1_IRQ Handler.
  * @param  None.
  * @return None.
  */
void CAN1_IRQHandler(void)
{
    uint32_t u8IIDRstatus;

    u8IIDRstatus = inpw(REG_CAN1_IIDR);

    if(u8IIDRstatus == 0x00008000) {      /* Check Status Interrupt Flag (Error status Int and Status change Int) */
        /**************************/
        /* Status Change interrupt*/
        /**************************/
        if(inpw(REG_CAN1_STATUS) & CAN_STATUS_RXOK_Msk) {  
            outpw(REG_CAN1_STATUS, inpw(REG_CAN1_STATUS) & ~CAN_STATUS_RXOK_Msk);  /* Clear Rx Ok status*/

            sysprintf("RX OK INT\n") ;
        }

        if(inpw(REG_CAN1_STATUS) & CAN_STATUS_TXOK_Msk) { 
            outpw(REG_CAN1_STATUS, inpw(REG_CAN1_STATUS) & ~CAN_STATUS_TXOK_Msk);  /* Clear Tx Ok status*/

            sysprintf("TX OK INT\n") ;
        }

        /**************************/
        /* Error Status interrupt */
        /**************************/
        if(inpw(REG_CAN1_STATUS) & CAN_STATUS_EWARN_Msk) {
            sysprintf("EWARN INT\n") ;
        }

        if(inpw(REG_CAN1_STATUS) & CAN_STATUS_BOFF_Msk) {
            sysprintf("BOFF INT\n") ;
        }
    } else if (u8IIDRstatus!=0) {
        sysprintf("=> Interrupt Pointer = %d\n", (u8IIDRstatus-1));

        CAN_MsgInterrupt(CAN1, u8IIDRstatus);

        CAN_CLR_INT_PENDING_BIT(CAN1, (u8IIDRstatus-1));      /* Clear Interrupt Pending */

    } else if(inpw(REG_CAN1_WU_STATUS) == 1) {
        sysprintf("Wake up\n");
                  
        outpw(REG_CAN1_WU_STATUS, 0x0);/* Write '0' to clear */
    }

}

/*----------------------------------------------------------------------------*/
/*  Some description about how to create test environment                     */
/*----------------------------------------------------------------------------*/
void Note_Configure()
{
    sysprintf("\n\n");
    sysprintf("+------------------------------------------------------------------------+\n");
    sysprintf("|  About CAN sample code configure                                       |\n");
    sysprintf("+------------------------------------------------------------------------+\n");
    sysprintf("|   The sample code provide a simple sample code for you study CAN       |\n");
    sysprintf("|   Before execute it, please check description as below                 |\n");
    sysprintf("|                                                                        |\n");
    sysprintf("|   1.CAN0 and CAN1 connect to the same CAN BUS                          |\n");
    sysprintf("|   2.Using UART0 as print message port(Both of NUC472/442 module boards)|\n");
    sysprintf("|                                                                        |\n");
    sysprintf("|  |--------|       |-----------|  CANBUS |-----------|       |--------| |\n");
    sysprintf("|  |        |------>|           |<------->|           |<------|        | |\n");
    sysprintf("|  |        |CAN0_TX|   CAN0    |  CAN1_H |   CAN1    |CAN1_TX|        | |\n");
    sysprintf("|  | NUC472 |       |Transceiver|         |Transceiver|       | NUC472 | |\n");
    sysprintf("|  | NUC442 |<------|           |<------->|           |------>| NUC442 | |\n");
    sysprintf("|  |        |CAN0_RX|           |  CAN1_L |           |CAN1_RX|        | |\n");
    sysprintf("|  |--------|       |-----------|         |-----------|       |--------| |\n");
    sysprintf("|   |                                                           |        |\n");
    sysprintf("|   |                                                           |        |\n");
    sysprintf("|   V                                                           V        |\n");
    sysprintf("| UART0                                                         UART0    |\n");
    sysprintf("|(print message)                                          (print message)|\n");
    sysprintf("+------------------------------------------------------------------------+\n");
}

/*----------------------------------------------------------------------------*/
/*  Test Function                                                             */
/*----------------------------------------------------------------------------*/
void CAN_ShowMsg(STR_CANMSG_T* Msg)
{
    uint8_t i;
    sysprintf("Read ID=%8X, Type=%s, DLC=%d,Data=",Msg->Id,Msg->IdType?"EXT":"STD",Msg->DLC);
    for(i=0; i<Msg->DLC; i++)
        sysprintf("%02X,",Msg->Data[i]);
    sysprintf("\n\n");
}

/*----------------------------------------------------------------------------*/
/*  Send Tx Msg by Normal Mode Function (With Message RAM)                    */
/*----------------------------------------------------------------------------*/
void Test_NormalMode_Tx(UINT32 uCAN)
{
    STR_CANMSG_T tMsg;
    uint32_t i;

	#if 1
    /* Send a 11-bits message */
    tMsg.FrameType= DATA_FRAME;
    tMsg.IdType   = CAN_STD_ID;
    tMsg.Id       = 0x7FF;
    tMsg.DLC      = 2;
    tMsg.Data[0]  = 7;
    tMsg.Data[1]  = 0xFF;

    if(CAN_Transmit(uCAN, MSG(0),&tMsg) == FALSE) { // Configure Msg RAM and send the Msg in the RAM
        sysprintf("Set Tx Msg Object failed\n");
        return;
    }

    sysprintf("MSG(0).Send STD_ID:0x7FF, Data[07,FF]done\n");
	#endif
	
    /* Send a 29-bits message */
    tMsg.FrameType= DATA_FRAME;
    tMsg.IdType   = CAN_EXT_ID;
    tMsg.Id       = 0x12345;
    tMsg.DLC      = 3;
    tMsg.Data[0]  = 1;
    tMsg.Data[1]  = 0x23;
    tMsg.Data[2]  = 0x45;

    if(CAN_Transmit(uCAN, MSG(1),&tMsg) == FALSE) {
        sysprintf("Set Tx Msg Object failed\n");
        return;
    }
	
    sysprintf("MSG(1).Send EXT:0x12345 ,Data[01,23,45]done\n");

    /* Send a data message */
    tMsg.FrameType= DATA_FRAME;
    tMsg.IdType   = CAN_EXT_ID;
    tMsg.Id       = 0x7FF01;
    tMsg.DLC      = 4;
    tMsg.Data[0]  = 0xA1;
    tMsg.Data[1]  = 0xB2;
    tMsg.Data[2]  = 0xC3;
    tMsg.Data[3]  = 0xD4;

    if(CAN_Transmit(uCAN, MSG(3),&tMsg) == FALSE) {
        sysprintf("Set Tx Msg Object failed\n");
        return;
    }

    sysprintf("MSG(3).Send EXT:0x7FF01 ,Data[A1,B2,C3,D4]done\n");

    for(i=0; i < 10000; i++);

    sysprintf("Trasmit Done!\nCheck the receive host received data\n\n");

}

/*----------------------------------------------------------------------------*/
/*  Receive Rx Msg by Normal Mode Function (With Message RAM)                    */
/*----------------------------------------------------------------------------*/
void Test_NormalMode_SetRxMsg(UINT32 uCAN)
{
    if(CAN_SetRxMsg(uCAN, MSG(0),CAN_STD_ID, 0x7FF) == FALSE) {
        sysprintf("Set Rx Msg Object failed\n");
        return;
    }

    if(CAN_SetRxMsg(uCAN, MSG(5),CAN_EXT_ID, 0x12345) == FALSE) {
        sysprintf("Set Rx Msg Object failed\n");
        return;
    }

    if(CAN_SetRxMsg(uCAN, MSG(31),CAN_EXT_ID, 0x7FF01) == FALSE) {
        sysprintf("Set Rx Msg Object failed\n");
        return;
    }

}

void Test_NormalMode_WaitRxMsg(UINT32 uCAN)
{
    /*Choose one mode to test*/
#if 1
	UINT32 uOffset = uCAN * CAN_OFFSET;
	
    /* Polling Mode */
    while(1) {
        while(inpw(REG_CAN0_IIDR+uOffset) == 0);            /* Wait IDR is changed */
        sysprintf("IDR = 0x%x\n",inpw(REG_CAN0_IIDR+uOffset));
        CAN_Receive(uCAN, (inpw(REG_CAN0_IIDR+uOffset) -1), &rrMsg);
        CAN_ShowMsg(&rrMsg);
    }
#else
    /* INT Mode */
    CAN_EnableInt(uCAN, CAN_CON_IE_Msk);

	if(uCAN == CAN0)
	{
	    sysInstallISR((IRQ_LEVEL_1 | HIGH_LEVEL_SENSITIVE), CAN0_IRQn, (PVOID)CAN0_IRQHandler);
	    sysSetLocalInterrupt(ENABLE_IRQ);							 /* enable CPSR I bit */
	    sysEnableInterrupt(CAN0_IRQn);
	}
	else if(uCAN == CAN1)
	{
		sysInstallISR((IRQ_LEVEL_1 | HIGH_LEVEL_SENSITIVE), CAN1_IRQn, (PVOID)CAN1_IRQHandler);
	    sysSetLocalInterrupt(ENABLE_IRQ);							 /* enable CPSR I bit */
	    sysEnableInterrupt(CAN1_IRQn);
	}
	
    sysprintf("Wait Msg\n");
    sysprintf("Enter any key to exit\n");
    sysGetChar();
#endif
}

int main()
{
    //SYS_Init();
    //UART0_Init();
	
	sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

	// CAN0
	outpw(REG_SYS_GPH_MFPL, (inpw(REG_SYS_GPH_MFPL) & 0xffff00ff) | 0xCC00 ); // GPH_2,GPH_3 // RX, TX

	//CAN_1
	outpw(REG_SYS_GPH_MFPH, (inpw(REG_SYS_GPH_MFPH) & 0x00ffffff) | 0xCC000000 ); // GPH_14,GPH_15 // RX, TX

	outpw(REG_CLK_PCLKEN1, (inpw(REG_CLK_PCLKEN1) | (1 << 8) | (1 << 9)) );
	
    Note_Configure();

    CAN_Open(CAN0,  500000, CAN_NORMAL_MODE);
    CAN_Open(CAN1,  500000, CAN_NORMAL_MODE);

    sysprintf("\n");
    sysprintf("+------------------------------------------------------------------ +\n");
    sysprintf("|  Nuvoton CAN BUS DRIVER DEMO                                      |\n");
    sysprintf("+-------------------------------------------------------------------+\n");
    sysprintf("|  Transmit/Receive a message by normal mode                        |\n");
    sysprintf("+-------------------------------------------------------------------+\n");

    sysprintf("Press any key to continue ...\n\n");
    sysGetChar();

    Test_NormalMode_SetRxMsg(CAN1);

    Test_NormalMode_Tx(CAN0);

    Test_NormalMode_WaitRxMsg(CAN1);

    while(1) ;

}



