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

#ifndef SETUP_H
#define SETUP_H

#include <stdint.h>

typedef struct
{
    char* ptr_entry_string;
    char* key_input;
    unsigned int anchor_point;
    unsigned int x;
    unsigned int y;
    unsigned int max_x;
    unsigned int max_y;
} SETUP_MenuEntryStruct;

typedef struct
{
    unsigned int index;
    SETUP_MenuEntryStruct entry[40];
} SETUP_MenuStruct;

typedef struct
{
    unsigned char* PtrScriptBuffer;
    unsigned int NumberOfLines;
    unsigned int* PtrScriptLine;
} SETUP_ScriptDataStruct;

extern unsigned int  SETUP_Language;
extern unsigned char SETUP_TargetDrive;
extern unsigned char SETUP_CdDrive;
extern unsigned char SETUP_SourcePath[256];
extern unsigned char SETUP_TargetPath[256];
extern char* SETUP_PtrArgvPath;
extern char* SETUP_PtrArgv;
extern unsigned short SETUP_CriticalErrorFlag;
extern unsigned char SETUP_CriticalErrorRetries;
extern unsigned int SETUP_DevError;
extern unsigned int SETUP_ErrCode;

extern SETUP_MenuStruct SETUP_Menu;

#endif /* SETUP_H */