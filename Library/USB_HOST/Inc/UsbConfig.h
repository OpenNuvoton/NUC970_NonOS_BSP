/*************************************************************************
 * Nuvoton Electronics Corporation confidential
 *
 * Copyright (c) 2008 by Nuvoton Electronics Corporation
 * All rights reserved
 *
 * FILENAME
 *     usbconfig.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     NUC900 USB Host driver header file 
 *
 * HISTORY
 *     2008.06.24       Created
 *
 * REMARK
 *     None
 **************************************************************************/
#ifndef  _USBCONFIG_H_
#define  _USBCONFIG_H_

/// @cond HIDDEN_SYMBOLS


/*---  CPU clock speed ---*/
#define HZ               		(200)

#define CONFIG_HAVE_OHCI
#define CONFIG_HAVE_EHCI

//#define DEBUG
//#define VDEBUG
//#define DUMP_DESCRIPTOR

#define AUTO_SUSPEND					/* automatically suspend and resume */

#define EHCI_ISO_DELAY			0		//8
#define OHCI_ISO_DELAY			8


/*
 *  Size of static memory buffers
 */
#define USB_MEMORY_POOL_SIZE   (4*1024*1024)
#define USB_MEM_BLOCK_SIZE      128

#define PERIODIC_FL_SIZE		4096
#define HCCA_SIZE				4096


#define USB_error				sysprintf
#ifdef DEBUG
#define USB_debug				sysprintf
#ifdef VDEBUG
#define USB_vdebug				sysprintf
#else
#define USB_vdebug(...)
#endif
#else
#define USB_debug(...)	
#define USB_vdebug(...)	
#endif

/// @endcond HIDDEN_SYMBOLS

#endif  /* _USBCONFIG_H_ */


