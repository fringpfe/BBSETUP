/**
 *
 *  Copyright (C) 2021 Fabian Ringpfeil
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 *  this software and associated documentation files (the "Software"), to deal in
 *  the Software without restriction, including without limitation the rights to
 *  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *  of the Software, and to permit persons to whom the Software is furnished to do
 *  so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */

#ifndef DSA_H
#define DSA_H

#include <stdint.h>
#include "OPM.h"

typedef struct {
    char* text;
    int data1;
    int data2;
} DSA_ErrorStruct;

typedef struct
{
    unsigned char peRed;
    unsigned char peGreen;
    unsigned char peBlue;
    unsigned char peFlags;
} LBM_PaletteEntry;

typedef struct
{
    unsigned short number_of_entries;
    unsigned short version;
    LBM_PaletteEntry pal_entry[256];
} LBM_LogPalette;

typedef struct {
    unsigned int flags;
    OPM_Struct* global_pixel_map;
    LBM_LogPalette* global_palette_data;
    unsigned int method_flags;
} DSA_GlobalParamStruct;

extern unsigned short DSA_VideoMode;
extern unsigned short DSA_PreviousVideoMode;
extern unsigned short DSA_InternalMode;

extern unsigned int DSA_MouseUpdateFlag;
extern int max_4_word_77E00;
extern int bank;
extern OPM_Struct bmouseback;
extern OPM_Struct bmouseoldback;
extern OPM_Struct mouseback[4];
extern OPM_Struct mouseoldback[4];
extern DSA_GlobalParamStruct* DSA_ptrGlobalParams;
extern DSA_GlobalParamStruct DSA_globalParams;

extern char *DSA_ScreenBuffer[4];

extern unsigned int DSA_MouseValue_X_Min;
extern unsigned int DSA_MouseValue_X_Max;
extern unsigned int DSA_MouseValue_Y_Min;
extern unsigned int DSA_MouseValue_Y_Max;
extern unsigned int DSA_MouseValue_X_Current;
extern unsigned int DSA_MouseValue_Y_Current;

extern int DSA_Init(void);
extern unsigned int DSA_OpenScreen(OPM_Struct *pixel_map, int method);
extern void DSA_CloseScreen(void);
extern void DSA_CopyMainOPMToScreen(unsigned short flag);
extern void DSA_LoadPal(LBM_LogPalette* src_pal, unsigned int src_offset, unsigned short length, unsigned int dest_offset);
extern void DSA_ActivatePal(void);
extern void DSA_SetPalEntry(int paletteIndex, char redValue, char greenValue, char blueValue);
extern void DSA_StretchOPMToScreen(OPM_Struct *src_pixel_map, OPM_Struct *dest_pixel_map);
extern void VGA_CopyMouseCursor(char *screenBuffer, unsigned char *opmBuffer, int stride, int x, int y, unsigned int opmWidth, unsigned int opmHeight);
extern void VGA_CopyScreenSectionToBuffer(unsigned char *opmBuffer, char *screenBuffer, int stride, int x, int y, unsigned int opmWidth, unsigned int opmHeight);
extern void ASM_copyMouseCursorWithTransparency(char transparent_color, int count, unsigned char *mouse_body, unsigned char *buffer1, unsigned char *buffer2);
extern void VGA_ActivatePal(unsigned int length, LBM_PaletteEntry* pal_entry);

#endif /* DSA_H */