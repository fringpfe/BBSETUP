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

/**********************************************************
 * TODOs:
 * - Refactoring of decompiled code (structure, naming, etc.)
 * - Mouse support
 * - VGA mode 12 support
 * - Replace malloc calls
 *********************************************************/

#include "SETUP.h"
#include "INI.h"
#include "SYSTEM.h"
#include "DSA.h"
#include "OPM.h"
#include "GUI.h"
#include "FILE.h"
#include "LBM.h"
#include "DPMI.h"
#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <conio.h>
#include <direct.h>
#include <malloc.h>

typedef struct
{
    char* command_string;
    int command_table_index;
} SETUP_CommandTableStruct;

/* Command Table which links each script command with a numeral. The numerals are used internally by the script interpreter. */
SETUP_CommandTableStruct SETUP_CommandTable[38] =
{
    {"GOTO", 1000},
    {"END", 1001},
    {"UPDATE_INI", 1002},
    {"IF_EXISTS", 2000},
    {"IF_NOT_EXISTS", 2001},
    {"IF_LANGUAGE", 2002},
    {"ELSE", 2004},
    {"ENDIF", 2005},
    {"COPY", 3000},
    {"INSTALL", 3001},
    {"INSTALL_DIRS", 3015},
    {"MAKEDIR", 3002},
    {"MKDIR", 3002},
    {"MD", 3002},
    {"CREATE", 3003},
    {"DELETE", 3004},
    {"RENAME", 3005},
    {"EXECUTE_SILENT", 3006},
    {"EXECUTE_OWN_SCREEN", 3007},
    {"CD", 3008},
    {"CHDIR", 3008},
    {"SELECT_TARGET_DRIVE", 3009},
    {"SELECT_TARGET_PATH", 3010},
    {"CD_SOURCE", 3011},
    {"CD_TARGET", 3012},
    {"SELECT_CD_ROM_DRIVE", 3014},
    {"WRITE_INI_ENTRY", 3017},
    {"MENU_START", 4000},
    {"MENU_ENTRY", 4001},
    {"MENU_END", 4002},
    {"PRINT", 4003},
    {"INFO", 4006},
    {"ASSERT", 4005},
    {"TEXT", 4004},
    {"ERROR", 4007},
    {"LOAD_BACKGROUND", 4008},
    {"SET_LANGUAGE", 4009},
    {"", 0},
};

/* Bitmap of the default mouse cursor */
unsigned char SETUP_MouseCursor[140] =
{
    0xFA, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFA, 0xFF, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFA, 0xFF, 0xFF, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFA, 0xFF, 0xFF, 0xFF, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFA, 0x00, 0x00, 0x00, 0x00,
    0xFA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFA, 0x00, 0x00, 0x00,
    0xFA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFA, 0x00, 0x00,
    0xFA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFA, 0x00,
    0xFA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFA, 0xFA, 0xFA, 0x00,
    0xFA, 0xFF, 0xFF, 0xFA, 0xFF, 0xFF, 0xFA, 0x00, 0x00, 0x00,
    0xFA, 0xFF, 0xFA, 0xFA, 0xFF, 0xFF, 0xFF, 0xFA, 0x00, 0x00,
    0xFA, 0xFA, 0x00, 0x00, 0xFA, 0xFF, 0xFF, 0xFA, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFA, 0xFF, 0xFF, 0xFA, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xFA, 0xFA, 0x00, 0x00, 0x00
};

unsigned int  SETUP_Language; /* Target language of the setup program */
unsigned char SETUP_TargetDrive; /* Stores the target drive that will be used for the installation */
unsigned char SETUP_CdDrive; /* Stores the number of the system's CD drive */
unsigned char SETUP_SourcePath[256]; /* Path to the files to be installed */
unsigned char SETUP_TargetPath[256]; /* Target path for the installation files */
unsigned char SETUP_ScriptLineBuffer[256]; /* Temporary buffer for each setup script line (used for parsing) */
int* SETUP_KeywordBuffer[20]; /* Temporary buffer for a keyword (or argument) after a script command */

unsigned short SETUP_CriticalErrorFlag;
unsigned char SETUP_CriticalErrorRetries;
unsigned int SETUP_DevError;
unsigned int SETUP_ErrCode;
unsigned __far *SETUP_Devhdr;
bool SETUP_IniUpdated;

char* SETUP_PtrArgv;
char* SETUP_PtrArgvPath;

SETUP_ScriptDataStruct SETUP_ScriptData; /* Contains the relevant script data after parsing */
SETUP_MenuStruct SETUP_Menu;

int __far SETUP_CriticalErrorHandler(unsigned int deverr, unsigned int errcode, unsigned __far *devhdr)
{
    SETUP_CriticalErrorFlag = 1;
    SETUP_DevError = deverr;
    SETUP_ErrCode = errcode;
    SETUP_Devhdr = devhdr;

    if (!errcode)
    {
       return (_HARDERR_FAIL);
    }
    if(errcode == 2)
    {
        return(_HARDERR_FAIL);
    }
    if(SETUP_CriticalErrorRetries > 2)
    {
        return(_HARDERR_FAIL);
    }
    SETUP_CriticalErrorRetries++;
    return(_HARDERR_RETRY);
}

/* Get the configured language and the CD drive number from the already existing INI file. */
void SETUP_GetCdDriveAndLanguage(void)
{
    unsigned char* ptrLanguage;
    unsigned char* ptrCdDrive;

    ptrLanguage = INI_ReadEntry((unsigned char *)&INI_WriteBuffer, "SYSTEM", "LANGUAGE");
    if(ptrLanguage)
    {
        SETUP_Language = atoi((const char *)&ptrLanguage[0]) - 1;
    }

    ptrCdDrive = INI_ReadEntry((unsigned char *)&INI_WriteBuffer, "SYSTEM", "CD_ROM_DRIVE");
    if(ptrCdDrive)
    {
        SETUP_CdDrive = *ptrCdDrive;
    }
}

/* Load and read the script located at 'script_path'; parse and store the script's contents in 'script_data'. */
void SETUP_ParseScript(SETUP_ScriptDataStruct *script_data, const char *script_path)
{
    int script_file_handle;
    int script_length;
    int line_length;
    int line_length_2;
    char* line_ptr;

    /* Open, read and close script file */
    script_file_handle = open(script_path, O_BINARY);
    if (script_file_handle <= 0)
    {
        GUI_ErrorHandler(1002, (char *)&script_path);
    }

    script_length = filelength(script_file_handle);
    if (script_length < 0)
    {
        GUI_ErrorHandler(1003, (char *)&script_path);
    }

    script_data->PtrScriptBuffer = (unsigned char*)malloc(script_length + 1);

    if (read(script_file_handle, script_data->PtrScriptBuffer, script_length) != script_length)
    {
        GUI_ErrorHandler(1003, (char *)&script_path);
    }
    script_data->PtrScriptBuffer[script_length] = 0;

    close(script_file_handle);

    /* Parse script file */
    script_data->NumberOfLines = 0;
    for (int i = 0; i < script_length; ++i )
    {
        if ( script_data->PtrScriptBuffer[i] == '\r' )
        ++script_data->NumberOfLines;
    }

    script_data->PtrScriptLine = (unsigned int*)malloc(4 * (script_data->NumberOfLines + 1));
    script_data->NumberOfLines = 0;
    script_data->PtrScriptLine[script_data->NumberOfLines++] = (unsigned int)script_data->PtrScriptBuffer; /* First entry points to complete ini buffer */

    for (int j = 0; j < script_length; ++j ) /* Save byte index for each new line */
    {
        if ( script_data->PtrScriptBuffer[j] == '\r' )
        {
            script_data->PtrScriptLine[script_data->NumberOfLines++] = (unsigned int)&script_data->PtrScriptBuffer[j + 1];
            script_data->PtrScriptBuffer[j] = 0; /* Replace new line with 0 */
        }
    }
    --script_data->NumberOfLines;

    for (int k = 0; k < script_data->NumberOfLines; ++k )
    {
        line_length = strlen((const char *)script_data->PtrScriptLine[k]);
        for (int l = 0; l < line_length; ++l )
        {
            if ( *(char *)(l + script_data->PtrScriptLine[k]) == '/' && *(char *)(l + script_data->PtrScriptLine[k] + 1) == '/' )
            {
                *(char *)(l + script_data->PtrScriptLine[k]) = 0;
                break;
            }
        }

        for (int m = 0; m < line_length; ++m )
        {
            if ( *(char *)(m + script_data->PtrScriptLine[k]) == 92 && *(char *)(m + script_data->PtrScriptLine[k] + 1) == 'n' )
            {
                *(char *)(m + script_data->PtrScriptLine[k]) = '\n';
                strcpy((char *)(m + script_data->PtrScriptLine[k] + 1), (const char *)(m + script_data->PtrScriptLine[k] + 2));
                --line_length;
                --m;
            }
        }

        while ( *(char *)script_data->PtrScriptLine[k]
             && (*(char *)script_data->PtrScriptLine[k] == '\n'
              || *(char *)script_data->PtrScriptLine[k] == '\r'
              || *(char *)script_data->PtrScriptLine[k] == ' '
              || *(char *)script_data->PtrScriptLine[k] == '\t') )
        {
           ++script_data->PtrScriptLine[k];
        }

        while(strlen((const char*)script_data->PtrScriptLine[k]))
        {
            line_length_2 = strlen((const char*)script_data->PtrScriptLine[k]);
            line_ptr = (char*)script_data->PtrScriptLine[k];

            if(line_ptr[line_length_2 - 1] != '\n')
            {
                if(line_ptr[line_length_2 - 1] != '\r')
                {
                    if(line_ptr[line_length_2 - 1] != 0x1A) /* ASCII: SUB */
                    {
                        if(line_ptr[line_length_2 - 1] != ' ')
                        {
                            if(line_ptr[line_length_2 - 1] != '\t')
                            {
                                break;
                            }
                        }
                    }
                }
            }

            line_ptr[line_length_2 - 1] = 0;
        }
    }
}

unsigned int SETUP_GetCommandTableIndex(SETUP_ScriptDataStruct *script_data, int line_number, SETUP_CommandTableStruct *command_table)
{
    int i;
    int command_length;
    char* line_buffer;

    /* Check if empty line or keyword */
    if ( !*(char*)script_data->PtrScriptLine[line_number] || *(char*)script_data->PtrScriptLine[line_number] == ':' )
    {
        return 0;
    }

    for ( i = 0; ; i++ )
    {
        if ( !command_table[i].command_string )
        {
            return -1;
        }
        command_length = strlen(command_table[i].command_string);

        if (!strncmp(command_table[i].command_string, (char*)(script_data->PtrScriptLine[line_number]), command_length))
        {
            line_buffer = (char*)script_data->PtrScriptLine[line_number];
            if ((line_buffer[command_length] == ' ') || (line_buffer[command_length+1] == '\n') || (line_buffer[command_length+1] == '\r'))
            {
                break;
            }
            /* FIXME: Workaround implemented, decompilation not understandable */
        }
    }
    return command_table[i].command_table_index;
}

/* Stores a specific keyword (or argument) after a script command. Pass the 'script_data', select a 'line_number' and the index of the keyword by using 'keyword_count' */
int * SETUP_GetKeywordsAfterCommand(SETUP_ScriptDataStruct *script_data, int line_number, unsigned int *keyword_count)
{
    int i;
    int string_length;
    int keyword_count_loc;

    strcpy((char*)&SETUP_ScriptLineBuffer, (const char *)script_data->PtrScriptLine[line_number]);

    string_length = strlen((const char*)&SETUP_ScriptLineBuffer);
    keyword_count_loc = 0;

    for ( i = 0; i < string_length; ++i )
    {
        SETUP_KeywordBuffer[keyword_count_loc] = (int*)&SETUP_ScriptLineBuffer[i];
        if ( SETUP_ScriptLineBuffer[i] == '"' ) /* Check if beginning of text */
        {
            SETUP_KeywordBuffer[keyword_count_loc] = (int*)&SETUP_ScriptLineBuffer[i+1];
            ++i;
            while ( i < string_length )
            {
                if ( SETUP_ScriptLineBuffer[i] == '"' && SETUP_ScriptLineBuffer[i] != 92 )
                {
                    SETUP_ScriptLineBuffer[i] = 0;
                    if ( string_length - 1 != i )
                    {
                      ++i;
                    }
                    break;
                }
                ++i;
            }
            if ( i == string_length )
            {
                GUI_ErrorHandler(1011, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            SETUP_ScriptLineBuffer[i] = 0;
        }
        else if ( SETUP_ScriptLineBuffer[i] == ' ' ) /* Check if space between keywords */
        {
            if ( SETUP_ScriptLineBuffer[i] == ' ' )
            {
                continue;
            }
        }
        else
        {
            ++i;
            while ( i < string_length && SETUP_ScriptLineBuffer[i] != ' ' )
            {
               ++i;
            }
            SETUP_ScriptLineBuffer[i] = 0;
        }
        ++keyword_count_loc;
    }
      if ( keyword_count )
      {
        *keyword_count = keyword_count_loc;
      }

      return (int*)SETUP_KeywordBuffer;
}

/* Scan the contents of 'script_data' for a label or anchor point (designated by a leading colon). */
unsigned int SETUP_SearchLabel(SETUP_ScriptDataStruct *script_data, char* label)
{
    unsigned int i;
    char* ptr;
    
    for (i = 0;; ++i)
    {
        if (i >= script_data->NumberOfLines)
        {
            return -1;
        }
        
        ptr = (char *)script_data->PtrScriptLine[i];
        
        if((ptr[0] == ':') && (strcmp(ptr+1, label) == 0))
        {
            break;
        }
    }
    return i;
}

int SETUP_GetLabelLine(SETUP_ScriptDataStruct *script_data, int signifier_position)
{
    int cntr;
    signed int i;
    unsigned int cmdCode;
    
    cntr = 0;
    for ( i = 1; signifier_position != i; ++i )
    {
        cmdCode = SETUP_GetCommandTableIndex((SETUP_ScriptDataStruct *)&script_data, signifier_position, SETUP_CommandTable);
        if ( cmdCode >= 2000 )
        {
            if ( cmdCode <= 2003 )
            {
                ++cntr;
            }
            else if ( cmdCode == 2005 )
            {
                --cntr;
            }
        }
    }
    return cntr;
}

/* */
int SETUP_NextStateConditionalCommand(SETUP_ScriptDataStruct *script_data, int line_number, int target_state)
{
    int index;
    signed int line_offset;
    signed int cmdNr;

    line_offset = 1;
    for ( index = line_number + 1; ; ++index )
    {
        /* Does the index exceed the number of lines? */
        if ( index >= script_data->NumberOfLines )
        {
            GUI_ErrorHandler(1024);
        }

        cmdNr = SETUP_GetCommandTableIndex(script_data, index, SETUP_CommandTable);

        if ( cmdNr < 2000 )
        {
            if ( cmdNr == -1 )
            {
                GUI_ErrorHandler(1006, index + 1, (char*)&script_data->PtrScriptLine[index]);
            }
            continue;
        }

        if ( cmdNr <= 2003 )
        {
            ++line_offset;
            continue;
        }
        if ( cmdNr > 2004 )
        {
            break;
        }
        if ( target_state != 2004 )
        {
            GUI_ErrorHandler(1023, index + 1, (char*)&script_data->PtrScriptLine[index]);
        }
        if ( line_offset == 1 )
        {
            return index + 1;
        }

        if ( cmdNr != 2005 )
        {
            continue;
        }
        if ( --line_offset )
        {
            continue;
        }
    }
    return index;
}

/* Read ASCII text from 'keyword_buffer' and convert it to integer. */
int SETUP_ConvertAsciiToInteger(const char *keyword_buffer, int line_number)
{
    signed int i;
    signed int string_length;
    
    string_length = strlen(keyword_buffer);
    for ( i = 0; i < string_length; ++i )
    {
        if ( keyword_buffer[i] != ' ' && keyword_buffer[i] != '+' && keyword_buffer[i] != '-' && (keyword_buffer[i] < '0' || keyword_buffer[i] > '9') )
        {
            GUI_ErrorHandler(1019, line_number);
        }
    }
    return atoi(keyword_buffer);
}

/* Copy a single file from the path defined in 'src' to the path defined in 'dest'. */
void SETUP_CopyFiles(char* src, char* dest)
{
    int file_handle_src;
    int file_handle_dest;
    signed int len;
    void *buffer;

    file_handle_src = open(src, O_BINARY);

    if(file_handle_src < 0)
    {
        GUI_ErrorHandler(1015, (char*)&src);
    }

    file_handle_dest = open(dest, O_BINARY|O_TRUNC|O_CREAT|O_WRONLY, 128);

    if(file_handle_dest < 0)
    {
        close(file_handle_src);
        GUI_ErrorHandler(1015, (char*)&dest);
    }

    buffer = malloc(32768);

    while(1)
    {
        len = read(file_handle_src, buffer, 32768);
        if(len <=0)
        {
            break;
        }
        GUI_ProgressBarCurrentLength += len;
        if(write(file_handle_dest, buffer, len) != len)
        {
            GUI_ErrorHandler(1016, (char*)&dest);
        }
    }
    _nfree(buffer);
    close(file_handle_src);
    close(file_handle_dest);

    GUI_DrawProgressBar(0);
}

/* Prepare a file's path for proper access and copy the file from its source path 'srcPath' to its target path 'destPath'. */
void SETUP_PrepareAndCopy(char* srcPath, char* destPath, short flag)
{
    char *drive;
    DIR *srcDirp;
    DIR *dirpa;
    struct dirent *src_file_handle;
    struct dirent *src_file_handlea;
    char *buf;
    char *srcPathBuf;
    char *destPathBuf;
    char *fname;
    char *dir;
    char *ext;

    buf = (char *)malloc(200u);
    srcPathBuf = (char *)malloc(200u);
    destPathBuf = (char *)malloc(200u);
    drive = (char *)malloc(3u);
    dir = (char *)malloc(130u);
    fname = (char *)malloc(9u);
    ext = (char *)malloc(5u);

    SETUP_CriticalErrorFlag = 0;
    srcDirp = opendir(srcPath);
    if ( srcDirp )
    {
        src_file_handle = readdir(srcDirp);
    }
    else
    {
        src_file_handle = 0;
    }
    if ( SETUP_CriticalErrorFlag )
    {
        GUI_ErrorHandler(1031);
    }

    while ( src_file_handle )
    {
        if ( !(src_file_handle->d_attr & (_A_VOLID|_A_SUBDIR)))
        {
            _splitpath(srcPath, drive, dir, fname, ext);
            _makepath(srcPathBuf, drive, dir, src_file_handle->d_name, 0);

            _splitpath(destPath, drive, dir, fname, ext);
            _makepath(destPathBuf, drive, dir, src_file_handle->d_name, 0);

            SETUP_CopyFiles(srcPathBuf, destPathBuf);
        }
        src_file_handle = readdir(srcDirp); /* The readdir function can be called repeatedly to obtain the list of file names contained in the directory specified by the pathname given to opendir. */
    }
    closedir(srcDirp);

    if(flag)
    {
        _splitpath(srcPath, drive, dir, 0, 0);
        _makepath(buf, drive, dir, "*", "*");

        dirpa = opendir(buf);
        if(dirpa)
        {
            src_file_handlea = readdir(dirpa);
        }
        else
        {
            src_file_handle = 0;
        }

        while(src_file_handlea)
        {
            if(src_file_handlea->d_attr & 0x10 && strcmp(src_file_handlea->d_name, ".") && strcmp(src_file_handlea->d_name, ".."))
            {
                _splitpath(destPath, drive, dir, fname, ext);
                _makepath(buf, drive, dir, 0, 0);
                sprintf(srcPathBuf, "%s%s", buf, src_file_handlea->d_name);
                mkdir(srcPathBuf);

                sprintf(buf, "%s\%s%s", srcPathBuf, fname, ext);
                _splitpath(srcPath, drive, dir, fname, ext);
                _makepath(srcPathBuf, drive, dir, 0, 0);
                sprintf(destPathBuf, "%s%s\\%s%s", srcPathBuf, src_file_handlea->d_name, fname, ext);
                SETUP_PrepareAndCopy(destPathBuf, buf, flag);
            }
            src_file_handlea = readdir(dirpa);
        }
        closedir(dirpa);
    }
    _nfree(buf);
    _nfree(srcPathBuf);
    _nfree(destPathBuf);
    _nfree(drive);
    _nfree(dir);
    _nfree(fname);
    _nfree(ext);
}

void SETUP_InstallFiles(char* src, char* dest, int flag)
{
    char destPath[100];
    char srcPath[100];

    /* Format source path */
    if ( SETUP_SourcePath[strlen((const char*)&SETUP_SourcePath)] == '\\' )
    {
        sprintf(srcPath, "%s%s", SETUP_SourcePath, src);
    }
    else
    {
        sprintf(srcPath, "%s\\%s", SETUP_SourcePath, src);
    }

    /* Format target path */
    if ( SETUP_TargetPath[strlen((const char*)SETUP_TargetPath)] == '\\' )
    {
        sprintf(destPath, "%s%s", SETUP_TargetPath, dest);
    }
    else
    {
        sprintf(destPath, "%s\\%s", SETUP_TargetPath, dest);
    }

    SETUP_PrepareAndCopy(srcPath, destPath, flag);
}

void SETUP_RunOnConsole(int NrOfScriptKeywords, const char *string)
{
    char dest[256];
    int i;
    
    strcpy((char *)&dest, string++);
    for ( i = 2; i < NrOfScriptKeywords; ++i )
    {
        strcat((char *)&dest, " ");
        strcat((char *)&dest, (const char*)&string[i]);
    }
    system((char *)&dest);
}

unsigned int SETUP_ScriptHandler(SETUP_ScriptDataStruct *script_data, unsigned int line_number, unsigned int *conditional_command)
{
    int command_number;
    int actual_command_number;
    int *keyword_buffer;
    unsigned int keyword_count;
    int signifier_position;
    int retVal;
    int copy4Arg;
    OPM_Struct background_pixel_map;
    unsigned int menu_keyword_length;
    int i;
    int lang_loc;
    int cdrom_ret;
    int targetdrive_ret;
    char* targetpath_ret;

    OPM_Struct pixel_map_loc;

    if ( line_number >= script_data->NumberOfLines || line_number < 0 ) /* report error if line number passed as parameter is higher than the number of lines altogether */
    {
        return -1;
    }
    /* Get command table index from command string */
    command_number = SETUP_GetCommandTableIndex(script_data, line_number, (SETUP_CommandTableStruct *)&SETUP_CommandTable);

    if ( !command_number )
    {
       return line_number + 1; /* Go for next line */
    }

    if ( command_number == -1 )
    {
        GUI_ErrorHandler(1006, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
    }

    /* Get keyword data after command string */
    keyword_buffer = SETUP_GetKeywordsAfterCommand(script_data, line_number, &keyword_count);

    actual_command_number = command_number - 1000;

    if ( (unsigned int)(command_number - 1000) > 3009 )
    {
        GUI_ErrorHandler(1010, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
    }

    switch(actual_command_number)
    {
        case 0: /* GOTO */
        {
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }

            signifier_position = SETUP_SearchLabel(script_data, (char *)keyword_buffer[1]);
            *conditional_command = SETUP_GetLabelLine(script_data, signifier_position); /* Save new line number for next iteration */

            if ( signifier_position == -1 )
            {
                GUI_ErrorHandler(1007, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            return signifier_position;
        break;
        }
        case 1: /* END */
        {
            if (!SETUP_IniUpdated)
            {
                strcpy((char*)&SETUP_TargetPath, "$$$");
                INI_WriteEntry_Path();
            }
            *conditional_command = 0;
            if ( keyword_count != 1 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            return -1;
            break;
        }
        case 2: /* UPDATE_INI */
        {
            if ( keyword_count != 1 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            INI_MakePath();
            INI_WriteEntry_System();
            INI_WriteEntry_Path();
            if ( DPMI_IsVesaAvailable() )
            {
                INI_WriteEntry_Vesa();
            }
            SETUP_IniUpdated = 1;

            return line_number + 1;
            break;
        }
        case 1000: /* IF_EXISTS */
        {
            ++*conditional_command;
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            if ( FILE_IsFileAccessPermitted((const char *)keyword_buffer[1]) )
            {
                retVal = line_number + 1; /* scan for next instruction */
            }
            else
            {
                retVal = SETUP_NextStateConditionalCommand((SETUP_ScriptDataStruct *)&script_data, line_number, 2004);  // go to else
            }
            return retVal;
            break;
        }
        case 1001: /* IF_NOT_EXISTS */
        {
            ++*conditional_command;
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            if ( FILE_IsFileAccessPermitted((const char *)keyword_buffer[1]) )
            {
                retVal = SETUP_NextStateConditionalCommand((SETUP_ScriptDataStruct *)&script_data, line_number, 2004);
            }
            else
            {
                retVal = line_number + 1;
            }
            return retVal;
            break;
        }
        case 1002: /* IF_LANGUAGE */
        {
            ++*conditional_command;
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            if ( SETUP_ConvertAsciiToInteger((const char *)keyword_buffer[1], line_number) == SETUP_Language + 1 )
            {
                retVal = line_number + 1;
            }
            else
            {
                retVal = SETUP_NextStateConditionalCommand((SETUP_ScriptDataStruct *)&script_data, line_number, 2004);
            }
            return retVal;
            break;
        }
        case 1004: /* ELSE */
        {
            if ( keyword_count != 1 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            if ( !*conditional_command )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            return SETUP_NextStateConditionalCommand((SETUP_ScriptDataStruct *)&script_data, line_number, 2005);

            break;
        }
        case 1005: /* ENDIF */
        {
            if ( keyword_count != 1 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            if ( !*conditional_command )
            {
                GUI_ErrorHandler(1022, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            --*conditional_command;
            return line_number + 1;
            break;
        }
        case 2000: /* COPY */
        {
            if ( keyword_count != 3 && keyword_count != 4)
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            if ( keyword_count == 4 && stricmp((char*)keyword_buffer[3], "/s"))
            {
                GUI_ErrorHandler(1030, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }

            copy4Arg = keyword_count == 4;
            GUI_DrawProgressBar(1);
            SETUP_PrepareAndCopy((char *)keyword_buffer[1], (char *)keyword_buffer[2], copy4Arg);
            GUI_DrawProgressBar(-1);

            return line_number + 1;
            break;
        }
        case 2001: /* INSTALL */
        {
            if ( keyword_count != 2 && keyword_count != 3 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            GUI_DrawProgressBar(1);
            if ( keyword_count == 3 )
            {
                SETUP_InstallFiles((char*)keyword_buffer[1], (char*)keyword_buffer[2], 0);
            }
            else
            {
                SETUP_InstallFiles((char*)keyword_buffer[1], "/s", 0);
            }
            GUI_DrawProgressBar(-1);

            return line_number + 1;
            break;
        }
        case 2002: /* MAKEDIR */
        {
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            FILE_CreateDir((char *)keyword_buffer[1]);

            return line_number + 1;
            break;
        }
        case 2003: /* CREATE */
        {
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            FILE_Create((const char *)keyword_buffer[1]);

            return line_number + 1;
            break;
        }
        case 2004: /* DELETE */
        {
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            FILE_Delete((char *)keyword_buffer[1]);

            return line_number + 1;
            break;
        }
        case 2005: /* RENAME */
        {
            if ( keyword_count != 3 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            unlink((const char *)keyword_buffer[2]);
            if ( rename((const char *)keyword_buffer[1], (const char *)keyword_buffer[2]) )
            {
                GUI_ErrorHandler(1017, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }

            return line_number + 1;
            break;
        }
        case 2006: /* EXECUTE_SILENT */
        {
            SETUP_RunOnConsole(keyword_count, (const char *)keyword_buffer);
            GUI_CreateMouseCursor(0xAu, 0xEu, 1, 1, (unsigned char *)&SETUP_MouseCursor);
            DSA_CopyMainOPMToScreen(1);
            DSA_LoadPal((LBM_LogPalette*)&pal, 0, 256u, 0);
            DSA_ActivatePal();
            GUI_SetPal();

            return line_number + 1;
            break;
        }
        case 2007: /* EXECUTE_OWN_SCREEN */
        {
            if ( keyword_count == 1 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            DSA_CloseScreen();
            SYSTEM_Deinit();
            SETUP_RunOnConsole(keyword_count, (const char *)keyword_buffer);
            SYSTEM_Init();
            DSA_Init();
            if (!DSA_OpenScreen(&GUI_ScreenOpm, 0))
            {
                GUI_ErrorHandler(1000);
            }
            GUI_CreateMouseCursor(0xAu, 0xEu, 1, 1, (unsigned char *)&SETUP_MouseCursor);
            DSA_CopyMainOPMToScreen(1);
            DSA_LoadPal((LBM_LogPalette*)&pal, 0, 256u, 0);
            DSA_ActivatePal();
            GUI_SetPal();

            return line_number + 1;
            break;
        }
        case 2008: /* CHDIR */
        {
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            FILE_ChangeDir((char *)keyword_buffer[1]);

            return line_number + 1;
            break;
        }
        case 2009: /* SELECT_TARGET_DRIVE */
        {
            if ( keyword_count != 3 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }

            lang_loc = SETUP_ConvertAsciiToInteger((const char *)keyword_buffer[1], line_number) - 1;
            targetdrive_ret = GUI_DrawTargetDriveMenu(lang_loc);
            if (targetdrive_ret)
            {
                return line_number + 1;
            }
            signifier_position = SETUP_SearchLabel(script_data, (char *)keyword_buffer[2]);
            *conditional_command = SETUP_GetLabelLine(script_data, signifier_position);
            if ( signifier_position == -1 )
            {
                GUI_ErrorHandler(1007, line_number + 1,  (char*)&script_data->PtrScriptLine[line_number]);
            }
            return signifier_position;
            break;
        }
        case 2010: /* SELECT_TARGET_PATH */
        {
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            targetpath_ret = GUI_DrawTargetPathMenu((char*)GUI_StringData[SETUP_Language][48], (char *)keyword_buffer[1]); /* "Please enter target path:" */
            strcpy((char*)&SETUP_TargetPath, targetpath_ret);
            FILE_CreateDir((char*)&SETUP_TargetPath);
            return line_number + 1;
            break;
        }
        case 2011: /* CD_SOURCE */
        {
            if ( keyword_count != 1 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            FILE_ChangeDir((char*)&SETUP_SourcePath);

            return line_number + 1;
            break;
        }
        case 2012: /* CD_TARGET */
        {
            if ( keyword_count != 1 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }

            FILE_ChangeDir((char*)SETUP_TargetPath);

            return line_number + 1;
            break;
        }
        case 2014: /* SELECT_CD_ROM_DRIVE */
        {
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }

            cdrom_ret = GUI_WriteIniEntry_Cdrom();
            if (cdrom_ret)
            {
                return line_number + 1;
            }

            signifier_position = SETUP_SearchLabel(script_data, (char *)keyword_buffer[1]);
            *conditional_command = SETUP_GetLabelLine(script_data, signifier_position);
            if ( signifier_position == -1 )
            {
                GUI_ErrorHandler(1007, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            return signifier_position;
            break;
        }
        case 2015: /* INSTALL_DIRS */
        {
            if ( keyword_count != 2 && keyword_count != 3 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            GUI_DrawProgressBar(1);
            if ( keyword_count == 3 )
            {
                SETUP_InstallFiles((char*)keyword_buffer[1], (char*)keyword_buffer[2], 1);
            }
            else
            {
                SETUP_InstallFiles((char*)keyword_buffer[1], "/s", 1);
            }
            GUI_DrawProgressBar(-1);

            return line_number + 1;
            break;
        }
        case 2017: /* WRITE_INI_ENTRY */
        {
            if ( keyword_count != 3 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }

            if (FILE_IsFileExisting((const char *)&INI_WriteBuffer))
            {
                INI_WriteEntry("SYSTEM", (const char *)keyword_buffer[1], (const char *)keyword_buffer[2]);
            }

            return line_number + 1;
            break;
        }
        case 3000: /* MENU_START */
        {
            SETUP_Menu.index = 0;
            return line_number + 1;
            break;
        }
        case 3001: /* MENU_ENTRY */
        {
            if ( keyword_count > 3 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            if ( SETUP_Menu.index >= 39 )
            {
                GUI_ErrorHandler(1028, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            switch ( keyword_count )
            {
                case 1:
                {
                    SETUP_Menu.entry[SETUP_Menu.index].ptr_entry_string = 0;
                    SETUP_Menu.entry[SETUP_Menu.index].anchor_point = -1;
                    break;
                }
                case 2:
                {
                    menu_keyword_length = strlen((const char *)keyword_buffer[1]);
                    SETUP_Menu.entry[SETUP_Menu.index].ptr_entry_string = (char *)malloc(menu_keyword_length);
                    strcpy(SETUP_Menu.entry[SETUP_Menu.index].ptr_entry_string, (const char *)keyword_buffer[1]);
                    SETUP_Menu.entry[SETUP_Menu.index].anchor_point = -1;
                    break;
                }
                case 3:
                {
                    menu_keyword_length = strlen((const char *)keyword_buffer[1]);
                    SETUP_Menu.entry[SETUP_Menu.index].ptr_entry_string = (char *)malloc(menu_keyword_length);
                    strcpy(SETUP_Menu.entry[SETUP_Menu.index].ptr_entry_string, (const char *)keyword_buffer[1]);
                    SETUP_Menu.entry[SETUP_Menu.index].anchor_point = SETUP_SearchLabel(script_data, (char *)keyword_buffer[2]);
                    if ( SETUP_Menu.entry[SETUP_Menu.index].anchor_point == -1 )
                    {
                        GUI_ErrorHandler(1007, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
                    }
                    break;
                }
            }
            ++SETUP_Menu.index;

            return line_number + 1;
            break;
        }
        case 3002: /* MENU_END */
        {
            if ( !SETUP_Menu.index )
            {
                GUI_ErrorHandler(1027, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            if ( keyword_count != 1 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            for (i = 0; i < SETUP_Menu.index && (!SETUP_Menu.entry[i].ptr_entry_string || SETUP_Menu.entry[i].anchor_point == -1); ++i )
            {}

            if ( i >= SETUP_Menu.index )
            {
                GUI_ErrorHandler(1033, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }

            signifier_position = GUI_DrawMenu(&SETUP_Menu);
            *conditional_command = SETUP_GetLabelLine(script_data, signifier_position);
            return signifier_position;
            break;
        }
        case 3003: /* PRINT */
        {
            if ( keyword_count == 2 )
            {
                GUI_DrawTextBox((char *)keyword_buffer[1]);
            }
            else
            {
                if ( keyword_count != 1 )
                {
                    GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
                }
                GUI_DrawTextBox(0);
            }

            return line_number + 1;
            break;
        }
        case 3004: /* TEXT */
        {
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }

            GUI_DrawReadmeText((char *)keyword_buffer[1]);

            return line_number + 1;
            break;
        }
        case 3005: /* ASSERT */
        {
            if ( keyword_count != 3 && keyword_count != 4 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            ;
            if ( GUI_DrawAssertBox((char *)keyword_buffer[1]) )
            {
                if ( keyword_count == 4 )
                {
                    signifier_position = SETUP_SearchLabel(script_data, (char *)keyword_buffer[3]);
                    *conditional_command = SETUP_GetLabelLine(script_data, signifier_position);
                    retVal = signifier_position;
                }
                else
                {
                    return line_number + 1;
                }
            }
            else
            {
                signifier_position = SETUP_SearchLabel(script_data, (char *)keyword_buffer[2]);
                *conditional_command = SETUP_GetLabelLine(script_data, signifier_position);
                return signifier_position;
            }
            break;
        }
        case 3006: /* INFO */
        {
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            GUI_PrintInfoBox((char *)keyword_buffer[1]);

            return line_number + 1;
            break;
        }
        case 3007: /* ERROR */
        {
            if ( keyword_count != 2 )
            {
               GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            GUI_PrintErrorBox((char *)keyword_buffer[1]);

            return line_number + 1;
            break;
        }
        case 3008: /* LOAD_BACKGROUND */
        {
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number + 1, (char*)&script_data->PtrScriptLine[line_number]);
            }
            if ( !LBM_DisplayLBM((char *)keyword_buffer[1], (OPM_Struct*)&GUI_ScreenOpm, (LBM_LogPalette*)&pal, 1) )
            {
                GUI_ErrorHandler(1009, (char *)keyword_buffer[1]);
            }
            /* DSA_StretchOPMToScreen(&pixel_map_loc, &GUI_ScreenOpm); */ /* FIXME: Function not verified yet */
            DSA_LoadPal((LBM_LogPalette*)&pal, 0, 256u, 0);
            DSA_ActivatePal();
            GUI_SetPal();
            DSA_CopyMainOPMToScreen(1);
            OPM_Del(&pixel_map_loc);

            return line_number + 1;
            break;
        }
        case 3009: /* SET_LANGUAGE */
        {
            if ( keyword_count != 2 )
            {
                GUI_ErrorHandler(1012, line_number, (char*)&script_data->PtrScriptLine[line_number]);
            }
            if ( SETUP_ConvertAsciiToInteger((const char *)keyword_buffer[1], line_number) < 1
              || SETUP_ConvertAsciiToInteger((const char *)keyword_buffer[1], line_number) > 4 )
            {
                GUI_ErrorHandler(1020, line_number, (char*)&script_data->PtrScriptLine[line_number]);
            }

            SETUP_Language = SETUP_ConvertAsciiToInteger((const char *)keyword_buffer[1], line_number) - 1;

            return line_number + 1;
            break;
        }
        default:
        {
            return line_number + 1;
            break;
        }
    }
    return 0;
}

int main( int argc, char *argv[] )
{
    char drive[4];
    char dir[132];
    char fname[12];

    unsigned int SETUP_ConditionalCommand;
    unsigned int SETUP_CurrentCommand;

    SETUP_Language = 0;
    SETUP_CdDrive = 0;
    SETUP_TargetDrive = 'C';

    printf("\nStarting Blue Byte\'s setup-program...\n(C) 1996 Blue Byte Software GmbH\n");

    _harderr(SETUP_CriticalErrorHandler);
    SETUP_CriticalErrorFlag = 0;

    SETUP_PtrArgv = *argv;
    _splitpath((const char *)&SETUP_PtrArgv[0], drive, dir, 0, 0);
    _makepath((char *)&SETUP_SourcePath[0], drive, dir, 0, 0);

    _splitpath((const char *)&SETUP_PtrArgv[0], drive, dir, fname, 0);
    _makepath((char *)&INI_WriteBuffer[0], drive, dir, fname, "ini");

    SETUP_GetCdDriveAndLanguage();

    /* Is path passed as a command line parameter? */
    if (argc == 2)
    {
        SETUP_PtrArgvPath = argv[1];
    }
    else
    {
        SETUP_PtrArgvPath = 0;
    }

    SETUP_ParseScript((SETUP_ScriptDataStruct *)&SETUP_ScriptData, "INSTALL.SCR");
    SYSTEM_MouseStatusFlags |= 0x4;

    SYSTEM_Init();
    DSA_Init();

    while (1)
    {
        OPM_New(GUI_ScreenWidth, GUI_ScreenHeight, 1u, &GUI_ScreenOpm, 0);
        if (DSA_OpenScreen(&GUI_ScreenOpm, 0))
        {
            break;
        }
        if (GUI_ScreenWidth != 640)
        {
            GUI_ErrorHandler(1000, (char *)&dir);
        }
        OPM_Del(&GUI_ScreenOpm);
        GUI_ScreenWidth = 320;
        GUI_ScreenHeight = 200;
    }

    GUI_SetPal();
    GUI_CreateMouseCursor(0xAu, 0xEu, 1, 1, (unsigned char *)&SETUP_MouseCursor);
    GUI_DrawFilledBackground(&GUI_ScreenOpm, 0, 0, GUI_ScreenWidth - 1, GUI_ScreenHeight - 1, 0xF9, 0xF8u, 0xF7, 0xF6);

    DSA_CopyMainOPMToScreen(1);

    SETUP_ConditionalCommand = 0;
    for (SETUP_CurrentCommand = 1; SETUP_CurrentCommand != -1; SETUP_CurrentCommand = SETUP_ScriptHandler((SETUP_ScriptDataStruct *)&SETUP_ScriptData, SETUP_CurrentCommand, (unsigned int*)&SETUP_ConditionalCommand))
    {
        kbhit();
    }
    OPM_Del(&GUI_ScreenOpm);
    DSA_CloseScreen();
    SYSTEM_Deinit();

    return 0;
}
