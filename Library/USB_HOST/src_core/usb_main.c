/**************************************************************************//**
 * @file     usb_main.c
 * @version  V1.10
 * $Revision: 3 $
 * $Date: 15/05/18 3:55p $
 * @brief   USB Host driver main function.
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"

#include "usb.h"
#include "host/ehci.h"
#include "usbh_lib.h"


/// @cond HIDDEN_SYMBOLS

extern void usb_poll_root_hubs(void);
extern int  hub_events(void);
extern int  usb_ehci_init(void);
extern int  usb_ohci_init(void);
extern void ehci_irq(void);
extern void ohci_irq(void);
//extern void USB_InitializeMemoryPool(void);

INT    _IsInUsbInterrupt = 0;

extern struct usb_hcd  *_g_ohci_usb_hcd;
extern struct usb_hcd  *_g_ehci_usb_hcd;

/// @endcond HIDDEN_SYMBOLS


/**
  * @brief       Initialize NUC970 USB Host controller and USB stack.
  *
  * @return      None.
  */
void  usbh_core_init()
{
	volatile int  loop;
	
	USB_InitializeMemoryPool();
	
	outpw(REG_CLK_HCLKEN, (inpw(REG_CLK_HCLKEN) | 0x40000));/* enable USB Host clock */
	for (loop = 0; loop < 0x1000; loop++);

	outpw(USBH_BA + 0xC4, 0x160); 					/* enable PHY 0          */
	outpw(USBH_BA + 0xC8, 0x520); 					/* enable PHY 1          */

	outpw(USBO_BA + 0x204, 0x0);					/* for over-current 		 */

	bus_register(&usb_bus_type);
	usb_ohci_init();
	usb_ehci_init();

	// install ISR
	sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, EHCI_IRQn, (PVOID)ehci_irq);
	sysInstallISR(HIGH_LEVEL_SENSITIVE | IRQ_LEVEL_1, OHCI_IRQn, (PVOID)ohci_irq);
  	sysSetLocalInterrupt(ENABLE_IRQ);	/* enable CPSR I bit */
	sysEnableInterrupt(EHCI_IRQn);
	sysEnableInterrupt(OHCI_IRQn);

	usb_hub_init();
}


/**
  * @brief       Check and process status change of all down-stream hubs, including root hubs. 
  *
  * @retval      0   There's no any hub events since the last call.
  * @retval      1   There's hub events occurred since the last call.
  */
int usbh_pooling_hubs()
{
	int  ret;

	usb_poll_root_hubs();

	ret = hub_events();
	
	if (ret == 0)	
		return 0;
	else
		return 1;
}


/// @cond HIDDEN_SYMBOLS


void dump_ohci_regs()
{
	sysprintf("Dump OCHI registers:\n");
	sysprintf("    REG_HcRev =      0x%x\n", inpw(HcRev));
	sysprintf("    REG_HcControl =  0x%x\n", inpw(HcControl));
	sysprintf("    REG_HcComSts =   0x%x\n", inpw(HcComSts));
	sysprintf("    REG_HcIntSts =   0x%x\n", inpw(HcIntSts));
	sysprintf("    REG_HcIntEn =    0x%x\n", inpw(HcIntEn));
	sysprintf("    REG_HcHCCA =     0x%x\n", inpw(HcHCCA));
	sysprintf("    REG_HcCtrHED =   0x%x\n", inpw(HcCtrHED));
	sysprintf("    REG_HcBlkHED =   0x%x\n", inpw(HcBlkHED));
	sysprintf("    REG_HcRhSts =    0x%x\n", inpw(HcRhSts));
	sysprintf("    REG_HcRhPrt1 =   0x%x\n", inpw(HcRhPrt1));
	sysprintf("    REG_HcRhPrt2 =   0x%x\n", inpw(HcRhPrt2));
}

void dump_ohci_ports()
{
	sysprintf("OHCI port0=0x%x, port1=0x%x\n", inpw(HcRhPrt1), inpw(HcRhPrt2));
}

void dump_ehci_regs()
{
	sysprintf("Dump EHCI registers:\n");
	sysprintf("    REG_UCMDR =      0x%x\n", inpw(UCMDR));
	sysprintf("    REG_USTSR =      0x%x\n", inpw(USTSR));
	sysprintf("    REG_UIENR =      0x%x\n", inpw(UIENR));
	sysprintf("    REG_UFINDR =     0x%x\n", inpw(UFINDR));
	sysprintf("    REG_UPFLBAR =    0x%x\n", inpw(UPFLBAR));
	sysprintf("    REG_UCALAR =     0x%x\n", inpw(UCALAR));
	sysprintf("    REG_UASSTR =     0x%x\n", inpw(UASSTR));
	sysprintf("    REG_UCFGR =      0x%x\n", inpw(UCFGR));
	sysprintf("    REG_UPSCR0 =     0x%x\n", inpw(UPSCR0));
	sysprintf("    REG_UPSCR1 =     0x%x\n", inpw(UPSCR1));
	sysprintf("    REG_USBPCR0 =    0x%x\n", inpw(USBPCR0));
	sysprintf("    REG_USBPCR1 =    0x%x\n", inpw(USBPCR1));
}

void dump_ehci_ports()
{
	sysprintf("EHCI port0=0x%x, port1=0x%x\n", inpw(USBPCR0), inpw(USBPCR1));
}


void dump_ehci_qtd_list(struct ehci_qtd *qtd)
{
	qtd = (struct ehci_qtd *) ((u32)qtd & 0xfffffff0);
	while (1)
	{
		sysprintf("   QTD addr = 0x%x\n", (u32)qtd);
		sysprintf("        Next qtd:     0x%x\n", qtd->hw_next);
		sysprintf("        Alt next qtd: 0x%x\n", qtd->hw_alt_next);
		sysprintf("        token:        0x%x\n", qtd->hw_token);
		sysprintf("        buffer ptr:   0x%x\n", qtd->hw_buf[0]);
		
		if (qtd->hw_next & 0x1)
			return;
		qtd = (struct ehci_qtd *)qtd->hw_next;
	};
}


void dump_ehci_asynclist(void)
{
	struct ehci_qh_hw  *qh;
	
	//if (!(inpw(REG_UCMDR) & 0x20))
	//	return ;
	
	sysprintf("ASYNCLISTADDR = 0x%x\n", inpw(UCALAR));
	qh = (struct ehci_qh_hw *)inpw(UCALAR);
	while (qh != NULL)
	{
		sysprintf("*************************\n");
		sysprintf("Queue Head address = 0x%x\n", (u32)qh);
		sysprintf("    Next link:    0x%x\n", (u32)qh->hw_next);
		sysprintf("    Info1:        0x%x\n", (u32)qh->hw_info1);
		sysprintf("    Info2:        0x%x\n", (u32)qh->hw_info2);
		sysprintf("    Next qtd:     0x%x\n", (u32)qh->hw_qtd_next);
		sysprintf("    dev addr:     %d\n", qh->hw_info1 & 0x7f);
		sysprintf("    endpoint:     %d\n", (qh->hw_info1 >> 8) & 0xf);
		if (0) // (qh->hw_current)
		{
			sysprintf("    ========================================\n");
			dump_ehci_qtd_list((struct ehci_qtd *)qh->hw_qtd_next);
		}
		qh = (struct ehci_qh_hw *)qh->hw_next;		
	}
}


void dump_ehci_asynclist_simple(char *msg)
{
	struct ehci_qh_hw  *qh;
	int  cnt = 0;
	
	//if (!(inpw(REG_UCMDR) & 0x20))
	//	return ;
	
	if (msg != NULL)
		sysprintf("%s - ", msg);
	
	sysprintf("QH 0x%x ", inpw(UCALAR));
	qh = (struct ehci_qh_hw *)inpw(UCALAR);
	while ((qh != NULL) && (cnt++ < 5))
	{
		if (!(inpw(UCMDR) & 0x20))
			break;
		sysprintf(" => 0x%x (0x%x)", qh->hw_next, qh->hw_qtd_next);
		qh = (struct ehci_qh_hw *)(qh->hw_next & 0xfffffff0);		
	}
	sysprintf("\n");
}


/// @endcond HIDDEN_SYMBOLS

