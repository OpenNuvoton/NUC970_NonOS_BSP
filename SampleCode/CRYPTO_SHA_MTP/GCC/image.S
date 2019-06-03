;/*---------------------------------------------------------------------------------------------------------*/
;/*                                                                                                         */
;/* Copyright(c) 2017 Nuvoton Technology Corp. All rights reserved.                                         */
;/*                                                                                                         */
;/*---------------------------------------------------------------------------------------------------------*/


	.syntax	unified

	.globl	ImageDataBase
	.globl	ImageDataLimit

	.align	4
 
	.text
        
ImageDataBase:
    .incbin  "../../image.bin"
ImageDataLimit:
    .space   4
    
    .end
    
    