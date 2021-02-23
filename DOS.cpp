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

#include "BASEMEM.h"
#include "ERROR.h"
#include "DOS.h"
#include <stdio.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <direct.h>
#include <ctype.h>
#include <dos.h>
#include <io.h>
#include <errno.h>
#include <fcntl.h>

#if !defined(O_RDONLY)
    #define O_RDONLY _O_RDONLY
#endif
#if !defined(O_WRONLY)
    #define O_WRONLY _O_WRONLY
#endif
#if !defined(O_RDWR)
    #define O_RDWR _O_RDWR
#endif
#if !defined(O_APPEND)
    #define O_APPEND _O_APPEND
#endif
#if !defined(O_CREAT)
    #define O_CREAT _O_CREAT
#endif
#if !defined(O_TRUNC)
    #define O_TRUNC _O_TRUNC
#endif
#if !defined(O_BINARY)
    #if defined(_O_BINARY)
        #define O_BINARY _O_BINARY
    #else
        #define O_BINARY 0
    #endif
#endif

typedef struct {
    const char *text;
    const char *filename;
    int data;
    short oserror;
} DOS_ErrorStruct;

typedef struct {
    unsigned int flags;
    int fd;
} DOS_EntryStruct;

#define NUM_ENTRIES 50
#define HISTORY_SIZE 32

static int DOS_initialized = 0;
static int DOS_lseek_origins[3] = {SEEK_CUR, SEEK_SET, SEEK_END};
static unsigned int DOS_history_index = 0;

static char DOS_getcwd_buffer[144];

static char DOS_filenames[NUM_ENTRIES][128];
static DOS_EntryStruct DOS_file_entries[NUM_ENTRIES];
static char DOS_filenames_history[HISTORY_SIZE][128];

static void DOS_LocalPrintError(char *buffer, const char *data);

int DOS_Init(void)
{
    int file_handle;
    DOS_ErrorStruct data;

    DOS_history_index = 0;
    if (!BASEMEM_Init())
    {
        data.text = "DOS_Init: Cannot init BASEMEM";
        data.filename = " ";
        data.data = 0;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return 0;
    }

    if (DOS_initialized)
    {
        return 1;
    }

    for (file_handle = 0; file_handle < NUM_ENTRIES; file_handle++)
    {
        DOS_file_entries[file_handle].flags = 0;
    }

    DOS_initialized = 1;
    return 1;
}

void DOS_Exit(void)
{
    int file_handle;

    if (DOS_initialized)
    {
        for (file_handle = 0; file_handle < NUM_ENTRIES; file_handle++)
        {
            if (DOS_file_entries[file_handle].flags & 1)
            {
                DOS_Close(file_handle);
            }
        }

        DOS_initialized = 0;
    }
}

int DOS_Open(const char *path, unsigned int mode)
{
    int file_handle, open_flags;
    int fd;
    DOS_ErrorStruct data;

    mode = (unsigned short) mode;

    for (file_handle = 0; file_handle < NUM_ENTRIES; file_handle++)
    {
        if (DOS_file_entries[file_handle].flags == 0)
        {
            break;
        }
    }

    if (file_handle >= NUM_ENTRIES)
    {
        data.text = "DOS_Open: No free entry";

        strcpy(DOS_filenames_history[DOS_history_index], path);
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE)
        {
            DOS_history_index = 0;
        }

        data.data = 0;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return -1;
    }

    switch (mode)
    {
        case DOS_OPEN_MODE_READ:
            open_flags = O_BINARY | O_RDONLY;
            break;
        case DOS_OPEN_MODE_CREATE:
            open_flags = O_BINARY | O_WRONLY | O_CREAT | O_TRUNC;
            break;
        case DOS_OPEN_MODE_RDWR:
            open_flags = O_BINARY | O_RDWR;
            break;
        case DOS_OPEN_MODE_APPEND:
            open_flags = O_BINARY | O_WRONLY | O_APPEND;
            break;
        default:
            open_flags = O_RDONLY;
            break;
    }

    if (open_flags | (O_APPEND | O_CREAT))
    {
        fd = open(path, open_flags, 0x1FF);
    }
    else
    {
        fd = open(path, open_flags);
    }

    if (fd == -1)
    {
        data.text = "DOS_Open: open() oserror";

        strcpy(DOS_filenames_history[DOS_history_index], path);
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE) DOS_history_index = 0;

        data.data = 0;
        data.oserror = errno;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return -1;
    }

    DOS_file_entries[file_handle].flags = mode | 1;
    DOS_file_entries[file_handle].fd = fd;
    strcpy(DOS_filenames[file_handle], path);

#if 0
    if (0 == strcmp(path, "XLDLIBS\\CURRENT\\MERCHDT2.XLD"))
    {
        off_t origpos, endpos;

        origpos = lseek(fd, 0, SEEK_CUR);
        if (origpos != -1)
        {
            endpos = lseek(fd, 0, SEEK_END);
            lseek(fd, origpos, SEEK_SET);
        }
    }
#endif

    return (short) file_handle;
}

int DOS_Close(int file_handle)
{
    DOS_ErrorStruct data;

    file_handle = (short) file_handle;

    if (!(DOS_file_entries[file_handle].flags & 1))
    {
        data.text = "DOS_Close: File is not open";
        data.filename = " ";
        data.data = file_handle;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return 0;
    }

    close(DOS_file_entries[file_handle].fd);
    DOS_file_entries[file_handle].flags = 0;
    return 1;
}

int DOS_Read(int file_handle, void *buffer, unsigned int length)
{
    int retval;
    DOS_ErrorStruct data;

    file_handle = (short) file_handle;

    if (!(DOS_file_entries[file_handle].flags & 1))
    {
        data.text = "DOS_Read: File not open";
        data.filename = " ";
        data.data = file_handle;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return -1;
    }

    retval = read(DOS_file_entries[file_handle].fd, buffer, length);

    if (retval != length)
    {
        data.text = "DOS_Read: read() oserror";

        strcpy(DOS_filenames_history[DOS_history_index], DOS_filenames[file_handle]);
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE) DOS_history_index = 0;

        data.data = file_handle;
        data.oserror = errno;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return -1;
    }

    return retval;
}

int DOS_Write(int file_handle, const void *buffer, unsigned int length)
{
    int retval;
    int err;
    DOS_ErrorStruct data;

    file_handle = (short) file_handle;

    if (!(DOS_file_entries[file_handle].flags & 1))
    {
        data.text = "DOS_Write: File not open";
        data.filename = " ";
        data.data = file_handle;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return -1;
    }

    retval = write(DOS_file_entries[file_handle].fd, buffer, length);

    if (retval == -1)
    {
        err = errno;

        data.text = "DOS_Write: write() oserror";

        strcpy(DOS_filenames_history[DOS_history_index], DOS_filenames[file_handle]);
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE) DOS_history_index = 0;

        data.data = file_handle;
        data.oserror = err;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return -1;
    }

    return retval;
}

int DOS_Seek(int file_handle, int origin, int offset)
{
    DOS_ErrorStruct data;

    file_handle = (short) file_handle;

    if (!(DOS_file_entries[file_handle].flags & 1))
    {
        data.text = "DOS_Seek: File not open";
        data.filename = " ";
        data.data = file_handle;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return 0;
    }

    if (lseek(DOS_file_entries[file_handle].fd, offset, DOS_lseek_origins[origin]) == -1)
    {
        data.text = "DOS_Seek: SetFPos() oserror";

        strcpy(DOS_filenames_history[DOS_history_index], DOS_filenames[file_handle]);
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE) DOS_history_index = 0;

        data.data = file_handle;
        data.oserror = errno;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return 0;
    }

    return 1;
}

void *DOS_ReadFile(const char *path, void *buffer, unsigned int buf_len)
{
    int file_handle, length, buf_allocated;
    DOS_ErrorStruct data;

    buf_allocated = 0;
    length = DOS_GetFileLength(path);
    if (length < 0)
    {
        data.text = "DOS_ReadFile: DOS_GetFileLength() error";

        strcpy(DOS_filenames_history[DOS_history_index], path);
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE) DOS_history_index = 0;

        data.data = 0;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return 0;
    }

    if (buffer == NULL)
    {
        buffer = BASEMEM_Alloc(length, BASEMEM_XMS_MEMORY | BASEMEM_ZERO_MEMORY);
        if (buffer == NULL)
        {
            data.text = "DOS_ReadFile: No Mem for File Buf";

            strcpy(DOS_filenames_history[DOS_history_index], path);
            data.filename = DOS_filenames_history[DOS_history_index];
            DOS_history_index++;
            if (DOS_history_index >= HISTORY_SIZE)
            {
                DOS_history_index = 0;
            }

            data.data = length;
            data.oserror = 0;

            ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

            return 0;
        }

        buf_allocated |= 1;
    }

    file_handle = DOS_Open(path, DOS_OPEN_MODE_READ);
    if (file_handle < 0)
    {
        data.text = "DOS_ReadFile: xxx DOS_Open() error";

        strcpy(DOS_filenames_history[DOS_history_index], path);
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE)
        {
            DOS_history_index = 0;
        }

        data.data = file_handle;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        if (buf_allocated & 1)
        {
            BASEMEM_Free(buffer);
        }

        return 0;
    }

    if (DOS_Read(file_handle, buffer, length) < 0)
    {
        data.text = "DOS_ReadFile: DOS_Read() error";

        strcpy(DOS_filenames_history[DOS_history_index], path);
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE) DOS_history_index = 0;

        data.data = file_handle;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        DOS_Close(file_handle);
        return 0;
    }

    DOS_Close(file_handle);
    return buffer;
}

static int DOS_WriteFile(const char *path, const void *buffer, unsigned int length)
{
    int file_handle;
    DOS_ErrorStruct data;

    file_handle = DOS_Open(path, DOS_OPEN_MODE_CREATE);
    if (file_handle < 0)
    {
        data.text = "DOS_WriteFile: DOS_Open() error";

        strcpy(DOS_filenames_history[DOS_history_index], path);
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE) DOS_history_index = 0;

        data.data = file_handle;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return 0;
    }

    if (DOS_Write(file_handle, buffer, length) < 0)
    {
        data.text = "DOS_WriteFile: DOS_Write() error";

        strcpy(DOS_filenames_history[DOS_history_index], path);
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE) DOS_history_index = 0;

        data.data = file_handle;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        DOS_Close(file_handle);
        return 0;
    }

    DOS_Close(file_handle);
    return 1;
}

int DOS_GetFileLength(const char *path)
{
    int file_handle;
    int origpos, endpos;
    DOS_ErrorStruct data;

    file_handle = DOS_Open(path, DOS_OPEN_MODE_READ);
    if (file_handle < 0)
    {
        data.text = "DOS_GetFileLength: Open error";

        strcpy(DOS_filenames_history[DOS_history_index], path);
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE) DOS_history_index = 0;

        data.data = file_handle;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return -1;
    }

    origpos = lseek(DOS_file_entries[file_handle].fd, 0, SEEK_CUR);
    if (origpos == -1)
    {
        data.text = "DOS_GetFileLength: filelength error";

        strcpy(DOS_filenames_history[DOS_history_index], path);
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE) DOS_history_index = 0;

        data.data = file_handle;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return -1;
    }

    endpos = lseek(DOS_file_entries[file_handle].fd, 0, SEEK_END);
    lseek(DOS_file_entries[file_handle].fd, origpos, SEEK_SET);

    DOS_Close(file_handle);
    return endpos;
}

int DOS_exists(const char *path)
{
    int fd;
    DOS_ErrorStruct data;

    fd = open((const char *) &path, O_RDONLY | O_BINARY);
    if (fd == -1)
    {
        if (errno == ENOENT)
        {
            return 0;
        }
        else
        {
            data.text = "DOS_exists: unknown error";

            strcpy(DOS_filenames_history[DOS_history_index], path);
            data.filename = DOS_filenames_history[DOS_history_index];
            DOS_history_index++;
            if (DOS_history_index >= HISTORY_SIZE)
            {
                DOS_history_index = 0;
            }

            data.data = 0;
            data.oserror = errno;

            ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

            return 0;
        }
    }
    else
    {
        close(fd);

        return 1;
    }
}

static const char *DOS_getcurrentdir(void)
{
    char *cur_dir;
    int len, err;
    const char *retval;
    DOS_ErrorStruct data;

    cur_dir = getcwd(DOS_getcwd_buffer, sizeof(DOS_getcwd_buffer));
    if (cur_dir != NULL)
    {
        retval = cur_dir;
    }

    if (retval == NULL)
    {
        data.text = "DOS_getcurrentdir: unknown error";

        strcpy(DOS_filenames_history[DOS_history_index], "---");
        data.filename = DOS_filenames_history[DOS_history_index];
        DOS_history_index++;
        if (DOS_history_index >= HISTORY_SIZE)
        {
            DOS_history_index = 0;
        }

        data.data = 0;
        data.oserror = err;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return NULL;
    }

    return retval;
}

int DOS_setcurrentdir(const char *path)
{
    DOS_ErrorStruct data;
    unsigned int no_of_drives;
    unsigned int drive;

    if (strlen(path) > 2)
    {
        if (path[1] == ':')
        {
            drive = toupper((int)(path[0] - 0x40));
            _dos_setdrive(drive, &no_of_drives);
        }
    }

    if(chdir(path) != -1)
    {
        return 1;
    }

    data.text = "DOS_setcurrentdir: unknown error";

    strcpy(DOS_filenames_history[DOS_history_index], path);
    data.filename = DOS_filenames_history[DOS_history_index];
    DOS_history_index++;
    if (DOS_history_index >= HISTORY_SIZE)
    {
        DOS_history_index = 0;
    }

    data.data = 0;
    data.oserror = ENOENT;

    ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

    return 0;
}

static int DOS_eof(int file_handle)
{
    off_t origpos, endpos;
    DOS_ErrorStruct data;

    file_handle = (short) file_handle;

    if (!(DOS_file_entries[file_handle].flags & 1))
    {
        data.text = "DOS_eof: File not open";
        data.filename = " ";
        data.data = file_handle;
        data.oserror = errno;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return 0;
    }

    origpos = lseek(DOS_file_entries[file_handle].fd, 0, SEEK_CUR);
    if (origpos == -1)
    {
        data.text = "DOS_eof: invalid File Handle";
        data.filename = " ";
        data.data = file_handle;
        data.oserror = errno;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return -1;
    }


    endpos = lseek(DOS_file_entries[file_handle].fd, 0, SEEK_END);
    lseek(DOS_file_entries[file_handle].fd, origpos, SEEK_SET);

    return (origpos == endpos)?1:0;
}

int DOS_GetSeekPosition(int file_handle)
{
    DOS_ErrorStruct data;
    file_handle = (short) file_handle;

    if (!(DOS_file_entries[file_handle].flags & 1))
    {
        data.text = "DOS_GetSeekPosition: File not open";
        data.filename = " ";
        data.data = file_handle;
        data.oserror = 0;

        ERROR_PushError((ERROR_PrintErrorPtr)DOS_LocalPrintError, "BBDOS Library", sizeof(data), (const char *) &data);

        return 0;
    }

    return lseek(DOS_file_entries[file_handle].fd, 0, SEEK_CUR);
}

static void DOS_LocalPrintError(char *buffer, const char *data)
{
#define DATA (((DOS_ErrorStruct *)data))
    sprintf(buffer, " %s - FILENAME: %s - DATA: %ld - OSERROR: %d", DATA->text, DATA->filename, (long int)DATA->data, (int)DATA->oserror);
#undef DATA
}
