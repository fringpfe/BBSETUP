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
 * Functions handling the Direct Screen Access(?).
 *************************************************************************/

#include "BASEMEM.h"
#include "SYSTEM.h"
#include "ERROR.h"
#include "DSA.h"
#include <string.h>
#include <conio.h>
#include "LBM.h"

short DSA_initialized;

typedef struct {
    /* Mandatory information for all VBE revisions */
    unsigned short ModeAttributes;
    unsigned char WinAAttributes;
    unsigned char WinBAttributes;
    unsigned short WinGranularity;
    unsigned short WinSize;
    unsigned short WinASegment;
    unsigned short WinBSegment;
    unsigned int WinFuncPtr;
    unsigned short BytesPerScanLine;
    /* Mandatory information for VBE 1.2 and above */
    unsigned short XResolution;
    unsigned short YResolution;
    unsigned char XCharSize;
    unsigned char YCharSize;
    unsigned char NumberOfPlanes;
    unsigned char BitsPerPixel;
    unsigned char NumberOfBanks;
    unsigned char MemoryModel;
    unsigned char BankSize;
    unsigned char NumberOfImagePages;
    unsigned char Reserved_page;
    /* Direct Color fields (required for direct/6 and YUV/7 memory models) */
    unsigned char RedMaskSize;
    unsigned char RedFieldPosition;
    unsigned char GreenMaskSize;
    unsigned char GreenFieldPosition;
    unsigned char BlueMaskSize;
    unsigned char BlueFieldPosition;
    unsigned char RsvdMaskSize;
    unsigned char RsvdFieldPosition;
    unsigned char DirectColorModeInfo;
    /* Mandatory information for VBE 2.0 and above */
    unsigned int PhysBasePtr;
    unsigned int OffScreenMemOffset;
    unsigned short OffScreenMemSize;
    unsigned char Reserved[206];
} VBEModeInfoBlock;

typedef struct
{
    char            VESASignature[4];    /* 4 signature bytes */
    unsigned short  VESAVersion;         /* VESA version number */
    char*           OEMStringPtr;        /* Pointer to OEM string */
    unsigned int    Capabilities;        /* Capabilities of the video environment */
    unsigned int*   VideoModePtr;        /* Pointer to supported Super VGA modes */
    unsigned short  TotalMemory;         /* Number of 64K memory blocks on board */
    unsigned char   Reserved[242];       /* Reserved for future use. */
} VBEHardWare;

typedef struct
{
    unsigned int edi, esi, ebp, esp;
    unsigned int ebx, edx, ecx, eax;
    unsigned short flags;
    unsigned short es, ds;
    unsigned short fs, gs;
    unsigned short ip, cs;
    unsigned short sp, ss;
} DPMI_CALLREGS;

typedef struct {
    unsigned short width;
    unsigned short height;
    unsigned short bytes_per_pixel;
    unsigned short vesa_mode_number;
    unsigned short mode_number_internal;
    unsigned short field_A;
} VBEModeConfig;

VBEModeConfig VBEModeConfigData[11] =
{
/* width, height, bytes_per_pixel, vesa_mode_number, mode_number_internal, field_A  */
    {320,  200,  1, 0x13,  1, 0},
    {320,  200,  1, 0x153, 0, 1},
    {320,  200,  1, 0x13,  2, 1},
    {360,  240,  1, 0x157, 0, 1},
    {360,  240,  1, 0x13,  2, 1},
    {640,  400,  1, 0x100, 0, 1},
    {640,  480,  1, 0x101, 0, 1},
    {800,  600,  1, 0x103, 0, 1},
    {1024, 768,  1, 0x105, 0, 1},
    {1280, 1024, 1, 0x107, 0, 1},
    {0,    0,    0, 0,     0, 0},
};

DSA_GlobalParamStruct DSA_globalParams;
DSA_GlobalParamStruct* DSA_ptrGlobalParams;

unsigned short DSA_VideoMode;
unsigned short DSA_PreviousVideoMode;
unsigned short DSA_InternalMode;

unsigned int DSA_MouseValue_X_Min;
unsigned int DSA_MouseValue_X_Max;
unsigned int DSA_MouseValue_Y_Min;
unsigned int DSA_MouseValue_Y_Max;
unsigned int DSA_MouseValue_X_Current;
unsigned int DSA_MouseValue_Y_Current;

unsigned int DSA_Screen_Size;
unsigned int DSA_Screen_Stride;
unsigned int DSA_Screen_Height;

VBEHardWare* DSA_VbeHardWare;
VBEModeInfoBlock* DSA_VgaInfoBlock;
DPMI_CALLREGS DSA_DpmiCallRegs;
unsigned short DSA_WinAttrib_Status;
unsigned short DSA_currentBank;
unsigned int DSA_MaxSupportedScreenSize;

unsigned int DSA_MouseUpdateFlag;

OPM_Struct bmouseback;
OPM_Struct bmouseoldback;
OPM_Struct mouseback[4];
OPM_Struct mouseoldback[4];

unsigned int DSA_ErrorFlags;

char *DSA_ScreenBuffer[4];
unsigned int DSA_ScanlineStart[4];
/* FIXME: Purpose of the following variables unclear */
unsigned short word_77DF6;
int max_4_word_77E00;
int bank;
int winSizeInBytes;

void DSA_CloseScreen(void);
static void DSA_PrintData(char *buffer, DSA_ErrorStruct *data);

int DSA_Init(void)
{
    DSA_ErrorStruct data;
    int retVal;

    if(BASEMEM_Init())
    {
        if (DSA_initialized == 0)
        {
            DSA_initialized = 1;
        }
        retVal = 1;
    }
    else
    {
        data.text = "DSA_Init: Cannot init BASEMEM";
        data.data1 = 0;
        data.data2 = 0;
        ERROR_PushError((ERROR_PrintErrorPtr)DSA_PrintData, "BBDSA Library", sizeof(data), (const char *) &data);
        retVal = 0;
    }
    return retVal;
}

void DSA_DeInit(void)
{
    if (DSA_initialized != 0)
    {
        if (DSA_ptrGlobalParams == 0)
        {
            DSA_CloseScreen();
        }
        DSA_initialized = 0;
    }
}

unsigned int DSA_OpenScreen(OPM_Struct *pixel_map, int method)
{
    unsigned int index;
    int v8;
    unsigned int retVal;
    DSA_ErrorStruct data;
    union REGS inregs;
    struct SREGS sregs;
    LBM_LogPalette* pal_loc = (LBM_LogPalette*)&pal;
    unsigned char bank_index;
    void far *ptr_dpmi_callregs;
    unsigned int DSA_windowsSegmentAddress;
    unsigned int irgendwas_mit_bank_dword_77988;

    DSA_globalParams.method_flags = method;
    DSA_VideoMode = 0;

    for (index = 0; VBEModeConfigData[index].width != 0; index++)
    {
        if (VBEModeConfigData[index].width != pixel_map->width || VBEModeConfigData[index].height != pixel_map->height || VBEModeConfigData[index].bytes_per_pixel != pixel_map->bytes_per_pixel)
        {
          continue;
        }

        v8 = 1;
        if (DSA_globalParams.method_flags & 0xC)
        {
          if ((DSA_globalParams.method_flags & 4) && (VBEModeConfigData[index].mode_number_internal))
          {
            v8 = 0;
          }
          if ((DSA_globalParams.method_flags & 8) && (VBEModeConfigData[index].mode_number_internal != 2))
          {
            v8 = 0;
          }
        }
        if (!v8)
        {
          continue;
        }

        if (!(DSA_globalParams.method_flags & 0x10000))
        {
            DSA_VideoMode = VBEModeConfigData[index].vesa_mode_number;
            DSA_InternalMode = VBEModeConfigData[index].mode_number_internal;
            break;
        }

        if (VBEModeConfigData[index].field_A == 1)
        {
            DSA_VideoMode = VBEModeConfigData[index].vesa_mode_number;
            DSA_InternalMode = VBEModeConfigData[index].mode_number_internal;
            break;
        }
    }

    if(DSA_VideoMode != 0)
    {
        word_77DF6 = 0;
        DSA_globalParams.flags = 1;
        DSA_globalParams.global_pixel_map = pixel_map;
        DSA_globalParams.global_palette_data = pal_loc;
        pal_loc->number_of_entries = 256;
        pal_loc->version = 0;
        for(int i = 0; i < 256; i++)
        {
            pal_loc->pal_entry->peRed = 0;
            pal_loc->pal_entry->peGreen = 0;
            pal_loc->pal_entry->peBlue = 0;
            pal_loc->pal_entry->peFlags = 0;
        }

        DSA_ptrGlobalParams = (DSA_GlobalParamStruct*)&DSA_globalParams;

        DSA_MouseValue_X_Min = 0;
        DSA_MouseValue_Y_Min = 0;
        DSA_MouseValue_X_Max = DSA_globalParams.global_pixel_map->width - 1;
        DSA_MouseValue_Y_Max = DSA_globalParams.global_pixel_map->height - 1;
        DSA_MouseValue_X_Current = DSA_MouseValue_X_Max / 2;
        DSA_MouseValue_Y_Current = DSA_MouseValue_Y_Max / 2;

        DSA_Screen_Size = pixel_map->size;
        DSA_Screen_Stride = pixel_map->stride;
        DSA_Screen_Height = pixel_map->height;

        if(DSA_InternalMode == 1)
        {
            /* Get current video mode */
            inregs.w.ax = 0x0F00;
            int386(0x10, &inregs, &inregs);
            DSA_PreviousVideoMode = inregs.h.al;
            /* Set video mode to Mode 13*/
            inregs.w.ax = 0x13;
            int386(0x10, &inregs, &inregs);
            /* Set screen buffer address for all four banks */
            for(bank_index = 0; bank_index < 4; bank_index++)
            {
                DSA_ScreenBuffer[bank_index] = (char*)0xA0000L;
                DSA_ScanlineStart[bank_index] = 0;
            }
            max_4_word_77E00 = 1;
            bank = 0;
            DSA_globalParams.method_flags &= 0xFEu;
            retVal = 1;
        }
        else if (DSA_InternalMode == 2)
        {
            /* Get current video mode */
            inregs.w.ax = 0x0F00;
            int386(0x10, &inregs, &inregs);
            DSA_PreviousVideoMode = inregs.h.al;
            /* Set video mode */
            inregs.w.ax = DSA_VideoMode;
            int386(0x10, &inregs, &inregs);

            winSizeInBytes = DSA_Screen_Size / 4;
            /* FIXME: Missing function; not implemented yet */
            if(DSA_globalParams.method_flags & 0x10000)
            {
                for(bank_index = 0; bank_index < 4; bank_index++)
                {
                    DSA_ScreenBuffer[bank_index] = (char*)(0xA0000L + winSizeInBytes + bank_index);
                    DSA_ScanlineStart[bank_index] = 0;
                }
                max_4_word_77E00 = 2;
                bank = 1;
                /* FIXME: Missing function; not implemented yet */
            }
            else
            {
                /* Set screen buffer address for all four banks */
                for(bank_index = 0; bank_index < 4; bank_index++)
                {
                    DSA_ScreenBuffer[bank_index] = (char*)0xA0000L;
                    DSA_ScanlineStart[bank_index] = 0;
                    max_4_word_77E00 = 1;
                    bank = 0;
                    /* FIXME: Missing function; not implemented yet */
                }
            }
            retVal = 1;
        }
        else
        {
            if(DSA_InternalMode != 0)
            {
                return 1;
            }
            /* Get SVGA information */
            DSA_VbeHardWare = (VBEHardWare *)BASEMEM_Alloc(256u, 0x102u);
            ptr_dpmi_callregs = (void far *)&DSA_DpmiCallRegs;
            BASEMEM_FillMemByte(&DSA_DpmiCallRegs, sizeof(DSA_DpmiCallRegs), 0);
            DSA_DpmiCallRegs.eax = 0x4F00;
            DSA_DpmiCallRegs.es = (unsigned int)DSA_VbeHardWare >> 4;
            DSA_DpmiCallRegs.edi = (unsigned int)DSA_VbeHardWare & 0x0F;
            DSA_DpmiCallRegs.ss = 0;
            DSA_DpmiCallRegs.sp = 0;
            sregs.es = FP_SEG(&DSA_DpmiCallRegs);
            inregs.x.edi = FP_OFF(&DSA_DpmiCallRegs);
            inregs.w.ax = 0x300;
            inregs.w.bx = 0x10;
            inregs.w.cx = 0;
            int386x(0x31, &inregs, &inregs, &sregs);
            if(DSA_DpmiCallRegs.eax != 0x4f)
            {
                /* Get SVGA mode information */
                DSA_VgaInfoBlock = (VBEModeInfoBlock *)BASEMEM_Alloc(256u, 0x102u);
                ptr_dpmi_callregs = (void far *)&DSA_DpmiCallRegs;
                BASEMEM_FillMemByte(&DSA_DpmiCallRegs, sizeof(DSA_DpmiCallRegs), 0);
                DSA_DpmiCallRegs.eax = 0x4F01;
                DSA_DpmiCallRegs.ecx = *(unsigned int *)((char *)&DSA_VgaInfoBlock + 2) >> 16;
                DSA_DpmiCallRegs.es = (unsigned int)DSA_VgaInfoBlock >> 4;
                DSA_DpmiCallRegs.edi = 0; /* TODO: Verify proper functionality (unsigned int)DSA_VgaInfoBlock & 0x0F; */
                DSA_DpmiCallRegs.ss = 0;
                DSA_DpmiCallRegs.sp = 0;
                sregs.es = FP_SEG(&DSA_DpmiCallRegs);
                inregs.x.edi = FP_OFF(&DSA_DpmiCallRegs);
                inregs.w.ax = 0x300;
                inregs.w.bx = 0x10;
                inregs.w.cx = 0;
                int386x(0x31, &inregs, &inregs, &sregs);
                if(DSA_DpmiCallRegs.eax != 0x4f)
                {
                    if((DSA_VgaInfoBlock->WinAAttributes & 2) && (DSA_VgaInfoBlock->WinAAttributes & 4))
                    {
                        DSA_WinAttrib_Status = 0;
                    }
                    else
                    {
                        DSA_WinAttrib_Status = -1;
                    }
                    /* Get current video mode */
                    inregs.w.ax = 0x4F03;
                    int386(0x10, &inregs, &inregs);
                    DSA_PreviousVideoMode = inregs.w.bx;
                    /* Set video mode */
                    inregs.w.ax = 0x4F02;
                    inregs.w.bx = DSA_VideoMode;
                    int386(0x10, &inregs, &inregs);
                    if ( inregs.h.ah ) /* Failed */
                    {
                        BASEMEM_Free(DSA_VbeHardWare);
                        BASEMEM_Free(DSA_VgaInfoBlock);
                        DSA_VbeHardWare = 0;
                        DSA_VgaInfoBlock = 0;
                        DSA_ErrorFlags = 2;
                        DSA_globalParams.method_flags = 0;
                        return 0;
                    }
                    else
                    {
                        DSA_currentBank = -1;
                        if(DSA_globalParams.method_flags & 0x10000)
                        {
                            /* Set logical scan line length */
                            inregs.w.ax = 0x4F06;
                            inregs.h.bl = 1;
                            int386(0x10, &inregs, &inregs);
                            if ( inregs.h.ah )
                            {
                                BASEMEM_Free(DSA_VbeHardWare);
                                BASEMEM_Free(DSA_VgaInfoBlock);
                                DSA_VbeHardWare = 0;
                                DSA_VgaInfoBlock = 0;
                                /* Set video mode */
                                inregs.w.ax = 0x4F02;
                                inregs.w.bx = DSA_PreviousVideoMode;
                                int386(0x10, &inregs, &inregs);
                                DSA_ErrorFlags = 4;
                                DSA_globalParams.method_flags = 0;
                                return 0;
                            }
                            DSA_MaxSupportedScreenSize = inregs.w.dx * inregs.w.bx; /* Max number of scanlines * bytes per scan line */
                            if(2* DSA_Screen_Size > DSA_MaxSupportedScreenSize)
                            {
                                BASEMEM_Free(DSA_VbeHardWare);
                                BASEMEM_Free(DSA_VgaInfoBlock);
                                DSA_VbeHardWare = 0;
                                DSA_VgaInfoBlock = 0;
                                /* Set video mode */
                                inregs.w.ax = 0x4F02;
                                inregs.w.bx = DSA_PreviousVideoMode;
                                int386(0x10, &inregs, &inregs);
                                DSA_ErrorFlags = 8;
                                DSA_globalParams.method_flags = 0;
                                return 0;
                            }
                            max_4_word_77E00 = 2;
                            bank = 1;
                            DSA_windowsSegmentAddress = 16 * DSA_VgaInfoBlock->WinASegment;
                            for(bank_index = 0; bank_index < 4; bank_index++)
                            {
                                DSA_ScreenBuffer[bank_index] = (char*)&DSA_windowsSegmentAddress;
                                DSA_ScanlineStart[bank_index] = pixel_map->height * bank_index;
                            }
                            winSizeInBytes = DSA_VgaInfoBlock->WinSize * 1024;
                            irgendwas_mit_bank_dword_77988 = DSA_VgaInfoBlock->WinSize / DSA_VgaInfoBlock->WinGranularity;
                            /* Get/Set display start */
                            inregs.w.ax = 0x4F07;
                            inregs.w.bx = 0;
                            inregs.w.cx = 0;
                            inregs.w.dx = DSA_ScanlineStart[0];
                            int386(0x10, &inregs, &inregs);
                            if (inregs.h.ah)
                            {
                                BASEMEM_Free(DSA_VbeHardWare);
                                BASEMEM_Free(DSA_VgaInfoBlock);
                                DSA_VbeHardWare = 0;
                                DSA_VgaInfoBlock = 0;
                                /* Set video mode */
                                inregs.w.ax = 0x4F02;
                                inregs.w.bx = DSA_PreviousVideoMode;
                                int386(0x10, &inregs, &inregs);
                                DSA_ErrorFlags = 4;
                                DSA_globalParams.method_flags = 0;
                                return 0;
                            }
                        }
                        else
                        {
                            max_4_word_77E00 = 1;
                            bank = 0;
                            DSA_windowsSegmentAddress = 16 * DSA_VgaInfoBlock->WinASegment;
                            for(bank_index = 0; bank_index < 4; bank_index++)
                            {
                                DSA_ScreenBuffer[bank_index] = (char*)&DSA_windowsSegmentAddress;
                                DSA_ScanlineStart[bank_index] = 0;
                            }
                            winSizeInBytes = DSA_VgaInfoBlock->WinSize * 1024;
                            irgendwas_mit_bank_dword_77988 = DSA_VgaInfoBlock->WinSize / DSA_VgaInfoBlock->WinGranularity;
                            if(DSA_VgaInfoBlock->BytesPerScanLine != pixel_map->stride)
                            {
                                pixel_map->stride = DSA_VgaInfoBlock->BytesPerScanLine;
                                word_77DF6 = 1;
                            }
                            retVal = 1;
                        }
                    }
                }
                else
                {
                    BASEMEM_Free(DSA_VbeHardWare);
                    BASEMEM_Free(DSA_VgaInfoBlock);
                    DSA_VbeHardWare = 0;
                    DSA_VgaInfoBlock = 0;
                    DSA_ErrorFlags = 2;
                    DSA_globalParams.method_flags = 0;
                    retVal = 0;
                }
            }
            else
            {
                BASEMEM_Free(DSA_VbeHardWare);
                DSA_VbeHardWare = 0;
                DSA_VgaInfoBlock = 0;
                DSA_globalParams.method_flags = 0;
                retVal = 0;
            }
        }

    }
    else
    {

        data.text = "DSA_OpenScreen: unknown resolution width,height:";
        data.data1 = pixel_map->width;
        data.data2 = pixel_map->height;
        ERROR_PushError((ERROR_PrintErrorPtr)DSA_PrintData, "BBDSA Library", sizeof(data), (const char *) &data);
        DSA_globalParams.flags = 0;
        retVal = 0;
    }

    return retVal;
}

void DSA_CloseScreen(void)
{
    union REGS inregs;

    if(DSA_globalParams.flags & 1)
    {
        SYSTEM_DrawMousePtr();
        DSA_globalParams.flags = 0;

        if(DSA_VbeHardWare != 0)
        {
            BASEMEM_Free(DSA_VbeHardWare);
            DSA_VbeHardWare = 0;
        }
        if(DSA_VgaInfoBlock != 0)
        {
            BASEMEM_Free(DSA_VgaInfoBlock);
            DSA_VgaInfoBlock = 0;
        }
        /* Set video mode */
        inregs.w.ax = DSA_PreviousVideoMode;
        int386(0x10, &inregs, &inregs);
        DSA_VideoMode = -1;
    }

}

void VGA_CopyScreenBuffer(void *dest, const void *src)
{
    memcpy(dest, src, DSA_Screen_Size);
}

void VGA_CopyMouseCursor(char *screenBuffer, unsigned char *opmBuffer, int stride, int x, int y, unsigned int opmWidth, unsigned int opmHeight)
{
    /* TODO: Implementation not verified yet */
    unsigned char *pixelLocation;
    unsigned char *dst;
    unsigned char *src;

    pixelLocation = (unsigned char *)&screenBuffer[x + DSA_Screen_Stride * y];

    do
    {
        memcpy(pixelLocation, opmBuffer, opmWidth);
        opmBuffer += opmWidth;
        pixelLocation += opmWidth;
        opmBuffer += stride;
        pixelLocation += DSA_Screen_Stride - opmWidth;

        opmHeight--;
    }
    while (opmHeight > 0);
}

void VGA_CopyScreenSectionToBuffer(unsigned char *opmBuffer, char *screenBuffer, int stride, int x, int y, unsigned int opmWidth, unsigned int opmHeight)
{
    /* TODO: Implementation not verified yet */
    unsigned char *pixelLocation;
    unsigned char *dst;
    unsigned char *src;

    pixelLocation = (unsigned char *)&screenBuffer[x + DSA_Screen_Stride * y];

    do
    {
        memcpy(opmBuffer, pixelLocation, opmWidth);
        opmBuffer += opmWidth;
        pixelLocation += opmWidth;
        opmBuffer += stride;
        pixelLocation += DSA_Screen_Stride - opmWidth;

        opmHeight--;
    }
    while (opmHeight > 0);
}

void ASM_copyMouseCursorWithTransparency(char transparent_color, int count, unsigned char *mouse_body, unsigned char *buffer1, unsigned char *buffer2)
{
    /* TODO: Implementation not verified yet */
    char value;
    do
    {
        value = *buffer2;
        if (*mouse_body != transparent_color)
        {
            *buffer2 = *mouse_body;
        }
        *buffer1 = value;
        ++buffer2;
        ++buffer1;
        ++mouse_body;
        count--;
    }
    while (count > 0);
}

void DSA_CopyMainOPMToScreen(unsigned short flag)
{
    int v2;
    OPM_Struct *src_pixel_map;
    unsigned short old_mouse_position_x;
    unsigned short old_mouse_position_y;
    unsigned short new_mouse_position_x;
    unsigned short new_mouse_position_y;

    if(DSA_globalParams.flags & 1)
    {
        src_pixel_map = DSA_globalParams.global_pixel_map;

        if((src_pixel_map->flags & BBOPM_MODIFIED) || flag)
        {
            v2 = 0;
            if ( DSA_MouseUpdateFlag && !(DSA_globalParams.method_flags & 0x10000) )
            {
                DSA_MouseUpdateFlag = 0;
                old_mouse_position_x = DSA_MouseValue_X_Current;
                old_mouse_position_y = DSA_MouseValue_Y_Current;
                v2 = 1;
                new_mouse_position_x = SYSTEM_MouseCursorPtr->position_x + DSA_MouseValue_X_Current;
                new_mouse_position_y = SYSTEM_MouseCursorPtr->position_y + DSA_MouseValue_Y_Current;

                switch (DSA_InternalMode)
                {
                    case 0:
                    {
                        OPM_CopyOPMOPM(src_pixel_map, (OPM_Struct *)&bmouseback,
                                       new_mouse_position_x, DSA_ScanlineStart[bank] + new_mouse_position_y,
                                       SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, 0, 0);
                        break;
                    }
                    case 1:
                    {
                        OPM_CopyOPMOPM(src_pixel_map, (OPM_Struct *)&bmouseback,
                                       new_mouse_position_x, new_mouse_position_y,
                                       SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, 0, 0);
                        break;
                    }
                    case 2:
                    {
                        OPM_CopyOPMOPM(src_pixel_map, (OPM_Struct *)&bmouseback,
                                       new_mouse_position_x, new_mouse_position_y,
                                       SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, 0, 0);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                ASM_copyMouseCursorWithTransparency(SYSTEM_MouseCursorPtr->transparent_color, SYSTEM_MouseCursorSize,
                                                    SYSTEM_MouseCursorPtr->body_data, bmouseoldback.buffer, bmouseback.buffer);
                OPM_CopyOPMOPM(&bmouseback, src_pixel_map, 0, 0,
                                SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height,
                                new_mouse_position_x, new_mouse_position_y);
            }

            switch (DSA_InternalMode)
            {
                case 0:
                {
                    if(word_77DF6)
                    {
                        /* FIXME: Missing function; not implemented yet */
                    }
                    else if(bank == 1)
                    {
                        /* FIXME: Missing function; not implemented yet */
                    }
                    else
                    {
                        /* FIXME: Missing function; not implemented yet */
                    }
                }
                case 1:
                {
                    VGA_CopyScreenBuffer((void *)DSA_ScreenBuffer[bank], src_pixel_map->buffer);
                    break;
                }
                case 2:
                {
                    /* FIXME: Missing function; not implemented yet */
                    break;
                }
                default:
                {

                    break;
                }

            }
            if(v2)
            {
                OPM_CopyOPMOPM(&bmouseoldback, src_pixel_map, 0, 0, SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, new_mouse_position_x, new_mouse_position_y);
                OPM_CopyOPMOPM(src_pixel_map, &mouseoldback[bank], new_mouse_position_x, new_mouse_position_y, SYSTEM_MouseCursorPtr->width, SYSTEM_MouseCursorPtr->height, 0, 0);
                if ( old_mouse_position_x != DSA_MouseValue_X_Current || old_mouse_position_y != DSA_MouseValue_Y_Current )
                {
                    SYSTEM_DrawMousePtr();
                    SYSTEM_RefreshMousePtr();
                }
                DSA_MouseUpdateFlag = 1;
            }
            src_pixel_map->flags &= 0xFDu;
        }
    }
}

void DSA_LoadPal(LBM_LogPalette* src_pal, unsigned int src_offset, unsigned short length, unsigned int dest_offset)
{
    LBM_LogPalette* dest_pal = DSA_globalParams.global_palette_data;

    if (src_offset < dest_pal->number_of_entries)
    {
        for (int i = 0; i < length && i + dest_offset < dest_pal->number_of_entries; i++)
        {
            dest_pal->pal_entry[dest_offset+i].peRed = src_pal->pal_entry[src_offset+i].peRed;
            dest_pal->pal_entry[dest_offset+i].peGreen = src_pal->pal_entry[src_offset+i].peGreen;
            dest_pal->pal_entry[dest_offset+i].peBlue = src_pal->pal_entry[src_offset+i].peBlue;
            dest_pal->pal_entry[dest_offset+i].peFlags = src_pal->pal_entry[src_offset+i].peFlags;
        }
    }
}

void VGA_ActivatePal(unsigned int length, LBM_PaletteEntry* pal_entry)
{
    outp(0x3c8, 0);

    for(int i = 0; i < length; i++)
    {
        /* Set palette colors (see https://stackoverflow.com/questions/12326759/mapped-colors-to-the-vga-palette-turn-out-wrong for shift operation. */
        outp(0x3c9, pal_entry[i].peRed >> 2);
        outp(0x3c9, pal_entry[i].peGreen >> 2);
        outp(0x3c9, pal_entry[i].peBlue >> 2);
    }
}

void DSA_ActivatePal(void)
{
    VGA_ActivatePal(DSA_globalParams.global_palette_data->number_of_entries, DSA_globalParams.global_palette_data->pal_entry);
}

void DSA_SetPalEntry(int paletteIndex, char redValue, char greenValue, char blueValue)
{
    LBM_PaletteEntry* pal_entry;
    LBM_LogPalette* pal = DSA_globalParams.global_palette_data;

    if (paletteIndex < pal->number_of_entries)
    {
        pal_entry = (LBM_PaletteEntry*)&pal->pal_entry[paletteIndex];
        pal_entry->peRed = redValue;
        pal_entry->peGreen = greenValue;
        pal_entry->peBlue = blueValue;
        pal_entry->peFlags = 0;
    }
}

void DSA_StretchOPMToScreen(OPM_Struct *src_pixel_map, OPM_Struct *dest_pixel_map)
{
    int src_pixel_width_index;
    signed int j;
    signed int i;
    int v5;
    unsigned char *src_pixel;
    unsigned char *dest_pixel;
    
    for ( i = 0; dest_pixel_map->height > i; ++i )
    {
        src_pixel = &src_pixel_map->buffer[i * src_pixel_map->height / dest_pixel_map->height * src_pixel_map->width];
        dest_pixel = &dest_pixel_map->buffer[i * dest_pixel_map->width];
        v5 = (src_pixel_map->width << 10) / dest_pixel_map->width;
        src_pixel_width_index = 0;
        
        for ( j = 0; dest_pixel_map->width > j; ++j )
        {
            dest_pixel[j] = src_pixel[src_pixel_width_index >> 10];
            src_pixel_width_index += v5;
        }
    }
}

static void DSA_PrintData(char *buffer, DSA_ErrorStruct *data)
{
    sprintf(buffer, "ERROR!: %s  %ld, %ld", data->text, data->data1, data->data2);
}