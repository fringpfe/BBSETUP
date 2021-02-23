/**
 *
 *  Copyright (C) 2018 Roman Pauer
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

#ifndef OPM_H
#define OPM_H

#include <stdint.h>

#define BBOPM_ALLOCATED_BUFFER 1
#define BBOPM_MODIFIED 2
#define BBOPM_UNKNOWN4 4
#define BBOPM_VIEW 8
#define BBOPM_TRANSPARENCY 0x10

typedef struct OPM_Struct_{
    unsigned int flags;
    short width;
    short height;
    short bytes_per_pixel;
    unsigned char *buffer;
    unsigned int  size;
    short bytes_per_pixel2;
    short stride;
    short origin_x;
    short origin_y;
    short clip_x;
    short clip_y;
    short clip_width;
    short clip_height;
    unsigned short transparent_color;
    short view_x;
    short view_y;
    struct OPM_Struct_ *base_pixel_map;
} OPM_Struct;

typedef struct {
    unsigned char height;
    unsigned char width;
    unsigned char* bitmap;
} OPM_FontStruct;

extern OPM_FontStruct OPM_SmallFont;
extern OPM_FontStruct OPM_MediumFont;

extern unsigned int ASM_StringFontWidth;

extern int OPM_New(unsigned int width, unsigned int height, unsigned int bytes_per_pixel, OPM_Struct *pixel_map, unsigned char *buffer);
extern void OPM_Del(OPM_Struct *pixel_map);
extern void OPM_SetVirtualClipStart(OPM_Struct *view_pixel_map, int clip_x, int clip_y);
extern void OPM_CreateVirtualOPM(OPM_Struct *base_pixel_map, OPM_Struct *view_pixel_map, int view_x, int view_y, int view_width, int view_height);
extern void OPM_SetPixel(OPM_Struct *pixel_map, int x, int y, unsigned char color);
extern unsigned char OPM_GetPixel(OPM_Struct *pixel_map, int x, int y);
extern void OPM_HorLine(OPM_Struct *pixel_map, int x, int y, int length, unsigned char color);
extern void OPM_VerLine(OPM_Struct *pixel_map, int x, int y, int length, unsigned char color);
extern void OPM_Box(OPM_Struct *pixel_map, int x, int y, int width, int height, unsigned char color);
extern void OPM_FillBox(OPM_Struct *pixel_map, int x, int y, int width, int height, unsigned char color);
extern void OPM_CopyOPMOPM(OPM_Struct *src_pixel_map, OPM_Struct *dst_pixel_map, int src_x, int src_y, int src_width, int src_height, int dst_x, int dst_y);
extern void OPM_DrawString(OPM_Struct *pixel_map, char *string, int x, int y, unsigned char color);
extern void ASM_CopyCursorWithTransparency(unsigned char* old_buffer, unsigned char* new_buffer, unsigned char* mouse_cursor, unsigned char transparent_color, unsigned int length);
extern void ASM_SetFontOptions(int unk1, int unk2, int width, int height, char *buffer, int stride);
extern void ASM_DrawString(int x, int y, int color, int font, char *string);
extern void ASM_DrawCharFromBitmap(char color, int x, int y, int font_height, int font_width, char *bitmap_offset);

#endif /* OPM_H */
