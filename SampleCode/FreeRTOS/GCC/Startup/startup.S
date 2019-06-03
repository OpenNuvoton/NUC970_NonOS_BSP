;/***************************************************************************
; *                                                                         *
; * Copyright (c) 2019 Nuvoton Technology. All rights reserved.             *
; *                                                                         *
; ***************************************************************************/

    USR_MODE    =     0x10
    FIQ_MODE    =     0x11
    IRQ_MODE    =     0x12
    SVC_MODE    =     0x13
    ABT_MODE    =     0x17
    UDF_MODE    =     0x1B
    SYS_MODE    =     0x1F
       
    I_BIT          = 0x80        /* Disables IRQ when I bit is set               */
    F_BIT          = 0x40        /* Disables FIQ when F bit is set               */

    UND_Stack_Size = 0x00000100
    ABT_Stack_Size = 0x00000100
    FIQ_Stack_Size = 0x00000200
    SVC_Stack_Size = 0x00000C00
    IRQ_Stack_Size = 0x00004000
    USR_Stack_Size = 0x00004000

    REG_SDIC_SIZE0 = 0xB0001810  // DDR size register
    REG_AIC_MDCR   = 0xB8002138  // Mask disable command register
    REG_AIC_MDCRH  = 0xB800213C  // Mask disable command register (High)


/*********************************************************************
*
*       Vector table
*
**********************************************************************
*/
    .text
    .global  __vector
//    .global  end
    .extern  IRQ_Handler
    .extern  vPortYieldProcessor
    .arm
    .section .vectors, "ax"

__vector:
    ldr     pc,Reset_Addr   /* RESET                 vector */
    ldr     pc,Undef_Addr   /* Undefined instruction vector */
    ldr     pc,SWI_Addr     /* Software interrupt    vector */
    ldr     pc,PAbt_Addr    /* Prefetch abort        vector */
    ldr     pc,DAbt_Addr    /* Data abort            vector */
    nop                     /* Reserved              vector */
    ldr     pc,IRQ_Addr     /* Interrupt             vector */
    ldr     pc,FIQ_Addr     /* Fast interrupt        vector */

Reset_Addr:     .word   Reset_Handler
Undef_Addr:     .word   Undef_Handler
SWI_Addr:       .word   vPortYieldProcessor
PAbt_Addr:      .word   PAbt_Handler
DAbt_Addr:      .word   DAbt_Handler
ZeroAddr:       .word   0
IRQ_Addr:       .word   IRQ_Handler
FIQ_Addr:       .word   FIQ_Handler

Undef_Handler:  b       Undef_Handler
PAbt_Handler:   b       PAbt_Handler
DAbt_Handler:   b       DAbt_Handler
IRQ_Handler:    b       IRQ_Handler
FIQ_Handler:    b       FIQ_Handler
__vector_end:

Reset_Handler:

        // Disable Interrupt in case code is load by ICE while other firmware is executing
    LDR    r0, =REG_AIC_MDCR
    LDR    r1, =0xFFFFFFFF
    STR    r1, [r0]
    LDR    r0, =REG_AIC_MDCRH
    STR    r1, [r0]

    //INIT_STACK
    LDR    R2, =REG_SDIC_SIZE0
    LDR    R3,[R2]
    AND    R3, R3, #0x00000007
    MOV    R1,#2
    MOV    R0,#1
LOOP_DRAMSIZE:
    CMP    R0,R3
    BEQ    DONE_DRAMSIZE
    LSL    R1,R1,#1
    ADD    R0,R0,#1
    B    LOOP_DRAMSIZE
DONE_DRAMSIZE:
    // Using DRAM Size to set Stack Pointer
    LSL    R0,R1,#20

    // Enter Undefined Instruction Mode and set Stack Pointer
    MSR    CPSR_c, #UDF_MODE | I_BIT | F_BIT
    MOV    SP, R0
    SUB    R0, R0, #UND_Stack_Size

    // Enter Abort Mode and set Stack Pointer
    MSR    CPSR_c, #ABT_MODE | I_BIT | F_BIT
    MOV    SP, R0
    SUB    R0, R0, #ABT_Stack_Size

    // Enter IRQ Mode and set Stack Pointer
    MSR    CPSR_c, #IRQ_MODE | I_BIT | F_BIT
    MOV    SP, R0
    SUB    R0, R0, #IRQ_Stack_Size

    // Enter FIQ Mode and set Stack Pointer
    MSR    CPSR_c, #FIQ_MODE | I_BIT | F_BIT
    MOV    SP, R0
    SUB    R0, R0, #FIQ_Stack_Size

    // Enter User Mode and set Stack Pointer
    MSR    CPSR_c, #SYS_MODE | I_BIT | F_BIT
    MOV    SP, R0
    SUB    R0, R0, #USR_Stack_Size

    // Enter Supervisor Mode and set Stack Pointer
    MSR    CPSR_c, #SVC_MODE | I_BIT | F_BIT
    MOV    SP, R0
    SUB    R0, R0, #SVC_Stack_Size

    MRC p15, 0, r0 , c1, c0     /* r0 := cp15 register 1 */
    BIC r0, r0, #0x2000         /* Clear bit13 in r1 */
    MCR p15, 0, r0 , c1, c0     /* cp15 register 1 := r0 */

    /*
     * Clear .bss section
     */
    LDR   r1, =__bss_start__
    LDR   r2, =__bss_end__
    LDR   r3, =0
bss_clear_loop:
    CMP   r1, r2
    STRNE r3, [r1], #+4
    BNE   bss_clear_loop

    MOV   r0, #0                         /* No arguments are passed to main */
    MOV   r1, #0
    LDR   r2, =main
    MOV   lr, pc
    BX    r2

end:    B     end

    .end






