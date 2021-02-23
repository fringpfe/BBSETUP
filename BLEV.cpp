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

#include "BLEV.h"
#include "DSA.h"
#include "ERROR.h"
#include "SYSTEM.h"
#include <bios.h>

#define BLEV_MAX_EVENTS 32u

unsigned short BLEV_Initialized;
unsigned char BLEV_Waiting;
unsigned int BLEV_GetEventIndex;
unsigned int BLEV_PutEventIndex;

unsigned short BLEV_KeyTable[128];

BLEV_EventStruct BLEV_EventList[BLEV_MAX_EVENTS];

static void BLEV_PrintData(char* buffer, BLEV_ErrorStruct *data);
void BLEV_WriteKeyTable();

int BLEV_Init()
{
    BLEV_ErrorStruct data;
    int retVal;
    
    if (DSA_Init())
    {
        if (!BLEV_Initialized)
        {
            BLEV_Waiting = 0;
            BLEV_GetEventIndex = 0;
            BLEV_PutEventIndex = 0;
            BLEV_WriteKeyTable();
            BLEV_Initialized = 1;
        }
        retVal = 1;
    }
    else
    {
        data.text = "BLEV_Init: Cannot Init DSA";
        data.data = 0;
        ERROR_PushError((ERROR_PrintErrorPtr)BLEV_PrintData, "BBEVENT Library", sizeof(data), (const char *)&data);
        retVal = 0;
    }
    return retVal;
}

void BLEV_Exit(void)
{
    if (BLEV_Initialized)
    {
        BLEV_Initialized = 0;
    }
}

void BLEV_WriteKeyTable()
{
    unsigned int i;
    
    for (i = 0; i < 128; ++i)
    {
        BLEV_KeyTable[i] = 0;
    }
    BLEV_KeyTable[59] = 0x101;
    BLEV_KeyTable[60] = 0x102;
    BLEV_KeyTable[61] = 0x103;
    BLEV_KeyTable[62] = 0x104;
    BLEV_KeyTable[63] = 0x105;
    BLEV_KeyTable[64] = 0x106;
    BLEV_KeyTable[65] = 0x107;
    BLEV_KeyTable[66] = 0x108;
    BLEV_KeyTable[67] = 0x109;
    BLEV_KeyTable[68] = 0x10A;
    BLEV_KeyTable[5] = 0x10B;
    BLEV_KeyTable[6] = 0x10C;
    BLEV_KeyTable[82] = 0x10E;
    BLEV_KeyTable[83] = 0x10F;
    BLEV_KeyTable[71] = 0x111;
    BLEV_KeyTable[79] = 0x112;
    BLEV_KeyTable[73] = 0x113;
    BLEV_KeyTable[81] = 0x114;
    BLEV_KeyTable[75] = 0x115;
    BLEV_KeyTable[72] = 0x116;
    BLEV_KeyTable[77] = 0x117;
    BLEV_KeyTable[80] = 0x118;
    BLEV_KeyTable[104] = 0x101;
    BLEV_KeyTable[105] = 0x102;
    BLEV_KeyTable[106] = 0x103;
    BLEV_KeyTable[107] = 0x104;
    BLEV_KeyTable[108] = 0x105;
    BLEV_KeyTable[109] = 0x106;
    BLEV_KeyTable[110] = 0x107;
    BLEV_KeyTable[111] = 0x108;
    BLEV_KeyTable[112] = 0x109;
    BLEV_KeyTable[113] = 0x10A;
    BLEV_KeyTable[11] = 0x10B;
    BLEV_KeyTable[12] = 0x10C;
    BLEV_KeyTable[34] = 0x10E;
    BLEV_KeyTable[35] = 0x10F;
    BLEV_KeyTable[23] = 0x111;
    BLEV_KeyTable[31] = 0x112;
    BLEV_KeyTable[25] = 0x113;
    BLEV_KeyTable[33] = 0x114;
    BLEV_KeyTable[27] = 0x115;
    BLEV_KeyTable[29] = 0x116;
    BLEV_KeyTable[24] = 0x117;
    BLEV_KeyTable[32] = 0x118;
    BLEV_KeyTable[94] = 0x101;
    BLEV_KeyTable[95] = 0x102;
    BLEV_KeyTable[96] = 0x103;
    BLEV_KeyTable[97] = 0x104;
    BLEV_KeyTable[98] = 0x105;
    BLEV_KeyTable[99] = 0x106;
    BLEV_KeyTable[100] = 0x107;
    BLEV_KeyTable[101] = 0x108;
    BLEV_KeyTable[102] = 0x109;
    BLEV_KeyTable[103] = 0x10A;
    BLEV_KeyTable[9] = 0x10B;
    BLEV_KeyTable[10] = 0x10C;
    BLEV_KeyTable[20] = 0x10D;
    BLEV_KeyTable[18] = 0x10E;
    BLEV_KeyTable[19] = 0x10F;
    BLEV_KeyTable[119] = 0x111;
    BLEV_KeyTable[117] = 0x112;
    BLEV_KeyTable[4] = 0x113;
    BLEV_KeyTable[118] = 0x114;
    BLEV_KeyTable[115] = 0x115;
    BLEV_KeyTable[116] = 0x116;
    BLEV_KeyTable[13] = 0x117;
    BLEV_KeyTable[17] = 0x118;
    BLEV_KeyTable[84] = 0x101;
    BLEV_KeyTable[85] = 0x102;
    BLEV_KeyTable[86] = 0x103;
    BLEV_KeyTable[87] = 0x104;
    BLEV_KeyTable[88] = 0x105;
    BLEV_KeyTable[89] = 0x106;
    BLEV_KeyTable[90] = 0x107;
    BLEV_KeyTable[91] = 0x108;
    BLEV_KeyTable[92] = 0x109;
    BLEV_KeyTable[93] = 0x10A;
    BLEV_KeyTable[7] = 0x10B;
    BLEV_KeyTable[8] = 0x10C;
}

void BLEV_GetKeyState()
{
    BLEV_EventStruct event;
    int keybrdRetVal;
    int ASCII_character;
    int evtl_scan_code;
    
    if (!(SYSTEM_MouseStatusFlags & 4))
    {
        for (keybrdRetVal = _bios_keybrd(_NKEYBRD_READY); ; keybrdRetVal = _bios_keybrd(_NKEYBRD_READY))
        {
            if (!keybrdRetVal)
            {
                return;
            }
            keybrdRetVal = _bios_keybrd(_NKEYBRD_READ);
            
            ASCII_character = keybrdRetVal;
            evtl_scan_code = (keybrdRetVal & 0x7F00) >> 8;
            
            event.blev_flags = 1;
            event.global_params = (DSA_GlobalParamStruct*)&DSA_ptrGlobalParams;
            event.system_flags = SYSTEM_SetUserInputFlags();
            event.mouse_pos_x = DSA_MouseValue_X_Current;
            event.mouse_pos_y = DSA_MouseValue_Y_Current;
            
            if (ASCII_character < 9u)
            {
                if (ASCII_character)
                {
                    if (ASCII_character == 8)
                    {
                        event.key = 0x110;
                        BLEV_PutEvent(&event);
                    }
                    event.key = ASCII_character;
                    BLEV_PutEvent(&event);
                }
            }
            else
            {
                if (ASCII_character <= 9u)
                {
                    event.key = 0x10D;
                    BLEV_PutEvent(&event);
                }
                if (ASCII_character < 0x1Bu)
                {
                    if (ASCII_character == 0xD)
                    {
                        event.key = 0x119;
                        BLEV_PutEvent(&event);
                    }
                    event.key = ASCII_character;
                    BLEV_PutEvent(&event);
                }
                if (ASCII_character <= 0x1Bu)
                {
                    event.key = 0x100;
                    BLEV_PutEvent(&event);
                }
                if (ASCII_character != 0xE0)
                {
                    event.key = ASCII_character;
                    BLEV_PutEvent(&event);
                }
            }
            
            if (BLEV_KeyTable[evtl_scan_code])
            {
                event.key = BLEV_KeyTable[evtl_scan_code];
            }
            else
            {
                event.key = ASCII_character;
            }
            BLEV_PutEvent(&event);
        }
    }
}

bool BLEV_IsEventWaiting(void)
{
    return BLEV_Waiting != 0;
}

void BLEV_ClearAllEvents()
{
    BLEV_Waiting = 0;
    BLEV_GetEventIndex = 0;
    BLEV_PutEventIndex = 0;
}

void BLEV_PutEvent(const void *event)
{
    if (BLEV_Waiting != BLEV_EVENT_LIST_FULL)
    {
        memcpy(&BLEV_EventList[BLEV_PutEventIndex++], event, sizeof(BLEV_EventStruct));
        
        if (BLEV_PutEventIndex == BLEV_MAX_EVENTS)
        {
            BLEV_PutEventIndex = 0;
        }
        if (BLEV_PutEventIndex == BLEV_GetEventIndex)
        {
            BLEV_Waiting = BLEV_EVENT_LIST_FULL;
        }
        else
        {
            BLEV_Waiting = BLEV_EVENT_WAITING;
        }
    }
}

void BLEV_GetEvent(BLEV_EventStruct *event)
{
    BLEV_GetKeyState();
    if ( BLEV_Waiting )
    {
        memcpy(event, &BLEV_EventList[BLEV_GetEventIndex++], sizeof(BLEV_EventStruct));
        
        if (BLEV_GetEventIndex == 32)
        {
            BLEV_GetEventIndex = 0;
        }
        BLEV_Waiting = BLEV_GetEventIndex != BLEV_PutEventIndex;
    }
    else /* No event waiting, so return current data? */
    {
        event->blev_flags = 0;
        event->global_params = (DSA_GlobalParamStruct*)&DSA_ptrGlobalParams;
        event->system_flags = SYSTEM_SetUserInputFlags(); 
        event->key = 0;
        event->mouse_pos_x = DSA_MouseValue_X_Current;
        event->mouse_pos_y = DSA_MouseValue_Y_Current;
    }
}

void BLEV_GetMousePos(unsigned short *result)
{
    *result = DSA_MouseValue_X_Current;
    result[1] = DSA_MouseValue_Y_Current;
}

static void BLEV_PrintData(char* buffer, BLEV_ErrorStruct *data)
{
    sprintf(buffer, "ERROR! %s %ld", data->text, data->data);
}