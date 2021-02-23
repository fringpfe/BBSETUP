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

#include "FILE.h"
#include "SETUP.h"
#include "DPMI.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bios.h>
#include <io.h>
#include <i86.h>
#include <direct.h>
#include <ctype.h>
#include <dos.h>
#include <fcntl.h>

unsigned char INI_ReadBuffer[100];
unsigned char INI_WriteBuffer[119];

bool INI_ParseEntry(const char *category, const char *item, unsigned char *buffer, int length)
{
    FILE *fp;
    char local_buffer[200];
    char *ptr_local_buffer;
    bool category_found = 0;
    bool retVal = 0;

    unsigned int line_length;
    unsigned int string_length;
    unsigned int category_length;
    unsigned int item_length;

    memset(buffer, 0, length);

    fp = fopen((const char *)&INI_WriteBuffer, "rt");
    if (!fp)
    {
        return 0;
    }

    while (1)
    {
        ptr_local_buffer = fgets((char *)&local_buffer, sizeof(local_buffer), fp);
        if (!ptr_local_buffer)
        {
            break;
        }

        line_length = strspn(local_buffer, " \n\r");
        if (line_length)
        {
            string_length = strlen(&local_buffer[-line_length + 1]);
            memmove(local_buffer, &local_buffer[line_length], string_length); /* remove initial white space chars? */
        }

        line_length = strcspn(local_buffer, " \n\r");
        local_buffer[line_length] = 0;
        if (strlen(local_buffer))
        {
            if (local_buffer[0] == '[') /* FIXME Check for closing bracket */
            {
                if (category_found)
                {
                    break;
                }
                category_length = strlen(&local_buffer[0]) - 2;
                if (!strnicmp(&local_buffer[1], category, category_length))
                {
                    category_found = 1;
                }
            }
            else
            {

                if (strlen(local_buffer) >= strlen(item))
                {
                    item_length = strlen(item);
                }
                else
                {
                    item_length = strlen(local_buffer);
                }
                if (!strnicmp(local_buffer, item, item_length))
                {
                    ptr_local_buffer = strchr(&local_buffer[0], '='); /* get position of = */
                    if (ptr_local_buffer)
                    {
                        strncpy((char *)&buffer[0], (const char *)&ptr_local_buffer[1], length - 1);
                        buffer[length - 1] = 0;
                    }
                    else
                    {
                        *buffer = 0;
                    }
                    retVal = 1;
                    break;
                }
            }
        }
    }

    fclose(fp);
    return retVal;
}

/* Get an entry from the INI file for an 'item' in a specified 'category'. */
unsigned char *INI_ReadEntry(unsigned char *buffer, const char *category, const char *item)
{
    unsigned char *retVal;

    if (INI_ParseEntry(category, item, (unsigned char *)&INI_ReadBuffer, sizeof(INI_ReadBuffer)))
    {
        retVal = (unsigned char *)&INI_ReadBuffer;
    }
    else
    {
        retVal = 0;
    }

    return retVal;
}

int INI_WriteEntry(const char *category, const char *item, const char *value)
{
    char drive[4];
    char dir[132];
    char fname[12];
    char ext[8];

    char path[144];

    FILE *read_fp;
    FILE *write_fp;

    int category_found;
    int itemWritten;

    char *buffer;
    char string[200];
    char buf[200];
    int segLen;
    int newLen;
    int fputsRetVal;
    int strLenCategory;
    int len;
    int retVal;

    /* Split path in buffer to its components and replace the extension with '.tmp' */
    _splitpath((const char *)&INI_WriteBuffer, drive, dir, fname, ext);
    _makepath((char *)&path[0], drive, dir, fname, "TMP");

    /* open file in buffer for reading */
    read_fp = fopen((const char *)&INI_WriteBuffer, "rt");
    if (!read_fp)
    {
        return 0;
    }

    /* open temporary file for writing */
    write_fp = fopen(path, "wt");
    if (write_fp)
    {
        category_found = 0;
        itemWritten = 0;

        while (1)
        {
            buffer = fgets(string, sizeof(string), read_fp);
            if (!buffer)
            {
                break;
            }

            segLen = strspn(string, " \r\n\t");
            if (segLen)
            {
                newLen = strlen(string - segLen + 1);
                memmove(string, string + segLen, newLen);
            }
            segLen = strcspn(string, " \r\n\t");
            string[segLen] = 0;

            if (strlen(string))
            {
                if (!itemWritten)
                {
                    if (string[0] == '[') /* FIXME: if-condition incomplete */
                    {
                        if (category_found)
                        {
                            _bprintf(buf, sizeof(buf), "%s=%s\n", item, value);
                            fputsRetVal = fputs(buf, write_fp);

                            if (fputsRetVal <= 0)
                            {
                                break;
                            }
                            itemWritten = 1;
                        }
                        else
                        {
                            strLenCategory = strlen(&string[0]) - 2;
                            if (!strnicmp(&string[1], category, strLenCategory))
                            {
                                category_found = 1;
                            }
                        }
                    }
                    else
                    {
                        if (strlen(string) >= strlen(item))
                        {
                            len = strlen(item);
                        }
                        else
                        {
                            len = strlen(string);
                        }
                        if (!strnicmp(&string[0], item, len))
                        {
                            _bprintf(string, 200u, "%s=%s", item, value);
                            itemWritten = 1;
                        }
                    }
                }
                fputsRetVal = fputs(string, write_fp);
                if (fputsRetVal <= 0)
                {
                    break;
                }
                fputsRetVal = fputs("\n", write_fp);
                if (fputsRetVal <= 0)
                {
                    break;
                }
            }
            else
            {
                fputsRetVal = fputs("\n", write_fp);
                if (fputsRetVal <= 0)
                {
                    break;
                }
            }
        }

        /* Write item if category already exists. */
        if (category_found)
        {
            if (!itemWritten)
            {
                _bprintf(string, sizeof(string), "%s=%s\n", item, value);
                fputsRetVal = fputs(string, write_fp);
                if (fputsRetVal > 0)
                {
                    itemWritten = 1;
                }
            }
        }
        else /* Write category and item if both do not exist. */
        {
            _bprintf(string, sizeof(string), "[%s]\n", category);
            fputsRetVal = fputs(string, write_fp);
            if (fputsRetVal > 0)
            {
                _bprintf(string, sizeof(string), "%s=%s\n", item, value);
                fputsRetVal = fputs(string, write_fp);
                if (fputsRetVal > 0)
                {
                    itemWritten = 1;
                }
            }
        }
        fclose(read_fp);
        fclose(write_fp);

        if (itemWritten)
        {
            _unlink((const char *)&INI_WriteBuffer);                     /* Erase the file that's stored in the buffer ...*/
            rename((const char *)&path, (const char *)&INI_WriteBuffer); /* ... and replace it with the temporary file. */
        }
        else
        {
            _unlink(path);
        }
        retVal = itemWritten;
    }
    else
    {
        fclose(read_fp);
        retVal = 0;
    }
    return retVal;
}

void INI_WriteEntry_System(void)
{
    char buf[200];

    if (FILE_IsFileExisting((const char *)&INI_WriteBuffer))
    {
        if (DPMI_IsVesaAvailable())
        {
            INI_WriteEntry("SYSTEM", "VESA", "Y");
        }
        else
        {
            INI_WriteEntry("SYSTEM", "VESA", "N");
        }

        if (_bios_equiplist() & 2) /* bit 1: Set to 1 if a math coprocessor is installed */
        {
            INI_WriteEntry("SYSTEM", "FPU", "Y");
        }
        else
        {
            INI_WriteEntry("SYSTEM", "FPU", "N");
        }

        if (SETUP_CdDrive)
        {
            sprintf(buf, "%c", SETUP_CdDrive);
            INI_WriteEntry("SYSTEM", "CD_ROM_DRIVE", buf);
        }

        if (SETUP_TargetDrive)
        {
            sprintf(buf, "%c", SETUP_SourcePath);
            INI_WriteEntry("SYSTEM", "SOURCE_PATH", buf);
        }

        itoa(SETUP_Language + 1, buf, 10);

        INI_WriteEntry("SYSTEM", "LANGUAGE", buf);
    }
}

int INI_WriteEntry_Vesa(void)
{
    union REGS inregs;
    struct SREGS segregs;
    char buf[4];

    if (!DPMI_VbeInfo)
    {
        DPMI_VbeInfo = (VbeInfoBlock *)DPMI_Alloc(sizeof(VbeInfoBlock));
    }

    memset(&rmregs, 0, sizeof(rmregs));
    memset(&segregs, 0, sizeof(SREGS));

    rmregs.eax = 0x4f00;
    rmregs.es = (unsigned int)DPMI_VbeInfo >> 4;
    rmregs.edi = (unsigned int)DPMI_VbeInfo & 0x0F;
    inregs.w.ax = 0x300;
    inregs.w.bx = 0x10;
    inregs.w.cx = 0;

    segregs.es = FP_SEG(&rmregs);
    inregs.x.edi = FP_OFF(&rmregs);
    int386x(0x31, &inregs, &inregs, &segregs);

    if (rmregs.eax != 0x4f)
    {
        return 0;
    }

    if (rmregs.flags & 0x1 == 0x1)
    {
        return 0;
    }

    if (memcmp(DPMI_VbeInfo, "VESA", 4u))
    {
        return 0;
    }

    if (FILE_IsFileExisting((const char *)&INI_WriteBuffer))
    {
        if (DPMI_IsVideoModeSupported(280, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_1024x768x16.2M", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_1024x768x16.2M", "N");
        }

        if (DPMI_IsVideoModeSupported(277, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_800x600x16.2M", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_800x600x16.2M", "N");
        }

        if (DPMI_IsVideoModeSupported(274, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_640x480x16.2M", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_640x480x16.2M", "N");
        }

        if (DPMI_IsVideoModeSupported(279, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_1024x768x64k", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_1024x768x64k", "N");
        }

        if (DPMI_IsVideoModeSupported(276, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_800x600x64k", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_800x600x64k", "N");
        }

        if (DPMI_IsVideoModeSupported(273, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_640x480x64k", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_640x480x64k", "N");
        }

        if (DPMI_IsVideoModeSupported(278, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_1024x768x32k", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_1024x768x32k", "N");
        }

        if (DPMI_IsVideoModeSupported(275, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_800x600x32k", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_800x600x32k", "N");
        }

        if (DPMI_IsVideoModeSupported(272, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_640x480x32k", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_640x480x32k", "N");
        }

        if (DPMI_IsVideoModeSupported(261, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_1024x768x256", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_1024x768x256", "N");
        }

        if (DPMI_IsVideoModeSupported(259, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_800x600x256", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_800x600x256", "N");
        }

        if (DPMI_IsVideoModeSupported(257, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_640x480x256", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_640x480x256", "N");
        }

        if (DPMI_IsVideoModeSupported(256, (short *)DPMI_VbeInfo->VideoModePtr))
        {
            INI_WriteEntry("VESA", "MODE_640x400x256", "Y");
        }
        else
        {
            INI_WriteEntry("VESA", "MODE_640x400x256", "N");
        }

        sprintf(buf, "%li", DPMI_VbeInfo->TotalMemory);
        INI_WriteEntry("VESA", "MEMORY", buf);

        sprintf(buf, "%li", DPMI_VbeInfo->VESAVersion & 0xF);
        INI_WriteEntry("VESA", "VESA_VERSION_SUBNUMBER", buf);

        sprintf(buf, "%li", DPMI_VbeInfo->VESAVersion >> 8);
        INI_WriteEntry("VESA", "VESA_VERSION_NUMBER", buf);

        INI_WriteEntry("VESA", "OEM", (const char *)DPMI_VbeInfo->OEMStringPtr);
    }

    return 1;
}

void INI_WriteEntry_Path(void)
{
    int driveLetter;
    unsigned int pathLen;
    size_t strLenTargetPath;
    unsigned short lenDir;
    char buffer[144];
    char dir[132];
    char path_loc[16];
    char pathBuf[16];
    char fname[12];
    char ext[8];
    unsigned int drive;
    int err;
    int handle;
    unsigned int totalNrOfDrives;
    char drive_1;

    strcpy(pathBuf, "[PATH]\nPATH=");
    if (SETUP_PtrArgvPath)
    {
        _dos_getdrive(&drive);
        getcwd((char *)&buffer, sizeof(buffer));
        _splitpath(SETUP_PtrArgvPath, (char *)&drive_1, (char *)&dir, (char *)&fname, (char *)&ext);
        if (strlen((char *)&drive_1))
        {
            driveLetter = toupper(drive_1) - 0x40;
            _dos_setdrive(driveLetter, &totalNrOfDrives);
        }
        lenDir = strlen(dir);
        if ((signed int)lenDir > 1 && buffer[lenDir + 143] == '\\')
        {
            buffer[lenDir + 143] = 0;
        }
        err = chdir(dir);
        if (!err)
        {
            _bprintf(path_loc, 14u, "%s%s", fname, ext);
            handle = open(path_loc, O_TRUNC | O_CREAT | O_WRONLY, 432);
            if (handle != -1)
            {
                pathLen = strlen(pathBuf);
                write(handle, pathBuf, pathLen);
                strLenTargetPath = strlen((char *)&SETUP_TargetPath);
                write(handle, SETUP_TargetPath, strLenTargetPath + 1);
                close(handle);
            }
        }
        _dos_setdrive(drive, &totalNrOfDrives);
        chdir(buffer);
    }
}

void INI_MakePath(void)
{
    char buf[200];
    char dir[132];
    char fname[12];
    char drive[4];

    if (SETUP_TargetPath)
    {
        sprintf((char *)&buf, "%s\\x.x", (char *)&SETUP_TargetPath);
        _splitpath((char *)&buf, (char *)&drive, (char *)&dir, (char *)&fname, 0);
    }
    else
    {
        _splitpath(SETUP_PtrArgv, (char *)&drive, (char *)&dir, (char *)&fname, 0);
    }

    _splitpath(SETUP_PtrArgv, 0, 0, (char *)&fname, 0);
    _makepath((char *)&INI_WriteBuffer, (char *)&drive, (char *)&dir, (char *)&fname, "ini");
}