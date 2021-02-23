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

#ifndef SYSTEM_H
#define SYSTEM_H

#include "BLEV.h"
#include <stdint.h>

typedef struct
{
    const char* text;
    unsigned int data1;
    unsigned int data2;
} SYSTEM_ErrorDataStruct;


typedef struct
{
    unsigned char info_str[4];
    unsigned char info_data[8];
    unsigned char transparent_color;
    /* unsigned char field_D; */ /* FIXME: Purpose unclear */
    unsigned short position_x;
    unsigned short position_y;
    unsigned short width;
    unsigned short height;
    unsigned char body_str[4];
    unsigned char body_data[256];
    unsigned char ende_str[4];
} SYSTEM_MouseCursorStruct;

typedef struct
{
    unsigned short flags;
    unsigned short posY;
    unsigned short posX;
} SYSTEM_MousePositionStruct;

typedef struct
{
    unsigned short button_status;
    unsigned short field_2;
    unsigned short field_4;
    signed short current_horizontal_count;
    signed short current_vertical_count;
    signed short old_horizontal_count;
    signed short old_vertical_count;
    unsigned short LeftButtonPressed;
    unsigned short RightButtonPressed;
    unsigned short LeftButtonReleased;
    unsigned short RightButtonReleased;
    unsigned short LeftButtonCounter;
    unsigned short RightButtonCounter;
    BLEV_EventStruct left_button_pressed;
    BLEV_EventStruct right_button_pressed;
    BLEV_EventStruct left_button_released;
    BLEV_EventStruct right_button_released;
} SYSTEM_MouseCallbackDataStruct;

extern SYSTEM_MouseCursorStruct SYSTEM_MouseCursorSmall;
extern SYSTEM_MouseCursorStruct* SYSTEM_MouseCursorPtr;
extern unsigned int SYSTEM_MouseCursorSize;
extern unsigned int SYSTEM_MouseStatusFlags;
extern SYSTEM_MouseCallbackDataStruct SYSTEM_MouseCallbackData;

extern void SYSTEM_Init(void);
extern void SYSTEM_Deinit(void);
extern void SYSTEM_mouse_position_sub_21968();
extern void SYSTEM_ShowMousePtr(SYSTEM_MouseCursorStruct* ptrMouseCursor);
extern void SYSTEM_UpdateMousePtr(void);
extern void SYSTEM_RefreshMousePtr(void);
extern void SYSTEM_DrawMousePtr(void);
extern char SYSTEM_SetUserInputFlags(void);

#endif /* SYSTEM_H */