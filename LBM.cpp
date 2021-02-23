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
 * Functions handling the Interleaved Bitmap (ILBM) image file format.
 *************************************************************************/

#include "BASEMEM.h"
#include "ERROR.h"
#include "DSA.h"
#include "DOS.h"
#include <stdio.h>
#include <stdlib.h>

#define CHUNK_LENGTH 4

#define BMHD_W_HIBYTE 8
#define BMHD_W_LOBYTE 9
#define BMHD_H_HIBYTE 10
#define BMHD_H_LOBYTE 11

#define LBM_W_HIBYTE 20
#define LBM_W_LOBYTE 21
#define LBM_H_HIBYTE 22
#define LBM_H_LOBYTE 23

#define LBM_COMPRESSION_BYTE 30

LBM_LogPalette pal;

void LBM_GetPal(unsigned char* data, LBM_LogPalette *pal)
{
    unsigned char* val;
    unsigned short index;
    LBM_PaletteEntry* palEntry = pal->pal_entry;

    val = &data[48];

    for(index = 0; index < 256; index++)
    {
        palEntry[index].peRed = *val++;

        palEntry[index].peGreen = *val++;

        palEntry[index].peBlue = *val++;
    }
    pal->number_of_entries = 256;
}

char * LBM_FindChunk(const char *data, char *chunk)
{
    unsigned char chunk_counter;
    unsigned short chunk_index;
    unsigned int  data_counter;
    char* ptrData;

    ptrData = (char*)&data[0];

    data_counter = 0;

    while(data_counter <= *((unsigned int *)data))
    {
        chunk_counter = 0;

        for(chunk_index = 0; chunk_index < CHUNK_LENGTH; chunk_index++)
        {
            if(ptrData[chunk_index] == chunk[chunk_index])
            {
                chunk_counter++;
            }
        }

        if (chunk_counter == CHUNK_LENGTH)
        {
            return (char*)(data+data_counter);
        }
        ptrData++;
        data_counter++;
    }
    return 0;
}

void LBM_DisplayLBMinOPM(char *file_handle, OPM_Struct *pixel_map)
{
    unsigned short lbmWidth;
    unsigned short lbmHeight;
    unsigned int index_width;
    unsigned int index_height;
    char* body;
    char color;
    int x,y, rept, idxX, idxY;

    body = LBM_FindChunk(file_handle, "BODY");
    body += 4;

    lbmWidth = abs((file_handle[LBM_W_HIBYTE] << 8) + file_handle[LBM_W_LOBYTE]);
    lbmHeight = abs((file_handle[LBM_H_HIBYTE] << 8) + file_handle[LBM_H_LOBYTE]);

    if(file_handle[LBM_COMPRESSION_BYTE] != 0)
    {
        x = 0;
        y = 0;
        do
        {
            rept = *body++;
            if ( rept >= 128 )
            {
                if ( rept > 128 )
                {
                    color = *body++;
                    for ( idxX = 0; idxX < (257 - rept); ++idxX )
                    {
                        OPM_SetPixel(pixel_map, x, y, color);
                        x++;
                    }
                }
            }
            else
            {
                for ( idxY = 0; idxY < (rept + 1); ++idxY )
                {
                    color = *body++;
                    OPM_SetPixel(pixel_map, x, y, color);
                    x++;
                }
            }
            if ( x >= lbmWidth )
            {
                x = 0;
                ++y;
            }
        }
        while ( y < lbmHeight );
    }
    else
    {
        for ( index_height = 0; index_height < lbmHeight; ++index_height )
        {
          for ( index_width = 0; index_width < lbmWidth; ++index_width )
          {
            color = *body++;
            OPM_SetPixel(pixel_map, index_width, index_height, color);
          }
        }
    }
}

unsigned int LBM_DisplayLBM(char *fileName, OPM_Struct *pixel_map, LBM_LogPalette *pal, int flags)
{
    const char *file_handle;
    int fileLen;
    char* bmhd;
    char* body;
    char* cmap;
    unsigned short lbmWidth;
    unsigned short lbmHeight;
    unsigned int pixel_map_size;
    unsigned int retVal;

    if ( flags & 0x10000 )
    {
        file_handle = fileName;
        fileLen = 100000000;
    }
    else
    {
        fileLen = DOS_GetFileLength(fileName);
        if ( fileLen < 0 )
        {
            return 0;
        }

        file_handle = (const char*)DOS_ReadFile(fileName, 0, 0);
        if ( !file_handle )
        {
            return 0;
        }
    }

    bmhd = LBM_FindChunk(file_handle, "BMHD");
    if (bmhd == NULL)
    {
        return 0;
    }

    body = LBM_FindChunk(file_handle, "BODY");
    if (body == NULL)
    {
        return 0;
    }

    cmap = LBM_FindChunk(file_handle, "CMAP");
    if (cmap == NULL)
    {
        return 0;
    }

    if ( pal )
    {
        LBM_GetPal((unsigned char*)file_handle, pal);
    }

    lbmWidth = abs((bmhd[BMHD_W_HIBYTE] << 8) + bmhd[BMHD_W_LOBYTE]);
    lbmHeight = abs((bmhd[BMHD_H_HIBYTE] << 8) + bmhd[BMHD_H_LOBYTE]);

    if ( flags & 1 )
    {
        retVal = OPM_New(lbmWidth, lbmHeight, 1u, pixel_map, 0);
        if ( !retVal )
        {
            return retVal;
        }
    }
    else if ( flags & 2 )
    {
        pixel_map->width = lbmWidth;
        pixel_map->height = lbmHeight;
        pixel_map->stride = pixel_map->width;
        pixel_map_size = pixel_map->width * pixel_map->height;
        pixel_map->clip_x = 0;
        pixel_map->clip_y = 0;
        pixel_map->size = pixel_map->bytes_per_pixel * pixel_map_size;
        pixel_map->clip_width = pixel_map->width;
        pixel_map->clip_height = pixel_map->height;
    }

    LBM_DisplayLBMinOPM((char *)file_handle, pixel_map);

    if ( !(flags & 1) )
    {
        BASEMEM_Free((void *)file_handle);
    }

    return 1;
}
