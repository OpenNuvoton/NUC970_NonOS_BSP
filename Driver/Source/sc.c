/**************************************************************************//**
 * @file     sc.c
 * @version  V3.00
 * $Revision: 1 $
 * $Date: 15/06/11 7:20p $
 * @brief    NUC970 series Smartcard(SC) driver source file
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "nuc970.h"
#include "sc.h"

// Below are variables used locally by SC driver and does not want to parse by doxygen unless HIDDEN_SYMBOLS is defined
/// @cond HIDDEN_SYMBOLS
static uint32_t u32CardStateIgnore[SC_INTERFACE_NUM] = {0, 0};

/// @endcond HIDDEN_SYMBOLS

/** @addtogroup NUC970_Device_Driver NUC970 Device Driver
  @{
*/

/** @addtogroup NUC970_SC_Driver SC Driver
  @{
*/


/** @addtogroup NUC970_SC_EXPORTED_FUNCTIONS SC Exported Functions
  @{
*/

/**
  * @brief This function indicates specified smartcard slot status.
  * @param[in] sc Smartcard module number
  * @retval TRUE Card insert.
  * @retval FALSE Card remove.
  * @details This function is used to check if specified smart card slot is presented.
  */
UINT SC_IsCardInserted(UINT sc)
{
    uint32_t cond1;
    uint32_t cond2;

    if(sc == 0) {
        cond1 = (inpw(REG_SC0_STATUS) & 0x2000) >> 13;
        cond2 = (inpw(REG_SC0_CTL) & 0x4000000) >> 26;
    } else {
        cond1 = (inpw(REG_SC1_STATUS) & 0x2000) >> 13;
        cond2 = (inpw(REG_SC1_CTL) & 0x4000000) >> 26;
    }

    if(sc == 0 && u32CardStateIgnore[0] == 1)
        return TRUE;
    else if(sc == 1 && u32CardStateIgnore[1] == 1)
        return TRUE;
    else if(cond1 != cond2)
        return FALSE;
    else
        return TRUE;
}

/**
  * @brief Reset the Tx/Rx FIFO.
  * @param[in] sc Smartcard module number
  * @return None
  * @details This function reset both transmit and receive FIFO of specified smartcard module.
  */
void SC_ClearFIFO(UINT sc)
{
    if(sc == 0)
        outpw(REG_SC0_ALTCTL, inpw(REG_SC0_ALTCTL) | 0x3);
    else
        outpw(REG_SC1_ALTCTL, inpw(REG_SC1_ALTCTL) | 0x3);
}

/**
  * @brief This function disable specified smartcard module.
  * @param[in] sc Smartcard module number
  * @return None
  * @details SC will force all transition to IDLE state.
  */
void SC_Close(UINT sc)
{
    if(sc == 0) {
        outpw(REG_SC0_INTEN, 0);
        outpw(REG_SC0_PINCTL, 0);
        outpw(REG_SC0_ALTCTL, 0);
        outpw(REG_SC0_CTL, 0);
    } else {
        outpw(REG_SC1_INTEN, 0);
        outpw(REG_SC1_PINCTL, 0);
        outpw(REG_SC1_ALTCTL, 0);
        outpw(REG_SC1_CTL, 0);
    }
}

/**
  * @brief This function initialized smartcard module.
  * @param[in] sc Smartcard module number
  * @param[in] u32CD Card detect polarity, select the CD pin state which indicates card absent. Could be:
  *                 -\ref SC_PIN_STATE_HIGH.
  *                 -\ref SC_PIN_STATE_LOW.
  *                 -\ref SC_PIN_STATE_IGNORE, no card detect pin, always assumes card present.
  * @param[in] u32PWR Power off polarity, select the PWR pin state which could set smartcard VCC to high level. Could be:
  *                 -\ref SC_PIN_STATE_HIGH.
  *                 -\ref SC_PIN_STATE_LOW.
  * @return None
  * @details Initialization process configures smartcard and enables engine clock.
  */
void SC_Open(UINT sc, UINT u32CD, UINT u32PWR)
{
    uint32_t u32Reg = 0;

    if(u32CD != SC_PIN_STATE_IGNORE) {
        u32Reg = u32CD ? 0: SC_CTL_CDLV_Msk;
        u32CardStateIgnore[sc] = 0;
    } else {
        u32CardStateIgnore[sc] = 1;
    }
    if(sc == 0) {
        while(inpw(REG_SC0_PINCTL) & SC_PINCTL_SYNC_Msk);
        outpw(REG_SC0_PINCTL, u32PWR ? 0 : SC_PINCTL_PWRINV_Msk);
        while(inpw(REG_SC0_CTL) & SC_CTL_SYNC_Msk);
        outpw(REG_SC0_CTL, SC_CTL_SCEN_Msk | u32Reg);
    } else {
        while(inpw(REG_SC1_PINCTL) & SC_PINCTL_SYNC_Msk);
        outpw(REG_SC1_PINCTL, u32PWR ? 0 : SC_PINCTL_PWRINV_Msk);
        while(inpw(REG_SC1_CTL) & SC_CTL_SYNC_Msk);
        outpw(REG_SC1_CTL, SC_CTL_SCEN_Msk | u32Reg);
    }
}

/**
  * @brief This function reset specified smartcard module to its default state for activate smartcard.
  * @param[in] sc Smartcard module number
  * @return None
  * @details Reset the Tx/Rx FIFO & clock & initial default parameter.
  */
void SC_ResetReader(UINT sc)
{
    if(sc == 0) {
        // Reset FIFO, enable auto de-activation while card removal
        outpw(REG_SC0_ALTCTL, inpw(REG_SC0_ALTCTL) | SC_ALTCTL_TXRST_Msk | SC_ALTCTL_RXRST_Msk | SC_ALTCTL_ADACEN_Msk);
        // Set Rx trigger level to 1 character, longest card detect debounce period, disable error retry (EMV ATR does not use error retry)
        while(inpw(REG_SC0_CTL) & SC_CTL_SYNC_Msk);
        // Enable auto convention, and all three smartcard internal timers
        outpw(REG_SC0_CTL, (inpw(REG_SC0_CTL) & ~(SC_CTL_RXTRGLV_Msk | SC_CTL_CDDBSEL_Msk | SC_CTL_TXRTY_Msk | SC_CTL_TXRTYEN_Msk | SC_CTL_RXRTY_Msk | SC_CTL_RXRTYEN_Msk)) | SC_CTL_AUTOCEN_Msk | SC_CTL_TMRSEL_Msk);

        // Disable Rx timeout
        outpw(REG_SC0_RXTOUT, 0);
        // 372 clocks per ETU by default
        outpw(REG_SC0_ETUCTL, 371);

        /* Enable necessary interrupt for smartcard operation */
        if(u32CardStateIgnore[0]) // Do not enable card detect interrupt if card present state ignore
            outpw(REG_SC0_INTEN, SC_INTEN_RDAIEN_Msk |
                  SC_INTEN_TERRIEN_Msk |
                  SC_INTEN_TMR0IEN_Msk |
                  SC_INTEN_TMR1IEN_Msk |
                  SC_INTEN_TMR2IEN_Msk |
                  SC_INTEN_BGTIEN_Msk |
                  SC_INTEN_ACERRIEN_Msk);
        else
            outpw(REG_SC0_INTEN, SC_INTEN_RDAIEN_Msk |
                  SC_INTEN_TERRIEN_Msk |
                  SC_INTEN_TMR0IEN_Msk |
                  SC_INTEN_TMR1IEN_Msk |
                  SC_INTEN_TMR2IEN_Msk |
                  SC_INTEN_BGTIEN_Msk |
                  SC_INTEN_ACERRIEN_Msk |
                  SC_INTEN_CDIEN_Msk);
    } else {
        // Reset FIFO, enable auto de-activation while card removal
        outpw(REG_SC1_ALTCTL, inpw(REG_SC1_ALTCTL) | SC_ALTCTL_TXRST_Msk | SC_ALTCTL_RXRST_Msk | SC_ALTCTL_ADACEN_Msk);
        // Set Rx trigger level to 1 character, longest card detect debounce period, disable error retry (EMV ATR does not use error retry)
        while(inpw(REG_SC1_CTL) & SC_CTL_SYNC_Msk);
        // Enable auto convention, and all three smartcard internal timers
        outpw(REG_SC1_CTL, (inpw(REG_SC1_CTL) & ~(SC_CTL_RXTRGLV_Msk | SC_CTL_CDDBSEL_Msk | SC_CTL_TXRTY_Msk | SC_CTL_RXRTY_Msk)) | SC_CTL_AUTOCEN_Msk | SC_CTL_TMRSEL_Msk);

        // Disable Rx timeout
        outpw(REG_SC1_RXTOUT, 0);
        // 372 clocks per ETU by default
        outpw(REG_SC1_ETUCTL, 371);

        /* Enable necessary interrupt for smartcard operation */
        if(u32CardStateIgnore[1]) // Do not enable card detect interrupt if card present state ignore
            outpw(REG_SC1_INTEN, SC_INTEN_RDAIEN_Msk |
                  SC_INTEN_TERRIEN_Msk |
                  SC_INTEN_TMR0IEN_Msk |
                  SC_INTEN_TMR1IEN_Msk |
                  SC_INTEN_TMR2IEN_Msk |
                  SC_INTEN_BGTIEN_Msk |
                  SC_INTEN_ACERRIEN_Msk);
        else
            outpw(REG_SC1_INTEN, SC_INTEN_RDAIEN_Msk |
                  SC_INTEN_TERRIEN_Msk |
                  SC_INTEN_TMR0IEN_Msk |
                  SC_INTEN_TMR1IEN_Msk |
                  SC_INTEN_TMR2IEN_Msk |
                  SC_INTEN_BGTIEN_Msk |
                  SC_INTEN_ACERRIEN_Msk |
                  SC_INTEN_CDIEN_Msk);
    }
    return;
}

/**
  * @brief Set Block Guard Time.
  * @param[in] sc Smartcard module number
  * @param[in] u32BGT Block guard time using ETU as unit, valid range are between 1 ~ 32.
  * @return None
  * @details This function block guard time (BGT) of specified smartcard module.
  */
void SC_SetBlockGuardTime(UINT sc, UINT u32BGT)
{
    if(sc == 0)
        outpw(REG_SC0_CTL, (inpw(REG_SC0_CTL) & ~0x1F00) | ((u32BGT - 1) << 8));
    else
        outpw(REG_SC1_CTL, (inpw(REG_SC1_CTL) & ~0x1F00) | ((u32BGT - 1) << 8));
}

/**
  * @brief Set character guard time.
  * @param[in] sc Smartcard module number
  * @param[in] u32CGT Character guard time using ETU as unit, valid range are between 11 ~ 267.
  * @return None
  * @details This function character guard time (CGT) of specified smartcard module.
  */
void SC_SetCharGuardTime(UINT sc, UINT u32CGT)
{
    if(sc == 0) {
        u32CGT -= inpw(REG_SC0_CTL) & SC_CTL_NSB_Msk ? 11 : 12;
        outpw(REG_SC0_EGT, u32CGT);
    } else {
        u32CGT -= inpw(REG_SC1_CTL) & SC_CTL_NSB_Msk ? 11 : 12;
        outpw(REG_SC1_EGT, u32CGT);
    }
}

/**
  * @brief Stop all Timer counting.
  * @param[in] sc Smartcard module number
  * @return None
  * @details This function stop all smartcard timer of specified smartcard module.
  * @note This function stop the timers within smartcard module, \b not timer module.
  */
void SC_StopAllTimer(UINT sc)
{
    if(sc == 0)
        outpw(REG_SC0_ALTCTL, inpw(REG_SC0_ALTCTL) & ~(SC_ALTCTL_CNTEN0_Msk | SC_ALTCTL_CNTEN1_Msk | SC_ALTCTL_CNTEN2_Msk));
    else
        outpw(REG_SC1_ALTCTL, inpw(REG_SC1_ALTCTL) & ~(SC_ALTCTL_CNTEN0_Msk | SC_ALTCTL_CNTEN1_Msk | SC_ALTCTL_CNTEN2_Msk));
}

/**
  * @brief This function configure and start a smartcard timer of specified smartcard module.
  * @param[in] sc Smartcard module number
  * @param[in] u32TimerNum Timer(s) to start. Valid values are 0, 1, 2.
  * @param[in] u32Mode Timer operating mode, valid values are:
  *             - \ref SC_TMR_MODE_0
  *             - \ref SC_TMR_MODE_1
  *             - \ref SC_TMR_MODE_2
  *             - \ref SC_TMR_MODE_3
  *             - \ref SC_TMR_MODE_4
  *             - \ref SC_TMR_MODE_5
  *             - \ref SC_TMR_MODE_6
  *             - \ref SC_TMR_MODE_7
  *             - \ref SC_TMR_MODE_8
  *             - \ref SC_TMR_MODE_F
  * @param[in] u32ETUCount Timer timeout duration, ETU based. For timer 0, valid  range are between 1~0x1000000ETUs.
  *                        For timer 1 and timer 2, valid range are between 1 ~ 0x100 ETUs.
  * @return None
  * @details Enable Timer starting, counter will count when condition match.
  * @note This function start the timer within smartcard module, \b not timer module.
  * @note Depend on the timer operating mode, timer may not start counting immediately.
  */
void SC_StartTimer(UINT sc, UINT u32TimerNum, UINT u32Mode, UINT u32ETUCount)
{
    uint32_t reg = u32Mode | (0xFFFFFF & (u32ETUCount - 1));

    if(sc == 0) {
        if(u32TimerNum == 0) {
            outpw(REG_SC0_TMRCTL0, reg);
            outpw(REG_SC0_ALTCTL, inpw(REG_SC0_ALTCTL) | SC_ALTCTL_CNTEN0_Msk);
        } else if(u32TimerNum == 1) {
            outpw(REG_SC0_TMRCTL1, reg);
            outpw(REG_SC0_ALTCTL, inpw(REG_SC0_ALTCTL) | SC_ALTCTL_CNTEN1_Msk);
        } else {   // timer 2
            outpw(REG_SC0_TMRCTL2, reg);
            outpw(REG_SC0_ALTCTL, inpw(REG_SC0_ALTCTL) | SC_ALTCTL_CNTEN2_Msk);
        }
    } else {
        if(u32TimerNum == 0) {
            outpw(REG_SC1_TMRCTL0, reg);
            outpw(REG_SC1_ALTCTL, inpw(REG_SC1_ALTCTL) | SC_ALTCTL_CNTEN0_Msk);
        } else if(u32TimerNum == 1) {
            outpw(REG_SC1_TMRCTL1, reg);
            outpw(REG_SC1_ALTCTL, inpw(REG_SC1_ALTCTL) | SC_ALTCTL_CNTEN1_Msk);
        } else {   // timer 2
            outpw(REG_SC1_TMRCTL2, reg);
            outpw(REG_SC1_ALTCTL, inpw(REG_SC1_ALTCTL) | SC_ALTCTL_CNTEN2_Msk);
        }

    }
}

/**
  * @brief Stop Timer counting.
  * @param[in] sc Smartcard module number
  * @param[in] u32TimerNum Timer(s) to stop. Valid values are 0, 1, 2.
  * @return None
  * @details This function stop a smartcard timer of specified smartcard module.
  * @note This function stop the timer within smartcard module, \b not timer module.
  */
void SC_StopTimer(UINT sc, UINT u32TimerNum)
{
    if(sc == 0) {
        if(u32TimerNum == 0)
            outpw(REG_SC0_ALTCTL, inpw(REG_SC0_ALTCTL) & ~SC_ALTCTL_CNTEN0_Msk);
        else if(u32TimerNum == 1)
            outpw(REG_SC0_ALTCTL, inpw(REG_SC0_ALTCTL) & ~SC_ALTCTL_CNTEN1_Msk);
        else    // timer 2
            outpw(REG_SC0_ALTCTL, inpw(REG_SC0_ALTCTL) & ~SC_ALTCTL_CNTEN2_Msk);

    } else {
        if(u32TimerNum == 0)
            outpw(REG_SC1_ALTCTL, inpw(REG_SC1_ALTCTL) & ~SC_ALTCTL_CNTEN0_Msk);
        else if(u32TimerNum == 1)
            outpw(REG_SC1_ALTCTL, inpw(REG_SC1_ALTCTL) & ~SC_ALTCTL_CNTEN1_Msk);
        else    // timer 2
            outpw(REG_SC1_ALTCTL, inpw(REG_SC1_ALTCTL) & ~SC_ALTCTL_CNTEN2_Msk);
    }
}


/*@}*/ /* end of group NUC970_SC_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC970_SC_Driver */

/*@}*/ /* end of group NUC970_Device_Driver */

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
