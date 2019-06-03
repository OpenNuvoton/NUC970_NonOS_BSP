;/***************************************************************************
; *                                                                         *
; * Copyright (c) 2015 Nuvoton Technology. All rights reserved.             *
; *                                                                         *
; ***************************************************************************/


    AREA NUC_INIT, CODE, READONLY

;--------------------------------------------
; Mode bits and interrupt flag (I&F) defines
;--------------------------------------------
USR_MODE    EQU     0x10
FIQ_MODE    EQU     0x11
IRQ_MODE    EQU     0x12
SVC_MODE    EQU     0x13
ABT_MODE    EQU     0x17
UDF_MODE    EQU     0x1B
SYS_MODE    EQU     0x1F

I_BIT       EQU     0x80
F_BIT       EQU     0x40

;----------------------------
; System / User Stack Memory
;----------------------------
RAM_Limit       EQU     0x2000000            ; For unexpanded hardware board

UND_Stack_Size  EQU     0x00000100
ABT_Stack_Size  EQU     0x00000100
FIQ_Stack_Size  EQU     0x00000200
SVC_Stack_Size  EQU     0x00000C00
IRQ_Stack_Size  EQU     0x00004000
USR_Stack_Size  EQU     0x00004000

REG_AIC_MDCR    EQU     0xB8002138  ; Mask disable command register
REG_AIC_MDCRH   EQU     0xB800213C  ; Mask disable command register (High)

    ENTRY
    EXPORT  Reset_Go

        EXPORT  Vector_Table
Vector_Table
        B       Reset_Go    ; Modified to be relative jumb for external boot
        LDR     PC, Undefined_Addr
        LDR     PC, SWI_Addr
        LDR     PC, Prefetch_Addr
        LDR     PC, Abort_Addr
        DCD     0x0
        LDR     PC, IRQ_Addr
        LDR     PC, FIQ_Addr


Reset_Addr      DCD     Reset_Go
Undefined_Addr  DCD     Undefined_Handler
SWI_Addr        DCD     SWI_Handler1
Prefetch_Addr   DCD     Prefetch_Handler
Abort_Addr      DCD     Abort_Handler
                DCD     0
IRQ_Addr        DCD     IRQ_Handler
FIQ_Addr        DCD     FIQ_Handler


        ; ************************
        ; Exception Handlers
        ; ************************

        ; The following dummy handlers do not do anything useful in this example.
        ; They are set up here for completeness.

Undefined_Handler
        B       Undefined_Handler
SWI_Handler1
        B       SWI_Handler1
Prefetch_Handler
        B       Prefetch_Handler
Abort_Handler
        B       Abort_Handler
IRQ_Handler
        B       IRQ_Handler
FIQ_Handler
        B       FIQ_Handler


Reset_Go
    ; Disable Interrupt in case code is load by ICE while other firmware is executing
    LDR    r0, =REG_AIC_MDCR
    LDR    r1, =0xFFFFFFFF
    STR    r1, [r0]
    LDR    r0, =REG_AIC_MDCRH
    STR    r1, [r0]
    ;--------------------------------
    ; Initial Stack Pointer register
    ;--------------------------------
    ;INIT_STACK
    LDR    R2, =0xB0001800
    LDR    R3,[R2,#0x0010]
    AND    R3, R3, #0x00000007
    MOV    R1,#2
    MOV    R0,#1
LOOP_DRAMSIZE
    CMP    R0,R3
    BEQ    DONE_DRAMSIZE
    LSL    R1,R1,#1
    ADD    R0,R0,#1
    B    LOOP_DRAMSIZE
DONE_DRAMSIZE
    ; Using DRAM Size to set Stack Pointer
    LSL    R0,R1,#20

    ; Enter Undefined Instruction Mode and set Stack Pointer
    MSR    CPSR_c, #UDF_MODE:OR:I_BIT:OR:F_BIT
    MOV    SP, R0
    SUB    R0, R0, #UND_Stack_Size

    ; Enter Abort Mode and set Stack Pointer
    MSR    CPSR_c, #ABT_MODE:OR:I_BIT:OR:F_BIT
    MOV    SP, R0
    SUB    R0, R0, #ABT_Stack_Size

    ; Enter IRQ Mode and set Stack Pointer
    MSR    CPSR_c, #IRQ_MODE:OR:I_BIT:OR:F_BIT
    MOV    SP, R0
    SUB    R0, R0, #IRQ_Stack_Size

    ; Enter FIQ Mode and set Stack Pointer
    MSR    CPSR_c, #FIQ_MODE:OR:I_BIT:OR:F_BIT
    MOV    SP, R0
    SUB    R0, R0, #FIQ_Stack_Size

    ; Enter User Mode and set Stack Pointer
    MSR    CPSR_c, #SYS_MODE:OR:I_BIT:OR:F_BIT
    MOV    SP, R0
    SUB    R0, R0, #USR_Stack_Size

    ; Enter Supervisor Mode and set Stack Pointer
    MSR    CPSR_c, #SVC_MODE:OR:I_BIT:OR:F_BIT
    MOV    SP, R0
    SUB    R0, R0, #SVC_Stack_Size

    ;------------------------------------------------------
    ; Set the normal exception vector of CP15 control bit
    ;------------------------------------------------------
    MRC p15, 0, r0 , c1, c0     ; r0 := cp15 register 1
    BIC r0, r0, #0x2000         ; Clear bit13 in r1
    MCR p15, 0, r0 , c1, c0     ; cp15 register 1 := r0


    IMPORT  __main
    ;-----------------------------
    ;   enter the C code
    ;-----------------------------
    B   __main

    END




