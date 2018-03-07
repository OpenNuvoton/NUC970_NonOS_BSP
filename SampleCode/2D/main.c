/**************************************************************************//**
* @file     main.c
* @version  V1.00
* $Revision: 3 $
* $Date: 15/07/08 11:04a $
* @brief    NUC970 LCD sample source file
*
* @note
* Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nuc970.h"
#include "sys.h"
#include "lcd.h"
#include "2d.h"

UINT8 ColorPatternData[2][8*8*4] = { // 8*8*4
    {
#include "pat8x8-0.dat"
    },
    {
#include "pat8x8-1.dat"
    },
};

UINT8 picRGB88840x30[] = {
#include "40x30RGB888.dat"
};

UINT8 picRGB565160x120[] = {
#include "160x120RGB565.dat"
};
UINT8 picRGB56564x80[] = {
#include "64x80RGB565.dat"
};

const int ColorTbl[3]= {0xff0000, 0x00ff00, 0x0000ff};

void GE_Rotate(void)
{
    int idx=0;
    void *_ColorSrcBufferPtr2;

    _ColorSrcBufferPtr2 = malloc(800*480*2);

    memcpy((void *)_ColorSrcBufferPtr2, (void *)picRGB565160x120, 160*120*2);

    ge2dSpriteBlt_Screen(0,0,160,120,_ColorSrcBufferPtr2);

    for(idx=0; idx<=7; idx++) {
        ge2dRotation(80, 60, 220, 150, 80, 60,idx);
    }

    free(_ColorSrcBufferPtr2);
}

void GE_StyledLineDraw(void)
{
    int j = 10,k = 10;

    for( ; j<k+5; j++) {
        ge2dLine_DrawStyledLine(10, j, 200, j, PS_SOLID, 0xff, 0x00ff00, MODE_OPAQUE);
    }
    j+=2;
    k = j;
    for( ; j<k+5; j++) {
        ge2dLine_DrawStyledLine(10, j, 200, j, PS_DASH, 0xff, 0x00ff00, MODE_OPAQUE);
    }
    j+=2;
    k = j;
    for( ; j<k+5; j++) {
        ge2dLine_DrawStyledLine(10, j, 200, j, PS_DOT, 0xff, 0x00ff00, MODE_OPAQUE);
    }
    j+=2;
    k = j;
    for( ; j<k+5; j++) {
        ge2dLine_DrawStyledLine(10, j, 200, j, PS_DASHDOT, 0xff, 0x00ff00, MODE_OPAQUE);
    }
    j+=2;
    k = j;
    for( ; j<k+5; j++) {
        ge2dLine_DrawStyledLine(10, j, 200, j, PS_DASHDOTDOT, 0xff, 0x00ff00, MODE_OPAQUE);
    }

    j = 100;
    k = 100;
    for( ; j<k+5; j++) {
        ge2dLine_DrawStyledLine(10, j, 200, j, PS_SOLID, 0xff, 0x00ff00, MODE_TRANSPARENT);
    }
    j+=2;
    k = j;
    for( ; j<k+5; j++) {
        ge2dLine_DrawStyledLine(10, j, 200, j, PS_DASH, 0xff, 0x00ff00, MODE_TRANSPARENT);
    }
    j+=2;
    k = j;
    for( ; j<k+5; j++) {
        ge2dLine_DrawStyledLine(10, j, 200, j, PS_DOT, 0xff, 0x00ff00, MODE_TRANSPARENT);
    }
    j+=2;
    k = j;
    for( ; j<k+5; j++) {
        ge2dLine_DrawStyledLine(10, j, 200, j, PS_DASHDOT, 0xff, 0x00ff00, MODE_TRANSPARENT);
    }
    j+=2;
    k = j;
    for( ; j<k+5; j++) {
        ge2dLine_DrawStyledLine(10, j, 200, j, PS_DASHDOTDOT, 0xff, 0x00ff00, MODE_TRANSPARENT);
    }
}

void GE_LineDraw(void)
{
    int x1, y1, x2, y2;
    int color, i;
    static int idx = 0; // index to color table
    int scrwitch, scrheight;

    scrwitch = 800 - 1;
    scrheight = 480 -1;

    color = ColorTbl[idx++];
    if (idx==3) idx = 0;

    /*
    ** LCD 240 * 320
    ** 400/4 = 100 (INC_STEP=4)
    */
    x1 = 0;
    y1 = 0;
    x2 = scrwitch;
    y2 = 0+1;

    for (i=0; i<80; i++) {
        ge2dLine_DrawSolidLine(x1, y1, x2, y2, color);
        x1 += 4;
        y2 += 3;
    }

    color = ColorTbl[1];
    x1 = scrwitch;
    y1 = 0;
    x2 = scrwitch;
    y2 = scrheight;
    for (i=0; i<80; i++) {
        ge2dLine_DrawSolidLine(x1, y1, x2, y2, color);
        y1 += 3;
        x2 -= 4;
    }

    color = ColorTbl[2];
    x1 = scrwitch;
    y1 = scrheight;
    x2 = 0;
    y2 = scrheight;
    for (i=0; i<80; i++) {
        ge2dLine_DrawSolidLine(x1, y1, x2, y2, color);
        x1 -= 4;
        y2 -= 3;
    }

    color = 0xFFFFFF;
    x1 = 0;
    y1 = scrheight;
    x2 = 0;
    y2 = 0;
    for (i=0; i<80; i++) {
        ge2dLine_DrawSolidLine(x1, y1, x2, y2, color);
        y1 -= 3;
        x2 += 4;
    }
}

void GE_Alpha(void)
{
    unsigned int ks=0,kd=0;
    void *pic1_ptr,*pic2_ptr;
    void *_ColorSrcBufferPtr2;

    _ColorSrcBufferPtr2 = malloc(160*120*2);
    pic1_ptr = malloc(160*120*2);
    pic2_ptr = malloc(64*80*2);

    ks = kd = 128;

    memcpy((void *)_ColorSrcBufferPtr2, (void *)picRGB565160x120, 160*120*2);
    memcpy((void *)pic1_ptr, (void *)picRGB565160x120, 160*120*2);
    memcpy((void *)pic2_ptr, (void *)picRGB56564x80, 64*80*2);

    ge2dSpriteBlt_Screen(0, 0, 160, 120, _ColorSrcBufferPtr2);
    ge2dBitblt_SetAlphaMode(1, ks, kd);
    ge2dClip_SetClipMode(0);
    ge2dClip_SetClip(20, 80, 160-1, 120-1);
    ge2dSpriteBlt_Screen(20,80, 160,120,pic1_ptr);
    ge2dClip_SetClipMode(1);
    ge2dClip_SetClip(20, 80, 160-1, 120-1);
    ge2dBitblt_SetAlphaMode(0, 0, 0);
    ge2dSpriteBlt_Screen(20,80, 160,120,pic1_ptr);

    ge2dBitblt_SetAlphaMode(1, ks, kd);
    ge2dClip_SetClipMode(0);
    ge2dClip_SetClip(120,55,160-1,120-1);
    ge2dSpriteBlt_Screen(120,55, 64,80,pic2_ptr);
    ge2dClip_SetClipMode(1);
    ge2dClip_SetClip(120,55,160-1,120-1);
    ge2dBitblt_SetAlphaMode(0, 0, 0);
    ge2dSpriteBlt_Screen(120,55, 64,80,pic2_ptr);

    free(_ColorSrcBufferPtr2);
    free(pic1_ptr);
    free(pic2_ptr);

    ge2dClip_SetClip(-1,0,0,0);
    ge2dBitblt_SetAlphaMode(0, 0, 0);
}

void GE_Rop256(void)
{
    int idx;
    int x, y;
    int rop;
    void *_ColorSrcBufferPtr2;

    _ColorSrcBufferPtr2 = (void *)malloc(800*480*2);

    memcpy((void *)_ColorSrcBufferPtr2, (void *)picRGB88840x30, 40*30*4);
    ge2dSpriteBlt_Screen(0, 0, 40, 30, _ColorSrcBufferPtr2);

    x = 0;
    y = 0;

    for (idx=0; idx<256; idx++) { // ROP range from 0 to 255
        rop = idx;
        ge2dSpriteBlt_ScreenRop(x, y, 40, 30, _ColorSrcBufferPtr2, rop);
        x += 40;

        if(x >= 320) {
            x = 0;
            y += 30;
        }

        if(y+30 >= 240) {
            ge2dClearScreen(0x808080);
            x = y = 0;
        }
    }

    ge2dClearScreen(0x808080);

    /* rop testing of color pattern  */
    x = 0;
    y = 0;
    ge2dInitColorPattern(RGB888, ColorPatternData[1]);
    ge2dFill_MonoPattern(0,0,40,30,0);

    for (idx=0; idx<256; idx++) {
        rop = idx;
        ge2dFill_ColorPatternROP(x, y, 40, 30, rop);
        x += 40;
        if(x >= 320) {
            x = 0;
            y += 30;
        }
        if(y+30 >= 240) {
            ge2dClearScreen(0x808080);
            x = y = 0;
        }
    }
    ge2dClearScreen(0x808080);


    /* rop testing of mono pattern */
    x = 0;
    y = 0;
    ge2dInitMonoPattern(HS_DIAGCROSS, 0xff0000, 0x0000ff);
    ge2dFill_MonoPattern(0,0,40,30,MODE_OPAQUE);
    ge2dFill_MonoPattern(40,0,40,30,MODE_TRANSPARENT);

    for (idx=0; idx<256; idx++) {
        rop = idx;
        ge2dFill_MonoPatternROP(x, y, 40, 30, rop, MODE_OPAQUE);

        x += 40;
        if(x >= 320) {
            x = 0;
            y += 30;
        }
        if(y+30 >= 240) {
            ge2dClearScreen(0x808080);
            x = y = 0;
        }
    }

    free(_ColorSrcBufferPtr2);
}

void GE_MonoPatternFill(void)
{
    ge2dInitMonoPattern(HS_HORIZONTAL, 0xffffff, 0x000000);
    ge2dFill_MonoPattern(0, 0, 50, 50, MODE_OPAQUE);

    ge2dInitMonoPattern(HS_VERTICAL, 0xffffff, 0x000000);
    ge2dFill_MonoPattern(50, 0, 50, 50, MODE_OPAQUE);

    ge2dInitMonoPattern(HS_FDIAGONAL, 0xffffff, 0x000000);
    ge2dFill_MonoPattern(100, 0, 50, 50, MODE_OPAQUE);

    ge2dInitMonoPattern(HS_BDIAGONAL, 0xffffff, 0x000000);
    ge2dFill_MonoPattern(150, 0, 50, 50, MODE_OPAQUE);

    ge2dInitMonoPattern(HS_CROSS, 0xffffff, 0x000000);
    ge2dFill_MonoPattern(0, 50, 50, 50, MODE_OPAQUE);

    ge2dInitMonoPattern(HS_DIAGCROSS, 0x0000ff, 0x000000);
    ge2dFill_MonoPattern(50, 50, 50, 50, MODE_OPAQUE);

    ge2dInitMonoPattern(HS_HORIZONTAL, 0xffffff, 0x000000);
    ge2dFill_MonoPattern(0, 150, 50, 50, MODE_TRANSPARENT);

    ge2dInitMonoPattern(HS_VERTICAL, 0xffffff, 0x000000);
    ge2dFill_MonoPattern(50, 150, 50, 50, MODE_TRANSPARENT);

    ge2dInitMonoPattern(HS_FDIAGONAL, 0xffffff, 0x000000);
    ge2dFill_MonoPattern(100, 150, 50, 50, MODE_TRANSPARENT);

    ge2dInitMonoPattern(HS_BDIAGONAL, 0xffffff, 0x000000);
    ge2dFill_MonoPattern(150, 150, 50, 50, MODE_TRANSPARENT);

    ge2dInitMonoPattern(HS_CROSS, 0xffffff, 0x000000);
    ge2dFill_MonoPattern(0, 180, 50, 50, MODE_TRANSPARENT);

    ge2dInitMonoPattern(HS_DIAGCROSS, 0x0000ff, 0x000000);
    ge2dFill_MonoPattern(50, 180, 50, 50, MODE_TRANSPARENT);
}

void GE_ColorPatternFill(void)
{
    ge2dInitColorPattern(RGB888, ColorPatternData[0]);
    ge2dFill_ColorPattern(0, 0, 200, 100);
    ge2dFill_ColorPattern(0, 120, 50, 100);

    ge2dInitColorPattern(RGB888, ColorPatternData[1]);
    ge2dFill_ColorPattern(100, 120, 16, 64);
    ge2dFill_ColorPattern(150, 120, 64, 110);
}

void GE_SolidFill(void)
{
    ge2dFill_Solid(4, 4, 48, 48, 0x0000ff); // NG
    ge2dFill_Solid(3, 49, 64, 100, 0x00ff00); // NG

    ge2dFill_Solid(0, 0, 320, 240, 0xff0000);

    ge2dClip_SetClipMode(MODE_INSIDE_CLIP);
    ge2dClip_SetClip(40, 60, 40+60, 60+100);
    ge2dFill_Solid(0, 0, 320, 240, 0x000000);

    /* color key is destination */
    ge2dClip_SetClip(-1,0,0,0);
    ge2dClearScreen(0x808080);

    ge2dFill_Solid(0, 0, 120, 60, 0xffff00);
    ge2dFill_Solid(10, 10, 80, 40, 0xff0000);

    ge2dFill_Solid(0, 100, 120, 60, 0x00ff00);
    ge2dFill_Solid(10, 110, 80, 40, 0x0000ff);

    ge2dBitblt_SetDrawMode(MODE_DEST_TRANSPARENT, 0x0000ff, 0xffffff);
    ge2dBitblt_SetAlphaMode(1, 128, 128);
    ge2dBitblt_ScreenToScreen(0, 0, 0, 100, 120, 60);

    ge2dBitblt_SetAlphaMode(0, 0, 0);
    ge2dBitblt_SetDrawMode(MODE_OPAQUE, 0x0, 0x0);

    ge2dClip_SetClip(-1,0,0,0);
}

void GE_ScreenToScreenBLT(void)
{
    ge2dFill_Solid(50, 50, 40, 40, 0xffff00);
    ge2dFill_SolidBackground(100, 100, 50, 50, 0x0);
    ge2dBitblt_ScreenToScreen(50, 50, 180, 80, 80, 80);

    ge2dClearScreen(0x808080);

    ge2dFill_Solid(0, 50, 40, 40, 0x00ff00);
    ge2dBitblt_ScreenToScreen(0, 50, 10, 0, 40, 40);

    ge2dClearScreen(0x808080);

    ge2dFill_Solid(10, 0, 40, 40, 0xff0000);
    ge2dBitblt_ScreenToScreen(10, 0, 0, 50, 40, 40);

    ge2dClearScreen(0x808080);

    ge2dFill_Solid(0, 0, 40, 40, 0x0000ff);
    ge2dBitblt_ScreenToScreen(0, 0, 50, 50, 40, 40);
}

void GE_FontBLT(void)
{
	ge2dFont_PutString(10,50, "8x8 font",0xff0000, 0xff, MODE_TRANSPARENT, F8x8);
	ge2dFont_PutString(100,50,"nuvoton...", 0xff0000, 0xff, MODE_OPAQUE, F8x8);
 	ge2dFont_PutString(100,80,"NUVOTON", 0xff00, 0xff, MODE_TRANSPARENT, F8x8);
 	
 	ge2dFont_PutString(100,100,"nuvoton...", 0xff0000, 0xff, MODE_OPAQUE, F8x16);
	ge2dFont_PutString(10,100, "8x16 font", 0xff00, 0xff, MODE_TRANSPARENT, F8x16);
 	ge2dFont_PutString(100,130,"NUVOTON", 0xff00, 0xff, MODE_TRANSPARENT, F8x16);
}

int32_t main(void)
{
    uint8_t *u8FrameBufPtr;
    uint32_t item;

    outpw(REG_CLK_HCLKEN, 0x0527);
    outpw(REG_CLK_PCLKEN0, 0);
    outpw(REG_CLK_PCLKEN1, 0);

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();

    // Configure multi-function pin for LCD interface
    //GPG6 (CLK), GPG7 (HSYNC)
    outpw(REG_SYS_GPG_MFPL, (inpw(REG_SYS_GPG_MFPL)& ~0xFF000000) | 0x22000000);
    //GPG8 (VSYNC), GPG9 (DEN)
    outpw(REG_SYS_GPG_MFPH, (inpw(REG_SYS_GPG_MFPH)& ~0xFF) | 0x22);

    //DATA pin
    //GPA0 ~ GPA7 (DATA0~7)
    outpw(REG_SYS_GPA_MFPL, 0x22222222);
    //GPA8 ~ GPA15 (DATA8~15)
    outpw(REG_SYS_GPA_MFPH, 0x22222222);
    //GPD8~D15 (DATA16~23)
    outpw(REG_SYS_GPD_MFPH, (inpw(REG_SYS_GPD_MFPH)& ~0xFFFFFFFF) | 0x22222222);

    // LCD clock is selected from UPLL and divide to 20MHz
    outpw(REG_CLK_DIVCTL1, (inpw(REG_CLK_DIVCTL1) & ~0xff1f) | 0xe18);

    // Init LCD interface for E50A2V1 LCD module
    vpostLCMInit(DIS_PANEL_E50A2V1);
    // Set scale to 1:1
    vpostVAScalingCtrl(1, 0, 1, 0, VA_SCALE_INTERPOLATION);

    // Set display color depth
    vpostSetVASrc(VA_SRC_RGB565);

    // Get pointer of video frame buffer
    // Note: before get pointer of frame buffer, must set display color depth first
    u8FrameBufPtr = vpostGetFrameBuffer();
    if(u8FrameBufPtr == NULL) {
        sysprintf("Get buffer error !!\n");
        return 0;
    }

    ge2dInit(16, 800, 480, (void *)u8FrameBufPtr);
    ge2dClearScreen(0x808080);

    vpostVAStartTrigger();

    // main menu
    while(1) {
        sysprintf(">>> Test Items <<<\n\n");
        sysprintf("01. Bitblt Test\n");
        sysprintf("02. Soild Fill\n");
        sysprintf("03. Color Pattern Fill\n");
        sysprintf("04. Mono Pattern Fill\n");
        sysprintf("05. ROP Test\n");
        sysprintf("06. Alpha Test\n");
        sysprintf("07. Line Draw\n");
        sysprintf("08. Styled Line Draw\n");
        sysprintf("09. Roatate\n");
        sysprintf("00.  Font\n");
        sysprintf("\n");
        sysprintf("Choose the test item : ");

        item = sysGetChar() - 0x30;

        ge2dClearScreen(0x808080);

        switch(item) {
        case 1: /* BitBlt Test */
            GE_ScreenToScreenBLT();
            break;

        case 2: /* Solid Fill */
            GE_SolidFill();
            break;

        case 3: /* Color Pattern Fill */
            GE_ColorPatternFill();
            break;

        case 4: /* Mono Pattern Fill */
            GE_MonoPatternFill();
            break;

        case 5: /* ROP TEST */
            GE_Rop256();
            break;

        case 6: /* Alpha Test */
            GE_Alpha();
            break;

        case 7: /* Line draw */
            GE_LineDraw();
            break;

        case 8: /* Styled Line Draw */
            GE_StyledLineDraw();
            break;

        case 9: /* Rotate */
            GE_Rotate();
            break;
        
        case 0: /* Font */
            GE_FontBLT();
            break;
        }
    }
}
/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/

