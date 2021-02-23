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

/*************************************************************************
 * Functions handling file access.
 *************************************************************************/

#include "GUI.h"
#include "SETUP.h"
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdlib.h>
#include <ctype.h>

bool FILE_IsDriveNumberValid(unsigned char drive_number)
{
    return (drive_number >= 3 && drive_number <= 26);
}

bool FILE_IsFileAccessPermitted(const char *path)
{
    unsigned int attributes = _A_NORMAL;

    return (_dos_getfileattr(path, &attributes) == 0); /* Get the current attributes of the file or directory that path points to. Returns zero if successful.*/
}

bool FILE_IsFileExisting(const char *path)
{
    int handle;

    handle = open(path, O_BINARY); /* Opens a file at the operating system level. Causes the file to be opened in binary mode which means that data
                                    * will be transmitted to and from the file unchanged.*/
    if (!handle)
    {
        return 0;
    }
    close(handle); /* Closes a file at the operating system level. */

    return 1;
}

bool FILE_IsFolderExisting(const char* path)
{
    char path_buffer[200];
    struct dirent *dirp;

    sprintf(path_buffer, "%s\\*.*", path);

    dirp = opendir(path_buffer); /* Used to obtain the list of file names contained in the directory specified by dirname. Returns a pointer to a structure
                                  * required for subsequent calls to readdir to retrieve the file names matching the pattern specified by dirname. */
    if (dirp == NULL) /* The opendir function returns NULL if dirname is not a valid pathname, or if there are no files matching dirname. */
    {
        return 0;
    }
    closedir(dirp); /* Closes the directory specified by dirp and frees the memory allocated by opendir. */

    return 1;
}

int FILE_Create(const char *file_name)
{
    int handle;

    handle = open(file_name, O_BINARY|O_CREAT|O_WRONLY, 0x80); /* Opens a file at the operating system level.
                                                                *  O_BINARY causes the file to be opened in binary mode which means that data will be transmitted to and from the file unchanged.
                                                                *  O_CREAT has no effect when the file indicated by filename already exists; otherwise, the file is created;
                                                                *  O_WRONLY permits the file to be only written.
                                                                */
    if ( handle < 0 )
    {
        GUI_ErrorHandler(1018, (char *)&file_name);
    }
    return close(handle); /* Closes a file at the operating system level. */
}

void FILE_Delete(char *input_path)
{
    /* TODO: Rework function */

    const char *dirname;
    char *stringBuffer;
    char* charLocation;

    char *path;
    char *drive;
    char *ext;
    char *fname;
    char *dir;
    int v20;

    DIR *dirp;
    DIR *dirpa;
    struct dirent *pDirent;
    struct dirent *pDirent1;
    int uVal;

    dirname = input_path;
    SETUP_CriticalErrorFlag = 0;
    if ( input_path[strlen(input_path) - 1] == 0x5C ) /* Add *.* to delete all files if last char in path is a backslash */
    {
        stringBuffer = (char *)malloc(200u);
        sprintf(stringBuffer, "%s*.*", dirname);
        dirname = stringBuffer;
        charLocation = strchr(stringBuffer, '*'); /* Locates the first occurrence of c (converted to a char) in the string pointed to by s. */
    }
    else
    {
        stringBuffer = 0;
        charLocation = strchr(dirname, '*');
    }

    v20 = charLocation || strchr(dirname, '?');
    path = (char *)malloc(200u);
    drive = (char *)malloc(3u);
    dir = (char *)malloc(130u);
    fname = (char *)malloc(9u);
    ext = (char *)malloc(5u);
    if (v20)
    {
        dirp = opendir(dirname); /* Used to obtain the list of file names contained in the directory specified by dirname. */
        if (dirp)
        {
            pDirent = readdir(dirp); /* Obtains information about the next matching file name from the argument dirp. Can be called repeatedly
                                      * to obtain the list of file names contained in the directory specified by the pathname given to opendir. */
        }
        else
        {
            pDirent = 0;
        }
        if(SETUP_CriticalErrorFlag)
        {
            GUI_ErrorHandler(1031);
        }
        while (pDirent)
        {
            if (!(pDirent->d_attr & 0x10))
            {
                _splitpath(dirname, drive, dir, fname, ext); /* Splits up a full pathname into four components consisting of a drive letter, directory path, file name and file name extension. */
                _makepath(path, drive, dir, pDirent->d_name, 0); /* Constructs a full pathname from the components consisting of a drive letter, directory path, file name and file name extension. */
                uVal = unlink(path); /* Deletes the file whose name is the string pointed to by path */
            }
            pDirent = readdir(dirp); /* Obtains information about the next matching file name from the argument dirp. */
        }
        closedir(dirp); /* Closes the directory specified by dirp and frees the memory allocated by opendir. */
    }
    else if (FILE_IsFolderExisting(dirname)) /* Delete all files in specified folder, scan subfolders next. */
    {
        sprintf(path, "%s\\*.*", dirname);
        FILE_Delete(path);

        dirpa = opendir(dirname); /* Used to obtain the list of file names contained in the directory specified by dirname. */
        if (dirpa)
        {
            pDirent1 = readdir(dirpa); /* Obtains information about the next matching file name from the argument dirp. */
        }
        else
        {
            pDirent1 = 0;
        }

        while (pDirent1)
        {
            if ( pDirent1->d_attr & 0x10 && strcmp(pDirent1->d_name, ".\0") && strcmp(pDirent1->d_name, "..\0"))
            {
                sprintf(path, "%s\\%s", dirname, pDirent1->d_name);
                FILE_Delete(path);
            }
            pDirent1 = readdir(dirpa); /* Obtains information about the next matching file name from the argument dirp. */
        }
        closedir(dirpa); /* Closes the directory specified by dirp and frees the memory allocated by opendir. */
        rmdir(dirname); /*  Deletes the specified directory. The directory must not contain any files or directories. */
    }
    else if (FILE_IsFileExisting(dirname))
    {
        uVal = unlink(dirname); /* Deletes the file whose name is the string pointed to by path */
    }
    /* Deallocates the memory block(s) located by the argument ptr */
    _nfree(path);
    _nfree(drive);
    _nfree(dir);
    _nfree(fname);
    _nfree(ext);
    if(stringBuffer)
    {
        _nfree(stringBuffer);
    }
}

unsigned char FILE_GetMaxDriveNumber(void)
{
    return 26u;
}

void FILE_ChangeDir(char *path)
{
    int result;
    char path_buffer[256];
    char dir[132];
    char current_drive;
    char total; /* total number of drives */
    char drive;

    printf("FILE_ChangeDir: %s\n", path);

    /* Add backslash to path if last character is not a backslash (e.g. C:/Temp => C:/Temp/ */
    if (path[strlen(path) - 1] != '\\' && (path[1] != ':' || path[2]))
    {
        sprintf(path_buffer, "%s\\", path);
    }
    else
    {
        strcpy(path_buffer, path);
    }

    _splitpath(path_buffer, (char *)&drive, dir, 0, 0);  /* Splits up a full pathname into four components consisting of a drive letter, directory path, file name and file name extension. */

    /* Set drive to target and check for success */
    drive = toupper(drive); /* Converts input to a uppercase letter if input represents a lowercase letter */
    if (drive >= 'A' && drive <= 'Z')
    {
        _dos_setdrive((unsigned)(drive - 0x40), (unsigned *)&total); /* Sets the current default disk drive to be the drive specified by drive, where 1 = drive A, 2 = drive B, etc. */
        _dos_getdrive((unsigned *)&current_drive); /* Gets the current disk drive number. */
        
        /* Throw error if setting the drive failed. */
        if (drive - 0x40 != current_drive)
        {
            GUI_ErrorHandler(1013, (char *)&drive);
        }
    }

    if (strcmp(dir, "\\") && strlen(dir) && path_buffer[strlen(dir) + 255] == '\\')
    {
        path_buffer[strlen(dir) + 255] = 0;
    }

    /* Check if directory path has actual content and change to specified path. Throw error if changing failed.*/
    if (dir[0])
    {
        if (chdir(dir) == -1)
        {
            GUI_ErrorHandler(1014, (char *)&dir);
        }
    }
}

void FILE_CreateDir(const char *path)
{
    size_t stringLen;
    size_t string_length;
    int path_length;
    char string[256];
    void *folder_array[30];
    int index;
    
    string[0] = 0;
    index = 0;
    do
    {
        do
        {
            do
            {
                string[strlen(string) + 1] = 0;
                stringLen = strlen(string);
                string[stringLen] = path[stringLen];
                path_length = strlen(string);
                if ( path[path_length] == '\\' )
                {
                    break;
                }
            }
            while (path_length != strlen(path));
        }
        while (strlen(string) == 2 && string[1] == ':');
        
        if (!FILE_IsFileAccessPermitted(string))
        {
            if (mkdir(string))
            {
                while (index > 0)
                {
                    rmdir((const char *)folder_array[--index]);
                    _nfree((void *)&folder_array[index]);
                }
                GUI_ErrorHandler(1025, (char *)&path);
            }
            folder_array[index] = malloc(256u);
            strcpy((char *)folder_array[index++], string);
        }
    }
    while (strlen(string) != strlen(path));
    
    while (index > 0)
    {
        _nfree((void *)&folder_array[--index]);
    }
}