/**************************************************************************//**
* @file     i2c.c
* @version  V1.00
* $Revision: 2 $
* $Date: 15/05/18 10:43a $
* @brief    NUC970 I2C driver source file
*
* @note
* Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "i2c.h"

/** @addtogroup NUC970_Device_Driver NUC970 Device Driver
  @{
*/

/** @addtogroup NUC970_I2C_Driver I2C Driver
  @{
*/

/** @addtogroup NUC970_I2C_EXPORTED_CONSTANTS I2C Exported Constants
  @{
*/

/// @cond HIDDEN_SYMBOLS

#define i2c_out(dev, byte, addr)        outpw((dev)->base + addr, byte)
#define i2c_in(dev, addr)               inpw((dev)->base + addr)

#define i2cDisable(dev)     i2c_out(dev, 0x00, I2C_CSR)  /* Disable i2c core and interrupt */
#define i2cEnable(dev)      i2c_out(dev, 0x03, I2C_CSR)  /* Enable i2c core and interrupt  */
#define i2cIsBusFree(dev) ((i2c_in(dev, I2C_SWR) & 0x18) == 0x18 && (i2c_in(dev, I2C_CSR) & 0x0400) == 0) ? 1 : 0

/*-----------------------------------------*/
/* global file scope (static) variables    */
/*-----------------------------------------*/
typedef struct
{
    int32_t base;       /* i2c bus number */
    int32_t openflag;
    volatile int32_t state;
    int32_t addr;
    uint32_t last_error;

    uint32_t subaddr;
    int32_t subaddr_len;

    uint8_t buffer[I2C_MAX_BUF_LEN];
    volatile uint32_t pos, len;

} i2c_dev;
/// @endcond /* HIDDEN_SYMBOLS */

/*@}*/ /* end of group NUC970_I2C_EXPORTED_CONSTANTS */

/** @addtogroup NUC970_I2C_EXPORTED_FUNCTIONS I2C Exported Functions
  @{
*/

/// @cond HIDDEN_SYMBOLS

static i2c_dev i2c_device[I2C_NUMBER];
static int32_t bNackValid;

/**
  * @brief  Set i2c interface speed
  * @param[in] dev i2c device structure pointer
  * @param[in] sp i2c speed
  * @return always 0
  */
static int32_t _i2cSetSpeed(i2c_dev *dev, int32_t sp)
{
    uint32_t d;

    if( sp != 100 && sp != 400)
        return(I2C_ERR_NOTTY);

    d = I2C_INPUT_CLOCK/(sp * 5) -1;

    i2c_out(dev, d & 0xffff, I2C_DIVIDER);

    return 0;
}

/**
  * @brief  Configure i2c command
  * @param[in] dev i2c device structure pointer
  * @param[in] cmd command
  * @return None
  */
static void _i2cCommand(i2c_dev *dev, int32_t cmd)
{
    bNackValid = (cmd & I2C_CMD_WRITE) ? 1 : 0;
    i2c_out(dev, cmd, I2C_CMDR);
}

/**
  * @brief Configure slave address data
  * @param[in] dev i2c device structure pointer
  * @param[in] mode could be write or read
  * @return None
  */
static void _i2cCalcAddr(i2c_dev *dev, int32_t mode)
{
    int32_t i;
    uint32_t subaddr;

    subaddr = dev->subaddr;

    dev->buffer[0] = (((dev->addr << 1) & 0xfe) | I2C_WRITE) & 0xff;

    for(i = dev->subaddr_len; i > 0; i--)
    {
        dev->buffer[i] = subaddr & 0xff;
        subaddr >>= 8;
    }

    if(mode == I2C_STATE_READ)
    {
        i = dev->subaddr_len + 1;
        dev->buffer[i] = (((dev->addr << 1) & 0xfe)) | I2C_READ;
    }
}

/**
  * @brief Reset some variables
  * @param[in] dev i2c device structure pointer
  * @return None
  */
static void _i2cReset(i2c_dev *dev)
{
    dev->addr = -1;
    dev->last_error = 0;
    dev->subaddr = 0;
    dev->subaddr_len = 0;
}

/**
  * @brief Interrupt service routine for i2c-0
  * @param[in] None
  * @return None
  */
static void i2c0ISR(void)
{
    int32_t csr, val;
    i2c_dev *dev;

    dev = (i2c_dev *) ( (uint32_t)&i2c_device[0] );

    csr = i2c_in(dev, I2C_CSR);
    csr |= 0x04;

    i2c_out(dev, csr, I2C_CSR);  /* clear interrupt flag */

    if(dev->state == I2C_STATE_NOP)
        return;

    if((csr & 0x800) && bNackValid)     /* NACK only valid in WRITE */
    {
        dev->last_error = I2C_ERR_NACK;
        _i2cCommand(dev, I2C_CMD_STOP);
        dev->state = I2C_STATE_NOP;
    }
    else if(csr & 0x200)                /* Arbitration lost */
    {
        sysprintf("Arbitration lost\n");
        dev->last_error = I2C_ERR_LOSTARBITRATION;
        dev->state = I2C_STATE_NOP;
    }
    else if(!(csr & 0x100))             /* transmit complete */
    {
        if(dev->pos < dev->subaddr_len + 1)     /* send address state */
        {
            val = dev->buffer[dev->pos++] & 0xff;
            i2c_out(dev, val, I2C_TxR);
            _i2cCommand(dev, I2C_CMD_WRITE);
        }
        else if(dev->state == I2C_STATE_READ)
        {

            /* sub address send over , begin restart a read command */

            if(dev->pos == dev->subaddr_len + 1)
            {
                val = dev->buffer[dev->pos++];
                i2c_out(dev, val, I2C_TxR);
                _i2cCommand(dev, I2C_CMD_START | I2C_CMD_WRITE);
            }
            else
            {

                dev->buffer[dev->pos++] = i2c_in(dev, I2C_RxR) & 0xff;

                if( dev->pos < dev->len)
                {
                    if(dev->pos == dev->len -1 )    /* last character */
                        _i2cCommand(dev, I2C_CMD_READ |
                                    I2C_CMD_STOP |
                                    I2C_CMD_NACK);
                    else
                        _i2cCommand(dev, I2C_CMD_READ);
                }
                else
                {
                    dev->state = I2C_STATE_NOP;
                }
            }
        }
        else if(dev->state == I2C_STATE_WRITE)  /* write data */
        {

            if( dev->pos < dev->len)
            {
                val = dev->buffer[dev->pos];

                i2c_out(dev, val, I2C_TxR);

                if(dev->pos == dev->len -1 )    /* last character */
                    _i2cCommand(dev, I2C_CMD_WRITE| I2C_CMD_STOP);
                else
                    _i2cCommand(dev, I2C_CMD_WRITE);

                dev->pos ++;
            }
            else
            {
                dev->state = I2C_STATE_NOP;
            }
        }
    }
}

/**
  * @brief Interrupt service routine for i2c-1
  * @param[in] None
  * @return None
  */
static void i2c1ISR(void)
{
    int32_t csr, val;
    i2c_dev *dev;

    dev = (i2c_dev *) ( (uint32_t)&i2c_device[1] );

    csr = i2c_in(dev, I2C_CSR);
    csr |= 0x04;

    i2c_out(dev, csr, I2C_CSR);  /* clear interrupt flag */

    if(dev->state == I2C_STATE_NOP)
        return;

    if((csr & 0x800) && bNackValid)     /* NACK only valid in WRITE */
    {
        dev->last_error = I2C_ERR_NACK;
        _i2cCommand(dev, I2C_CMD_STOP);
        dev->state = I2C_STATE_NOP;
    }
    else if(csr & 0x200)                /* Arbitration lost */
    {
        sysprintf("Arbitration lost\n");
        dev->last_error = I2C_ERR_LOSTARBITRATION;
        dev->state = I2C_STATE_NOP;
    }
    else if(!(csr & 0x100))             /* transmit complete */
    {
        if(dev->pos < dev->subaddr_len + 1)     /* send address state */
        {
            val = dev->buffer[dev->pos++] & 0xff;
            i2c_out(dev, val, I2C_TxR);
            _i2cCommand(dev, I2C_CMD_WRITE);
        }
        else if(dev->state == I2C_STATE_READ)
        {

            /* sub address send over , begin restart a read command */

            if(dev->pos == dev->subaddr_len + 1)
            {
                val = dev->buffer[dev->pos++];
                i2c_out(dev, val, I2C_TxR);
                _i2cCommand(dev, I2C_CMD_START | I2C_CMD_WRITE);
            }
            else
            {

                dev->buffer[dev->pos++] = i2c_in(dev, I2C_RxR) & 0xff;

                if( dev->pos < dev->len)
                {
                    if(dev->pos == dev->len -1 )    /* last character */
                        _i2cCommand(dev, I2C_CMD_READ |
                                    I2C_CMD_STOP |
                                    I2C_CMD_NACK);
                    else
                        _i2cCommand(dev, I2C_CMD_READ);
                }
                else
                {
                    dev->state = I2C_STATE_NOP;
                }
            }
        }
        else if(dev->state == I2C_STATE_WRITE)  /* write data */
        {

            if( dev->pos < dev->len)
            {
                val = dev->buffer[dev->pos];

                i2c_out(dev, val, I2C_TxR);

                if(dev->pos == dev->len -1 )    /* last character */
                    _i2cCommand(dev, I2C_CMD_WRITE| I2C_CMD_STOP);
                else
                    _i2cCommand(dev, I2C_CMD_WRITE);

                dev->pos ++;
            }
            else
            {
                dev->state = I2C_STATE_NOP;
            }
        }
    }
}

/// @endcond /* HIDDEN_SYMBOLS */

/**
  * @brief This function reset the i2c interface and enable interrupt.
  * @param[in] param is interface number.
  * @return open status.
  * @retval 0 success.
  * @retval I2C_ERR_BUSY Interface already opened.
  * @retval I2C_ERR_NODEV Interface number out of range.
  */
int32_t i2cOpen(PVOID param)
{
    i2c_dev *dev;

    if( (uint32_t)param >= I2C_NUMBER)
        return I2C_ERR_NODEV;

    dev = (i2c_dev *)((uint32_t)&i2c_device[(uint32_t)param] );

    if( dev->openflag != 0 )        /* a card slot can open only once */
        return(I2C_ERR_BUSY);

    /* Enable engine clock */
    if((uint32_t)param == 0)
        outpw(REG_CLK_PCLKEN1, inpw(REG_CLK_PCLKEN1) | 0x1);
    else
        outpw(REG_CLK_PCLKEN1, inpw(REG_CLK_PCLKEN1) | 0x2);

    memset(dev, 0, sizeof(i2c_dev));
    dev->base = ((uint32_t)param) ? I2C1_BA : I2C0_BA;

    _i2cReset(dev);

    dev->openflag = 1;

    return 0;
}

/**
  * @brief Disable I2C interrupt. And initialize some parameters.
  * @param[in] fd is interface number.
  * @return close status.
  * @retval 0 success.
  * @retval I2C_ERR_NODEV Interface number is out of range.
  */
int32_t i2cClose(int32_t fd)
{
    i2c_dev *dev;

    if(fd != 0 && fd != 1)
        return(I2C_ERR_NODEV);

    dev = (i2c_dev *)( (uint32_t)&i2c_device[fd] );

    dev->openflag = 0;

    if(fd == 0)
        sysDisableInterrupt(I2C0_IRQn);
    else
        sysDisableInterrupt(I2C1_IRQn);

    return 0;
}

/**
  * @brief Read data from I2C slave.
  * @param[in] fd is interface number.
  * @param[in] buf is receive buffer pointer.
  * @param[in] len is receive buffer length.
  * @return read status.
  * @retval >0 length when success.
  * @retval I2C_ERR_BUSY Interface busy.
  * @retval I2C_ERR_IO Interface not opened.
  * @retval I2C_ERR_NODEV No such device.
  * @retval I2C_ERR_NACK Slave returns an erroneous ACK.
  * @retval I2C_ERR_LOSTARBITRATION arbitration lost happen.
  */
int32_t i2cRead(int32_t fd, uint8_t* buf, uint32_t len)
{
    i2c_dev *dev;

    if(fd != 1 && fd != 0)
        return(I2C_ERR_NODEV);

    dev = (i2c_dev *)( (uint32_t)&i2c_device[fd] );

    if(dev->openflag == 0)
        return(I2C_ERR_IO);

    if(len == 0)
        return 0;

    if(!i2cIsBusFree(dev))
        return(I2C_ERR_BUSY);

    if(len > I2C_MAX_BUF_LEN - 10)
        len = I2C_MAX_BUF_LEN - 10;

    dev->state = I2C_STATE_READ;
    dev->pos = 1;
    /* Current ISR design will get one garbage byte */
    dev->len = dev->subaddr_len + 1 + len + 2;  /* plus 1 unused char                           */
    dev->last_error = 0;

    _i2cCalcAddr(dev, I2C_STATE_READ);

    i2cEnable(dev);

    i2c_out(dev, dev->buffer[0] & 0xff, I2C_TxR);

    if(!i2cIsBusFree(dev))
        return(I2C_ERR_BUSY);

    _i2cCommand(dev, I2C_CMD_START | I2C_CMD_WRITE);

    while(dev->state != I2C_STATE_NOP);

    i2cDisable(dev);

    if(dev->last_error)
        return(dev->last_error);

    memcpy(buf, dev->buffer + dev->subaddr_len + 3, len);

    dev->subaddr += len;

    return len;
}

/**
  * @brief Write data from I2C slave.
  * @param[in] fd is interface number.
  * @param[in] buf is transmit buffer pointer.
  * @param[in] len is transmit buffer length.
  * @return write status.
  * @retval >0 length when success.
  * @retval I2C_ERR_BUSY Interface busy.
  * @retval I2C_ERR_IO Interface not opened.
  * @retval I2C_ERR_NODEV No such device.
  * @retval I2C_ERR_NACK Slave returns an erroneous ACK.
  * @retval I2C_ERR_LOSTARBITRATION arbitration lost happen.
  */
int32_t i2cWrite(int32_t fd, uint8_t* buf, uint32_t len)
{
    i2c_dev *dev;

    if(fd != 1 && fd != 0)
        return(I2C_ERR_NODEV);

    dev = (i2c_dev *)( (uint32_t)&i2c_device[fd] );

    if(dev->openflag == 0)
        return(I2C_ERR_IO);

    if(len == 0)
        return 0;

    if(!i2cIsBusFree(dev))
        return(I2C_ERR_BUSY);

    if(len > I2C_MAX_BUF_LEN - 10)
        len = I2C_MAX_BUF_LEN - 10;

    memcpy(dev->buffer + dev->subaddr_len + 1 , buf, len);

    dev->state = I2C_STATE_WRITE;
    dev->pos = 1;
    dev->len = dev->subaddr_len + 1 + len;
    dev->last_error = 0;

    _i2cCalcAddr(dev, I2C_STATE_WRITE);

    i2cEnable(dev);

    i2c_out(dev, dev->buffer[0] & 0xff, I2C_TxR);

    if(!i2cIsBusFree(dev))
        return(I2C_ERR_BUSY);

    _i2cCommand(dev, I2C_CMD_START | I2C_CMD_WRITE);

    while(dev->state != I2C_STATE_NOP);

    i2cDisable(dev);

    if(dev->last_error)
        return(dev->last_error);

    dev->subaddr += len;

    return len;
}

/**
  * @brief Support some I2C driver commands for application.
  * @param[in] fd is interface number.
  * @param[in] cmd is command.
  * @param[in] arg0 is the first argument of command.
  * @param[in] arg1 is the second argument of command.
  * @return command status.
  * @retval 0 Success.
  * @retval I2C_ERR_IO Interface not opened.
  * @retval I2C_ERR_NODEV No such device.
  * @retval I2C_ERR_NOTTY Command not support, or parameter error.
  */
int32_t i2cIoctl(int32_t fd, uint32_t cmd, uint32_t arg0, uint32_t arg1)
{
    i2c_dev *dev;

    if(fd != 0 && fd != 1)
        return(I2C_ERR_NODEV);

    dev = (i2c_dev *)((uint32_t)&i2c_device[fd] );
    if(dev->openflag == 0)
        return(I2C_ERR_IO);

    switch(cmd)
    {
        case I2C_IOC_SET_DEV_ADDRESS:
            dev->addr = arg0;
            break;

        case I2C_IOC_SET_SPEED:

            return(_i2cSetSpeed(dev, (int32_t)arg0));

        case I2C_IOC_SET_SUB_ADDRESS:

            if(arg1 > 4)
            {
                return(I2C_ERR_NOTTY);
            }

            dev->subaddr = arg0;
            dev->subaddr_len = arg1;
            break;

        default:
            return(I2C_ERR_NOTTY);
    }

    return (0);
}

/**
  * @brief exit function.
  * @return always 0.
  * @retval 0 Success.
  */
int32_t i2cExit(void)
{
    return(0);
}

/**
  * @brief Install ISR.
  * @param[in] fd is interface number.
  * @return always 0.
  * @retval 0 Success.
  */
int32_t i2cInit(int32_t fd)
{
    if(fd == 0)
    {
        sysInstallISR(IRQ_LEVEL_1, I2C0_IRQn, (PVOID)i2c0ISR);
        sysEnableInterrupt(I2C0_IRQn);
        memset((void *)&i2c_device[0], 0, sizeof(i2c_dev));
    }
    else
    {
        sysInstallISR(IRQ_LEVEL_1, I2C1_IRQn, (PVOID)i2c1ISR);
        sysEnableInterrupt(I2C1_IRQn);
        memset((void *)&i2c_device[1], 0, sizeof(i2c_dev));
    }

    sysSetLocalInterrupt(ENABLE_IRQ);

    return(0);
}
/*@}*/ /* end of group NUC970_I2C_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC970_I2C_Driver */

/*@}*/ /* end of group NUC970_Device_Driver */

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/

