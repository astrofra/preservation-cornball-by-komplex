/*      dlg2c.c
 *
 * Converts the MIDAS setup dialog resources to C source
 *
 * $Id: dlg2c.c,v 1.2 1997/07/31 14:27:37 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS Digital
 * Audio System license, LICENSE.TXT. By continuing to use, modify or
 * distribute this file you indicate that you have read the license
 * and understand and accept it fully.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "resource.h"


void Error(char *msg)
{
    fprintf(stderr, "dlg2c error: %s, GetLastError(): %u\n", msg,
            (unsigned) GetLastError());
    exit(EXIT_FAILURE);
}


void ConvertDialog(const char *tableName, const char *resourceName)
{
    HRSRC res;
    HGLOBAL h;
    unsigned char *c;
    unsigned length;
    unsigned n;
    HMODULE module = GetModuleHandle(NULL);
        
    printf("unsigned char %s[] = {\n    ", tableName);

    if ( (res = FindResource(module, resourceName, RT_DIALOG))
         == NULL )
        Error("Couldn't find resource");

    if ( (h = LoadResource(module, res)) == NULL )
        Error("Couldn't load resource");

    if ( (length = SizeofResource(module, res)) == 0 )
        Error("Couldn't get resource length");

    c = (unsigned char*) h;

    n = 0;
    while ( length )
    {
        printf("0x%02x", (unsigned) *(c++));
        length--;
        n++;        
        if ( length )
            printf(", ");
        if ( n == 10 )
        {
            printf("\n    ");
            n = 0;
        }
    }
    
    FreeResource(h);

    printf("\n};\n\n");
}


int main(void)
{
    puts("/*      win32dlg.c\n"
         " *\n"
         " * MIDAS Win32 configuration dialog templates, created with dlg2c\n"
         " *\n"
         " * $Id: dlg2c.c,v 1.2 1997/07/31 14:27:37 pekangas Exp $\n"
         " *\n"
         " * Copyright 1997 Housemarque Inc.\n"
         " *\n"
         " * This file is part of MIDAS Digital Audio System, and may only be\n"
         " * used, modified and distributed under the terms of the MIDAS Digital\n"
         " * Audio System license, LICENSE.TXT. By continuing to use, modify or\n"
         " * distribute this file you indicate that you have read the license\n"
         " * and understand and accept it fully.\n"
         "*/\n\n");

    ConvertDialog("mConfDialog", (const char*) IDD_MIDASSETUPDLG);
    ConvertDialog("mConfCustomDialog",
                  (const char*) IDD_MIDASCUSTOMQUALITYDIALOG);

    puts("/*\n"
         " * \$Log\$\n"
         "*/");

    return 0;
}


/*
 * $Log: dlg2c.c,v $
 * Revision 1.2  1997/07/31 14:27:37  pekangas
 * Added "Never use DirectSound" -button, changed the layout slightly
 * Fixed CVS IDs
 *
 * Revision 1.1  1997/07/08 19:23:45  pekangas
 * Initial revision
 *
*/
