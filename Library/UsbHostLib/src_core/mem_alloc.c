/**************************************************************************//**
 * @file     mem_alloc.c
 * @version  V1.10
 * $Revision: 11 $
 * $Date: 14/10/03 1:54p $
 * @brief   USB host library memory allocation functions.
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usb.h"


/// @cond HIDDEN_SYMBOLS

//#define MEM_DEBUG

#ifdef MEM_DEBUG
#define mem_debug       sysprintf
#else
#define mem_debug(...)
#endif


UDEV_T * g_udev_list;
static volatile int  _device_num;


/*--------------------------------------------------------------------------*/
/*   Memory alloc/free recording                                            */
/*--------------------------------------------------------------------------*/

void usbh_memory_init(void)
{
	USB_InitializeMemoryPool();
	
    g_udev_list = NULL;
    _device_num = 1;
}

uint32_t  usbh_memory_used(void)
{
    sysprintf("USBH memory: available: %d, used: %d\n", USB_available_memory(), USB_allocated_memory());
    return USB_allocated_memory();
}

void * usbh_alloc_mem(int size)
{
    void  *p;

    p = USB_malloc(size, 16);
    if (p == NULL) {
        USB_error("usbh_alloc_mem failed! %d\n", size);
        return NULL;
    }
    memset(p, 0, size);
    return p;
}

void usbh_free_mem(void *p, int size)
{
    USB_free(p);
}


/*--------------------------------------------------------------------------*/
/*   USB device allocate/free                                               */
/*--------------------------------------------------------------------------*/

UDEV_T * alloc_device(void)
{
    UDEV_T  *udev;

    udev = (UDEV_T *)USB_malloc(sizeof(*udev), 16);
    if (udev == NULL) {
        USB_error("alloc_device failed!\n");
        return NULL;
    }

    memset(udev, 0, sizeof(*udev));
    udev->cur_conf = -1;                    /* must! used to identify the first SET CONFIGURATION */
    udev->next = g_udev_list;               /* chain to global device list */
    g_udev_list = udev;
    return udev;
}

void free_device(UDEV_T *udev)
{
    UDEV_T  *d;
    
    if (udev == NULL)
        return;

    if (udev->cfd_buff != NULL)
        usbh_free_mem(udev->cfd_buff, MAX_DESC_BUFF_SIZE);

    /*
     *  Remove it from the global device list
     */
    if (g_udev_list == udev) {
        g_udev_list = g_udev_list->next;
    } else {
        d = g_udev_list;
        while (d != NULL) {
            if (d->next == udev) {
                d->next = udev->next;
                break;
            }
            d = d->next;
        }
    }
    USB_free(udev);
}

int  alloc_device_number(void)
{
    int   dev_num;
    dev_num = _device_num;                  /* allocate device number */
    _device_num = (_device_num % 254) + 1;  /* cannot be 0            */
    return dev_num;
}

/*--------------------------------------------------------------------------*/
/*   UTR (USB Transfer Request) allocate/free                               */
/*--------------------------------------------------------------------------*/

UTR_T * alloc_utr(UDEV_T *udev)
{
    UTR_T  *utr;

    utr = (UTR_T *)USB_malloc(sizeof(*utr), 16);
    if (utr == NULL) {
        USB_error("alloc_utr failed!\n");
        return NULL;
    }

    memset(utr, 0, sizeof(*utr));
    utr->udev = udev;
    mem_debug("[ALLOC] [UTR] - 0x%x\n", (int)utr);
    return utr;
}

void free_utr(UTR_T *utr)
{
    if (utr == NULL)
        return;
        
    mem_debug("[FREE] [UTR] - 0x%x\n", (int)utr);
    USB_free(utr);
}

/*--------------------------------------------------------------------------*/
/*   OHCI ED allocate/free                                                  */
/*--------------------------------------------------------------------------*/

ED_T * alloc_ohci_ED(void)
{
    ED_T   *ed;
    
    ed = (ED_T *)USB_malloc(sizeof(ED_T), 32);

    if (ed != NULL) {
        memset(ed, 0, sizeof(*ed));
        mem_debug("[ALLOC] [ED] - 0x%x\n", (int)ed);
        return ed;
    }
    USB_error("alloc_ohci_ED failed!\n");
    return NULL;
}

void free_ohci_ED(ED_T *ed)
{
    mem_debug("[FREE]  [ED] - 0x%x\n", (int)ed);
	USB_free(ed);
}

/*--------------------------------------------------------------------------*/
/*   OHCI TD allocate/free                                                  */
/*--------------------------------------------------------------------------*/
TD_T * alloc_ohci_TD(UTR_T *utr)
{
    TD_T   *td;

    td = (TD_T *)USB_malloc(sizeof(TD_T), 32);

	if (td != NULL) {
    	memset(td, 0, sizeof(*td));
        td->utr = utr;
        mem_debug("[ALLOC] [TD] - 0x%x\n", (int)td);
        return td;
    }
    USB_error("alloc_ohci_TD failed!\n");
    return NULL;
}

void free_ohci_TD(TD_T *td)
{
    mem_debug("[FREE]  [TD] - 0x%x\n", (int)td);
    USB_free(td);
}

/*--------------------------------------------------------------------------*/
/*   EHCI QH allocate/free                                                  */
/*--------------------------------------------------------------------------*/
QH_T * alloc_ehci_QH(void)
{
    QH_T   *qh = NULL;

    qh = (QH_T *)USB_malloc(sizeof(QH_T), 32);
    if (qh == NULL) {
        USB_error("alloc_ehci_QH failed!\n");
        return NULL;
    }

    memset(qh, 0, sizeof(*qh));
    mem_debug("[ALLOC] [QH] - 0x%x\n", (int)qh);
    qh->Curr_qTD        = QTD_LIST_END;
    qh->OL_Next_qTD     = QTD_LIST_END;
    qh->OL_Alt_Next_qTD = QTD_LIST_END;
    qh->OL_Token        = QTD_STS_HALT;
    return qh;
}

void free_ehci_QH(QH_T *qh)
{
    mem_debug("[FREE]  [QH] - 0x%x\n", (int)qh);
    USB_free(qh);
}

/*--------------------------------------------------------------------------*/
/*   EHCI qTD allocate/free                                                 */
/*--------------------------------------------------------------------------*/
qTD_T * alloc_ehci_qTD(UTR_T *utr)
{
    qTD_T   *qtd;
    
    qtd = (qTD_T *)USB_malloc(sizeof(qTD_T), 32);

	if (qtd != NULL)
	{
    	memset(qtd, 0, sizeof(*qtd));
        qtd->Next_qTD     = QTD_LIST_END;
        qtd->Alt_Next_qTD = QTD_LIST_END;
        qtd->Token        = 0x1197B7F; // QTD_STS_HALT;  visit_qtd() will not remove a qTD with this mark. It means the qTD still not ready for transfer.
        qtd->utr = utr;
        mem_debug("[ALLOC] [qTD] - 0x%x\n", (int)qtd);
        return qtd;
    }
    USB_error("alloc_ehci_qTD failed!\n");
    return NULL;
}

void free_ehci_qTD(qTD_T *qtd)
{
    mem_debug("[FREE]  [qTD] - 0x%x\n", (int)qtd);
    USB_free(qtd);
}

/*--------------------------------------------------------------------------*/
/*   EHCI iTD allocate/free                                                 */
/*--------------------------------------------------------------------------*/
iTD_T * alloc_ehci_iTD(void)
{
    iTD_T   *itd;
    
    itd = USB_malloc(sizeof(iTD_T), 32);

	if (itd != NULL)
	{
    	memset(itd, 0, sizeof(*itd));
        mem_debug("[ALLOC] [iTD] - 0x%x\n", (int)itd);
        return itd;
    }
    USB_error("alloc_ehci_iTD failed!\n");
    return NULL;
}

void free_ehci_iTD(iTD_T *itd)
{
    mem_debug("[FREE]  [iTD] - 0x%x\n", (int)itd);
    USB_free(itd);
}

/*--------------------------------------------------------------------------*/
/*   EHCI iTD allocate/free                                                 */
/*--------------------------------------------------------------------------*/
siTD_T * alloc_ehci_siTD(void)
{
    siTD_T  *sitd;
    
    sitd = (siTD_T *)USB_malloc(sizeof(siTD_T), 32);
    
    if (sitd != NULL)
    {
        memset(sitd, 0, sizeof(*sitd));
        mem_debug("[ALLOC] [siTD] - 0x%x\n", (int)sitd);
        return sitd;
    }
    USB_error("alloc_ehci_siTD failed!\n");
    return NULL;
}

void free_ehci_siTD(siTD_T *sitd)
{
    mem_debug("[FREE]  [siTD] - 0x%x\n", (int)sitd);
    USB_free(sitd);
}

/// @endcond HIDDEN_SYMBOLS

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/

