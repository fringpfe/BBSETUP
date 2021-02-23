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

#ifndef GUI_H
#define GUI_H

#include <stdint.h>
#include "OPM.h"
#include "SETUP.h"

typedef struct
{
    unsigned int index;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} GUI_ColorMapStruct;

extern void GUI_DrawArrow(OPM_Struct *pixel_map, int x, int y, char color, int direction);
extern void GUI_DrawFrame(OPM_Struct *pixel_map, int leftBoundary, int upperBoundary, int rightBoundary, int lowerBoundary, unsigned char color);
extern void GUI_DrawEmbossedArea(OPM_Struct *pixel_map, int x, int y, int width, int height, unsigned char shadowColor, unsigned char fillColor, unsigned char highlightColor);
extern void GUI_DrawFilledBackground(OPM_Struct *pixel_map, int x, int y, int width, int height, unsigned char color0, unsigned char color1, unsigned char color2, unsigned char color3);
extern void GUI_DrawButton(OPM_Struct *pixel_map, int leftBndry, int upperBndry, int rightBndry, int lowerBndry, unsigned char color);
extern void GUI_DrawDropShadow(OPM_Struct *pixel_map, int leftBndry, int upperBndry, int rightBndry, int lowerBndry);
extern void GUI_DrawLoweredButton(OPM_Struct *pixel_map, int x_start, int y_start, int x_origin, int y_origin);
extern void GUI_DrawBox(OPM_Struct *pixel_map, int x, int y, int x_max, int y_max);
extern void GUI_SetPal(void);
extern void GUI_CreateMouseCursor(unsigned short width, unsigned short height, short x, short y, unsigned char *bitmap);
extern bool GUI_PrintText(char *string, int color, OPM_Struct *pixel_map, int x, int y, int max_x, int max_y);
extern bool GUI_PrintButtonText(char *string, int color, OPM_Struct *pixel_map, int x, int y, int max_x, int max_y);
extern int GUI_DrawAssertBox(char *string);
extern int GUI_DrawMenu(SETUP_MenuStruct *menu);
extern void GUI_DrawTextBox(char *string);
extern void GUI_ParseReadmeText(SETUP_ScriptDataStruct *text_data, char *filename);
extern void GUI_DrawReadmeText(char *text);
extern int GUI_MenuLoop(SETUP_MenuStruct *menu, int idx);
extern void GUI_ProcessEvents(void);
extern void GUI_DrawProgressBar(int flag);
extern int GUI_DrawDriveSelectMenu(SETUP_MenuStruct *menu);
extern signed int GUI_WriteIniEntry_Cdrom(void);
extern int GUI_DrawTargetDriveMenu(int required_free_space);
extern char * GUI_DrawTargetPathMenu(char* headerString, char* targetPath);
extern int GUI_DrawHorizontalMenu(int menu_index, int index, ...);//(int menu_index, int index, int x, int y, int max_x, int max_y, char *string);
extern void GUI_ErrorHandler(int number, ...);
extern void GUI_PrintInfoBox(char *string);
extern void GUI_PrintErrorBox(char *string);

extern unsigned int GUI_ScreenWidth;
extern unsigned int GUI_ScreenHeight;
extern OPM_Struct GUI_ScreenOpm;

extern int GUI_ProgressBarStatusFlag;
extern int GUI_ProgressBarMaxLength;
extern int GUI_ProgressBarCurrentLength;

extern const char** GUI_StringData[4];

#endif /* GUI_H */