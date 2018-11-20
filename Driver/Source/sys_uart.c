/**************************************************************************//**
* @file     sys_uart.c
* @version  V1.00
* $Revision: 5 $
* $Date: 15/05/18 4:02p $
* @brief    NUC970 SYS UART driver source file
*
* @note
* Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <string.h>
#include "nuc970.h"
#include "sys.h"

/// @cond HIDDEN_SYMBOLS
#define vaStart(list, param) list = (INT8*)((INT)&param + sizeof(param))
#define vaArg(list, type) ((type *)(list += sizeof(type)))[-1]

void _PutChar_f(UINT8 ch)
{
    volatile int loop;
    while ((inpw(REG_UART0_FSR) & (1<<23))); //waits for TX_FULL bit is clear
    outpw(REG_UART0_THR, ch);
    if(ch == '\n')
    {
        while((inpw(REG_UART0_FSR) & (1<<23))); //waits for TX_FULL bit is clear
        outpw(REG_UART0_THR, '\r');
    }
}


void sysPutString(INT8 *string)
{
    while (*string != '\0')
    {
        _PutChar_f(*string);
        string++;
    }
}


static void sysPutRepChar(INT8 c, INT count)
{
    while (count--)
        _PutChar_f(c);
}


static void sysPutStringReverse(INT8 *s, INT index)
{
    while ((index--) > 0)
        _PutChar_f(s[index]);
}


static void sysPutNumber(INT value, INT radix, INT width, INT8 fill)
{
    INT8    buffer[40];
    INT     bi = 0;
    UINT32  uvalue;
    UINT16  digit;
    UINT16  left = FALSE;
    UINT16  negative = FALSE;

    if (fill == 0)
        fill = ' ';

    if (width < 0)
    {
        width = -width;
        left = TRUE;
    }

    if (width < 0 || width > 80)
        width = 0;

    if (radix < 0)
    {
        radix = -radix;
        if (value < 0)
        {
            negative = TRUE;
            value = -value;
        }
    }

    uvalue = value;

    do
    {
        if (radix != 16)
        {
            digit = uvalue % radix;
            uvalue = uvalue / radix;
        }
        else
        {
            digit = uvalue & 0xf;
            uvalue = uvalue >> 4;
        }
        buffer[bi] = digit + ((digit <= 9) ? '0' : ('A' - 10));
        bi++;

        if (uvalue != 0)
        {
            if ((radix == 10)
                && ((bi == 3) || (bi == 7) || (bi == 11) | (bi == 15)))
            {
                buffer[bi++] = ',';
            }
        }
    }
    while (uvalue != 0);

    if (negative)
    {
        buffer[bi] = '-';
        bi += 1;
    }

    if (width <= bi)
        sysPutStringReverse(buffer, bi);
    else
    {
        width -= bi;
        if (!left)
            sysPutRepChar(fill, width);
        sysPutStringReverse(buffer, bi);
        if (left)
            sysPutRepChar(fill, width);
    }
}


static INT8 *FormatItem(INT8 *f, INT a)
{
    INT8   c;
    INT    fieldwidth = 0;
    INT    leftjust = FALSE;
    INT    radix = 0;
    INT8   fill = ' ';

    if (*f == '0')
        fill = '0';

    while ((c = *f++) != 0)
    {
        if (c >= '0' && c <= '9')
        {
            fieldwidth = (fieldwidth * 10) + (c - '0');
        }
        else if (c == 'l')
            continue;
        else
            switch (c)
            {
                case '\000':
                    return (--f);
                case '%':
                    _PutChar_f('%');
                    return (f);
                case '-':
                    leftjust = TRUE;
                    break;
                case 'c':
                    {
                        if (leftjust)
                            _PutChar_f(a & 0x7f);

                        if (fieldwidth > 0)
                            sysPutRepChar(fill, fieldwidth - 1);

                        if (!leftjust)
                            _PutChar_f(a & 0x7f);
                        return (f);
                    }
                case 's':
                    {
                        if (leftjust)
                            sysPutString((PINT8)a);

                        if (fieldwidth > strlen((PINT8)a))
                            sysPutRepChar(fill, fieldwidth - strlen((PINT8)a));

                        if (!leftjust)
                            sysPutString((PINT8)a);
                        return (f);
                    }
                case 'd':
                case 'i':
                    radix = -10;
                    break;
                case 'u':
                    radix = 10;
                    break;
                case 'x':
                    radix = 16;
                    break;
                case 'X':
                    radix = 16;
                    break;
                case 'o':
                    radix = 8;
                    break;
                default:
                    radix = 3;
                    break;      /* unknown switch! */
            }
        if (radix)
            break;
    }

    if (leftjust)
        fieldwidth = -fieldwidth;

    sysPutNumber(a, radix, fieldwidth, fill);

    return (f);
}


INT  sysIsKbHit()
{
    if (inpw(REG_UART0_FSR) & (1 << 14))
        return 0;
    else 
        return 1;
}
/// @endcond HIDDEN_SYMBOLS


/**
 *  @brief  UART initialize
 *
 *  @return   0
 */
INT32 sysInitializeUART(void)
{
    /* enable UART0 clock */
    outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 0x10000);

    /* GPE0, GPE1 */
    outpw(REG_SYS_GPE_MFPL, (inpw(REG_SYS_GPE_MFPL) & 0xffffff00) | 0x99);  // UART0 multi-function

    /* UART0 line configuration for (115200,n,8,1) */
    outpw(REG_UART0_LCR, inpw(REG_UART0_LCR) | 0x7);
    outpw(REG_UART0_BAUD, 0x30000066); /* 12MHz reference clock input, 115200 */
    return 0;
}


/**
 *  @brief  system UART printf
 *
 *  @param[in]  pcStr    output string
 *
 *  @return   None
 */
void sysprintf(PINT8 pcStr,...)
{
    INT8  *argP;

    vaStart(argP, pcStr);       /* point at the end of the format string */
    while (*pcStr)
    {                       /* this works because args are all ints */
        if (*pcStr == '%')
            pcStr = FormatItem(pcStr + 1, vaArg(argP, INT));
        else
            _PutChar_f(*pcStr++);
    }
}

/**
 *  @brief  system UART get char
 *
 *  @return   input char
 */
INT8 sysGetChar(void)
{
    int i;
    while (1)
    {
        for(i=0;i<0x1000;i++);
        if (!(inpw(REG_UART0_FSR) & (1 << 14)))
        {
            return (inpw(REG_UART0_RBR));
        }
    }
}

/**
 *  @brief  system UART put char
 *
 *  @param[in]  ucCh    output char
 *
 *  @return   None
 */
void sysPutChar(UINT8 ucCh)
{
    volatile int loop;
    while ((inpw(REG_UART0_FSR) & (1<<23))); //waits for TX_FULL bit is clear
    outpw(REG_UART0_THR, ucCh);
}

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
