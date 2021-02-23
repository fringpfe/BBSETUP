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

/*****************************************************************************************************************
 * Functions handling the DOS Protected Mode Interface (DPMI) - in particular the VESA BIOS Extension (VBE).
 *****************************************************************************************************************/

#include "DPMI.h"
#include "GUI.h"
#include <i86.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DPMI_real_segment(P) ((((unsigned int) (P)) >> 4) & 0xFFFF)
#define DPMI_real_offset(P)  (((unsigned int) (P)) & 0xF)

VbeInfoBlock *DPMI_VbeInfo;
ModeInfoBlock *ModeInfo;
RMREGS rmregs;

void* DPMI_Alloc(int size)
{
    union REGS inregs;

    /* Allocate DOS Memory Block: Allocates a block of memory from the DOS memory pool, i.e. memory below the 1 MB boundary that is controlled by DOS. */
    inregs.w.ax = 0x100;
    inregs.w.bx = size;

    int386(0x31, &inregs, &inregs);

    if (inregs.w.cflag)
    {
        GUI_ErrorHandler(1049);
    }

    return (void*)(inregs.w.ax << 4);
}

bool DPMI_CompareVideoModeNumber(short video_mode, short* vbe_info)
{
    short *i;

    for ( i = vbe_info; ; ++i )
    {
        if ( *i == video_mode )
        {
            return 1;
        }
        if (*i == -1)
        {
            break;
        }
    }
    return 0;
}

bool DPMI_IsVesaAvailable(void)
{
    union REGS inregs;
    struct SREGS segregs;

    if(!DPMI_VbeInfo)
    {
        DPMI_VbeInfo = (VbeInfoBlock *)DPMI_Alloc(sizeof(VbeInfoBlock));
    }

    printf("DPMI_IsVesaAvailable: Check if VESA is available.\n");

    /* VESA SuperVGA BIOS (VBE) - GET SuperVGA INFORMATION:
     * Determine whether VESA BIOS extensions are present and the capabilities supported by the display adapter.
     */
    memset(&rmregs, 0, 50u);
    memset(&segregs, 0, sizeof(SREGS)); /* Workaround to prevent segment violation. */

    rmregs.eax = 0x4f00;
    rmregs.es = DPMI_real_segment(DPMI_VbeInfo);
    rmregs.edi = DPMI_real_offset(DPMI_VbeInfo);
    inregs.w.ax = 0x300;
    inregs.w.bx = 0x10;
    inregs.w.cx = 0;

    segregs.es = FP_SEG(&rmregs);
    inregs.x.edi = FP_OFF(&rmregs);
    int386x(0x31,&inregs,&inregs,&segregs);

    if(rmregs.eax != 0x4f)
    {
        return 0;
    }

    return (memcmp(DPMI_VbeInfo, "VESA", 4u) == 0);
}

bool DPMI_IsVideoModeSupported(short video_mode, short* vbe_info)
{
    union REGS inregs;
    struct SREGS sregs;

    if ( !DPMI_CompareVideoModeNumber(video_mode, (short *)vbe_info) )
    {
        return 0;
    }

    if ( !ModeInfo )
    {
        ModeInfo = (ModeInfoBlock *)DPMI_Alloc(sizeof(ModeInfoBlock));
    }

    /* VESA SuperVGA BIOS - GET SuperVGA MODE INFORMATION: Determine the attributes of the specified video mode. */
    memset(&rmregs, 0, sizeof(rmregs));
    memset(&sregs, 0, sizeof(sregs));
    rmregs.eax = 0x4f01;
    rmregs.ecx = video_mode;
    rmregs.es = DPMI_real_segment(ModeInfo);
    rmregs.edi = DPMI_real_offset(ModeInfo);
    inregs.w.ax = 0x300;
    inregs.w.bx = 0x10;
    inregs.w.cx = 0;

    sregs.es = FP_SEG(&rmregs);
    inregs.x.edi = FP_OFF(&rmregs);

    int386x(0x31,&inregs,&inregs,&sregs);

    if(rmregs.eax != 0x4f)
    {
        return 0;
    }

    return (ModeInfo->ModeAttributes & 3) != 2;
}