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

#ifndef DPMI_H
#define DPMI_H

#include <stdint.h>

#pragma pack(push,1);
/* SuperVGA information block */
typedef struct
{
    char            VESASignature[4]; /* 4 signature bytes */
    unsigned short  VESAVersion;      /* VESA version number */
    long            OEMStringPtr;     /* Pointer to OEM string */
    long            Capabilities;     /* Capabilities of the video environment */
    long            VideoModePtr;     /* Pointer to supported Super VGA modes */
    unsigned short  TotalMemory;      /* Number of 64K memory blocks on board */
    unsigned char   Reserved[242];    /* Reserved for future use. */
} VbeInfoBlock;

/* SuperVGA mode information block */
typedef struct
{
    short       ModeAttributes;
    char        WindowAAttributes;
    char        WindowBAttributes;
    short       WindowGranularity;
    short       WindowSize;
    short       StartSegmentWindowA;
    short       StartSegmentWindowB;
    int         WindowPositioningFunction;
    short       BytesPerScanLine;
    /* Remainder of this structure is optional for VESA modes in v1.0/1.1, needed for OEM modes. */
    short       PixelWidth;
    short       PixelHeight;
    char        CharacterCellPixelWidth;
    char        CharacterCellPixelHeight;
    char        NumberOfMemoryPlanes;
    char        BitsPerPixel;
    char        NumberOfBanks;
    char        MemoryModelType;
    char        SizeOfBank;
    char        NumberOfImagePages;
    char        Reserved1;
    /* VBE v1.2+ */
    char        RedMaskSize;
    char        RedFieldPosition;
    char        GreenMaskSize;
    char        GreenFieldPosition;
    char        BlueMaskSize;
    char        BlueFieldPosition;
    char        ReservedMaskSize;
    char        ReservedFieldPosition;
    char        DirectColourModeInfo;
    int     PhysBasePtr;
    int         OffScreenMemOffset;
    short       OffScreenMemSize;
} ModeInfoBlock;

typedef struct {
    int     edi;
    int     esi;
    int     ebp;
    int     reserved;
    int     ebx;
    int     edx;
    int     ecx;
    int     eax;
    short   flags;
    short   es,ds,fs,gs,ip,cs,sp,ss;
} RMREGS;
#pragma pack(pop);

extern VbeInfoBlock *DPMI_VbeInfo;
extern RMREGS rmregs;

extern void* DPMI_Alloc(int size);
extern bool DPMI_IsVesaAvailable(void);
extern bool DPMI_IsVideoModeSupported(short video_mode, short* vbe_info);

#endif /* DPMI_H */