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

#include "ERROR.h"

typedef struct {
    ERROR_PrintErrorPtr PrintData;
    const char *prefix;
    unsigned char data[16];
} ERROR_MessageStruct ;

static ERROR_MessageStruct ERROR_messages[20];
static char ERROR_message_buffer[400];
static ERROR_OutputFuncPtr ERROR_LogPrint;

unsigned short ERROR_num_messages;

static void ERROR_SetLogprintProc(ERROR_OutputFuncPtr output_func_ptr);
static void ERROR_PrintData(char *buffer, const char *data);

void ERROR_Init(ERROR_OutputFuncPtr logprint_proc)
{
    ERROR_ClearStack();
    ERROR_SetLogprintProc(logprint_proc);
}

void ERROR_ClearStack(void)
{
    ERROR_num_messages = 0;
}

static void ERROR_SetLogprintProc(ERROR_OutputFuncPtr output_func_ptr)
{
    ERROR_LogPrint = output_func_ptr;
}

int ERROR_PushError(ERROR_PrintErrorPtr error_print_error_ptr, const char *error_prefix, int error_data_len, const char *error_data)
{
    int data_index;

    if (ERROR_num_messages == 20)
    {
        ERROR_num_messages--;
        ERROR_PushError((ERROR_PrintErrorPtr)ERROR_PrintData, "BBERROR Library", strlen("STACK FULL") + 1, (const char *) "STACK FULL");

        return 1;
    }

    if (error_data_len > 16)
    {
        ERROR_PushError((ERROR_PrintErrorPtr)ERROR_PrintData, "BBERROR Library", strlen("TOO MUCH DATA") + 1, (const char *) "TOO MUCH DATA");

        return 2;
    }

    ERROR_messages[ERROR_num_messages].PrintData = error_print_error_ptr;
    ERROR_messages[ERROR_num_messages].prefix = error_prefix;
    for (data_index = 0; data_index < error_data_len; data_index++)
    {
        ERROR_messages[ERROR_num_messages].data[data_index] = error_data[data_index];
    }
    ERROR_num_messages++;

    return 0;
}

void ERROR_PopError(void)
{
    if (ERROR_num_messages)
    {
        ERROR_num_messages--;
    }
}

int ERROR_IsStackEmpty(void)
{
    return (ERROR_num_messages == 0)?1:0;
}

void ERROR_PrintAllErrors(unsigned int flags)
{
    int index;
    int buflen;

    if (ERROR_num_messages == 0) return;

    if ((flags & 0x20) && (flags & 0x08))
    {
        if (ERROR_LogPrint != NULL)
        {
            ERROR_LogPrint("BBERROR: ERRORSTACK START:--------------\n");
        }
    }

    for (index = 0; index < ERROR_num_messages; index++)
    {
        ERROR_message_buffer[0] = 0;
        buflen = 0;

        if (flags & 0x02)
        {
            if (ERROR_messages[index].prefix != NULL)
            {
                sprintf(ERROR_message_buffer + buflen, "%s: ", ERROR_messages[index].prefix);
                buflen = strlen(ERROR_message_buffer);
            }
        }

        if (flags & 0x01)
        {
            if (ERROR_messages[index].PrintData != NULL)
            {
                ERROR_messages[index].PrintData((char*)(ERROR_message_buffer + buflen), (const char*)ERROR_messages[index].data);
                buflen = strlen(ERROR_message_buffer);
            }
        }

        if (flags & 0x04)
        {
            sprintf(ERROR_message_buffer + buflen, "\n");
            buflen = strlen(ERROR_message_buffer);
        }

        if (flags & 0x20)
        {
            if (ERROR_LogPrint != NULL)
            {
                ERROR_LogPrint(ERROR_message_buffer);
            }
        }
    }

    if ((flags & 0x20) && (flags & 0x10))
    {
        if (ERROR_LogPrint != NULL)
        {
            ERROR_LogPrint("BBERROR: ERRORSTACK END-----------------\n");
        }
    }

    ERROR_num_messages = 0;
}

static void ERROR_PrintData(char *buffer, const char *data)
{
    sprintf(buffer, "INTERNAL ERROR: %s", (const char *)data);
}