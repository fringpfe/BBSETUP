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

#ifndef BLEV_H
#define BLEV_H

#include <stdint.h>
#include "DSA.h"

#define BLEV_EVENT_WAITING 1u
#define BLEV_EVENT_LIST_FULL 2u

typedef struct
{
    char* text;
    int data;
} BLEV_ErrorStruct;

typedef struct
{
    unsigned int blev_flags;
    DSA_GlobalParamStruct* global_params;
    unsigned int system_flags;
    unsigned int key;
    unsigned int mouse_pos_x;
    unsigned int mouse_pos_y;
} BLEV_EventStruct;

extern int BLEV_Init(void);
extern void BLEV_PutEvent(const void *event);
extern void BLEV_GetEvent(BLEV_EventStruct *event);
extern void BLEV_GetKeyState(void);

#endif /* BLEV_H */