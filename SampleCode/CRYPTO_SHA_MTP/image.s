;/*---------------------------------------------------------------------------------------------------------*/
;/*                                                                                                         */
;/* Copyright(c) 2010 Nuvoton Technology Corp. All rights reserved.                                         */
;/*                                                                                                         */
;/*---------------------------------------------------------------------------------------------------------*/


    AREA _image, DATA, READONLY

    EXPORT  ImageDataBase
    EXPORT  ImageDataLimit

    ALIGN   4
        
ImageDataBase
    INCBIN .\image.bin
ImageDataLimit

    END