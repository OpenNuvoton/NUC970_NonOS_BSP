/*
 * standalone.c - minimal bootstrap for C library
 * Copyright (C) 2000 ARM Limited.
 * All rights reserved.
 */

/*
 * RCS $Revision: 2 $
 * Checkin $Date: 15/05/18 2:47p $ 0
 * Revising $Author: Hpchen0 $
 */

/*
 * This code defines a run-time environment for the C library.
 * Without this, the C startup code will attempt to use semi-hosting
 * calls to get environment information.
 */

extern unsigned int Image$$RW_RAM1$$ZI$$Limit;


void _sys_exit(int return_code)
{
label:
    goto label; /* endless loop */
}

void _ttywrch(int ch)
{
    char tempch = (char)ch;
    (void)tempch;
}


/// @cond HIDDEN_SYMBOLS
#pragma import(__use_two_region_memory)
__value_in_regs struct R0_R3 {
    unsigned heap_base, stack_base, heap_limit, stack_limit;
}
__user_initial_stackheap(unsigned int R0, unsigned int SP, unsigned int R2, unsigned int SL)

{
    struct R0_R3 config;


    config.heap_base = (unsigned int)&Image$$RW_RAM1$$ZI$$Limit;
    config.stack_base = SP;
    config.heap_limit = SP - 0x10000;


    return config;
}
/// @endcond HIDDEN_SYMBOLS


/* end of file standalone.c */
