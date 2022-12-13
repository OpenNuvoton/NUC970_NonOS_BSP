;/***************************************************************************
; *                                                                         *
; * Copyright (c) 2019 Nuvoton Technology. All rights reserved.             *
; *                                                                         *
; ***************************************************************************/

ENTRY(__vector)

MEMORY
{
  RAM  (rwx)   : ORIGIN = 0x000000, LENGTH = 0x08000000
}

SECTIONS
{
  .text :
  {
    PROVIDE(__image_start = .);
	PROVIDE(__text_start = .);
  
    PROVIDE(__vectors_start = .);
    *(.vectors);
    . = ALIGN(4);
    PROVIDE(__vectors_end = .);
    *(.init);
    . = ALIGN(4);
    *(.text);
    . = ALIGN(4);
    *(.rodata);
    . = ALIGN(4);
    *(.rodata*);
    . = ALIGN(4);

    etext = .;
    
    PROVIDE(__text_end = .);
  } > RAM

  . = ALIGN(4);
  _etext = . ;
  PROVIDE (etext = .);
   
  .data : AT (_etext)
  {
    PROVIDE(__data_start__ = .);
    _data = . ;
    *(.data)
    . = ALIGN(4);
    PROVIDE(__data_end__ = .);
  } > RAM

  . = ALIGN(4);
  _edata = . ;
  PROVIDE (edata = .);

  sbss = .;
  .bss :
  {
    PROVIDE (__bss_start__ = .);
    *(.bss)
    *(.bss.**)
    *(COMMON)
    . = ALIGN(4);
    PROVIDE (__bss_end__ = .);
  }>RAM  
  ebss = .;
  bss_size = ebss - sbss; 

  .heap :
  {
    . = ALIGN(8);
    end = .;
  } > RAM

  PROVIDE_HIDDEN (__exidx_start = .);
  .ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
  PROVIDE_HIDDEN (__exidx_end = .);
}

