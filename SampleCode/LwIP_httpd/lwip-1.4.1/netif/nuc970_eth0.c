/*
 * Copyright (c) 2015 Nuvoton Technology Corp.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Description:   NUC970 MAC driver source file
 */
#include "nuc970.h"
#include "sys.h"
#include "netif/nuc970_eth.h"
#include "lwip/opt.h"
#include "lwip/def.h"


#define ETH0_TRIGGER_RX()    outpw(REG_EMAC0_RSDR, 0)
#define ETH0_TRIGGER_TX()    outpw(REG_EMAC0_TSDR, 0)
#define ETH0_ENABLE_TX()     outpw(REG_EMAC0_MCMDR, inpw(REG_EMAC0_MCMDR) | 0x100)
#define ETH0_ENABLE_RX()     outpw(REG_EMAC0_MCMDR, inpw(REG_EMAC0_MCMDR) | 0x1)
#define ETH0_DISABLE_TX()    outpw(REG_EMAC0_MCMDR, inpw(REG_EMAC0_MCMDR) & ~0x100)
#define ETH0_DISABLE_RX()    outpw(REG_EMAC0_MCMDR, inpw(REG_EMAC0_MCMDR) & ~0x1)

#ifdef __ICCARM__
#pragma data_alignment=4
static struct eth_descriptor rx_desc[RX_DESCRIPTOR_NUM];
static struct eth_descriptor tx_desc[TX_DESCRIPTOR_NUM];
#else
static struct eth_descriptor rx_desc[RX_DESCRIPTOR_NUM] __attribute__ ((aligned(32)));
static struct eth_descriptor tx_desc[TX_DESCRIPTOR_NUM] __attribute__ ((aligned(32)));
#endif
static struct eth_descriptor volatile *cur_tx_desc_ptr, *cur_rx_desc_ptr, *fin_tx_desc_ptr;

static u8_t rx_buf[RX_DESCRIPTOR_NUM][PACKET_BUFFER_SIZE];
static u8_t tx_buf[TX_DESCRIPTOR_NUM][PACKET_BUFFER_SIZE];
static int plugged = 0;

extern void ethernetif_input(u16_t len, u8_t *buf);
extern void ethernetif_loopback_input(struct pbuf *p);


static void mdio_write(u8_t addr, u8_t reg, u16_t val)
{
    
    outpw(REG_EMAC0_MIID, val);
    outpw(REG_EMAC0_MIIDA, (addr << 8) | reg | 0xB0000);

    while (inpw(REG_EMAC0_MIIDA) & 0x20000);    // wait busy flag clear

}


static u16_t mdio_read(u8_t addr, u8_t reg)
{
    outpw(REG_EMAC0_MIIDA, (addr << 8) | reg | 0xA0000);
    while (inpw(REG_EMAC0_MIIDA) & 0x20000);    // wait busy flag clear

    return inpw(REG_EMAC0_MIID);
}

static int reset_phy(void)
{

    u16_t reg;
    u32_t delay;


    mdio_write(CONFIG_PHY_ADDR, MII_BMCR, BMCR_RESET);

    delay = 2000;
    while(delay-- > 0) {
        if((mdio_read(CONFIG_PHY_ADDR, MII_BMCR) & BMCR_RESET) == 0)
            break;

    }

    if(delay == 0) {
        sysprintf("Reset phy failed\n");
        return(-1);
    }

    mdio_write(CONFIG_PHY_ADDR, MII_ADVERTISE, ADVERTISE_CSMA |
               ADVERTISE_10HALF |
               ADVERTISE_10FULL |
               ADVERTISE_100HALF |
               ADVERTISE_100FULL);

    reg = mdio_read(CONFIG_PHY_ADDR, MII_BMCR);
    mdio_write(CONFIG_PHY_ADDR, MII_BMCR, reg | BMCR_ANRESTART);

    delay = 200000;
    while(delay-- > 0) {
        if((mdio_read(CONFIG_PHY_ADDR, MII_BMSR) & (BMSR_ANEGCOMPLETE | BMSR_LSTATUS))
                == (BMSR_ANEGCOMPLETE | BMSR_LSTATUS))
            break;
    }

    if(delay == 0) {
        sysprintf("AN failed. Set to 100 FULL\n");
        outpw(REG_EMAC0_MCMDR, inpw(REG_EMAC0_MCMDR) | 0x140000);
        plugged = 0;
        return(-1);
    } else {
        reg = mdio_read(CONFIG_PHY_ADDR, MII_LPA);
        plugged = 1;

        if(reg & ADVERTISE_100FULL) {
            sysprintf("100 full\n");
            outpw(REG_EMAC0_MCMDR, inpw(REG_EMAC0_MCMDR) | 0x140000);
        } else if(reg & ADVERTISE_100HALF) {
            sysprintf("100 half\n");
            outpw(REG_EMAC0_MCMDR, (inpw(REG_EMAC0_MCMDR) & ~0x40000) | 0x100000);
        } else if(reg & ADVERTISE_10FULL) {
            sysprintf("10 full\n");
            outpw(REG_EMAC0_MCMDR, (inpw(REG_EMAC0_MCMDR) & ~0x100000) | 0x40000);
        } else {
            sysprintf("10 half\n");
            outpw(REG_EMAC0_MCMDR, inpw(REG_EMAC0_MCMDR) & ~0x140000);
        }
    }

    return(0);
}


static void init_tx_desc(void)
{
    u32_t i;


    cur_tx_desc_ptr = fin_tx_desc_ptr = (struct eth_descriptor *)((UINT)(&tx_desc[0]) | 0x80000000);

    for(i = 0; i < TX_DESCRIPTOR_NUM; i++) {
        tx_desc[i].status1 = TXFD_PADEN | TXFD_CRCAPP | TXFD_INTEN;
        tx_desc[i].buf = (unsigned char *)((UINT)(&tx_buf[i][0]) | 0x80000000);
        tx_desc[i].status2 = 0;
        tx_desc[i].next = (struct eth_descriptor *)((UINT)(&tx_desc[(i + 1) % TX_DESCRIPTOR_NUM]) | 0x80000000);
    }
    outpw(REG_EMAC0_TXDLSA, (unsigned int)&tx_desc[0] | 0x80000000);
    return;
}

static void init_rx_desc(void)
{
    u32_t i;


    cur_rx_desc_ptr = (struct eth_descriptor *)((UINT)(&rx_desc[0]) | 0x80000000);

    for(i = 0; i < RX_DESCRIPTOR_NUM; i++) {
        rx_desc[i].status1 = OWNERSHIP_EMAC;
        rx_desc[i].buf = (unsigned char *)((UINT)(&rx_buf[i][0]) | 0x80000000);
        rx_desc[i].status2 = 0;
        rx_desc[i].next = (struct eth_descriptor *)((UINT)(&rx_desc[(i + 1) % RX_DESCRIPTOR_NUM]) | 0x80000000);
    }
    outpw(REG_EMAC0_RXDLSA, (unsigned int)&rx_desc[0] | 0x80000000);
    return;
}

static void set_mac_addr(u8_t *addr)
{

    outpw(REG_EMAC0_CAMxM_Reg(0), (addr[0] << 24) |
                                  (addr[1] << 16) |
                                  (addr[2] << 8) |
                                  addr[3]);
    outpw(REG_EMAC0_CAMxL_Reg(0), (addr[4] << 24) |
                                  (addr[5] << 16));
    outpw(REG_EMAC0_CAMCMR, 0x16);
    outpw(REG_EMAC0_CAMEN, 1);    // Enable CAM entry 0

}


void ETH0_halt(void)
{

    outpw(REG_EMAC0_MCMDR, inpw(REG_EMAC0_MCMDR) & ~0x101); // disable tx/rx on
    
}

void ETH0_RX_IRQHandler(void)
{
    unsigned int status;

    status = inpw(REG_EMAC0_MISTA) & 0xFFFF;
    outpw(REG_EMAC0_MISTA, status);
    
    if (status & 0x800) {
        // Shouldn't goes here, unless descriptor corrupted
    }

    do {
        status = cur_rx_desc_ptr->status1;

        if(status & OWNERSHIP_EMAC)
            break;

        if (status & RXFD_RXGD) {

            ethernetif_input0(status & 0xFFFF, cur_rx_desc_ptr->buf);


        }

        cur_rx_desc_ptr->status1 = OWNERSHIP_EMAC;
        cur_rx_desc_ptr = cur_rx_desc_ptr->next;

    } while (1);

    ETH0_TRIGGER_RX();

}

void ETH0_TX_IRQHandler(void)
{
    unsigned int cur_entry, status;

    status = inpw(REG_EMAC0_MISTA) & 0xFFFF0000;
    outpw(REG_EMAC0_MISTA, status);

    if(status & 0x1000000) {
        // Shouldn't goes here, unless descriptor corrupted
        return;
    }

    cur_entry = inpw(REG_EMAC0_CTXDSA);

    while (cur_entry != (u32_t)fin_tx_desc_ptr) {

        fin_tx_desc_ptr = fin_tx_desc_ptr->next;
    }

}

static void chk_link(void)
{
    unsigned int reg;

    reg = mdio_read(CONFIG_PHY_ADDR, MII_BMSR);

    if (reg & BMSR_LSTATUS) {
        if (!plugged) {
            plugged = 1;
            reset_phy();
            outpw(REG_EMAC0_MCMDR, inpw(REG_EMAC0_MCMDR) | 0x101);
        }
    } else {
        if (plugged) {
            plugged = 0;
            outpw(REG_EMAC0_MCMDR, inpw(REG_EMAC0_MCMDR) & ~0x101);
        }
    }
}

void ETH0_init(u8_t *mac_addr)
{

    outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN) | (1 << 16));            // EMAC0 clk
    outpw(REG_CLK_DIVCTL8, (inpw(REG_CLK_DIVCTL8) & ~0xFF) | 0xA0);     // MDC clk divider
    
    // Multi function pin setting
    outpw(REG_SYS_GPF_MFPL, 0x11111111);
    outpw(REG_SYS_GPF_MFPH, (inpw(REG_SYS_GPF_MFPH) & ~0xFF) | 0x11);
    
    // Reset MAC
    outpw(REG_EMAC0_MCMDR, 0x1000000);

    init_tx_desc();
    init_rx_desc();

    set_mac_addr(mac_addr);  // need to reconfigure hardware address 'cos we just RESET emc...
    reset_phy();

    outpw(REG_EMAC0_MCMDR, inpw(REG_EMAC0_MCMDR) | 0x121); // strip CRC, TX on, Rx on
    outpw(REG_EMAC0_MIEN, inpw(REG_EMAC0_MIEN) | 0x01250C11);  // Except tx/rx ok, enable rdu, txabt, tx/rx bus error.
    sysInstallISR(IRQ_LEVEL_1, EMC0_TX_IRQn, (PVOID)ETH0_TX_IRQHandler);
    sysInstallISR(IRQ_LEVEL_1, EMC0_RX_IRQn, (PVOID)ETH0_RX_IRQHandler);
    sysEnableInterrupt(EMC0_TX_IRQn);
    sysEnableInterrupt(EMC0_RX_IRQn);
    ETH0_TRIGGER_RX();
    
    sysSetTimerEvent(TIMER0, 200, (PVOID)chk_link);  // check link status every 2 sec
}


u8_t *ETH0_get_tx_buf(void)
{
    if(cur_tx_desc_ptr->status1 & OWNERSHIP_EMAC)
        return(NULL);
    else
        return(cur_tx_desc_ptr->buf);
}

void ETH0_trigger_tx(u16_t length, struct pbuf *p)
{
    struct eth_descriptor volatile *desc;
    cur_tx_desc_ptr->status2 = (unsigned int)length;
    desc = cur_tx_desc_ptr->next;    // in case TX is transmitting and overwrite next pointer before we can update cur_tx_desc_ptr
    cur_tx_desc_ptr->status1 |= OWNERSHIP_EMAC;
    cur_tx_desc_ptr = desc;

    ETH0_TRIGGER_TX();

}


