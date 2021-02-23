/**
 *
 *  Copyright (C) 2018-2019 Roman Pauer
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
#include <i86.h>

#define MAX_REGIONS 50

typedef struct
{
    unsigned short error_number;
    unsigned short error_parameter;
    unsigned short line_number;
    unsigned short reserved;
    const char *filename;
} BASEMEM_ErrorStruct;

typedef struct
{
    unsigned int flags;
    void *mem_ptr;
    unsigned int length;
    unsigned short base_selector;
    unsigned short base_address_allocated_block;
    unsigned int block;
} BASEMEM_RegionStruct;

typedef struct
{
    int largest_block_bytes;
    int max_unlocked_page_allocation;
    int largest_lockable_pages;
    int total_pages;
    int unlocked_pages;
    int unused_physical_pages;
    int total_physical_pages;
    int free_linear_pages;
    int paging_size_pages;
    int reserved[3];
} BASEMEM_MemData;

static const char *BASEMEM_error_str[] =
{
    "Illegal error code.",
    "No free entry in table.",
    "Out of DOS memory (DPMI error code : %#x).",
    "Out of XMS memory (DPMI error code : %#x).",
    "Could not allocate LDT segment descriptor (DPMI error code : %#x).",
    "Could not set segment base (DPMI error code : %#x).",
    "Could not set segment upper limit (DPMI error code : %#x).",
    "Could not find entry.",
    "Could not free DOS memory (DPMI error code : %#x).",
    "Could not free LDT segment descriptor (DPMI error code : %#x).",
    "Could not free XMS memory (DPMI error code : %#x).",
    "Couldn't resize DOS memory (DPMI error code : %#x).",
    "Could not resize XMS memory (DPMI error code : %#x).",
    "Unsupported memory type.",
    "Could not lock memory region (DPMI error code : %#x)",
    "Could not unlock memory region (DPMI error code : %#x)",
    "Illegal parameters."
};

static int BASEMEM_initialized = 0;
static BASEMEM_RegionStruct BASEMEM_regions[MAX_REGIONS];
static int BASEMEM_page_size;

static short BASEMEM_dpmiMajorVersion; /* DPMI major version as a binary number */
static short BASEMEM_dpmiMinorVersion; /* DPMI minor version as a binary number */
static short BASEMEM_dpmiFlags; /* Flags:
                                 * Bits | Significance
                                 *    0 | 0 = host is 16-bit DPMI implementation; 1 = host is 32-bit (80386) DPMI implementation
                                 *    1 | 0 = CPU returned to Virtual 86 mode for reflected interrupts; 1 = CPU returned to real mode for reflected interrupts
                                 *    2 | 0 = virtual memory not supported; 1 = virtual memory supported
                                 * 3-15 | reserved
                                 */
static short BASEMEM_cpuType; /* Processor type:
                               * Value | Significance
                               *   02H | 80286
                               *   03H | 80386
                               *   04H | 80486
                               */

static void BASEMEM_PushError(unsigned int error_number, int error_parameter, int line_number, const char *filename_ptr);
static void BASEMEM_LocalPrintError(char *buffer, const char *data);


int BASEMEM_Init(void)
{
    int index;
    struct SREGS sregs;
    union REGS inregs;

    if (!BASEMEM_initialized)
    {
        for (index = 0; index < MAX_REGIONS; index++)
        {
            BASEMEM_regions[index].flags = 0;
        }

        /* Get Version:
         * Returns the version number of the DPMI Specification implemented by the DPMI host.
         * Clients can use this information to determine which function calls are supported in the current environment.
         */
        BASEMEM_FillMemByte(&sregs, 12, 0);

        inregs.w.ax = 0x400;
        int386x(0x31, &inregs, &inregs, &sregs);

        BASEMEM_dpmiMajorVersion = inregs.h.ah;
        BASEMEM_dpmiMinorVersion = inregs.h.al;
        BASEMEM_dpmiFlags = inregs.w.bx;
        BASEMEM_cpuType = inregs.h.cl;

        /* Get DPMI Page Size: Returns the size of a single memory page in bytes. */
        BASEMEM_FillMemByte(&sregs, 12, 0);

        inregs.w.ax = 0x604;
        int386x(0x31, &inregs, &inregs, &sregs);
        if(inregs.w.cflag)
        {
            BASEMEM_page_size = 4096;
        }
        else
        {
            BASEMEM_page_size = (inregs.w.bx << 16) + inregs.w.cx;
        }

        BASEMEM_initialized = 1;
    }

    return 1;
}

void BASEMEM_Exit(void)
{
    int index;

    if (BASEMEM_initialized)
    {
        for (index = 0; index < MAX_REGIONS; index++)
        {
            if (BASEMEM_regions[index].flags & BASEMEM_MEMORY_ALLOCATED)
            {
                BASEMEM_Free(BASEMEM_regions[index].mem_ptr);
            }
        }

        BASEMEM_initialized = 0;
    }
}

unsigned int BASEMEM_GetFreeMemSize(unsigned int memory_flags)
{
    BASEMEM_MemData md;
    unsigned int retVal;
    struct SREGS sregs;
    union REGS inregs;

    switch (memory_flags & 0xff)
    {
        case BASEMEM_XMS_MEMORY:
        {
            /* Get Free Memory Information:
             * Returns information about the amount of available physical memory, linear address space, and disk space for page swapping.
             */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x500;
            sregs.es = FP_SEG(&md);
            inregs.x.edi = FP_OFF(&md);
            int386x(0x31, &inregs, &inregs, &sregs);
            if(!inregs.w.cflag)
            {
                retVal = md.largest_block_bytes - 65536;
            }
            break;
        }
        case BASEMEM_DOS_MEMORY:
        {
            /* Allocate DOS Memory Block:
             * Allocates a block of memory from the DOS memory pool, i.e. memory below the 1 MB boundary that is controlled by DOS.
             * Such memory blocks are typically used to exchange data with real mode programs, TSRs, or device drivers.
             * The function returns both the real mode segment base address of the block and one or more descriptors
             * that can be used by protected mode applications to access the block.
             */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x100;
            inregs.w.bx = 0xFFFF;
            int386x(0x31, &inregs, &inregs, &sregs);
            retVal = inregs.w.bx << 4;
            break;
        }
        default:
        {
            BASEMEM_PushError(13, 0, 353, "bbbasmem.c");
            retVal = 0;
            break;
        }
    }

    return retVal;
}

unsigned int BASEMEM_GetFreeVirtualMemSize(unsigned int memory_flags)
{
    BASEMEM_MemData md;
    unsigned int retVal;
    struct SREGS sregs;
    union REGS inregs;

    switch (memory_flags & 0xff)
    {
        case BASEMEM_XMS_MEMORY:
        {
            /* Get Free Memory Information:
             * Returns information about the amount of available physical memory, linear address space, and disk space for page swapping.
             */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x500;
            sregs.es = FP_SEG(&md);
            inregs.x.edi = FP_OFF(&md);
            int386x(0x31, &inregs, &inregs, &sregs);
            if(!inregs.w.cflag)
            {
                if(md.unused_physical_pages != -1)
                {
                    retVal = BASEMEM_page_size * md.unused_physical_pages;
                }
                else
                {
                    retVal = md.largest_block_bytes - 65536;
                }
            }
            break;
        }
        case BASEMEM_DOS_MEMORY:
        {
            /* Allocate DOS Memory Block:
             * Allocates a block of memory from the DOS memory pool, i.e. memory below the 1 MB boundary that is controlled by DOS.
             * Such memory blocks are typically used to exchange data with real mode programs, TSRs, or device drivers.
             * The function returns both the real mode segment base address of the block and one or more descriptors
             * that can be used by protected mode applications to access the block.
             */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x100;
            inregs.w.bx = 0xFFFF;
            int386x(0x31, &inregs, &inregs, &sregs);
            retVal = inregs.w.bx << 4;
            break;
        }
        default:
        {
            BASEMEM_PushError(13, 0, 353, "bbbasmem.c");
            retVal = 0;
            break;
        }
    }

    return retVal;
}

unsigned int BASEMEM_GetFreePhysicalMemSize(unsigned int memory_flags)
{
    BASEMEM_MemData md;
    unsigned int retVal;
    struct SREGS sregs;
    union REGS inregs;

    switch (memory_flags & 0xff)
    {
        case BASEMEM_XMS_MEMORY:
        {
            /* Get Free Memory Information:
             * Returns information about the amount of available physical memory, linear address space, and disk space for page swapping.
             */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x500;
            sregs.es = FP_SEG(&md);
            inregs.x.edi = FP_OFF(&md);
            int386x(0x31, &inregs, &inregs, &sregs);
            if(!inregs.w.cflag)
            {
                if(BASEMEM_dpmiFlags & BASEMEM_VIRTUAL_MEMORY_SUPPORTED)
                {
                    if(md.largest_lockable_pages != -1)
                    {
                        retVal = BASEMEM_page_size * md.largest_lockable_pages;
                    }
                }
                else
                {
                    retVal = md.largest_block_bytes - 65536;
                }
            }
            break;
        }
        case BASEMEM_DOS_MEMORY:
        {
            /* Allocate DOS Memory Block:
             * Allocates a block of memory from the DOS memory pool, i.e. memory below the 1 MB boundary that is controlled by DOS.
             * Such memory blocks are typically used to exchange data with real mode programs, TSRs, or device drivers.
             * The function returns both the real mode segment base address of the block and one or more descriptors
             * that can be used by protected mode applications to access the block.
             */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x100;
            inregs.w.bx = 0xFFFF;
            int386x(0x31, &inregs, &inregs, &sregs);
            retVal = inregs.w.bx << 4;
            break;
        }
        default:
        {
            BASEMEM_PushError(13, 0, 353, "bbbasmem.c");
            retVal = 0;
            break;
        }
    }

    return retVal;
}

void *BASEMEM_Alloc(unsigned int size, unsigned int memory_flags)
{
    int index, free_region_index;
    unsigned int heap;
    unsigned int block;
    void *mem_ptr;
    struct SREGS sregs;
    union REGS inregs;
    unsigned int retVal;
    unsigned int base_selector;
    unsigned int segment_limit;
    unsigned int base_address_allocated_block;

    if (size == 0)
    {
        return NULL;
    }

    free_region_index = -1;
    for (index = 0; index < MAX_REGIONS; index++)
    {
        if (BASEMEM_regions[index].flags == 0)
        {
            free_region_index = index;
            break;
        }
    }

    if (free_region_index == -1)
    {
        BASEMEM_PushError(1, 0, 617, "bbbasmem.c");
        return NULL;
    }

    switch (memory_flags & 0xff)
    {
        case BASEMEM_XMS_MEMORY:
        {
            size = (size + 0xFFF) & ~0xFFF;

            /* Allocate Memory Block: Allocates and commits a block of linear memory. */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x501;
            inregs.w.bx = size >> 16;
            inregs.w.cx = size & 0xFFFF;
            int386x(0x31, &inregs, &inregs, &sregs);

            heap = (inregs.w.bx << 16) + inregs.w.cx;
            block = (inregs.w.si << 16) + inregs.w.di;

            if (inregs.w.cflag)
            {
                BASEMEM_PushError(3, inregs.w.ax, 685, "bbbasmem.c");
                retVal = 0;
            }

            /* Allocate LDT Descriptors:
             * Allocates one or more descriptors in the task's Local Descriptor Table (LDT).
             * The descriptor(s) allocated must be initialized by the application with other function calls.
             */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x0;
            inregs.w.cx = 0x1;
            int386x(0x31, &inregs, &inregs, &sregs);

            base_selector = inregs.w.ax;

            if ( inregs.x.cflag )
            {
                /* Free Memory Block:
                 * Frees a memory block that was previously allocated with either
                 * the Allocate Memory Block function (Int 31H Function 0501H) or the Allocate Linear Memory Block function (Int 31H Function 0504H).
                 */
                BASEMEM_FillMemByte(&sregs, 12, 0);
    
                inregs.w.ax = 0x502;
                inregs.w.si = block >> 16;
                inregs.w.di = block & 0xFFFF;
                int386x(0x31, &inregs, &inregs, &sregs);
                BASEMEM_PushError(4, inregs.w.ax, 714, "bbbasmem.c");
                retVal = 0;
            }

            /* Set Segment Base Address: Sets the 32-bit linear base address field in the LDT descriptor for the specified segment. */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x7;
            inregs.w.bx = base_selector;
            inregs.w.cx = heap >> 16;
            inregs.w.dx = heap & 0xFFFF;
            int386x(0x31, &inregs, &inregs, &sregs);

            if ( inregs.x.cflag )
            {
                /* Free LDT Descriptor: Frees an LDT descriptor. */
                BASEMEM_FillMemByte(&sregs, 12, 0);
    
                inregs.w.ax = 0x1;
                inregs.w.bx = base_selector;
                int386x(0x31, &inregs, &inregs, &sregs);
    
                /* Free Memory Block:
                 * Frees a memory block that was previously allocated with either
                 * the Allocate Memory Block function (Int 31H Function 0501H) or the Allocate Linear Memory Block function (Int 31H Function 0504H).
                 */
                BASEMEM_FillMemByte(&sregs, 12, 0);
    
                inregs.w.ax = 0x502;
                inregs.w.si = block >> 16;
                inregs.w.di = block & 0xFFFF;
                int386x(0x31, &inregs, &inregs, &sregs);
    
                BASEMEM_PushError(4, inregs.w.ax, 714, "bbbasmem.c");
                retVal = 0;
            }

            segment_limit = size -1;
            if(segment_limit > 1048576)
            {
                segment_limit |= 0xFFF;
            }

            /* Set Segment Limit: Sets the limit field in the LDT descriptor for the specified segment. */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x8;
            inregs.w.bx = base_selector;
            inregs.w.cx = segment_limit >> 16;
            inregs.w.dx = segment_limit & 0xFFFF;
            int386x(0x31, &inregs, &inregs, &sregs);

            if ( inregs.x.cflag )
            {
                /* Free LDT Descriptor: Frees an LDT descriptor. */
                BASEMEM_FillMemByte(&sregs, 12, 0);
                
                inregs.w.ax = 0x1;
                inregs.w.bx = base_selector;
                int386x(0x31, &inregs, &inregs, &sregs);
                
                /* Free Memory Block:
                 * Frees a memory block that was previously allocated with either
                 * the Allocate Memory Block function (Int 31H Function 0501H) or the Allocate Linear Memory Block function (Int 31H Function 0504H).
                 */
                BASEMEM_FillMemByte(&sregs, 12, 0);
                
                inregs.w.ax = 0x502;
                inregs.w.si = block >> 16;
                inregs.w.di = block & 0xFFFF;
                int386x(0x31, &inregs, &inregs, &sregs);
                
                BASEMEM_PushError(4, inregs.w.ax, 714, "bbbasmem.c");
                retVal = 0;
            }
            break;
        }
        case BASEMEM_DOS_MEMORY:
        {
            size = (size + 15) & ~15;

            /* Allocate DOS Memory Block:
             * Allocates a block of memory from the DOS memory pool, i.e. memory below the 1 MB boundary that is controlled by DOS.
             * Such memory blocks are typically used to exchange data with real mode programs, TSRs, or device drivers.
             * The function returns both the real mode segment base address of the block and one or more descriptors
             * that can be used by protected mode applications to access the block.
             */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x100;
            inregs.w.bx = size >> 4;
            int386(0x31, &inregs, &inregs);

            base_address_allocated_block = inregs.w.ax;
            base_selector = inregs.h.dl;
            if (inregs.w.cflag)
            {
                BASEMEM_PushError(3, inregs.w.ax, 685, "bbbasmem.c");
                retVal = 0;
            }

            heap = base_address_allocated_block << 4;
            block = 0;

            break;
        }
        default:
        {
            BASEMEM_PushError(13, 0, 353, "bbbasmem.c");
            if ( !heap )
            {
                return (void*)heap;
            }
            break;
        }
    }

    BASEMEM_regions[free_region_index].flags = memory_flags | BASEMEM_MEMORY_ALLOCATED;
    BASEMEM_regions[free_region_index].mem_ptr = (void*)heap;
    BASEMEM_regions[free_region_index].length = size;
    BASEMEM_regions[free_region_index].base_selector = base_selector;
    BASEMEM_regions[free_region_index].base_address_allocated_block = base_address_allocated_block;
    BASEMEM_regions[free_region_index].block = block;

    if (memory_flags & BASEMEM_LOCK_MEMORY)
    {
        if (!BASEMEM_LockRegion(BASEMEM_regions[free_region_index].mem_ptr, size))
        {
            BASEMEM_Free(BASEMEM_regions[free_region_index].mem_ptr);
            return NULL;
        }

        BASEMEM_regions[free_region_index].flags |= BASEMEM_MEMORY_LOCKED;
    }

    if (memory_flags & BASEMEM_ZERO_MEMORY)
    {
        BASEMEM_FillMemByte((void*)heap, size, 0);
    }
    else
    {
        BASEMEM_FillMemByte((void*)heap, size, 0xCC);
    }

    return (void*)heap;
}

int BASEMEM_Free(void *mem_ptr)
{
    int index, found_region_index;
    struct SREGS sregs;
    union REGS inregs;

    found_region_index = -1;
    for (index = 0; index < MAX_REGIONS; index++)
    {
        if (BASEMEM_regions[index].flags & BASEMEM_MEMORY_ALLOCATED)
        {
            if (BASEMEM_regions[index].mem_ptr == mem_ptr)
            {
                found_region_index = index;
                break;
            }
        }
    }

    if (found_region_index == -1)
    {
        BASEMEM_PushError(7, 0, 914, "bbbasmem.c");
        return 0;
    }

    BASEMEM_FillMemByte(BASEMEM_regions[found_region_index].mem_ptr, BASEMEM_regions[found_region_index].length, 0xCC);

    if (BASEMEM_regions[found_region_index].flags & BASEMEM_MEMORY_LOCKED)
    {
        BASEMEM_UnlockRegion(BASEMEM_regions[found_region_index].mem_ptr, BASEMEM_regions[found_region_index].length);
        BASEMEM_regions[found_region_index].flags &= ~BASEMEM_MEMORY_LOCKED;
    }

    switch (BASEMEM_regions[found_region_index].flags & 0xff)
    {
        case BASEMEM_XMS_MEMORY:
        {
            /* Free LDT Descriptor: Frees an LDT descriptor. */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x1;
            inregs.w.bx = BASEMEM_regions[found_region_index].base_selector;
            int386x(0x31, &inregs, &inregs, &sregs);
            if (inregs.w.cflag)
            {
                BASEMEM_PushError(9, inregs.w.ax, 982, "bbbasmem.c");
            }
            else
            {
                /* Free Memory Block:
                 * Frees a memory block that was previously allocated with either
                 * the Allocate Memory Block function (Int 31H Function 0501H) or the Allocate Linear Memory Block function (Int 31H Function 0504H).
                 */
                BASEMEM_FillMemByte(&sregs, 12, 0);

                inregs.w.ax = 0x502;
                inregs.w.si = BASEMEM_regions[found_region_index].block >> 16;
                inregs.w.di = BASEMEM_regions[found_region_index].block & 0xFFFF;
                int386x(0x31, &inregs, &inregs, &sregs);
                if (inregs.w.cflag)
                {
                    BASEMEM_PushError(10, inregs.w.ax, 1001, "bbbasmem.c");
                }
            }
            break;
        }
        case BASEMEM_DOS_MEMORY:
        {
            /* Free DOS Memory Block: Frees a memory block that was previously allocated with the Allocate DOS Memory Block function (Int 31H Function 0100H). */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x101;
            inregs.w.bx = BASEMEM_regions[found_region_index].base_selector;
            int386x(0x31, &inregs, &inregs, &sregs);
            if (inregs.w.cflag)
            {
                BASEMEM_PushError(8, inregs.w.ax, 960, "bbbasmem.c");
            }
            break;
        }
        default:
        {
            BASEMEM_PushError(13, 0, 1009, "bbbasmem.c");
            break;
        }
    }
    BASEMEM_regions[found_region_index].flags = 0;
    return 1;
}

static void *BASEMEM_Realloc(void *mem_ptr, unsigned int size)
{
    int index, found_region_index;
    unsigned int old_size;
    void *new_mem_ptr;
    struct SREGS sregs;
    union REGS inregs;
    unsigned int block;

    if ( !size )
    {
        return NULL;
    }

    found_region_index = -1;
    for (index = 0; index < MAX_REGIONS; index++)
    {
        if (BASEMEM_regions[index].flags & BASEMEM_MEMORY_ALLOCATED)
        {
            if (BASEMEM_regions[index].mem_ptr == mem_ptr)
            {
                found_region_index = index;
                break;
            }
        }
    }

    if (found_region_index == -1)
    {
        BASEMEM_PushError(7, 0, 1078, "bbbasmem.c");
        return NULL;
    }

    old_size = BASEMEM_regions[found_region_index].length;
    if (old_size > size)
    {
        BASEMEM_FillMemByte((void *) (((unsigned int)BASEMEM_regions[found_region_index].mem_ptr) + size), old_size - size, 0xCC);
    }

    if (BASEMEM_regions[found_region_index].flags & BASEMEM_MEMORY_LOCKED)
    {
        BASEMEM_UnlockRegion(BASEMEM_regions[found_region_index].mem_ptr, BASEMEM_regions[found_region_index].length);
        BASEMEM_regions[found_region_index].flags &= ~BASEMEM_MEMORY_LOCKED;
    }

    switch (BASEMEM_regions[found_region_index].flags & 0xff)
    {
        case BASEMEM_XMS_MEMORY:
        {
            size = (size + 0xFFF) & ~0xFFF;
            BASEMEM_FillMemByte(&sregs, 12, 0);

            /* Resize Memory Block:
             * Changes the size of a memory block that was previously allocated with either
             * the Allocate Memory Block function (Int 31H Function 0501H) or the Allocate Linear Memory Block function (Int 31H Function 0504H).
             */
            inregs.w.ax = 0x503;
            inregs.w.bx = size >> 16;
            inregs.w.si = BASEMEM_regions[found_region_index].block >> 16;
            inregs.w.di = BASEMEM_regions[found_region_index].block;
            int386x(0x31, &inregs, &inregs, &sregs);

            new_mem_ptr = (void *)(inregs.w.cx + (inregs.w.bx << 16));
            block = inregs.w.di + (inregs.w.si << 16);

            if (inregs.w.cflag)
            {
                BASEMEM_PushError(12, inregs.w.ax, 1175, "bbbasmem.c");
                return NULL;
            }

            /* Set Segment Base Address: Sets the 32-bit linear base address field in the LDT descriptor for the specified segment. */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x7;
            inregs.w.bx = BASEMEM_regions[found_region_index].base_selector;
            inregs.w.cx = ((unsigned short)new_mem_ptr) >> 16;
            inregs.w.dx = ((unsigned short)new_mem_ptr) & 0xFFFF;
            int386x(0x31, &inregs, &inregs, &sregs);

            if ( inregs.x.cflag )
            {
                /* Free LDT Descriptor: Frees an LDT descriptor. */
                BASEMEM_FillMemByte(&sregs, 12, 0);
                
                inregs.w.ax = 0x1;
                inregs.w.bx = BASEMEM_regions[found_region_index].base_selector;
                int386x(0x31, &inregs, &inregs, &sregs);
                
                /* Free Memory Block:
                 * Frees a memory block that was previously allocated with either
                 * the Allocate Memory Block function (Int 31H Function 0501H) or the Allocate Linear Memory Block function (Int 31H Function 0504H).
                 */
                BASEMEM_FillMemByte(&sregs, 12, 0);
                
                inregs.w.ax = 0x502;
                inregs.w.si = block >> 16;
                inregs.w.di = block & 0xFFFF;
                int386x(0x31, &inregs, &inregs, &sregs);
                
                BASEMEM_PushError(5, inregs.w.ax, 1210, "bbbasmem.c");
                return NULL;
            }

            /* Set Segment Limit: Sets the limit field in the LDT descriptor for the specified segment. */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x8;
            inregs.w.bx = BASEMEM_regions[found_region_index].base_selector;
            inregs.w.cx = ((unsigned short)new_mem_ptr + size - 1) >> 16;
            inregs.w.dx = (unsigned short)new_mem_ptr + size - 1;
            int386x(0x31, &inregs, &inregs, &sregs);

            if ( inregs.x.cflag )
            {
                /* Free LDT Descriptor: Frees an LDT descriptor. */
                BASEMEM_FillMemByte(&sregs, 12, 0);
                
                inregs.w.ax = 0x1;
                inregs.w.bx = BASEMEM_regions[found_region_index].base_selector;
                int386x(0x31, &inregs, &inregs, &sregs);
                
                /* Free Memory Block:
                 * Frees a memory block that was previously allocated with either
                 * the Allocate Memory Block function (Int 31H Function 0501H) or the Allocate Linear Memory Block function (Int 31H Function 0504H).
                 */
                BASEMEM_FillMemByte(&sregs, 12, 0);
                
                inregs.w.ax = 0x502;
                inregs.w.si = block >> 16;
                inregs.w.di = block & 0xFFFF;
                int386x(0x31, &inregs, &inregs, &sregs);
                
                BASEMEM_PushError(6, inregs.w.ax, 1245, "bbbasmem.c");
                return NULL;
            }
            break;
        }
        case BASEMEM_DOS_MEMORY:
        {
            size = (size + 15) & ~15;

            /* Resize DOS Memory Block:
             * Changes the size of a memory block that was previously allocated with the Allocate DOS Memory Block function (Int 31H Function 0100H).
             */
            BASEMEM_FillMemByte(&sregs, 12, 0);

            inregs.w.ax = 0x102;
            inregs.w.bx = size >> 4;
            inregs.w.dx = BASEMEM_regions[found_region_index].base_selector;
            int386x(0x31, &inregs, &inregs, &sregs);

            if (inregs.w.cflag)
            {
                BASEMEM_PushError(11, inregs.w.ax, 1135, "bbbasmem.c");
                return NULL;
            }
            new_mem_ptr = BASEMEM_regions[found_region_index].mem_ptr;
            block = 0;

            break;
        }
        default:
        {
            BASEMEM_PushError(13, 0, 1254, "bbbasmem.c");
            return NULL;
        }
    }

    if ( !new_mem_ptr )
    {
        return new_mem_ptr;
    }

    if (BASEMEM_regions[found_region_index].flags & BASEMEM_LOCK_MEMORY)
    {
        if (BASEMEM_LockRegion(new_mem_ptr, size))
        {
            BASEMEM_regions[found_region_index].flags |= BASEMEM_MEMORY_LOCKED;
        }
        else
        {
            return NULL;
        }
    }

    if (old_size < size)
    {
        if (BASEMEM_regions[found_region_index].flags & BASEMEM_ZERO_MEMORY)
        {
            BASEMEM_FillMemByte((void *) (((unsigned int)new_mem_ptr) + old_size), size - old_size, 0);
        }
        else
        {
            BASEMEM_FillMemByte((void *) (((unsigned int)new_mem_ptr) + old_size), size - old_size, 0xCC);
        }
    }

    BASEMEM_regions[found_region_index].mem_ptr = new_mem_ptr;
    BASEMEM_regions[found_region_index].length = size;
    BASEMEM_regions[found_region_index].block = block;

    return new_mem_ptr;
}

int BASEMEM_LockRegion(void *mem_ptr, unsigned int length)
{
    struct SREGS sregs;
    union REGS inregs;
    int retVal;
    int region_locked;

    region_locked = 1;

    if (mem_ptr != NULL)
    {
        /* Lock Linear Region: Locks the specified linear address range. */
        BASEMEM_FillMemByte(&sregs, 12, 0);

        inregs.w.ax = 0x600;
        inregs.w.bx = (unsigned short)mem_ptr >> 16;
        inregs.w.cx = (unsigned short)mem_ptr;
        inregs.w.si = length >> 16;
        inregs.w.di = length;
        int386x(0x31, &inregs, &inregs, &sregs);

        if (inregs.w.cflag)
        {
            BASEMEM_PushError(14, inregs.w.ax, 1371, "bbbasmem.c");
            region_locked = 0;
        }
        retVal = region_locked;
    }
    else
    {
        BASEMEM_PushError(16, inregs.w.ax, 1348, "bbbasmem.c");
        retVal = 0;
    }
    return retVal;
}

int BASEMEM_UnlockRegion(void *mem_ptr, unsigned int length)
{
    struct SREGS sregs;
    union REGS inregs;
    int retVal;
    int region_unlocked;

    region_unlocked = 1;

    if (mem_ptr != NULL)
    {
        /* Unlock Linear Region: Unlocks a linear address range that was previously locked using the Lock Linear Region function (Int 31H Function 0600H). */
        BASEMEM_FillMemByte(&sregs, 12, 0);

        inregs.w.ax = 0x601;
        inregs.w.bx = (unsigned short)mem_ptr >> 16;
        inregs.w.cx = (unsigned short)mem_ptr;
        inregs.w.si = length >> 16;
        inregs.w.di = length;
        int386x(0x31, &inregs, &inregs, &sregs);

        if (inregs.w.cflag)
        {
            BASEMEM_PushError(15, inregs.w.ax, 1435, "bbbasmem.c");
            region_unlocked = 0;
        }
        retVal = region_unlocked;
    }
    else
    {
        BASEMEM_PushError(16, inregs.w.ax, 1412, "bbbasmem.c");
        retVal = 0;
    }
    return retVal;
}

void BASEMEM_FillMemByte(void *dst, unsigned int length, int c)
{
    if (dst != NULL)
    {
        memset(dst, c, length);
    }
    else
    {
        BASEMEM_PushError(16, 0, 1482, "bbbasmem.c");
    }
}

void BASEMEM_FillMemLong(void *dst, unsigned int length, unsigned int c)
{
    unsigned int *dst4;
    unsigned int index;

    if (dst != NULL)
    {
        dst4 = (unsigned int *) dst;
        length >>= 2;

        for (index = 0; index < length; index++)
        {
            *dst4 = c;
            dst4++;
        }
    }
    else
    {
        BASEMEM_PushError(16, 0, 1532, "bbbasmem.c");
    }
}

void BASEMEM_CopyMem(const void *src, void *dst, unsigned int length)
{
    if ((src != NULL) && (dst != NULL))
    {
        memmove(dst, src, length);
    }
    else
    {
        BASEMEM_PushError(16, 0, 1576, "bbbasmem.c");
    }
}

void *BASEMEM_AlignMemptr(void *mem_ptr)
{
    return (void *) ( (((unsigned int)mem_ptr) + 7) & ~((unsigned int)7) );
}

void BASEMEM_PushError(unsigned int error_number, int error_parameter, int line_number, const char *filename_ptr)
{
    BASEMEM_ErrorStruct data;

    data.error_number = error_number;
    data.error_parameter = error_parameter;
    data.line_number = line_number;
    data.filename = filename_ptr;

    ERROR_PushError((ERROR_PrintErrorPtr)BASEMEM_LocalPrintError, "BASEMEM", sizeof(data), (const char *) &data);
}

static void BASEMEM_LocalPrintError(char *buffer, const char *data)
{
    unsigned int error_number, error_parameter;
    char tempbuf[100];

#define DATA (((BASEMEM_ErrorStruct *)data))
    error_number = DATA->error_number;
    if (error_number > 16)
    {
        error_number = 0;
    }

    error_parameter = DATA->error_parameter;
    if (!(error_parameter & 0x8000))
    {
        error_parameter = 0;
    }

    snprintf(tempbuf, 100, BASEMEM_error_str[error_number], error_parameter);

    if ((DATA->line_number != 0) && (DATA->filename != NULL))
    {
        snprintf(buffer, 200, "%s\n(line %u of file %s)", tempbuf, DATA->line_number, DATA->filename);
    }
    else
    {
        strncpy(buffer, tempbuf, 199);
    }
#undef DATA
}

void BASEMEM_PrintReport(FILE *fp)
{
    struct SREGS sregs;
    union REGS inregs;
    BASEMEM_MemData md;

    if (fp == NULL)
    {
        fp = stdout;
    }

    fprintf(fp, "\nBASEMEM report\n\n");
    fprintf(fp, "DPMI version : %u.%u\n", BASEMEM_dpmiMajorVersion, BASEMEM_dpmiMinorVersion);
    fprintf(fp, "CPU type     : %u\n", BASEMEM_cpuType);
    fprintf(fp, "Flags        : %#x\n", BASEMEM_dpmiFlags);

    /* Get Free Memory Information:
     * Returns information about the amount of available physical memory, linear address space, and disk space for page swapping.
     */
    BASEMEM_FillMemByte(&sregs, 12, 0);

    inregs.w.ax = 0x500;
    sregs.es = FP_SEG(&md);
    inregs.x.edi = FP_OFF(&md);
    int386x(0x31, &inregs, &inregs, &sregs);

    fprintf(fp, "Largest available free block in bytes      : %lu\n", md.largest_block_bytes);
    if ( md.max_unlocked_page_allocation != -1 )
    {
        fprintf(fp, "Maximum unlocked page allocation       : %lu\n", md.max_unlocked_page_allocation);
    }
    if ( md.largest_lockable_pages != -1 )
    {
        fprintf(fp, "Maximum locked page allocation         : %lu\n", md.largest_lockable_pages);
    }
    if ( md.total_pages != -1 )
    {
        fprintf(fp, "Linear addr space size in pages        : %lu\n", md.total_pages);
    }
    if ( md.unlocked_pages != -1 )
    {
        fprintf(fp, "Total number of unlocked pages         : %lu\n", md.unlocked_pages);
    }
    if ( md.unused_physical_pages != -1 )
    {
        fprintf(fp, "Number of free pages                   : %lu\n", md.unused_physical_pages);
    }
    if ( md.total_physical_pages != -1 )
    {
        fprintf(fp, "Total number of physical pages         : %lu\n", md.total_physical_pages);
    }
    if ( md.free_linear_pages != -1 )
    {
        fprintf(fp, "Free linear address space in pages     : %lu\n", md.free_linear_pages);
    }
    if ( md.paging_size_pages != -1 )
    {
        fprintf(fp, "Size of paging file/partition in pages : %lu\n", md.paging_size_pages);
    }
    fprintf(fp, "\n");
}

