/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "nuc970.h"
#include "sys.h"

UINT32 sys_now(void)
{
    return sysGetTicks(TIMER0) * 10;
    
}

int sys_arch_protect(void)  
{
    int _old, _new;
#if defined ( __GNUC__ ) && !(__CC_ARM)
    asm
    (
        "mrs    %[old], cpsr  \n"
        "bic    %[new], %[old], #0x80  \n"
        "msr    CPSR_c, %[new]  \n"
        : [old]"=r" (_old)
        : [new]"r"  (_new)
	    :
    );
#else
    __asm
    {
        MRS    _old, CPSR
        ORR    _new, _old, DISABLE_FIQ_IRQ
        MSR    CPSR_c, _new
    }
#endif
    return(_old);
}

void sys_arch_unprotect(int pval)
{
#if defined ( __GNUC__ ) && !(__CC_ARM)
    asm
    (
        "msr    CPSR_c, %0  \n"
        : "=r" (pval)
        : "0"  (pval)
        :
    );
#else
    __asm
    {
        MSR    CPSR_c, pval
    }
#endif
    return;
}


