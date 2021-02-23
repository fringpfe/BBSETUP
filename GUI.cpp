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
 * Functions handling the graphical user interface (GUI).
 *************************************************************************/

#include "GUI.h"
#include "DSA.h"
#include "SYSTEM.h"
#include "BLEV.h"
#include "FILE.h"
#include "INI.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <conio.h>
#include <dos.h>
#include <stdarg.h>

const char *GUI_StringData_German[] =
{
    "Keine VGA-Karte vorhanden.",
    "Setup erlaubt maximal einen Parameter.",
    "Kann Scriptfile nicht öffnen:%s.",
    "Kann Script nicht lesen:%s.",
    "Nicht genug erweiterter Speicherplatz.",
    "Falsche Speicherplatzanforderung.",
    "Unbekanntes Token in Zeile %li:%s.",
    "Sprung zu unbekanntem Label in Zeile %li:%s.",
    "Fehler bei DSA_InitSystem.",
    "Kann Bild nicht laden:%s",
    "Token wurde nicht eingebaut in Zeile %li:%s.",
    "Offener String in Zeile %li:%s.",
    "Falsche Parameter-Anzahl in Zeile %li:%s.",
    "Kann nicht nach Laufwerk %c wechseln.",
    "Kann nicht in Verzeichnis %s wechseln.",
    "Kann copy-Datei nicht öffnen: %s.",
    "Kann %s nicht schreiben. Speicherplatz?",
    "Kann Datei nicht umbenennen in Zeile %i:%s",
    "Kann leere Datei %s nicht erzeugen.",
    "Integer erwartet in Zeile %li.",
    "Falsche Sprache angegeben in Zeile %li:%s.",
    "ELSE ohne IF in Zeile %li:%s.",
    "ENDIF ohne IF in Zeile %li:%s.",
    "Zweites ELSE in Zeile %li gefunden:%s",
    "EOF statt ENDIF gefunden.",
    "Konnte Verzeichnis %s nicht anlegen.",
    "<ERROR>-Text zu lang",
    "Kein Menü definiert in Zeile %li:%s",
    "Zu viele Menüeinträge in Zeile %li:%s",
    "Kann %s nicht anzeigen.",
    "Syntax Error in Zeile %li:%s",
    "Fehler beim Dateizugriff",
    "Kein CD-Laufwerk installiert!",
    "Menü ohne Optionen in Zeile %li:%s",
    "Ja",
    "Nein",
    "O.K.",
    "Laufwerk",
    "Beenden",
    "Dateien werden kopiert...",
    "Seite %li/%li",
    "Abbrechen",
    "Hinweis",
    "Frage",
    "Fehler!",
    "Bitte wählen Sie ein Ziellaufwerk:",
    "Bitte wählen Sie Ihr CD-Laufwerk:",
    "Sie brauchen mindestens %liKb Festplattenspeicher zur Installation!"
    "Bitte Zielpfad angeben:",
    "Laufwerk %c ist schreibgeschützt. Installation wird abgebrochen.",
    "In Laufwerk %c kann kein Datenträger gefunden werden. Installation wird abgebrochen.",
    "Fehler beim Zugriff auf Laufwerk %c. Installation wird abgebrochen.",
    "Wenn es sich bei dem Daten~träger um eine CD handelt, reinigen Sie diese bitte vor~sichtig von möglichen Finger~ab~drücken oder anderen Ver~un~reinigungen."
};

const char *GUI_StringData_English[] =
{
    "No VGA-card detected.",
    "Setup allows one command-line parameter only.",
    "Cannot open scriptfile:%s.",
    "Cannot read script:%s.",
    "Not enough memory.",
    "Invalid memory requirement.",
    "Unknown token in line %li:%s.",
    "Jump to unknown label in line %li:%s.",
    "Error at DSA_InitSystem.",
    "Cannot load picture:%s",
    "Token has not been implemented in line %li:%s.",
    "Open string in line %li:%s.",
    "Invalid number of parameters in line %li:%s.",
    "Cannot change to drive %c.",
    "Canot change to directory %s.",
    "Cannot open copy-file: %s.",
    "Cannot write %s . Capacity?",
    "Cannot rename file in line %i:%s",
    "Cannot create empty file %s.",
    "Integer expected in line %li.",
    "Invalid language entry in line %li:%s.",
    "ELSE without IF in line %li:%s.",
    "ENDIF without IF in line %li:%s.",
    "Found second ELSE in line %li :%s",
    "EOF found instead of ENDIF.",
    "Could not create directory %s.",
    "<ERROR>-text too long.",
    "No menu defined in line %li:%s.",
    "Too many menu entries in line %li:%s",
    "Cannot display %s.",
    "Syntax error in line %li:%s.",
    "Read error.",
    "No CD-ROM drive could be found!",
    "Menu without options in line %li:%s.",
    "Yes",
    "No",
    "O.K.",
    "Drive",
    "Done",
    "Copying files...",
    "Page %li/%li",
    "Cancel",
    "Note",
    "Question",
    "Error!",
    "Select the target drive:",
    "Select your CD-ROM drive:",
    "You need %liKb of free disk space to install the game!",
    "Please enter target path:",
    "Drive %c is write-protected. Installation aborted.",
    "No disk found in drive %c . Installation aborted.",
    "Cannot read from drive %c. Installation aborted.",
    "If you are installing from a CD, please carefully remove fingerprints and other stains."
};

const char *GUI_StringData_French[] =
{
    "Carte VGA non détectée.",
    "Setup n'admet qu'un seul paramètre.",
    "Ouverture du fichier script %s impossible.",
    "Impossible de lire le script %s.",
    "Mémoire insuffisante.",
    "Exigence mémoire non valide.",
    "Token inconnu à la ligne %li:%s.",
    "Saut à un label inconnu à la ligne %li:%s.",
    "Error at DSA_InitSystem.",
    "Impossible de charger l'image :%s",
    "Token non implémenté à la ligne %li:%s.",
    "Chaîne de caractères ouverte à la ligne %li:%s.",
    "Nombre de paramètres incorrect à la ligne %li:%s.",
    "Impossible d'accéder au lecteur %c.",
    "Impossible d'accéder au répertoire %s.",
    "Ouverture impossible du fichier de copie : %s.",
    "Impossible d'écrire %s . Disque plein?",
    "Impossible de renommer le fichier à la ligne %i:%s",
    "Impossible de créer le fichier vide %s.",
    "Nombre entier inattendu à la ligne %li.",
    "Paramètre de langage non valide à la ligne %li:%s.",
    "ELSE without IF in line %li:%s.",
    "ENDIF without IF in line %li:%s.",
    "Found second ELSE in line %li :%s",
    "EOF found instead of ENDIF.",
    "Création du répertoire %s impossible.",
    "<ERREUR>-texte trop long.",
    "Aucun menu défini à la ligne %li:%s.",
    "Trop d'entrées de menu à la ligne %li:%s"
    "Impossible d'afficher %s.",
    "Erreur de syntaxe à la ligne %li:%s.",
    "Erreur de lecture.",
    "Lecteur de CD-ROM non trouvé!",
    "Menu without options in line %li:%s.",
    "Oui",
    "Non",
    "O.K.",
    "Lecteur",
    "Terminé",
    "Copie des fichiers en cours...",
    "Page %li/%li",
    "Annuler",
    "Remarque",
    "Question"
    "Erreur!",
    "Choisissez le lecteur cible: ",
    "Sélectionnez votre lecteur de CD-ROM: ",
    "%li Ko d'espace disponible requis pour installer le jeu!",
    "Entrez le chemin de destination: ",
    "Le lecteur %c est protégé en écriture. Installation annulée.",
    "Unité %c non trouvée. Installation annulée.",
    "Impossible de lire sur le lecteur %c. Installation annulée.",
    "Si vous installez le jeu à partir du CD, essuyez délicatement les empreintes digitales et la poussière."
};

const char *GUI_StringData_Spanish[] =
{
    "No se puede encontrar tarjeta VGA.",
    "Setup solamente permite un parametro en la línea de comando.",
    "No se puede abrir el fichero de guión:%s.",
    "No se puede leer el guión:%s.",
    "Insuficiente memoria expandida.",
    "Petición de memoria no válida.",
    "Identificador desconocido en línea %li:%s.",
    "Salto a etiqueta desconocida en línea %li:%s.",
    "Error en procedimiento DSA_InitSystem.",
    "No se puede cargar la imágen:%s",
    "Identificador no ha sido implementado en línea %li:%s.",
    "Cadena abierta en línea %li:%s.",
    "Número incorrecto de parámetros en línea %li:%s.",
    "No se puede cambiar a unidad %c.",
    "No se puede cambiar a directorio %s.",
    "No se puede abrir el fichero de copia: %s.",
    "No se puede escribir %s. Memoria?",
    "No se puede renombrar fichero en línea %i:%s",
    "No se puede crear fichero vacío %s.",
    "Se espera un tipo Integer en línea %li.",
    "Idioma equivocado en línea %li:%s.",
    "ELSE sin IF en línea %li:%s.",
    "ENDIF sin IF en línea %li:%s.",
    "Segundo ELSE encontrado en línea %li:%s",
    "EOF encontrado en lugar de ENDIF.",
    "No se puede crear directorio %s.",
    "<ERROR>-texto demasiado largo",
    "Menú no definido en línea %li:%s",
    "Demasiadas entradas de menú en línea %li:%s",
    "%s no se puede mostrar.",
    "Error de sintáxis en línea %li:%s",
    "Error en el acceso a fichero",
    "Unidad de CD-ROM no encontrado!",
    "Menú sin opciones en línea %li:%s",
    "Si",
    "No",
    "O.K.",
    "Unidad",
    "Terminar",
    "Copiando ficheros...",
    "Página %li/%li",
    "Cancelar",
    "Nota",
    "Pregunta",
    "Error!",
    "Por favor, elija la unidad de destino:",
    "Por favor, elija su unidad CD-ROM:",
    "Se necesita un mínimo de %liKb de espacio libre en el disco duro para la instalación!",
    "Por favor, introduzca el directorio de destino:",
    "Unidad %c está protegida contra escritura. Se cancela la instalación.",
    "No se encuentra ningún disco en unidad %c. Se cancela la instalación.",
    "Error de acceso en unidad %c. Se cancela la instalación.",
    "Si está instalando el juego desde un disco CD-ROM, por favor, límpielo con cuidado para quitar huellas y otra suciedad."
};

const char **GUI_StringData[4] =
{
    GUI_StringData_German,
    GUI_StringData_English,
    GUI_StringData_French,
    GUI_StringData_Spanish
};

GUI_ColorMapStruct GUI_ColorMap[17] =
{
    {0xF0, 0x11, 0x3A, 0x00},
    {0xF1, 0x22, 0x4B, 0x00},
    {0xF2, 0x2D, 0x00, 0x00},
    {0xF3, 0x43, 0x06, 0x00},
    {0xF4, 0x54, 0x17, 0x00},
    {0xF5, 0x64, 0x29, 0x00},
    {0xF6, 0x00, 0x0D, 0x2A},
    {0xF7, 0x00, 0x1C, 0x3B},
    {0xF8, 0x09, 0x2D, 0x49},
    {0xF9, 0x3F, 0x48, 0x64},
    {0xFA, 0x10, 0x10, 0x10},
    {0xFB, 0x21, 0x21, 0x21},
    {0xFC, 0x32, 0x32, 0x32},
    {0xFD, 0x43, 0x43, 0x43},
    {0xFE, 0x56, 0x56, 0x56},
    {0xFF, 0x64, 0x64, 0x64},
    {0xFFFF, 0x00, 0x00, 0x00}
};

unsigned int GUI_OpmWidth = 320;
unsigned int GUI_OpmHeight = 200;

unsigned int GUI_ScreenWidth = 320;
unsigned int GUI_ScreenHeight = 200;
OPM_Struct GUI_ScreenOpm;
OPM_Struct GUI_TextBoxPixelMap;
OPM_Struct GUI_ProgressBarPixelMap;

int GUI_ProgressBarStatusFlag;
int GUI_ProgressBarMaxLength;
int GUI_ProgressBarCurrentLength;

bool GUI_TextBoxFlag;
int GUI_EventFlags;
int GUI_EventFlagsOld;

int GUI_MouseEvent_CurrX;
int GUI_MouseEvent_CurrY;

char GUI_TargetPathBuffer[200];
char GUI_ErrorBuffer[400];

unsigned char GUI_DriveNumber = 0; /* TODO: Purpose unclear */

void GUI_DrawMessageBox(char *text, char *heading, char *button, unsigned char color0, unsigned char color1, unsigned char color2, unsigned char color3, unsigned char color4, unsigned char color5, int color6);

void GUI_DrawArrow(OPM_Struct *pixel_map, int x, int y, char color, int direction)
{
    OPM_HorLine(pixel_map, (x - 1), (y - 4 * direction), 3, color);
    OPM_HorLine(pixel_map, (x - 1), (y - 3 * direction), 3, color);
    OPM_HorLine(pixel_map, (x - 1), (y - 2 * direction), 3, color);
    OPM_HorLine(pixel_map, (x - 1), (y - direction), 3, color);
    OPM_HorLine(pixel_map, (x - 4), y, 9, color);
    OPM_HorLine(pixel_map, (x - 3), (direction + y), 7, color);
    OPM_HorLine(pixel_map, (x - 2), (y + 2 * direction), 5, color);
    OPM_HorLine(pixel_map, (x - 1), (y + 3 * direction), 3, color);
    OPM_HorLine(pixel_map, x, (y + 4 * direction), 1, color);
}

void GUI_DrawFrame(OPM_Struct *pixel_map, int leftBoundary, int upperBoundary, int rightBoundary, int lowerBoundary, unsigned char color)
{
    if (rightBoundary >= leftBoundary && lowerBoundary >= upperBoundary)
    {
        OPM_HorLine(pixel_map, leftBoundary, upperBoundary, (rightBoundary - leftBoundary + 1), color);
        OPM_VerLine(pixel_map, leftBoundary, upperBoundary, (lowerBoundary - upperBoundary + 1), color);
        OPM_HorLine(pixel_map, leftBoundary, lowerBoundary, (rightBoundary - leftBoundary + 1), color);
        OPM_VerLine(pixel_map, rightBoundary, upperBoundary, (lowerBoundary - upperBoundary + 1), color);
    }
}

void GUI_DrawEmbossedArea(OPM_Struct *pixel_map, int x, int y, int width, int height, unsigned char shadowColor, unsigned char fillColor, unsigned char highlightColor)
{
    int i;

    if (width > x && height > y)
    {
        for (i = y; i <= height; ++i)
        {
            OPM_HorLine(pixel_map, x, i, (width - x + 1), fillColor);
        }
        OPM_HorLine(pixel_map, x, y, (width - x), shadowColor);
        OPM_VerLine(pixel_map, x, y, (height - y), shadowColor);
        OPM_HorLine(pixel_map, (x + 1), height, (width - x), highlightColor);
        OPM_VerLine(pixel_map, width, (y + 1), (height - y), highlightColor);
    }
}

void GUI_DrawFilledBackground(OPM_Struct *pixel_map, int x, int y, int width, int height, unsigned char color0, unsigned char color1, unsigned char color2, unsigned char color3)
{
    if (width > x && height > y)
    {
        GUI_DrawEmbossedArea(pixel_map, x, y, width, height, color0, color1, color2);
        GUI_DrawEmbossedArea(pixel_map, x + 3, y + 3, width - 3, height - 3, color2, color1, color0);
        GUI_DrawEmbossedArea(pixel_map, x + 4, y + 4, width - 4, height - 4, color3, color3, color3);
    }
}

void GUI_DrawButton(OPM_Struct *pixel_map, int leftBndry, int upperBndry, int rightBndry, int lowerBndry, unsigned char color)
{
    GUI_DrawEmbossedArea(pixel_map, leftBndry, upperBndry, rightBndry, lowerBndry, 0xFFu, 0xFDu, 0xFBu);
    GUI_DrawFrame(pixel_map, leftBndry - 1, upperBndry - 1, rightBndry + 1, lowerBndry + 1, color);
}

void GUI_DrawDropShadow(OPM_Struct *pixel_map, int leftBndry, int upperBndry, int rightBndry, int lowerBndry)
{
    int x;
    int y;

    if (rightBndry > leftBndry && lowerBndry > upperBndry)
    {
        for (y = upperBndry; y <= lowerBndry; ++y)
        {
            for (x = leftBndry; x <= rightBndry; ++x)
            {
                if ((y + 3 > lowerBndry || x + 3 > rightBndry) && x + 3 < GUI_OpmWidth && y + 3 < GUI_OpmHeight)
                {
                    OPM_SetPixel(pixel_map, (x + 3), (y + 3), 0);
                }
            }
        }
    }
}

void GUI_DrawLoweredButton(OPM_Struct *pixel_map, int x_start, int y_start, int current_x, int y_origin)
{
    char color;
    int x;
    int y;

    for (y = y_origin; y >= y_start; --y)
    {
        for (x = current_x; x >= x_start; --x)
        {
            if (x != x_start && y != y_start && x_start + 1 != x && y_start + 1 != y)
            {
                if (x_start + 2 != x && y_start + 2 != y)
                {
                    color = OPM_GetPixel(pixel_map, (x - 2), (y - 2));
                    OPM_SetPixel(pixel_map, x, y, color);
                }
                else
                {
                    OPM_SetPixel(pixel_map, x, y, 0xFD);
                }
            }
            else
            {
                OPM_SetPixel(pixel_map, x, y, 0xFB);
            }
        }
    }
}

void GUI_DrawBox(OPM_Struct *pixel_map, int x, int y, int x_max, int y_max)
{
    char pixelVal;
    int x_cnt;
    int y_cnt;

    for (y_cnt = y; y_cnt <= y_max; ++y_cnt)
    {
        for (x_cnt = x; x_cnt <= x_max; ++x_cnt)
        {
            if (x_cnt != x_max && y_cnt != y_max && x_max - 1 != x_cnt && y_max - 1 != y_cnt)
            {
                pixelVal = OPM_GetPixel(pixel_map, (x_cnt + 2), (y_cnt + 2));
                OPM_SetPixel(pixel_map, x_cnt, y_cnt, pixelVal);
            }
            else
            {
                OPM_SetPixel(pixel_map, x_cnt, y_cnt, 0xFD);
            }
        }
    }
    OPM_HorLine(pixel_map, x, y, (x_max - x), 0xFFu);
    OPM_VerLine(pixel_map, x, y, (y_max - y), 0xFFu);
    OPM_HorLine(pixel_map, (x + 1), y_max, (x_max - x), 0xFBu);
    OPM_VerLine(pixel_map, x_max, (y + 1), (y_max - y), 0xFBu);
}

void GUI_SetPal(void)
{
    unsigned int palIndex;

    for (int i = 0;; i++)
    {
        palIndex = GUI_ColorMap[i].index;
        if (palIndex == 0xFFFF)
        {
            break;
        }
        DSA_SetPalEntry(palIndex,
                        255 * (unsigned char)GUI_ColorMap[i].red / 100,
                        255 * (unsigned char)GUI_ColorMap[i].green / 100,
                        255 * (unsigned char)GUI_ColorMap[i].blue / 100);
    }
    DSA_ActivatePal();
}

static void GUI_WriteChunk(unsigned char *input, unsigned char *chunk)
{
    unsigned char *buffer;
    
    for (int i = 0; i < 4; i++)
    {
        buffer = input++;
        *buffer = chunk[i];
    }
}

void GUI_CreateMouseCursor(unsigned short width, unsigned short height, short x, short y, unsigned char *bitmap)
{
    unsigned int size;
    SYSTEM_MouseCursorStruct *mouse_buffer;
    
    size = height * width;
    mouse_buffer = (SYSTEM_MouseCursorStruct *)malloc(size + 30);
    
    GUI_WriteChunk(mouse_buffer->info_str, (unsigned char *)"INFO");
    mouse_buffer->info_data[0] = 0;
    mouse_buffer->transparent_color = 0x00;
    /* mouse_buffer->field_D = 0xFF; */ /* FIXME: Purpose not clear */
    mouse_buffer->position_x = -x;
    mouse_buffer->position_y = -y;
    mouse_buffer->width = width;
    mouse_buffer->height = height;
    GUI_WriteChunk(mouse_buffer->body_str, (unsigned char *)"BODY");
    memcpy(mouse_buffer->body_data, bitmap, size);
    GUI_WriteChunk((unsigned char *)&(mouse_buffer->body_data[size]), (unsigned char *)"ENDE");
    SYSTEM_DrawMousePtr();
    SYSTEM_ShowMousePtr(mouse_buffer);
}

int GUI_AdvanceTextCursor(char c)
{
    int retVal;
    
    if (c)
    {
        retVal = 8; /* FIXME: Set automatic width for font */
    }
    else
    {
        retVal = 0;
    }
    return retVal;
}

int GUI_GetTextLength(char *string)
{
    int i;
    int string_length;
    int text_length;
    
    string_length = strlen(string);
    text_length = 0;
    
    for (i = 0; i < string_length; ++i)
    {
        text_length += GUI_AdvanceTextCursor(string[i]);
    }
    return text_length;
}

bool GUI_PrintText(char *string, int color, OPM_Struct *pixel_map, int x, int y, int max_x, int max_y)
{
    int v8;
    int v10;
    int string_pixel_length;
    char string_buffer[256];
    bool v20;
    int current_x;
    int i;
    int j;
    unsigned int string_length;
    bool v26;
    bool v27;
    
    if (x < max_x && y + 8 < max_y)
    {
        current_x = x; /* Set current x coordinate to passed value for x */
        string_length = strlen(string);
        string_buffer[0] = 0;
        v26 = 0;
        v27 = 0;
        for (i = 0;; ++i)
        {
            if ((string_length + 1) <= i)
            {
                return 1;
            }
            
            if (string[i] == ' ' || string[i] == '~' || !string[i] || string[i] == '\n')
            {
                /* Perform hyphenation at ~ symbol, if necessary */
                /* TODO */
                v27 = v26;
                if (string[i] == '~')
                {
                    v26 = 1;
                }
                else
                {
                    v26 = 0;
                }
                v8 = GUI_AdvanceTextCursor('-') * v26;
                string_pixel_length = GUI_GetTextLength(string_buffer);
                if (string_pixel_length + v8 > max_x - x - 1)
                {
                    return 0;
                }
                
                if (current_x != x && !v27)
                {
                    v20 = 1;
                }
                else
                {
                    v20 = 0;
                }
                v10 = GUI_AdvanceTextCursor('-') * v20;
                string_pixel_length = GUI_GetTextLength(string_buffer);
                if (string_pixel_length + v10 > max_x - current_x - 1)
                {
                    if (v27)
                    {
                        OPM_DrawString(pixel_map, "-", current_x, y, color);
                    }
                    /* Shift to next line */
                    y += 8;
                    current_x = x;
                    if (max_y - 8 <= y)
                    {
                        return 0;
                    }
                }
                
                /* Print Space */
                if (current_x != x && !v27)
                {
                    OPM_DrawString(pixel_map, " ", current_x, y, color);
                    current_x += GUI_AdvanceTextCursor(' ');
                }
                
                /* Replace _ with Space */
                for (j = 0; j <= strlen(string_buffer); ++j)
                {
                    if (string_buffer[j] == '_')
                    {
                        string_buffer[j] = ' ';
                    }
                }
                
                OPM_DrawString(pixel_map, string_buffer, current_x, y, color);
                current_x += GUI_GetTextLength(string_buffer);
                if (string[i] == '\n')
                {
                    /* Shift to next line */
                    y += 8;
                    current_x = x;
                    if (max_y - 8 <= y)
                    {
                        return 0;
                    }
                }
                string_buffer[0] = 0;
            }
            else
            {
                string_buffer[strlen(string_buffer) + 1] = 0;
                string_buffer[strlen(string_buffer)] = string[i];
            }
        }
    }
    return 0;
}

bool GUI_PrintButtonText(char *string, int color, OPM_Struct *pixel_map, int x, int y, int max_x, int max_y)
{
    int button_text_length;
    
    button_text_length = GUI_GetTextLength(string);
    return GUI_PrintText(string, color, pixel_map, (max_x + x) / 2 - button_text_length / 2, y, max_x, max_y);
}

int GUI_DrawAssertBox(char *string)
{
    int src_width_loc;
    int retVal;
    OPM_Struct pixel_map;
    int i;
    
    GUI_DrawTextBox(0);
    OPM_New(GUI_ScreenWidth, GUI_ScreenHeight, 1u, &pixel_map, 0);
    OPM_CopyOPMOPM(&GUI_ScreenOpm, &pixel_map, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    
    for (i = 10; (signed int)GUI_ScreenHeight / 2 > i; ++i)
    {
        GUI_DrawFilledBackground(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10, GUI_ScreenHeight / 2 - i, GUI_ScreenWidth / 2 + 16 * i / 10, i + GUI_ScreenHeight / 2, 0xF9, 0xF8u, 0xF7, 0xF8);
        GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 4, GUI_ScreenHeight / 2 - i + 4, 16 * i / 10 + GUI_ScreenWidth / 2 - 4, GUI_ScreenHeight / 2 - i + 13, 0xF6u, 0xF6u, 0xF6u);
        GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 5, GUI_ScreenHeight / 2 - i + 14, 16 * i / 10 + GUI_ScreenWidth / 2 - 4, GUI_ScreenHeight / 2 - i + 14, 0xF9u, 0xF9u, 0xF9u);
        if (GUI_PrintButtonText((char *)GUI_StringData[SETUP_Language][43], 0xFE, &GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 6, GUI_ScreenHeight / 2 - i + 5, 16 * i / 10 + GUI_ScreenWidth / 2 - 6, GUI_ScreenHeight / 2 - i + 15)) /* "Question" */
        {
            GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 5, GUI_ScreenHeight / 2 - i + 15, 16 * i / 10 + GUI_ScreenWidth / 2 - 5, i + GUI_ScreenHeight / 2 - 34, 0xF7u, 0xF8u, 0xF9u);
            GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 6, GUI_ScreenHeight / 2 - i + 16, 16 * i / 10 + GUI_ScreenWidth / 2 - 6, i + GUI_ScreenHeight / 2 - 35, 0xFDu, 0xFDu, 0xFDu);
            if (GUI_PrintText(string, 0, &GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 7, GUI_ScreenHeight / 2 - i + 17, 16 * i / 10 + GUI_ScreenWidth / 2 - 7, i + GUI_ScreenHeight / 2 - 36))
            {
                GUI_DrawButton(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - i + 5, i + GUI_ScreenHeight / 2 - 25, GUI_ScreenWidth / 2 - 2, i + GUI_ScreenHeight / 2 - 12, 0xF6u);
                if (GUI_PrintButtonText((char *)GUI_StringData[SETUP_Language][34], 0, &GUI_ScreenOpm, GUI_ScreenWidth / 2 - i + 6, i + GUI_ScreenHeight / 2 - 22, GUI_ScreenWidth / 2 - 2, i + GUI_ScreenHeight / 2 - 12)) /* "Yes" */
                {
                    GUI_DrawButton(&GUI_ScreenOpm, GUI_ScreenWidth / 2 + 2, i + GUI_ScreenHeight / 2 - 25, i + GUI_ScreenWidth / 2 - 5, i + GUI_ScreenHeight / 2 - 12, 0xF6u);
                    if (GUI_PrintButtonText((char *)GUI_StringData[SETUP_Language][35], 0, &GUI_ScreenOpm, GUI_ScreenWidth / 2 + 2, i + GUI_ScreenHeight / 2 - 22, i + GUI_ScreenWidth / 2 - 6, i + GUI_ScreenHeight / 2 - 12)) /* "No" */
                    {
                        break;
                    }
                }
            }
        }
    }
    if (GUI_ScreenHeight / 2 <= i)
    {
        GUI_ErrorHandler(1026);
    }
    GUI_DrawDropShadow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10, GUI_ScreenHeight / 2 - i, GUI_ScreenWidth / 2 + 16 * i / 10, i + GUI_ScreenHeight / 2);
    DSA_CopyMainOPMToScreen(1);
    retVal = GUI_DrawHorizontalMenu(2, 0,
                                    GUI_ScreenWidth / 2 - i + 5, i + GUI_ScreenHeight / 2 - 25, GUI_ScreenWidth / 2 - 2, i + GUI_ScreenHeight / 2 - 12, "Jjyy",
                                    GUI_ScreenWidth / 2 + 2, i + GUI_ScreenHeight / 2 - 25, i + GUI_ScreenWidth / 2 - 5, i + GUI_ScreenHeight / 2 - 12, "Nn");
    OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    OPM_Del(&pixel_map);
    DSA_CopyMainOPMToScreen(1);
    
    return (retVal == 0);
}

int GUI_MenuLoop(SETUP_MenuStruct *menu, int idx)
{
    /* TODO Refactor decompiled code */
    OPM_Struct pixel_map;
    int i;
    int j;
    unsigned int character;
    
    int v22;
    int v23;
    
    int curr_mouse_x;
    int curr_mouse_y;
    
    v22 = 1;
    v23 = 0;
    ++GUI_MouseEvent_CurrX;
    
    if (menu->index <= 0)
    {
        return 0;
    }
    
    OPM_New(GUI_ScreenWidth, GUI_ScreenHeight, 1u, &pixel_map, 0);
    OPM_CopyOPMOPM(&GUI_ScreenOpm, &pixel_map, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    
    while (1)
    {
        GUI_EventFlagsOld = GUI_EventFlags;
        curr_mouse_x = GUI_MouseEvent_CurrX;
        curr_mouse_y = GUI_MouseEvent_CurrY;
        GUI_ProcessEvents();
        
        if (v22)
        {
            OPM_Box(&GUI_ScreenOpm, menu->entry[idx].x - 1, menu->entry[idx].y - 1, menu->entry[idx].max_x - menu->entry[idx].x + 3, menu->entry[idx].max_y - menu->entry[idx].y + 3, 0xF9u);
            OPM_Box(&GUI_ScreenOpm, menu->entry[idx].x - 2, menu->entry[idx].y - 2, menu->entry[idx].max_x - menu->entry[idx].x + 5, menu->entry[idx].max_y - menu->entry[idx].y + 5, 0xF9u);
            DSA_CopyMainOPMToScreen(1);
            v22 = 0;
        }
        
        if (!kbhit())
        {
            goto LABEL_38;
        }
        character = getch();
        if (!character)
        {
            character = getch() + 1000;
        }
        else
        {
            OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
            OPM_Del(&pixel_map);
            DSA_CopyMainOPMToScreen(1);
        }
        
        if (GUI_EventFlags & 1)
        {
            goto LABEL_38;
        }
        i = 0;
    LABEL_11:
    if (i < menu->index)
    {
        break;
    }
    if (character < 0x430)
    {
        if (character >= '\r' && (character <= '\r' || character == ' '))
        {
            OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
            OPM_Del(&pixel_map);
            DSA_CopyMainOPMToScreen(1);
            return idx;
        }
    }
    else
    {
        if (character <= 1072)
        {
            goto LABEL_35;
        }
        if (character < 0x435)
        {
            if (character != 1075)
            {
                goto LABEL_38;
            }
        LABEL_35:
            v22 = 1;
            OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
            idx = (menu->index + idx - 1) % menu->index;
            goto LABEL_38;
        }
        if (character <= 1077 || character == 1080) /* right arrow and down arrow */
        {
            v22 = 1;
            OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
            idx = (idx + 1) % menu->index;
        }
    }
    
    LABEL_38:
      /* TODO: Rework section */
#if 0
    for ( i = 0;
          i < menu->index
       && (menu->entry[i].x + 1 > GUI_MouseEvent_CurrX
        || menu->entry[i].max_x - 1 < GUI_MouseEvent_CurrX
        || menu->entry[i].y + 1 > (signed int)GUI_MouseEvent_CurrY
        || menu->entry[i].max_y - 1 < (signed int)GUI_MouseEvent_CurrY);
          ++i )
    {
      ;
    }
#endif
        
        if (idx != i && !(GUI_EventFlags & 1) && i < menu->index && (GUI_MouseEvent_CurrX != curr_mouse_x || GUI_MouseEvent_CurrY != curr_mouse_y))
        {
            idx = i;
            OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
            v22 = 1;
        }
        
        if (GUI_EventFlags & 1 && (GUI_MouseEvent_CurrX != curr_mouse_x || GUI_MouseEvent_CurrY != curr_mouse_y))
        {
            if (idx != i || v23)
            {
                if (idx != i && v23 == 1)
                {
                    GUI_DrawBox(&GUI_ScreenOpm, menu->entry[idx].x, menu->entry[idx].y, menu->entry[idx].max_x, menu->entry[idx].max_y);
                    DSA_CopyMainOPMToScreen(1);
                    v23 = 0;
                }
            }
            else
            {
                GUI_DrawLoweredButton(&GUI_ScreenOpm, menu->entry[i].x, menu->entry[i].y, menu->entry[i].max_x, menu->entry[i].max_y);
                DSA_CopyMainOPMToScreen(1);
                v23 = 1;
            }
        }
        
        if (GUI_EventFlags & 1 && !(GUI_EventFlagsOld & 1) && idx == i && !v23)
        {
            GUI_DrawLoweredButton(&GUI_ScreenOpm, menu->entry[i].x, menu->entry[i].y, menu->entry[i].max_x, menu->entry[i].max_y);
            DSA_CopyMainOPMToScreen(1);
            v23 = 1;
        }
        
        if (!(GUI_EventFlags & 1) && GUI_EventFlagsOld & 1 && idx == i && v23)
        {
            OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
            OPM_Del(&pixel_map);
            DSA_CopyMainOPMToScreen(1);
            return i;
        }
        
        if (!(GUI_EventFlags & 1) && GUI_EventFlagsOld & 1 && idx != i && !v23 && i < menu->index)
        {
            idx = i;
            OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
            v22 = 1;
        }
    }
    
    for (j = 0;; ++j)
    {
        if (strlen(menu->entry[i].key_input) <= j)
        {
            ++i;
            goto LABEL_11;
        }
        if (menu->entry[i].key_input[j] == 0xFF && menu->entry[i].key_input[j + 1] == character - 1000 || menu->entry[i].key_input[j] == character)
        {
            break;
        }
        if (menu->entry[i].key_input[j] == 0xFF)
        {
            ++j;
        }
    }
    
    OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    OPM_Del(&pixel_map);
    DSA_CopyMainOPMToScreen(1);
    return i;
}

void GUI_ProcessEventFlags(BLEV_EventStruct *event_data)
{
    /* TODO: Refactor decompiled code */
    unsigned short event_flags;
    
    SYSTEM_mouse_position_sub_21968();
    BLEV_GetEvent(event_data);
    GUI_MouseEvent_CurrX = event_data->mouse_pos_x;
    GUI_MouseEvent_CurrY = event_data->mouse_pos_y;
    
    event_flags = GUI_EventFlags & 0x11;
    if (event_data->blev_flags == 4 || event_data->blev_flags == 0x100)
    {
        event_flags |= 1u;
    }
    if (event_data->blev_flags == 8 || event_data->blev_flags == 0x200)
    {
        event_flags &= 0xFFFEu;
    }
    if (event_data->blev_flags == 0x20 || event_data->blev_flags == 0x400)
    {
        event_flags |= 0x10u;
    }
    if (event_data->blev_flags == 0x40 || event_data->blev_flags == 0x800)
    {
        event_flags &= 0xFFEFu;
    }
    GUI_EventFlags = 4 * (~(unsigned char)event_flags & 0x11 & (unsigned char)GUI_EventFlags) | 2 * (~(unsigned char)GUI_EventFlags & 0x11 & event_flags) | event_flags;
}

void GUI_ProcessEvents()
{
    BLEV_EventStruct event;
    
    do
    {
        GUI_ProcessEventFlags(&event);
    } while (event.blev_flags);
}

int GUI_DrawMenu(SETUP_MenuStruct *menu)
{
    OPM_Struct pixel_map;
    int x;
    int y;
    int i;
    int text_length;
    int y_offset;
    SETUP_MenuStruct menu_loc;
    
    menu_loc.index = 0;
    GUI_DrawTextBox(0);
    OPM_New(GUI_ScreenWidth, GUI_ScreenHeight, 1u, &pixel_map, 0);
    OPM_CopyOPMOPM(&GUI_ScreenOpm, &pixel_map, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    x = 0;
    y = 40;
    
    for (i = 0; i < menu->index; ++i)
    {
        if (menu->entry[i].ptr_entry_string && menu->entry[i].anchor_point != -1)
        {
            text_length = GUI_GetTextLength(menu->entry[i].ptr_entry_string);
            if (text_length + 54 > x)
            {
                x = GUI_GetTextLength(menu->entry[i].ptr_entry_string) + 54;
            }
            y += 16;
        }
        else if (menu->entry[i].ptr_entry_string && menu->entry[i].anchor_point == -1)
        {
            text_length = GUI_GetTextLength(menu->entry[i].ptr_entry_string);
            if (text_length + 42 > x)
            {
                x = GUI_GetTextLength(menu->entry[i].ptr_entry_string) + 42;
            }
        }
        else
        {
            y += 3;
        }
    }
    
    GUI_DrawFilledBackground(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - x / 2, GUI_ScreenHeight / 2 - y / 2, GUI_ScreenWidth / 2 - x / 2 + x, y + GUI_ScreenHeight / 2 - y / 2, 0xF9, 0xF8u, 0xF7, 0xF8);
    GUI_DrawDropShadow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - x / 2, GUI_ScreenHeight / 2 - y / 2, GUI_ScreenWidth / 2 - x / 2 + x, y + GUI_ScreenHeight / 2 - y / 2);
    GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - x / 2 + 4, GUI_ScreenHeight / 2 - y / 2 + 4, GUI_ScreenWidth / 2 - x / 2 + x - 4, GUI_ScreenHeight / 2 - y / 2 + 15, 0xF6u, 0xF6u, 0xF6u);
    GUI_DrawFrame(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - x / 2 + 5, GUI_ScreenHeight / 2 - y / 2 + 16, x + GUI_ScreenWidth / 2 - x / 2 - 4, GUI_ScreenHeight / 2 - y / 2 + 16, 0xF9u);
    
    y_offset = GUI_ScreenHeight / 2 - y / 2 + 29;
    
    for (i = 0; i < menu->index; ++i)
    {
        if (menu->entry[i].ptr_entry_string)
        {
            if (menu->entry[i].anchor_point == -1)
            {
                menu->entry[i].x = GUI_ScreenWidth / 2 - x / 2 + 4;
                menu->entry[i].y = GUI_ScreenHeight / 2 - y / 2 + 5;
                menu->entry[i].max_x = x + GUI_ScreenWidth / 2 - x / 2 - 4;
                menu->entry[i].max_y = GUI_ScreenHeight / 2 - y / 2 + 15;
                GUI_PrintButtonText(menu->entry[i].ptr_entry_string, 0xFE, &GUI_ScreenOpm, menu->entry[i].x + 1, menu->entry[i].y + 1, menu->entry[i].max_x, menu->entry[i].max_y);
            }
            else
            {
                menu->entry[i].x = GUI_ScreenWidth / 2 - x / 2 + 25;
                menu->entry[i].y = y_offset + 1;
                menu->entry[i].max_x = x + GUI_ScreenWidth / 2 - x / 2 - 25;
                menu->entry[i].max_y = y_offset + 12;
                GUI_DrawButton(&GUI_ScreenOpm, menu->entry[i].x, menu->entry[i].y, menu->entry[i].max_x, menu->entry[i].max_y, 0xF6u);
                GUI_PrintButtonText(menu->entry[i].ptr_entry_string, 0, &GUI_ScreenOpm, menu->entry[i].x, menu->entry[i].y + 2, menu->entry[i].max_x, menu->entry[i].max_y);
                y_offset += 16;
                menu_loc.entry[menu_loc.index].ptr_entry_string = 0;
                menu_loc.entry[menu_loc.index].key_input = (char *)&GUI_DriveNumber;
                menu_loc.entry[menu_loc.index].anchor_point = menu->entry[i].anchor_point;
                menu_loc.entry[menu_loc.index].x = menu->entry[i].x;
                menu_loc.entry[menu_loc.index].y = menu->entry[i].y;
                menu_loc.entry[menu_loc.index].max_x = menu->entry[i].max_x;
                menu_loc.entry[menu_loc.index++].max_y = menu->entry[i].max_y;
            }
        }
        else
        {
            y_offset += 3;
        }
    }
    
    DSA_CopyMainOPMToScreen(1);
    
    i = GUI_MenuLoop(&menu_loc, 0);
    
    OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    OPM_Del(&pixel_map);
    DSA_CopyMainOPMToScreen(1);
    
    return menu_loc.entry[i].anchor_point;
}

void GUI_DrawTextBox(char *string)
{
    int i;
    
    if (string)
    {
        if (GUI_TextBoxFlag)
        {
            GUI_DrawTextBox(0);
        }
        GUI_TextBoxFlag = 1;
        OPM_New(GUI_ScreenWidth, GUI_ScreenHeight, 1u, &GUI_TextBoxPixelMap, 0);
        OPM_CopyOPMOPM(&GUI_ScreenOpm, &GUI_TextBoxPixelMap, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
        for (i = 5; GUI_ScreenHeight / 2 > i; ++i)
        {
            GUI_DrawFilledBackground(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 5, GUI_ScreenHeight / 3 - i, GUI_ScreenWidth / 2 + 16 * i / 5, i + GUI_ScreenHeight / 3, 0xF9, 0xF8u, 0xF7, 0xF8);
            if (GUI_PrintText(string, 0, &GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 5, GUI_ScreenHeight / 3 - i + 5, 16 * i / 10 + GUI_ScreenWidth / 2 - 5, i + GUI_ScreenHeight / 3 - 4))
            {
                break;
            }
        }
        DSA_CopyMainOPMToScreen(1);
        
        if (GUI_ScreenHeight / 2 <= i)
        {
            GUI_ErrorHandler(1026);
        }
    }
    else if (GUI_TextBoxFlag)
    {
        GUI_TextBoxFlag = 0;
        OPM_CopyOPMOPM(&GUI_TextBoxPixelMap, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
        OPM_Del(&GUI_TextBoxPixelMap);
        DSA_CopyMainOPMToScreen(1);
    }
}

void GUI_ParseReadmeText(SETUP_ScriptDataStruct *text_data, char *filename)
{
    int line_length;
    unsigned int *line_ptr;
    int i;
    int j;
    int file_handle;
    int file_length;
    
    file_handle = open(filename, 0x200);
    if (file_handle <= 0)
    {
        GUI_ErrorHandler(1029, (char *)&filename);
    }
    file_length = filelength(file_handle);
    if (file_length < 0)
    {
        GUI_ErrorHandler(1029, (char *)&filename);
    }
    
    text_data->PtrScriptBuffer = (unsigned char *)malloc(file_length + 1);
    if (read(file_handle, text_data->PtrScriptBuffer, file_length) != file_length)
    {
        GUI_ErrorHandler(1003, (char *)&filename);
    }
    text_data->PtrScriptBuffer[file_length] = 0;
    close(file_handle);
    
    text_data->NumberOfLines = 0;
    for (i = 0; i < file_length; ++i)
    {
        if (text_data->PtrScriptBuffer[i] == '\r')
        {
            ++text_data->NumberOfLines;
        }
    }
    
    text_data->PtrScriptLine = (unsigned int *)malloc(4 * (text_data->NumberOfLines + 1));
    text_data->NumberOfLines = 0;
    text_data->PtrScriptLine[text_data->NumberOfLines++] = (unsigned int)text_data->PtrScriptBuffer; /* first entry is pointing to the beginning of the ini buffer */
    
    for (j = 0; j < file_length; ++j)
    {
        if (text_data->PtrScriptBuffer[j] == '\r')
        {
            text_data->PtrScriptLine[text_data->NumberOfLines] = (unsigned int)&text_data->PtrScriptBuffer[j + 1];
            text_data->PtrScriptBuffer[j] = 0;
            while (*(char *)text_data->PtrScriptLine[text_data->NumberOfLines - 1] && (*(char *)text_data->PtrScriptLine[text_data->NumberOfLines - 1] == '\n' || *(char *)text_data->PtrScriptLine[text_data->NumberOfLines - 1] == '\r'))
            {
                ++text_data->PtrScriptLine[text_data->NumberOfLines - 1];
            }
            while (strlen((const char *)text_data->PtrScriptLine[text_data->NumberOfLines - 1]))
            {
                line_ptr = &text_data->PtrScriptLine[text_data->NumberOfLines];
                line_length = strlen((const char *)*(line_ptr - 1));
                if (line_ptr[line_length - 1] != '\n')
                {
                    line_ptr = &text_data->PtrScriptLine[text_data->NumberOfLines];
                    line_length = strlen((const char *)*(line_ptr - 1));
                    if (line_ptr[line_length - 1] != '\r')
                    {
                        line_ptr = &text_data->PtrScriptLine[text_data->NumberOfLines];
                        line_length = strlen((const char *)*(line_ptr - 1));
                        if (line_ptr[line_length - 1] != 0x1A)
                        {
                            break;
                        }
                    }
                }
                line_ptr = &text_data->PtrScriptLine[text_data->NumberOfLines];
                line_length = strlen((const char *)*(line_ptr - 1));
                line_ptr[line_length - 1] = 0;
            }
            ++text_data->NumberOfLines;
        }
    }
    --text_data->NumberOfLines;
}

void GUI_DrawReadmeText(char *text)
{
    /* TODO: Refactor decompiled code */
    int v1;
    char buffer[80];
    OPM_Struct pixel_map;
    OPM_Struct pixel_map_gui; /* FIXME: Workaround for menu persistence */
    SETUP_ScriptDataStruct pText;
    int i;
    int v7;
    int line_number;
    
    GUI_ParseReadmeText(&pText, text);
    OPM_New(GUI_ScreenWidth, GUI_ScreenHeight, 1u, &pixel_map, 0);
    OPM_CopyOPMOPM(&GUI_ScreenOpm, &pixel_map, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    GUI_DrawFilledBackground(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 160, GUI_ScreenHeight / 2 - 100, GUI_ScreenWidth / 2 + 159, GUI_ScreenHeight / 2 + 99, 0xF9, 0xF8u, 0xF7, 0xF8);
    GUI_DrawDropShadow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 160, GUI_ScreenHeight / 2 - 100, GUI_ScreenWidth / 2 + 159, GUI_ScreenHeight / 2 + 99);
    GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 156, GUI_ScreenHeight / 2 - 96, GUI_ScreenWidth / 2 + 155, GUI_ScreenHeight / 2 - 87, 0xF6u, 0xF6u, 0xF6u);
    GUI_DrawFrame(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 155, GUI_ScreenHeight / 2 - 86, GUI_ScreenWidth / 2 + 155, GUI_ScreenHeight / 2 - 86, 0xF9u);
    GUI_PrintButtonText(text, 254, &GUI_ScreenOpm, GUI_ScreenWidth / 2 - 155, GUI_ScreenHeight / 2 - 95, GUI_ScreenWidth / 2 + 154, GUI_ScreenHeight / 2 - 86);
    GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 153, GUI_ScreenHeight / 2 - 85, GUI_ScreenWidth / 2 + 152, GUI_ScreenHeight / 2 + 76, 0xF7u, 0xF8u, 0xF9u);
    GUI_DrawButton(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 152, GUI_ScreenHeight / 2 + 80, GUI_ScreenWidth / 2 - 134, GUI_ScreenHeight / 2 + 92, 0xF6u);
    GUI_DrawArrow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 143, GUI_ScreenHeight / 2 + 86, 0, -1);
    GUI_DrawButton(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 130, GUI_ScreenHeight / 2 + 80, GUI_ScreenWidth / 2 - 112, GUI_ScreenHeight / 2 + 92, 0xF6u);
    GUI_DrawArrow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 121, GUI_ScreenHeight / 2 + 86, 0, 1);
    GUI_DrawButton(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 103, GUI_ScreenHeight / 2 + 80, GUI_ScreenWidth / 2 - 85, GUI_ScreenHeight / 2 + 92, 0xF6u);
    GUI_DrawArrow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 97, GUI_ScreenHeight / 2 + 86, 0, -1);
    GUI_DrawArrow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 91, GUI_ScreenHeight / 2 + 86, 0, -1);
    GUI_DrawButton(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 81, GUI_ScreenHeight / 2 + 80, GUI_ScreenWidth / 2 - 63, GUI_ScreenHeight / 2 + 92, 0xF6u);
    GUI_DrawArrow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 75, GUI_ScreenHeight / 2 + 86, 0, 1);
    GUI_DrawArrow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 69, GUI_ScreenHeight / 2 + 86, 0, 1);
    GUI_DrawButton(&GUI_ScreenOpm, GUI_ScreenWidth / 2 + 85, GUI_ScreenHeight / 2 + 80, GUI_ScreenWidth / 2 + 151, GUI_ScreenHeight / 2 + 92, 0xF6u);
    GUI_PrintButtonText((char *)GUI_StringData[SETUP_Language][38], 0, &GUI_ScreenOpm, GUI_ScreenWidth / 2 + 85, GUI_ScreenHeight / 2 + 82, GUI_ScreenWidth / 2 + 151, GUI_ScreenHeight / 2 + 92); /* "Done" */
    GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 50, GUI_ScreenHeight / 2 + 80, GUI_ScreenWidth / 2 + 65, GUI_ScreenHeight / 2 + 92, 0xF7u, 0xF8u, 0xF9u);
    
    line_number = 0;
    v7 = 0;
    OPM_CopyOPMOPM(&GUI_ScreenOpm, &pixel_map_gui, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0); /* FIXME: Workaround for menu persistence */
    do
    {
        OPM_CopyOPMOPM(&pixel_map_gui, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0); /* FIXME: Workaround for menu persistence */
        GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 49, GUI_ScreenHeight / 2 + 81, GUI_ScreenWidth / 2 + 64, GUI_ScreenHeight / 2 + 91, 0xFDu, 0xFDu, 0xFDu);
        sprintf(buffer, (char *)GUI_StringData[SETUP_Language][40], line_number / 20 + 1, (pText.NumberOfLines - 20) / 20 + 1); /* "Page %li/%li" */
        GUI_PrintButtonText(buffer, 0, &GUI_ScreenOpm, GUI_ScreenWidth / 2 - 49, GUI_ScreenHeight / 2 + 82, GUI_ScreenWidth / 2 + 64, GUI_ScreenHeight / 2 + 93);
        GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 152, GUI_ScreenHeight / 2 - 84, GUI_ScreenWidth / 2 + 151, GUI_ScreenHeight / 2 + 75, 0xFDu, 0xFDu, 0xFDu);
        for (i = 0; i < 20; ++i)
        {
            if (line_number + i >= 0 && line_number + i < pText.NumberOfLines && pText.PtrScriptLine[line_number + i])
            {
                if (strlen((const char *)pText.PtrScriptLine[line_number + i]) > 38)
                {
                    *(char *)(pText.PtrScriptLine[line_number + i] + 38) = 0;
                }
                GUI_PrintText((char *)pText.PtrScriptLine[line_number + i], 0, &GUI_ScreenOpm, GUI_ScreenWidth / 2 - 152, 8 * i + (int)GUI_ScreenHeight / 2 - 84, GUI_ScreenWidth / 2 + 154, 8 * i + 8 + (int)GUI_ScreenHeight / 2 + 75 + 3);
            }
        }
        DSA_CopyMainOPMToScreen(1);
        v1 = GUI_DrawHorizontalMenu(5, v7,
                                    GUI_ScreenWidth / 2 - 152, GUI_ScreenHeight / 2 + 80, GUI_ScreenWidth / 2 - 134, GUI_ScreenHeight / 2 + 92, 0x51,
                                    GUI_ScreenWidth / 2 - 130, GUI_ScreenHeight / 2 + 80, GUI_ScreenWidth / 2 - 112, GUI_ScreenHeight / 2 + 92, 0x49,
                                    GUI_ScreenWidth / 2 - 103, GUI_ScreenHeight / 2 + 80, GUI_ScreenWidth / 2 - 85, GUI_ScreenHeight / 2 + 92, 0x50,
                                    GUI_ScreenWidth / 2 - 81, GUI_ScreenHeight / 2 + 80, GUI_ScreenWidth / 2 - 63, GUI_ScreenHeight / 2 + 92, 0x48,
                                    GUI_ScreenWidth / 2 + 85, GUI_ScreenHeight / 2 + 80, GUI_ScreenWidth / 2 + 151, GUI_ScreenHeight / 2 + 92, 0x1B);
        v7 = v1;
        switch (v1)
        {
            case 0:
                --line_number;
                break;
            case 1:
                ++line_number;
                break;
            case 2:
                line_number -= 20;
                break;
            case 3:
                line_number += 20;
                break;
            default:
                break;
        }
        if (pText.NumberOfLines - 20 < line_number)
        {
            line_number = pText.NumberOfLines - 20;
        }
        if (line_number < 0)
        {
            line_number = 0;
        }
    } while (v7 != 4);
    OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    OPM_Del(&pixel_map);
    OPM_Del(&pixel_map_gui); /* FIXME: Workaround for menu persistence */
    DSA_CopyMainOPMToScreen(1);
}

void GUI_DrawProgressBar(int flag)
{
    unsigned int progress_bar_status;
    
    if (flag < 0)
    {
        if (flag == -1 && GUI_ProgressBarStatusFlag)
        {
            GUI_ProgressBarStatusFlag = 0;
            OPM_CopyOPMOPM(&GUI_ProgressBarPixelMap, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
            OPM_Del(&GUI_ProgressBarPixelMap);
            DSA_CopyMainOPMToScreen(1);
        }
    }
    else if (flag <= 0)
    {
        if (GUI_ProgressBarStatusFlag)
        {
            GUI_DrawFilledBackground(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 155, 2 * (GUI_ScreenHeight / 3) - 16, GUI_ScreenWidth / 2 + 155, 2 * (GUI_ScreenHeight / 3) + 16, 249, 0xF8u, 247, 248);
            GUI_DrawDropShadow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 155, 2 * (GUI_ScreenHeight / 3) - 16, GUI_ScreenWidth / 2 + 155, 2 * (GUI_ScreenHeight / 3) + 16);
            GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 151, 2 * (GUI_ScreenHeight / 3) - 12, GUI_ScreenWidth / 2 + 151, 2 * (GUI_ScreenHeight / 3) - 2, 0xF6u, 0xF6u, 0xF6u);
            GUI_DrawFrame(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 150, 2 * (GUI_ScreenHeight / 3) - 1, GUI_ScreenWidth / 2 + 151, 2 * (GUI_ScreenHeight / 3) - 1, 0xF9u);
            GUI_PrintText((char *)GUI_StringData[SETUP_Language][39], 0xFE, &GUI_ScreenOpm, GUI_ScreenWidth / 2 - 150, 2 * (GUI_ScreenHeight / 3) - 11, GUI_ScreenWidth / 2 + 150, 2 * (GUI_ScreenHeight / 3) + 1); /* "Copying files..." */
            GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 150, 2 * (GUI_ScreenHeight / 3), GUI_ScreenWidth / 2 + 150, 2 * (GUI_ScreenHeight / 3) + 11, 0xF7u, 0xF8u, 0xF9u);
            if (GUI_ProgressBarMaxLength)
            {
                progress_bar_status = GUI_ProgressBarCurrentLength;
                if (progress_bar_status > GUI_ProgressBarMaxLength)
                {
                    progress_bar_status = GUI_ProgressBarMaxLength;
                }
                GUI_DrawEmbossedArea(&GUI_ScreenOpm, (GUI_ScreenWidth / 2 - 149), (2 * (GUI_ScreenHeight / 3) + 1), 298 * progress_bar_status / GUI_ProgressBarMaxLength + (GUI_ScreenWidth / 2 - 149), 2 * (GUI_ScreenHeight / 3) + 10, 0xFFu, 0xFDu, 0xFBu);
            }
            DSA_CopyMainOPMToScreen(1);
        }
    }
    else if (flag == 1 && !GUI_ProgressBarStatusFlag)
    {
        GUI_ProgressBarStatusFlag = 1;
        OPM_New(GUI_ScreenWidth, GUI_ScreenHeight, 1u, &GUI_ProgressBarPixelMap, 0);
        OPM_CopyOPMOPM(&GUI_ScreenOpm, &GUI_ProgressBarPixelMap, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
        GUI_DrawProgressBar(0);
    }
}

int GUI_DrawDriveSelectMenu(SETUP_MenuStruct *menu)
{
    /* TODO: Refactor decompiled code */
    int v1;
    int v2;
    size_t v3;
    int v4;
    int v5;
    char *x;
    char string[40];
    SETUP_MenuStruct menu_loc;
    OPM_Struct pixel_map;
    int menu_index_loc;
    int v13;
    int v14;
    int v15;
    int v16;
    int text_length;
    int height_loc;
    int v19;
    int v20;
    int v21;
    int i;
    int upperBndry;
    int leftBndry;
    int rightBndry;
    int retVal;
    int lowerBndry;
    char v28;
    
    v20 = 5;
    v28 = 65;
    menu_loc.index = 0;
    GUI_DrawTextBox(0);
    
    OPM_New(GUI_ScreenWidth, GUI_ScreenHeight, 1u, &pixel_map, 0);
    OPM_CopyOPMOPM(&GUI_ScreenOpm, &pixel_map, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    height_loc = 40;
    
    for (i = 0; menu->index / v20 > i; ++i)
    {
        if (menu->entry[i].ptr_entry_string && menu->entry[i].anchor_point != -1)
        {
            height_loc += 19;
        }
    }
    
    height_loc += 68;
    text_length = GUI_GetTextLength(menu->entry[0].ptr_entry_string) + 42;
    GUI_DrawFilledBackground(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - text_length / 2, GUI_ScreenHeight / 2 - height_loc / 2, GUI_ScreenWidth / 2 - text_length / 2 + text_length, height_loc + GUI_ScreenHeight / 2 - height_loc / 2, 249, 0xF8u, 247, 248);
    GUI_DrawDropShadow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - text_length / 2, GUI_ScreenHeight / 2 - height_loc / 2, text_length + GUI_ScreenWidth / 2 - text_length / 2, height_loc + GUI_ScreenHeight / 2 - height_loc / 2);
    GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - text_length / 2 + 4, GUI_ScreenHeight / 2 - height_loc / 2 + 4, text_length + GUI_ScreenWidth / 2 - text_length / 2 - 4, GUI_ScreenHeight / 2 - height_loc / 2 + 15, 0xF6u, 0xF6u, 0xF6u);
    GUI_DrawFrame(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - text_length / 2 + 5, GUI_ScreenHeight / 2 - height_loc / 2 + 16, text_length + GUI_ScreenWidth / 2 - text_length / 2 - 4, GUI_ScreenHeight / 2 - height_loc / 2 + 16, 0xF9u);
    
    v19 = height_loc + GUI_ScreenHeight / 2 - height_loc / 2 - 19;
    text_length = 90;
    v14 = -90;
    v2 = GUI_ScreenHeight / 2 - height_loc / 2;
    v15 = v2 + 29;
    v16 = -90;
    v13 = v2 + 29;
    v21 = 0;
    for (i = 0; i < menu->index; ++i)
    {
        if (menu->entry[i].ptr_entry_string)
        {
            if (menu->entry[i].anchor_point == -1)
            {
                menu->entry[i].x = 0;
                menu->entry[i].y = GUI_ScreenHeight / 2 - height_loc / 2 + 5;
                menu->entry[i].max_x = GUI_ScreenWidth;
                menu->entry[i].max_y = GUI_ScreenHeight / 2 - height_loc / 2 + 15;
                GUI_PrintButtonText(menu->entry[i].ptr_entry_string, 254, &GUI_ScreenOpm, menu->entry[i].x + 1, menu->entry[i].y + 1, menu->entry[i].max_x, menu->entry[i].max_y);
            }
            else
            {
/* TODO: Figure out deactivated parts */
#if 0
                while ( v28 != *menu->entry[i].ptr_entry_string )
                {
                    leftBndry = GUI_ScreenWidth / 2 - text_length / 2 + 25 + v14;
                    upperBndry = v13 + 1;
                    rightBndry = GUI_ScreenWidth / 2 - text_length / 2 + text_length - 25 + v14;
                    lowerBndry = v13 + 12;
                    GUI_DrawButton(&GUI_ScreenOpm, leftBndry, v13 + 1, rightBndry, v13 + 12, 0xF6u);
                    string[0] = GUI_DriveNumber;
                    strcat(string, " ");
                    v3 = strlen(string);
                    *((_BYTE *)&x + v3 + 3) = v28;
                    strcat(string, ":");
                    GUI_PrintButtonText(string, 254, &GUI_ScreenOpm, leftBndry, upperBndry + 2, rightBndry, lowerBndry);
                    ++v28;
                    v14 += 45;
                    if ( v21 == v20 )
                    {
                        v13 += 16;
                        v21 = 0;
                        v14 = v16;
                    }
                    ++v21;
                }
#endif      
                leftBndry = GUI_ScreenWidth / 2 - text_length / 2 + 25 + v14;
                upperBndry = v13 + 1;
                rightBndry = GUI_ScreenWidth / 2 - text_length / 2 + text_length - 25 + v14;
                lowerBndry = v13 + 12;
                menu->entry[i].x = leftBndry;
                menu->entry[i].y = upperBndry;
                menu->entry[i].max_x = rightBndry;
                menu->entry[i].max_y = lowerBndry;
                GUI_DrawButton(&GUI_ScreenOpm, leftBndry, upperBndry, rightBndry, lowerBndry, 0xF6u);
                GUI_PrintButtonText(menu->entry[i].ptr_entry_string, 0, &GUI_ScreenOpm, leftBndry, upperBndry + 2, rightBndry, lowerBndry);
                menu_loc.entry[menu_loc.index].ptr_entry_string = 0;
                menu_loc.entry[menu_loc.index].key_input = (char *)&GUI_DriveNumber;
                menu_loc.entry[menu_loc.index].anchor_point = menu->entry[i].anchor_point;
                menu_loc.entry[menu_loc.index].x = menu->entry[i].x;
                menu_loc.entry[menu_loc.index].y = menu->entry[i].y;
                menu_loc.entry[menu_loc.index].max_x = menu->entry[i].max_x;
                menu_loc.entry[menu_loc.index++].max_y = menu->entry[i].max_y;
                v14 += 45;
                ++v28;
            }
        }
        if (v21 == v20)
        {
            v13 += 16;
            v21 = 0;
            v14 = v16;
        }
        ++v21;
    }
    x = (char *)((signed int)GUI_ScreenWidth / 2 - text_length / 2);
    v4 = v19;
    
    GUI_DrawButton(&GUI_ScreenOpm, (int)x, v19, (int)(x + 80), v19 + 11, 0xF6u);
    GUI_PrintButtonText((char *)GUI_StringData[SETUP_Language][41], 0, &GUI_ScreenOpm, (int)x, v4 + 2, (int)(x + 80), v4 + 11); /* "Cancel" */
    
    menu_loc.entry[menu_loc.index].ptr_entry_string = 0;
    menu_loc.entry[menu_loc.index].key_input = (char *)&GUI_DriveNumber;
    menu_loc.entry[menu_loc.index].anchor_point = menu->entry[i].anchor_point;
    menu_loc.entry[menu_loc.index].x = (int)x;
    menu_loc.entry[menu_loc.index].y = v4;
    menu_loc.entry[menu_loc.index].max_x = (int)(x + 80);
    menu_loc.entry[menu_loc.index].max_y = v4 + 11;
    menu_index_loc = menu_loc.index++;
    DSA_CopyMainOPMToScreen(1);
    i = GUI_MenuLoop(&menu_loc, 0);
    
    OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    OPM_Del(&pixel_map);
    DSA_CopyMainOPMToScreen(1);
    
    if (i == menu_index_loc)
    {
        retVal = -1;
    }
    else
    {
        retVal = menu_loc.entry[i].anchor_point;
    }
    return retVal;
}

signed int GUI_WriteIniEntry_Cdrom(void)
{
    int targetdrive_ret;
    SETUP_MenuStruct menu_loc;
    char buf[12];
    struct diskfree_t diskspace;
    unsigned int drive;
    int max_drive_number;
    
    menu_loc.entry[0].ptr_entry_string = (char *)GUI_StringData[SETUP_Language][46]; /* "Select your CD-ROM drive:" */
    menu_loc.entry[menu_loc.index].key_input = (char *)&GUI_DriveNumber;
    menu_loc.entry[0].anchor_point = -1;
    menu_loc.index = 1;
    max_drive_number = FILE_GetMaxDriveNumber();
    for (drive = 4; (signed int)drive <= 26 && (signed int)drive <= max_drive_number; ++drive)
    {
        if (!_dos_getdiskfree(drive, &diskspace))
        {
            menu_loc.entry[menu_loc.index].ptr_entry_string = (char *)malloc(0x50u);
            menu_loc.entry[menu_loc.index].key_input = (char *)malloc(2u);
            menu_loc.entry[menu_loc.index].anchor_point = drive + 64;
            sprintf(menu_loc.entry[menu_loc.index].ptr_entry_string, "%c:", drive + 64);
            sprintf(menu_loc.entry[menu_loc.index].key_input, "%c", drive + 64);
            ++menu_loc.index;
        }
    }
    SETUP_CriticalErrorFlag = 0;
    if (menu_loc.index == 1)
    {
        GUI_ErrorHandler(1032);
    }
    
    targetdrive_ret = GUI_DrawDriveSelectMenu(&menu_loc);
    if (targetdrive_ret == -1)
    {
        return 0;
    }
    SETUP_CdDrive = targetdrive_ret;
    INI_MakePath();
    if (!FILE_IsFileExisting((const char *)&INI_WriteBuffer) || !SETUP_CdDrive)
    {
        return 1;
    }
    sprintf((char *)&buf, "%c", SETUP_CdDrive);
    INI_WriteEntry("SYSTEM", "CD_ROM_DRIVE", (char *)&buf);
    return 1;
}

int GUI_DrawTargetDriveMenu(int required_free_space)
{
    int targetDriveVal;
    char buf[252];
    SETUP_MenuStruct menu_loc;
    
    unsigned int free_space;
    diskfree_t free_diskspace;
    int actual_free_space;
    
    signed int driveNumber;
    int retVal;
    
    GUI_ProgressBarMaxLength = required_free_space;
    menu_loc.entry[0].ptr_entry_string = (char *)GUI_StringData[SETUP_Language][45]; /* "Select the target drive:" */
    menu_loc.entry[menu_loc.index].key_input = (char *)&GUI_DriveNumber;
    menu_loc.entry[0].anchor_point = -1;
    menu_loc.index = 1;
    
    for (driveNumber = 3; driveNumber <= 26; ++driveNumber)
    {
        if (FILE_IsDriveNumberValid(driveNumber) && !_dos_getdiskfree(driveNumber, &free_diskspace))
        {
            free_space = (unsigned int)(free_diskspace.bytes_per_sector * free_diskspace.avail_clusters * free_diskspace.sectors_per_cluster);
            if (free_space > required_free_space)
            {
                menu_loc.entry[menu_loc.index].ptr_entry_string = (char *)malloc(80u);
                menu_loc.entry[menu_loc.index].key_input = (char *)malloc(2u);
                menu_loc.entry[menu_loc.index].anchor_point = driveNumber + 0x40;
                sprintf(menu_loc.entry[menu_loc.index].ptr_entry_string, "%c:", driveNumber + 0x40);
                sprintf(menu_loc.entry[menu_loc.index].key_input, "%c", driveNumber + 0x40);
                ++menu_loc.index;
            }
        }
    }
    if (menu_loc.index == 1)
    {
        sprintf((char *)&buf, (char *)GUI_StringData[SETUP_Language][47], required_free_space); /* "You need %liKb of free disk space to install the game!" */
        GUI_PrintErrorBox((char *)&buf);
        retVal = 0;
    }
    else
    {
        targetDriveVal = GUI_DrawDriveSelectMenu(&menu_loc);
        if (targetDriveVal == -1)
        {
            retVal = 0;
        }
        else
        {
            SETUP_TargetDrive = targetDriveVal;
            retVal = 1;
        }
    }
    return retVal;
}

char *GUI_DrawTargetPathMenu(char *headerString, char *targetPath)
{
    /* TODO: Refactor decompiled code */
    int src_width_loc;
    size_t strLen;
    int src_width_loc1;
    OPM_Struct pixel_map;
    int height;
    int v9;
    int getKeyPressed;
    int v11;
    int v13;
    int v14;
    
    v9 = 0;
    v14 = 0;
    GUI_DrawTextBox(0);
    
    OPM_New(GUI_ScreenWidth, GUI_ScreenHeight, 1u, &pixel_map, 0);
    OPM_CopyOPMOPM(&GUI_ScreenOpm, &pixel_map, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    
    sprintf((char *)&GUI_TargetPathBuffer, "%c:\\%s", SETUP_TargetDrive, targetPath);
    height = 16;
    while (1)
    {
        GUI_DrawFilledBackground(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 155, 2 * (GUI_ScreenHeight / 3) - height, GUI_ScreenWidth / 2 + 155, height + 2 * (GUI_ScreenHeight / 3), 249, 0xF8u, 247, 248);
        GUI_DrawDropShadow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 155, 2 * (GUI_ScreenHeight / 3) - height, GUI_ScreenWidth / 2 + 155, height + 2 * (GUI_ScreenHeight / 3));
        GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 151, 2 * (GUI_ScreenHeight / 3) - height + 4, GUI_ScreenWidth / 2 + 151, 2 * (GUI_ScreenHeight / 3) - height + 14, 0xF6u, 0xF6u, 0xF6u);
        GUI_DrawFrame(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 150, 2 * (GUI_ScreenHeight / 3) - height + 15, GUI_ScreenWidth / 2 + 151, 2 * (GUI_ScreenHeight / 3) - height + 15, 0xF9u);
        GUI_PrintText(headerString, 0xFE, &GUI_ScreenOpm, GUI_ScreenWidth / 2 - 150, 2 * (GUI_ScreenHeight / 3) - height + 5, GUI_ScreenWidth / 2 + 150, 2 * (GUI_ScreenHeight / 3) - height + 17);
        GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 150, 2 * (GUI_ScreenHeight / 3) - height + 16, GUI_ScreenWidth / 2 + 150, height + 2 * (GUI_ScreenHeight / 3) - 5, 0xF7u, 0xF8u, 0xF9u);
        GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 149, 2 * (GUI_ScreenHeight / 3) - height + 17, GUI_ScreenWidth / 2 + 149, height + 2 * (GUI_ScreenHeight / 3) - 6, 0xFDu, 0xFDu, 0xFDu);
        OPM_DrawString(&GUI_ScreenOpm, (char *)&GUI_TargetPathBuffer, GUI_ScreenWidth / 2 - 149, 2 * (GUI_ScreenHeight / 3) - height + 18, 0);
        DSA_CopyMainOPMToScreen(1);
        GUI_ProcessEvents();
        
/* FIXME: Incomprehensible decompilation; figure out purpose */
#if 0
        v13 = 0x46C;
        if ( MEMORY[0x46C] - v9 > 10 )
        {
            v11 = 1132;
            v9 = MEMORY[0x46C];
            v14 ^= 1u;
            if ( (_WORD)v14 )
            {
                strcat(GUI_TargetPathBuffer, asc_71EAA);
            }
            else
            {
                byte_749DF[strlen(GUI_TargetPathBuffer)] = 0;
            }
        }
#endif
        if (kbhit())
        {
#if 0
            if ( (_WORD)v14 )
            {
                v14 = 0;
                byte_749DF[strlen(GUI_TargetPathBuffer)] = 0;
            }
#endif
            getKeyPressed = getch();
            if (!getKeyPressed)
            {
                getch();
            }
            if (getKeyPressed == '\b' && strlen(GUI_TargetPathBuffer) > 3)
            {
                GUI_TargetPathBuffer[strlen(GUI_TargetPathBuffer) - 1] = 0;
            }
            if (getKeyPressed >= 'a' && getKeyPressed <= 'z')
            {
                getKeyPressed -= 0x20;
            }
            if ((getKeyPressed >= 'A' && getKeyPressed <= 'Z' || getKeyPressed >= '0' && getKeyPressed <= '9' || getKeyPressed == '\\' || getKeyPressed == '_' || getKeyPressed == '.') && strlen(GUI_TargetPathBuffer) < 35)
            {
                strLen = strlen(GUI_TargetPathBuffer);
                GUI_TargetPathBuffer[strLen] = getKeyPressed;
                GUI_TargetPathBuffer[strlen(GUI_TargetPathBuffer) + 1] = 0;
            }
            if (getKeyPressed == '\r')
            {
                break;
            }
        }
    }
    
    OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    OPM_Del(&pixel_map);
    DSA_CopyMainOPMToScreen(1);
    
    return (char *)&GUI_TargetPathBuffer;
}

int GUI_DrawHorizontalMenu(int menu_index, int index, ...)
{
    int result;
    SETUP_MenuStruct menu;
    int i;
    va_list va;
    
    va_start(va, index);
    menu.index = menu_index;
    
    for (i = 0; i < menu_index; ++i)
    {
        menu.entry[i].x = va_arg(va, int);
        menu.entry[i].y = va_arg(va, int);
        menu.entry[i].max_x = va_arg(va, int);
        menu.entry[i].max_y = va_arg(va, int);
        menu.entry[i].key_input = va_arg(va, char *);
    }
    
    result = GUI_MenuLoop(&menu, index);
    
    return result;
}

void GUI_ErrorHandler(int number, ...)
{
    va_list arglist;
    
    va_start(arglist, number);
    if (SETUP_CriticalErrorFlag)
    {
        if (SETUP_ErrCode)
        {
            if (SETUP_ErrCode == 2)
            {
                sprintf(GUI_ErrorBuffer, (char *)GUI_StringData[SETUP_Language][50], (SETUP_DevError & 0x7F) + 0x41); /* "No disk found in drive %c . Installation aborted." */
                GUI_PrintErrorBox(GUI_ErrorBuffer);
            }
            else
            {
                sprintf(GUI_ErrorBuffer, (char *)GUI_StringData[SETUP_Language][51], (SETUP_DevError & 0x7F) + 0x41); /* "No disk found in drive %c . Installation aborted." */
                GUI_PrintErrorBox(GUI_ErrorBuffer);
                if ((SETUP_DevError & 0x7F) + 0x41 == SETUP_CdDrive)
                {
                    GUI_PrintErrorBox((char *)GUI_StringData[SETUP_Language][52]); /* "If you are installing from a CD, please carefully remove fingerprints and other stains." */
                }
            }
        }
        else
        {
            sprintf(GUI_ErrorBuffer, (char *)GUI_StringData[SETUP_Language][49], (SETUP_DevError & 0x7F) + 0x41); /* "Drive %c is write-protected. Installation aborted." */
            GUI_PrintErrorBox(GUI_ErrorBuffer);
        }
    }
    OPM_Del(&GUI_ScreenOpm);
    DSA_CloseScreen();
    printf("\n\nInterner Fehler %i !\n", number);
    vprintf((const char *)GUI_StringData[SETUP_Language][number - 1000], arglist);
    printf("\n");
    arglist[0] = 0;
    SYSTEM_Deinit();
    exit(-1);
}

void GUI_DrawMessageBox(char *text, char *heading, char *button, unsigned char color0, unsigned char color1, unsigned char color2, unsigned char color3, unsigned char color4, unsigned char color5, int color6)
{
    OPM_Struct pixel_map;
    int i;
    
    GUI_DrawTextBox(0);
    OPM_New(GUI_ScreenWidth, GUI_ScreenHeight, 1u, &pixel_map, 0);
    OPM_CopyOPMOPM(&GUI_ScreenOpm, &pixel_map, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    for (i = 10; GUI_ScreenHeight / 2 > i; ++i)
    {
        GUI_DrawFilledBackground(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10, GUI_ScreenHeight / 2 - i, GUI_ScreenWidth / 2 + 16 * i / 10, i + GUI_ScreenHeight / 2, color1, color0, color2, color0);
        GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 4, GUI_ScreenHeight / 2 - i + 4, 16 * i / 10 + GUI_ScreenWidth / 2 - 4, GUI_ScreenHeight / 2 - i + 13, color3, color3, color3);
        GUI_DrawFrame(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 5, GUI_ScreenHeight / 2 - i + 14, 16 * i / 10 + GUI_ScreenWidth / 2 - 4, GUI_ScreenHeight / 2 - i + 14, color1);
        if (GUI_PrintButtonText(heading, color4, &GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 6, GUI_ScreenHeight / 2 - i + 5, 16 * i / 10 + GUI_ScreenWidth / 2 - 6, GUI_ScreenHeight / 2 - i + 15))
        {
            GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 5, GUI_ScreenHeight / 2 - i + 15, 16 * i / 10 + GUI_ScreenWidth / 2 - 5, i + GUI_ScreenHeight / 2 - 34, color2, color0, color1);
            GUI_DrawEmbossedArea(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 6, GUI_ScreenHeight / 2 - i + 16, 16 * i / 10 + GUI_ScreenWidth / 2 - 6, i + GUI_ScreenHeight / 2 - 35, 0xFDu, 0xFDu, 0xFDu);
            if (GUI_PrintText(text, 0, &GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10 + 7, GUI_ScreenHeight / 2 - i + 17, 16 * i / 10 + GUI_ScreenWidth / 2 - 7, i + (signed int)GUI_ScreenHeight / 2 - 36))
            {
                GUI_DrawButton(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - i + 5, i + GUI_ScreenHeight / 2 - 25, i + GUI_ScreenWidth / 2 - 5, i + GUI_ScreenHeight / 2 - 12, color5);
                if (GUI_PrintButtonText(button, 0, &GUI_ScreenOpm, GUI_ScreenWidth / 2 - i + 6, i + GUI_ScreenHeight / 2 - 22, i + GUI_ScreenWidth / 2 - 6, i + GUI_ScreenHeight / 2 - 12))
                {
                    break;
                }
            }
        }
    }
    if (GUI_ScreenHeight / 2 <= i)
    {
        GUI_ErrorHandler(1026);
    }
    GUI_DrawDropShadow(&GUI_ScreenOpm, GUI_ScreenWidth / 2 - 16 * i / 10, GUI_ScreenHeight / 2 - i, GUI_ScreenWidth / 2 + 16 * i / 10, i + GUI_ScreenHeight / 2);
    DSA_CopyMainOPMToScreen(1);
    GUI_DrawHorizontalMenu(1, 0, (GUI_ScreenWidth / 2 - i + 5), i + GUI_ScreenHeight / 2 - 25, i + GUI_ScreenWidth / 2 - 5, i + GUI_ScreenHeight / 2 - 12, 0x1B);
    OPM_CopyOPMOPM(&pixel_map, &GUI_ScreenOpm, 0, 0, GUI_ScreenWidth, GUI_ScreenHeight, 0, 0);
    OPM_Del(&pixel_map);
    DSA_CopyMainOPMToScreen(1);
}

void GUI_PrintInfoBox(char *string)
{
    GUI_DrawMessageBox(string, (char *)GUI_StringData[SETUP_Language][42], (char *)GUI_StringData[SETUP_Language][36], 0xF8u, 0xF9u, 0xF7u, 0xF6u, 0xFEu, 0xF6u, 0xF9u); /* "Note", "O.K." */
}

void GUI_PrintErrorBox(char *string)
{
    GUI_DrawMessageBox(string, (char *)GUI_StringData[SETUP_Language][44], (char *)GUI_StringData[SETUP_Language][36], 0xF4u, 0xF5u, 0xF3u, 0xF2, 0xFFu, 0xF2u, 0xF5); /* "Error!", "O.K." */
}
