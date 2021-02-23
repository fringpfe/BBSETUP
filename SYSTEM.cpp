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

#include "SYSTEM.h"
#include "DSA.h"
#include "BASEMEM.h"
#include "LBM.h"
#include "GUI.h"
#include "ERROR.h"
#include <dos.h>
#include <conio.h>

SYSTEM_MouseCursorStruct SYSTEM_MouseCursorSmall =
{
    {'I','N','F','O'},
    {0x00, 0x00, 0x00, 0x00, 0x64, 0xDF, 0x01, 0x00},
    0xEB,
    0x08,
    0x00,
    0x0B,
    0x0B,
    {'B','O','D','Y'},
    {
        0x00,0x00,0xEB,0xEB,0xEB,0xEB,0xEB,0xEB,0xEB,0xEB,0xEB,
        0x00,0xFF,0x00,0x00,0xEB,0xEB,0xEB,0xEB,0xEB,0xEB,0xEB,
        0x00,0xFF,0xFF,0x00,0x00,0xEB,0xEB,0xEB,0xEB,0xEB,0xEB,
        0x00,0xFF,0xFF,0xFF,0x00,0x00,0xEB,0xEB,0xEB,0xEB,0xEB,
        0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xEB,0xEB,0xEB,0xEB,
        0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0xEB,0xEB,0xEB,0xEB,
        0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0xEB,0xEB,0xEB,
        0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xEB,0xEB,0xEB,0xEB,
        0x00,0x00,0x00,0xFF,0xFF,0x00,0xEB,0xEB,0xEB,0xEB,0xEB,
        0xEB,0xEB,0x00,0xFF,0xFF,0x00,0xEB,0xEB,0xEB,0xEB,0xEB,
        0xEB,0xEB,0xEB,0x00,0x00,0xEB,0xEB,0xEB,0xEB,0xEB,0xEB
    },
    {'E','N','D','E'}
};

/* TODO: Add SYSTEM_MouseCursorStruct SYSTEM_MouseCursorBig */

SYSTEM_MouseCursorStruct* SYSTEM_MouseCursorPtr;
SYSTEM_MousePositionStruct SYSTEM_NewMouseCursorPosition;
unsigned int SYSTEM_MouseCursorSize;
unsigned int SYSTEM_MouseCursorWidth;
unsigned int SYSTEM_MouseCursorHeight;

unsigned short SYSTEM_Initialized;
unsigned int SYSTEM_MouseStatusFlags;

unsigned short vesa_window_write_status_word_797FE; /* FIXME: Purpose unclear */
unsigned short word_7980A; /* FIXME: Purpose unclear */

static void SYSTEM_PrintData(char* buffer, const char *data);

SYSTEM_MouseCallbackDataStruct SYSTEM_MouseCallbackData;
BLEV_EventStruct SYSTEM_MouseEventData;
unsigned short SYSTEM_LeftMouseButtonPressed;
unsigned short SYSTEM_RightMouseButtonPressed;
unsigned int SYSTEM_GlobalMouseCounter;

unsigned int SYSTEM_StatusInterruptHandlers;

unsigned char SYSTEM_KeyPressed[128];

OPM_Struct* mouseGraphBuffer;
int MouseCursorPosXBanks[4];
int MouseCursorPosYBanks[4];

void (__interrupt __far *SYSTEM_OldKeyboardIsr)();
void (__interrupt __far SYSTEM_KeyboardIsr)();

void (__interrupt __far *SYSTEM_OldTimerIsr)();
void (__interrupt __far SYSTEM_TimerIsr)();

void __interrupt __far SYSTEM_KeyboardIsr()
{
    unsigned char rawcode;

    rawcode = inp(0x60); /* read scancode from keyboard controller */

    if (rawcode >= 128)
    {
        SYSTEM_KeyPressed[rawcode & 0x7F] = 0;
    }
    else
    {
        SYSTEM_KeyPressed[rawcode] = 1;
    }

    outp(0x20, 0x20); /* must send EOI to finish interrupt */
}

void __fastcall SYSTEM_MouseTimerHandler()
{
    int cntr_val;
    
    cntr_val = SYSTEM_GlobalMouseCounter++ + 1;
    if ( SYSTEM_MouseCallbackData.LeftButtonCounter != -1
      && ((cntr_val - SYSTEM_MouseCallbackData.LeftButtonCounter) > 20
       || SYSTEM_MouseCallbackData.left_button_pressed.mouse_pos_x - DSA_MouseValue_X_Current >= 2
       || SYSTEM_MouseCallbackData.left_button_pressed.mouse_pos_y - DSA_MouseValue_Y_Current >= 2)
      && !SYSTEM_MouseCallbackData.LeftButtonPressed )
    {
        SYSTEM_MouseCallbackData.LeftButtonCounter = -1;
        BLEV_PutEvent(&SYSTEM_MouseCallbackData.left_button_pressed);
        if ( SYSTEM_MouseCallbackData.LeftButtonReleased )
        {
            SYSTEM_MouseCallbackData.LeftButtonReleased = 0;
            BLEV_PutEvent(&SYSTEM_MouseCallbackData.left_button_released);
        }
    }
    if ( SYSTEM_MouseCallbackData.RightButtonCounter != -1
      && ((SYSTEM_GlobalMouseCounter - SYSTEM_MouseCallbackData.RightButtonCounter) > 20
       || SYSTEM_MouseCallbackData.right_button_pressed.mouse_pos_x - DSA_MouseValue_X_Current >= 2
       || SYSTEM_MouseCallbackData.right_button_pressed.mouse_pos_y - DSA_MouseValue_Y_Current >= 2)
      && !SYSTEM_MouseCallbackData.RightButtonPressed )
    {
        SYSTEM_MouseCallbackData.RightButtonCounter = -1;
        BLEV_PutEvent(&SYSTEM_MouseCallbackData.right_button_pressed);
        if ( SYSTEM_MouseCallbackData.RightButtonReleased )
        {
            SYSTEM_MouseCallbackData.RightButtonReleased = 0;
            BLEV_PutEvent(&SYSTEM_MouseCallbackData.right_button_released);
        }
    }
}

void __interrupt __far SYSTEM_TimerIsr()
{
    /* TODO: Do complete implementation */
    SYSTEM_MouseTimerHandler();
    _chain_intr(SYSTEM_OldTimerIsr);
    outp(0x20, 0x20);
}

#pragma off( check_stack )
void _loadds far ISR_ClickHandler(int max, int mbx, int mcx, int mdx, int msi, int mdi)
{
#pragma aux click_handler parm [EAX] [EBX] [ECX] [EDX] [ESI] [EDI]
/* TODO: Do complete implementation */
#if 0
    SYSTEM_MouseCallbackData.button_status = (unsigned short)max;
    SYSTEM_NewMouseCursorPosition.flags = 1;
    if(!vesa_window_write_status_word_797FE && !word_7980A)
    {
        /* if init value, set old position to current position */
        if (SYSTEM_MouseCallbackData.current_horizontal_count == -10000)
        {
            SYSTEM_MouseCallbackData.old_horizontal_count = (signed short)mdi;
            SYSTEM_MouseCallbackData.old_vertical_count = (signed short)msi;
        }
        else
        {
            SYSTEM_MouseCallbackData.old_horizontal_count = SYSTEM_MouseCallbackData.current_horizontal_count;
            SYSTEM_MouseCallbackData.old_vertical_count = SYSTEM_MouseCallbackData.current_vertical_count;
        }
        SYSTEM_MouseCallbackData.current_horizontal_count = (signed short)mcx;
        SYSTEM_MouseCallbackData.current_vertical_count = (signed short)mdx;

        DSA_MouseValue_X_Current = SYSTEM_MouseCallbackData.current_horizontal_count;
        DSA_MouseValue_Y_Current = SYSTEM_MouseCallbackData.current_vertical_count;

        if (DSA_MouseValue_X_Current >= DSA_MouseValue_X_Min)
        {
            if(DSA_MouseValue_X_Current > DSA_MouseValue_X_Max)
            {
                DSA_MouseValue_X_Current = DSA_MouseValue_X_Max;
            }
        }
        else
        {
            DSA_MouseValue_X_Current = DSA_MouseValue_X_Min;
        }

        if (DSA_MouseValue_Y_Current >= DSA_MouseValue_Y_Min)
        {
            if(DSA_MouseValue_Y_Current > DSA_MouseValue_Y_Max)
            {
                DSA_MouseValue_Y_Current = DSA_MouseValue_Y_Max;
            }
        }
        else
        {
            DSA_MouseValue_Y_Current = DSA_MouseValue_Y_Min;
        }

        if (SYSTEM_MouseCallbackData.button_status & 2)
        {
            SYSTEM_MouseEventData.blev_flags = 0x100;
            SYSTEM_MouseEventData.global_params = DSA_ptrGlobalParams;
            SYSTEM_MouseEventData.system_flags = SYSTEM_SetUserInputFlags();
            SYSTEM_MouseEventData.key = 0;
            SYSTEM_MouseEventData.mouse_pos_x = DSA_MouseValue_X_Current;
            SYSTEM_MouseEventData.mouse_pos_y = DSA_MouseValue_Y_Current;
            BLEV_PutEvent((BLEV_EventStruct*)&SYSTEM_MouseEventData);
            SYSTEM_LeftMouseButtonPressed = 1;

            if (SYSTEM_MouseCallbackData.LeftButtonCounter != -1)
            {
                if ((SYSTEM_GlobalMouseCounter - SYSTEM_MouseCallbackData.LeftButtonCounter) < 20)
                {
                    SYSTEM_MouseCallbackData.LeftButtonPressed = 1;
                    SYSTEM_MouseCallbackData.LeftButtonCounter = -1;
                    SYSTEM_MouseEventData.blev_flags = 0x10;
                    SYSTEM_MouseEventData.global_params = DSA_ptrGlobalParams;
                    SYSTEM_MouseEventData.system_flags = SYSTEM_SetUserInputFlags();
                    SYSTEM_MouseEventData.key = 0;
                    SYSTEM_MouseEventData.mouse_pos_x = DSA_MouseValue_X_Current;
                    SYSTEM_MouseEventData.mouse_pos_y = DSA_MouseValue_Y_Current;
                    BLEV_PutEvent((BLEV_EventStruct*)&SYSTEM_MouseEventData);
                }
            }
            else
            {
                SYSTEM_MouseCallbackData.left_button_pressed.blev_flags = 4;
                SYSTEM_MouseCallbackData.left_button_pressed.global_params = DSA_ptrGlobalParams;
                SYSTEM_MouseCallbackData.left_button_pressed.system_flags = SYSTEM_SetUserInputFlags();
                SYSTEM_MouseCallbackData.left_button_pressed.key = 0;
                SYSTEM_MouseCallbackData.left_button_pressed.mouse_pos_x = DSA_MouseValue_X_Current;
                SYSTEM_MouseCallbackData.left_button_pressed.mouse_pos_y = DSA_MouseValue_Y_Current;
                SYSTEM_MouseCallbackData.LeftButtonCounter = SYSTEM_GlobalMouseCounter;
            }
        }
        else if (SYSTEM_MouseCallbackData.button_status & 4)
        {
            SYSTEM_MouseEventData.blev_flags = 0x200;
            SYSTEM_MouseEventData.global_params = DSA_ptrGlobalParams;
            SYSTEM_MouseEventData.system_flags = SYSTEM_SetUserInputFlags();
            SYSTEM_MouseEventData.key = 0;
            SYSTEM_MouseEventData.mouse_pos_x = DSA_MouseValue_X_Current;
            SYSTEM_MouseEventData.mouse_pos_y = DSA_MouseValue_Y_Current;
            BLEV_PutEvent(&SYSTEM_MouseEventData);
            SYSTEM_LeftMouseButtonPressed = 0;
            SYSTEM_MouseCallbackData.LeftButtonReleased = 1;
            SYSTEM_MouseCallbackData.left_button_released.blev_flags = 8;
            SYSTEM_MouseCallbackData.left_button_released.global_params = DSA_ptrGlobalParams;
            SYSTEM_MouseCallbackData.left_button_released.system_flags = SYSTEM_SetUserInputFlags();
            SYSTEM_MouseCallbackData.left_button_released.key = 0;
            SYSTEM_MouseCallbackData.left_button_released.mouse_pos_x = DSA_MouseValue_X_Current;
            SYSTEM_MouseCallbackData.left_button_released.mouse_pos_y = DSA_MouseValue_Y_Current;
            if (SYSTEM_MouseCallbackData.LeftButtonPressed || SYSTEM_MouseCallbackData.LeftButtonCounter == -1)
            {
                SYSTEM_MouseCallbackData.LeftButtonReleased = 0;
                SYSTEM_MouseCallbackData.LeftButtonPressed = 0;
                BLEV_PutEvent((BLEV_EventStruct*)&SYSTEM_MouseCallbackData.left_button_released);
            }
        }
    }
    #endif
#if 0
    if (DSA_MouseUpdateFlag && !vesa_window_write_status_word_797FE && !word_7980A)
    {
        if (DSA_InternalMode)
        {
            if (DSA_InternalMode == 2)
            {
                /* TODO: code */
            }
        }
        else
        {
            /* TODO: code */
        }
        SYSTEM_DrawMousePtr();
        SYSTEM_RefreshMousePtr();
        if (DSA_InternalMode)
        {
            if (DSA_InternalMode == 2)
            {
                /* TODO: code */
            }
        }
        else
        {
            /* TODO: code */
        }
    }
    SYSTEM_NewMouseCursorPosition.flags = 0;
    #endif
}

/* Dummy function so we can calculate size of
  code to lock (cbc_end - click_handler).
*/
void cbc_end( void )
{
}
#pragma on( check_stack )

void SYSTEM_Init(void)
{
    SYSTEM_ErrorDataStruct data;
    union REGS inregs;
    struct SREGS sregs;
    unsigned char isr_lock_status;
    unsigned char mouse_callback_lock_status;
    void (far *function_ptr)();

    if (!BLEV_Init())
    {
        data.text = "SYSTEM_Init: Cannot init EVENT";
        data.data1 = 0;
        data.data2 = 0;
        ERROR_PushError(SYSTEM_PrintData, "BBSYS Library", 12u, (const char *)&data);
        return;
    }
    if (!SYSTEM_Initialized)
    {
        SYSTEM_StatusInterruptHandlers = 0;

        for (int i = 0; i < 128; i++)
        {
            SYSTEM_KeyPressed[i] = 0;
        }

        if ((SYSTEM_MouseStatusFlags & 4) == 0)
        {
            SYSTEM_OldKeyboardIsr =_dos_getvect(9);
            _dos_setvect(9, SYSTEM_KeyboardIsr);
        }

        SYSTEM_GlobalMouseCounter = 0;

        if ((SYSTEM_MouseStatusFlags & 2) == 0)
        {
            outp(0x43, 0x36);
            outp(0x40, 0xAE);
            outp(0x40, 0x4D);
            SYSTEM_OldTimerIsr =_dos_getvect(8);
            _dos_setvect(8, SYSTEM_TimerIsr);
            SYSTEM_StatusInterruptHandlers |= 1;
        }

        DSA_MouseValue_X_Min = 0;
        DSA_MouseValue_Y_Min = 0;
        DSA_MouseValue_X_Max = 639;
        DSA_MouseValue_Y_Max = 479;
        DSA_MouseValue_X_Current = 319;
        DSA_MouseValue_Y_Current = 239;
        SYSTEM_MouseCallbackData.current_horizontal_count = -10000;
        DSA_MouseUpdateFlag = 0;

        for ( bank = 0; bank < 4; ++bank )
        {
            MouseCursorPosXBanks[bank] = -32000;
        }

        if ((SYSTEM_MouseStatusFlags & 1) == 0 || (SYSTEM_MouseStatusFlags & 8) != 0)
        {
            /* Mouse Reset/Get Mouse Installed Flag */
            inregs.w.ax = 0x00;
            int386(0x33, &inregs, &inregs);
            if (!inregs.w.ax)
            {
                SYSTEM_MouseStatusFlags |= 1; /* Mouse installed */
            }
        }

        if ((SYSTEM_MouseStatusFlags & 1) == 0)
        {
            mouse_callback_lock_status = BASEMEM_LockRegion(&SYSTEM_MouseCallbackData, sizeof(SYSTEM_MouseCallbackData)); /* TODO: Use return value */
            isr_lock_status = BASEMEM_LockRegion(ISR_ClickHandler, (char*)cbc_end - (char near *)ISR_ClickHandler);

            if (mouse_callback_lock_status & (isr_lock_status == 0))
            {
                data.text = "SYSTEM_Init: Cannot lock Region for Mouse Driver Interrupt";
                data.data1 = 0;
                data.data2 = 0;
                ERROR_PushError(SYSTEM_PrintData, "BBSYS Library", 12u, (const char *)&data);
                return;
            }
            inregs.w.ax = 0x1;
            int386( 0x33, &inregs, &inregs );
            /* Set Mouse User Defined Subroutine and Input Mask */
            BASEMEM_FillMemByte(&sregs, sizeof(sregs), 0);
            inregs.w.ax = 0x0C;
            inregs.w.cx = 0x7F; /* Input Mask  (http://helppc.netcore2k.net/interrupt/int-33-c) */
            function_ptr = (void (far *)(void))ISR_ClickHandler;
            inregs.x.edx = FP_OFF(function_ptr);
            sregs.es     = FP_SEG(function_ptr);
            int386x(0x33, &inregs, &inregs, &sregs);
        }
        SYSTEM_MouseCallbackData.LeftButtonPressed = 0;
        SYSTEM_MouseCallbackData.RightButtonPressed = 0;
        SYSTEM_MouseCallbackData.LeftButtonReleased = 0;
        SYSTEM_MouseCallbackData.RightButtonReleased = 0;
        SYSTEM_MouseCallbackData.LeftButtonCounter = -1;
        SYSTEM_MouseCallbackData.RightButtonCounter = -1;
        SYSTEM_LeftMouseButtonPressed = 0;
        SYSTEM_RightMouseButtonPressed = 0;
        SYSTEM_Initialized = 1;
    }
}

void SYSTEM_mouse_position_sub_21968()
{
    union REGS inregs;
    
    if ( SYSTEM_MouseStatusFlags & 0x10 )
    {
        ++SYSTEM_GlobalMouseCounter;
    }
    if ( SYSTEM_MouseStatusFlags & 8 )
    {
        SYSTEM_NewMouseCursorPosition.flags = 1;
        
        inregs.w.ax = 3;
        int386(0x33, &inregs, &inregs);
        SYSTEM_MouseCallbackData.button_status = inregs.w.bx;
        if ( !vesa_window_write_status_word_797FE && !word_7980A )
        {
            inregs.w.ax = 0xB;
            int386(0x33, &inregs, &inregs);
            SYSTEM_MouseCallbackData.old_horizontal_count = SYSTEM_MouseCallbackData.current_horizontal_count;
            SYSTEM_MouseCallbackData.old_vertical_count = SYSTEM_MouseCallbackData.current_vertical_count;
            SYSTEM_MouseCallbackData.current_horizontal_count = inregs.w.cx;
            SYSTEM_MouseCallbackData.current_vertical_count = inregs.w.dx;
            
            DSA_MouseValue_X_Current += SYSTEM_MouseCallbackData.current_vertical_count;
            DSA_MouseValue_Y_Current += SYSTEM_MouseCallbackData.current_horizontal_count;
            
            if ( DSA_MouseValue_X_Current >= DSA_MouseValue_X_Min )
            {
                if ( DSA_MouseValue_X_Current > DSA_MouseValue_X_Max )
                {
                    DSA_MouseValue_X_Current = DSA_MouseValue_X_Max;
                }
            }
            else
            {
                DSA_MouseValue_X_Current = DSA_MouseValue_X_Min;
            }
            if ( DSA_MouseValue_Y_Current >= DSA_MouseValue_Y_Min )
            {
                if ( DSA_MouseValue_Y_Current > DSA_MouseValue_Y_Max )
                {
                    DSA_MouseValue_Y_Current = DSA_MouseValue_Y_Max;
                }
            }
            else
            {
                DSA_MouseValue_Y_Current = DSA_MouseValue_Y_Min;
            }
        }
        
        if ( SYSTEM_MouseCallbackData.button_status & 1 )
        {
            SYSTEM_MouseEventData.blev_flags = 4;
            SYSTEM_MouseEventData.global_params = DSA_ptrGlobalParams;
            SYSTEM_MouseEventData.system_flags = SYSTEM_SetUserInputFlags();
            SYSTEM_MouseEventData.key = 0;
            SYSTEM_MouseEventData.mouse_pos_x = DSA_MouseValue_X_Current;
            SYSTEM_MouseEventData.mouse_pos_y = DSA_MouseValue_Y_Current;
            BLEV_PutEvent(&SYSTEM_MouseEventData);
            SYSTEM_LeftMouseButtonPressed = 1;
        }
        else
        {
            SYSTEM_MouseEventData.blev_flags = 8;
            SYSTEM_MouseEventData.global_params = DSA_ptrGlobalParams;
            SYSTEM_MouseEventData.system_flags = SYSTEM_SetUserInputFlags();
            SYSTEM_MouseEventData.key = 0;
            SYSTEM_MouseEventData.mouse_pos_x = DSA_MouseValue_X_Current;
            SYSTEM_MouseEventData.mouse_pos_y = DSA_MouseValue_Y_Current;
            BLEV_PutEvent(&SYSTEM_MouseEventData);
            SYSTEM_LeftMouseButtonPressed = 0;
        }
        if ( SYSTEM_MouseCallbackData.button_status & 2 )
        {
            SYSTEM_MouseEventData.blev_flags = 0x20;
            SYSTEM_MouseEventData.global_params = DSA_ptrGlobalParams;
            SYSTEM_MouseEventData.system_flags = SYSTEM_SetUserInputFlags();
            SYSTEM_MouseEventData.key = 0;
            SYSTEM_MouseEventData.mouse_pos_x = DSA_MouseValue_X_Current;
            SYSTEM_MouseEventData.mouse_pos_y = DSA_MouseValue_Y_Current;
            BLEV_PutEvent(&SYSTEM_MouseEventData);
            SYSTEM_RightMouseButtonPressed = 1;
        }
        else
        {
            SYSTEM_MouseEventData.blev_flags = 0x40;
            SYSTEM_MouseEventData.global_params = DSA_ptrGlobalParams;
            SYSTEM_MouseEventData.system_flags = SYSTEM_SetUserInputFlags();
            SYSTEM_MouseEventData.key = 0;
            SYSTEM_MouseEventData.mouse_pos_x = DSA_MouseValue_X_Current;
            SYSTEM_MouseEventData.mouse_pos_y = DSA_MouseValue_Y_Current;
            BLEV_PutEvent(&SYSTEM_MouseEventData);
            SYSTEM_RightMouseButtonPressed = 0;
        }
        
        if ( DSA_MouseUpdateFlag && !vesa_window_write_status_word_797FE && !word_7980A )
        {
            if ( DSA_InternalMode )
            {
                if ( DSA_InternalMode == 2 )
                {
                    /* FIXME: Missing function; not implemented yet */
                }
            }
            else
            {
                /* FIXME: Missing function; not implemented yet */
            }
            SYSTEM_DrawMousePtr();
            SYSTEM_RefreshMousePtr();
            if ( DSA_InternalMode )
            {
                if ( DSA_InternalMode == 2 )
                {
                    /* FIXME: Missing function; not implemented yet */
                }
            }
            else
            {
                /* FIXME: Missing function; not implemented yet */
            }
        }
        SYSTEM_NewMouseCursorPosition.flags = 0;
    }
}

char SYSTEM_SetUserInputFlags(void)
{
    char return_flags;
    
    return_flags = 0;
    
    if ( SYSTEM_LeftMouseButtonPressed )
    {
        return_flags = 1;
    }
    if ( SYSTEM_RightMouseButtonPressed )
    {
        return_flags |= 2u;
    }
    if ( SYSTEM_KeyPressed[42] || SYSTEM_KeyPressed[54] )
    {
        return_flags |= 8u;
    }
    if ( SYSTEM_KeyPressed[29] )
    {
        return_flags |= 0x10u;
    }
    if ( SYSTEM_KeyPressed[56] )
    {
        return_flags |= 0x20u;
    }
    
    return return_flags;
}

void SYSTEM_DrawMousePtr(void)
{
    /* TODO: Do complete implementation */
    #if 0
    int bank_loc;

    bank_loc = 1;

    mouseGraphBuffer = &mouseoldback[bank_loc];
    SYSTEM_MouseCallbackData.field_2 = MouseCursorPosXBanks[bank_loc];
    SYSTEM_MouseCallbackData.field_4 = MouseCursorPosYBanks[bank_loc];
    MouseCursorPosXBanks[bank_loc] = -32000;
    unsigned char* opm_buffer = mouseGraphBuffer->buffer;
    unsigned int opm_width = (unsigned __int16)mouseGraphBuffer->width;
    unsigned int opm_height = (unsigned __int16)mouseGraphBuffer->height;

    printf("opm_height, opm_width: %d, %d\n", opm_height, opm_width);
    printf("bmouseoldback, %x\n", &bmouseoldback);

    if ( opm_height >= 1 && opm_width >= 1 )
    {
        switch(DSA_InternalMode)
        {
            case 0:
                break;
            case 1:
                VGA_CopyMouseCursor(
                (char *)DSA_ScreenBuffer[bank_loc],
                opm_buffer,
                (int) mouseGraphBuffer->stride,
                *(unsigned int *)&SYSTEM_MouseCallbackData.field_2,
                *(unsigned int *)&SYSTEM_MouseCallbackData.field_4,
                opm_width,
                opm_height);
                break;
            case 2:
                break;
        }
    }
    #endif
}

void SYSTEM_RefreshMousePtr()
{
    /* TODO: Do complete implementation */
    #if 0
    int bank_loc;

    bank_loc = 1;
    MouseCursorPosXBanks[bank_loc] = SYSTEM_MouseCursorPtr->position_x + DSA_MouseValue_X_Current;
    MouseCursorPosYBanks[bank_loc] = SYSTEM_MouseCursorPtr->position_y + DSA_MouseValue_Y_Current;
    mouseGraphBuffer = &mouseback[bank_loc];
    SYSTEM_NewMouseCursorPosition.posX = MouseCursorPosXBanks[bank_loc];
    SYSTEM_NewMouseCursorPosition.posY = MouseCursorPosYBanks[bank_loc];
    unsigned char* opm_buffer = mouseGraphBuffer->buffer;
    unsigned int opm_width = (unsigned __int16)mouseGraphBuffer->width;
    unsigned int opm_height = (unsigned __int16)mouseGraphBuffer->height;

    /* TODO: Implement corner cases */
    if ( opm_height >= 1 && opm_width >= 1 )
    {
        word_7980A = 1;
        switch(DSA_InternalMode)
        {
            case 0:
                break;
            case 1:
            VGA_CopyScreenSectionToBuffer(
                opm_buffer,
                (char *)DSA_ScreenBuffer[bank_loc],
                (int)mouseGraphBuffer->stride,
                *(unsigned int *)&SYSTEM_NewMouseCursorPosition.posX,
                *(unsigned int *)&SYSTEM_NewMouseCursorPosition.posY,
                opm_width,
                opm_height);
                break;
            case 2:
                break;
        }
        ASM_copyMouseCursorWithTransparency(SYSTEM_MouseCursorPtr->transparent_color, SYSTEM_MouseCursorSize,
                                                    SYSTEM_MouseCursorPtr->body_data, bmouseoldback.buffer, bmouseback.buffer);
        switch(DSA_InternalMode)
        {
            case 0:
                break;
            case 1:
                VGA_CopyMouseCursor(
                    (char *)DSA_ScreenBuffer[bank_loc],
                    opm_buffer,
                    (int) mouseGraphBuffer->stride,
                    *(unsigned int *)&SYSTEM_NewMouseCursorPosition.posX,
                    *(unsigned int *)&SYSTEM_NewMouseCursorPosition.posY,
                    opm_width,
                    opm_height);
                    word_7980A = 0;
                    return;
                break;
            case 2:
                break;
        }
        word_7980A = 0;
    }
    #endif
}

void SYSTEM_ShowMousePtr(SYSTEM_MouseCursorStruct* ptrMouseCursor)
{
    /* TODO: Do complete implementation */
#if 0
    SYSTEM_ErrorDataStruct data;
    unsigned int v4;
    unsigned int mouse_index;
#if 0
    SYSTEM_MouseCursorPtr = (SYSTEM_MouseCursorStruct*)&SYSTEM_MouseCursorSmall;
                    if(OPM_New(SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, 1, &bmouseback, 0))
                    {
                        if(OPM_New(SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, 1, &bmouseoldback, 0))
                        {
                            //TODO

                            // DSA_MouseUpdateFlag = 1;

                             OPM_CopyOPMOPM(DSA_ptrGlobalParams->global_pixel_map,(OPM_Struct *)&bmouseoldback,
                                       0, 0,
                                       SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, 10, 10);

                            ASM_CopyCursorWithTransparency((unsigned char*)&bmouseoldback.buffer[0], (unsigned char*)&bmouseback.buffer[0], SYSTEM_MouseCursorPtr->body_data, SYSTEM_MouseCursorPtr->transparent_color, SYSTEM_MouseCursorPtr->width * SYSTEM_MouseCursorPtr->height);

                            OPM_CopyOPMOPM((OPM_Struct *)&bmouseback,DSA_ptrGlobalParams->global_pixel_map,
                                       0, 0,
                                       SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, 40, 40);
                                       printf("cursor initialised\n");

                        }
                        else
                        {
                            data.text = "SYSTEM_ShowMousePtr: Couldn\'t allocate Mem for bmouseoldback - width,height\'";
                            data.data1 = SYSTEM_MouseCursorPtr->width;
                            data.data2 = SYSTEM_MouseCursorPtr->height;
                            ERROR_PushError(SYSTEM_PrintData, "BBSYS Library", sizeof(data), (const char *) &data);
                        }
                    }
                    else
                        {
                            data.text = "SYSTEM_ShowMousePtr: Couldn\'t allocate Mem for bmouseback - width,height\'";
                            data.data1 = SYSTEM_MouseCursorPtr->width;
                            data.data2 = SYSTEM_MouseCursorPtr->height;
                            ERROR_PushError(SYSTEM_PrintData, "BBSYS Library", sizeof(data), (const char *) &data);
                        }
#endif

    if(!(SYSTEM_MouseStatusFlags & 1) || (SYSTEM_MouseStatusFlags & 8))
    {
        if ((unsigned int)ptrMouseCursor == 2) //TODO Pointer to 2?
        {
            if ( SYSTEM_MouseCursorWidth != -1 || SYSTEM_MouseCursorWidth != SYSTEM_MouseCursorHeight )
            {
                if ( DSA_MouseUpdateFlag )
                {
                    SYSTEM_DrawMousePtr();
                }
                SYSTEM_RefreshMousePtr();
                DSA_MouseUpdateFlag = 1;
            }
            else
            {
                data.text = "SYSTEM_ShowMousePtr: Parameter:SYSTEM_MousePointerOld - No MousePtr initialized";
                data.data1 = 0;
                data.data2 = 0;
                //ERROR_PushError(SYSTEM_PrintData, aBbsysLibrary, 0xCu, &data);
            }
        }
        else
        {
            if (ptrMouseCursor && (unsigned int)ptrMouseCursor != 1) //TODO clarify
            {
                SYSTEM_MouseCursorPtr = ptrMouseCursor;
            }
            else if (ptrMouseCursor)
            {
                //TODO SYSTEM_MouseCursorPtr
                SYSTEM_MouseCursorPtr = (SYSTEM_MouseCursorStruct*)&SYSTEM_MouseCursorSmall;
            }
            else
            {
                SYSTEM_MouseCursorPtr = (SYSTEM_MouseCursorStruct*)&SYSTEM_MouseCursorSmall;
            }

            if (SYSTEM_MouseCursorPtr->width != SYSTEM_MouseCursorWidth || SYSTEM_MouseCursorPtr->height != SYSTEM_MouseCursorHeight)
            {
                if (SYSTEM_MouseCursorWidth != -1)
                {
                    SYSTEM_UpdateMousePtr();
                    mouse_index = 0;
                    if ( max_4_word_77E00 )
                    {
                        do
                        {
                            OPM_Del(&mouseback[mouse_index]);
                            OPM_Del(&mouseoldback[mouse_index]);
                            mouse_index++;
                        }
                        while ( mouse_index < max_4_word_77E00 );
                    }
                    OPM_Del(&bmouseback);
                    OPM_Del(&bmouseoldback);
                }

                SYSTEM_MouseCursorWidth = SYSTEM_MouseCursorPtr->width;
                SYSTEM_MouseCursorHeight = SYSTEM_MouseCursorPtr->height;
                SYSTEM_MouseCursorSize = SYSTEM_MouseCursorPtr->height * SYSTEM_MouseCursorPtr->width;

                v4 = 0;
                if (max_4_word_77E00)
                {
                    while ( 1 )
                    {
                        if (!OPM_New(SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, 1u, &mouseback[v4], 0))
                        {
                            data.text = "SYSTEM_ShowMousePtr: Couldn\'t allocate Mem for mouseback - width,height";
                            data.data1 = SYSTEM_MouseCursorPtr->width;
                            data.data2 = SYSTEM_MouseCursorPtr->height;
                            //ERROR_PushError(SYSTEM_PrintData, aBbsysLibrary, 0xCu, &data);
                            return;
                        }
                        if (!OPM_New(SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, 1u, &mouseoldback[v4], 0))
                        {
                            break;
                        }
                        if ( ++v4 >= max_4_word_77E00 )
                        {
                            goto LABEL_31; //TODO Throw out goto
                            break;
                        }
                    }
                    data.text = "SYSTEM_ShowMousePtr: Couldn\'t allocate Mem for mouseoldback - width,height";
                    data.data1 = SYSTEM_MouseCursorPtr->width;
                    data.data2 = SYSTEM_MouseCursorPtr->height;
                    //ERROR_PushError(SYSTEM_PrintData, aBbsysLibrary, 0xCu, &data);
                }
                else
                {
LABEL_31:
                    if(OPM_New(SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, 1, &bmouseback, 0))
                    {
                        if(OPM_New(SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, 1, &bmouseoldback, 0))
                        {
                            //byte_77E0B = 0; //TODO
                            //byte_77E0A = 0;
                            SYSTEM_MouseCallbackData.current_horizontal_count = -10000;
                            SYSTEM_RefreshMousePtr();
                            word_7980A = 0;
                            vesa_window_write_status_word_797FE = 0;
                            DSA_MouseUpdateFlag = 1;
                        }
                        else
                        {
                            data.text = "SYSTEM_ShowMousePtr: Couldn\'t allocate Mem for bmouseoldback - width,height\'";
                            data.data1 = SYSTEM_MouseCursorPtr->width;
                            data.data2 = SYSTEM_MouseCursorPtr->height;
                            //ERROR_PushError(SYSTEM_PrintData, "BBSYS Library", 12u, &data);
                        }
                    }
                    else
                    {
                        data.text = "SYSTEM_ShowMousePtr: Couldn\'t allocate Mem for bmouseback - width,height\'";
                        data.data1 = SYSTEM_MouseCursorPtr->width;
                        data.data2 = SYSTEM_MouseCursorPtr->height;
                        //ERROR_PushError(SYSTEM_PrintData, "BBSYS Library", 12Cu, &data);
                    }
                }

            }
            else
            {
                if (DSA_MouseUpdateFlag)
                {
                    SYSTEM_DrawMousePtr();
                }
                SYSTEM_RefreshMousePtr();
                DSA_MouseUpdateFlag = 1;
            }

        }
    }
#endif
}

void SYSTEM_Deinit(void)
{
    union REGS inregs;
    unsigned int i;

    if (SYSTEM_Initialized)
    {
        if ((SYSTEM_MouseStatusFlags & 4) == 0)
        {
            _dos_setvect(9, SYSTEM_OldKeyboardIsr);
        }
        if ((SYSTEM_MouseStatusFlags & 2) == 0)
        {
            if ((SYSTEM_StatusInterruptHandlers & 1) != 0)
            {
                outp(0x43, 0x36);
                outp(0x40, 0xFFFFFFFF);
                outp(0x40, 0xFFFFFFFF);
                _dos_setvect(8, SYSTEM_OldTimerIsr);
                SYSTEM_StatusInterruptHandlers &= 0xFE;
            }
        }
        if ((SYSTEM_MouseStatusFlags & 1) == 0 && (SYSTEM_MouseStatusFlags & 8) == 0)
        {
            inregs.w.ax = 0x0;
            int386( 0x33, &inregs, &inregs );
            if ( SYSTEM_MouseCursorWidth != -1 )
            {
                if ( DSA_MouseUpdateFlag )
                {
                    SYSTEM_UpdateMousePtr();
                }
                for ( i = 0; i < max_4_word_77E00; ++i )
                {
                    OPM_Del(&mouseback[i]);
                    OPM_Del(&mouseoldback[i]);
                }
                OPM_Del(&bmouseback);
                OPM_Del(&bmouseoldback);
            }
        }
        SYSTEM_StatusInterruptHandlers = 0;
        SYSTEM_Initialized = 0;
    }
}

void SYSTEM_UpdateMousePtr(void)
{
    if ((SYSTEM_MouseStatusFlags & 1) == 0 || (SYSTEM_MouseStatusFlags & 8) != 0)
    {
        if ( DSA_MouseUpdateFlag )
        {
            DSA_MouseUpdateFlag = 0;
            SYSTEM_DrawMousePtr();
            if ( max_4_word_77E00 == 2 )
            {
                bank = 1 - bank;
                SYSTEM_DrawMousePtr();
                bank = 1 - bank;
            }
        }
    }
}

static void SYSTEM_PrintData(char* buffer, const char *data)
{
    #define DATA (((SYSTEM_ErrorDataStruct *)data))
        sprintf(buffer, "ERROR!: %s  %d, %d", DATA->text, DATA->data1, DATA->data2);
    #undef DATA
}
