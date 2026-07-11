/*      dlgtest.c
 *
 * MIDAS setup dialog test
 *
 * $Id: dlgtest.c,v 1.1 1997/07/08 19:23:46 pekangas Exp $
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


extern const unsigned char mConfDialog[];
extern const unsigned char mConfCustomDialog[];

void Error(char *msg)
{
    fprintf(stderr, "dlg2c error: %s, GetLastError(): %u\n", msg,
            (unsigned) GetLastError());
    exit(EXIT_FAILURE);
}


BOOL CALLBACK CustomDialogProc(HWND hwnd, UINT msg, WPARAM wparam,
                               LPARAM lparam)
{
    hwnd = hwnd;
    lparam = lparam;
    
    switch ( msg )
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            switch ( LOWORD(wparam) )
            {
                case IDOK:
                    EndDialog(hwnd, 1);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
    }

    return FALSE;
}


BOOL CALLBACK ConfDialogProc(HWND hwnd, UINT msg, WPARAM wparam,
                             LPARAM lparam)
{
    int result;
        
    hwnd = hwnd;
    lparam = lparam;
    
    switch ( msg )
    {
        case WM_INITDIALOG:
            SendDlgItemMessage(hwnd, IDC_MEDIUMQUALITY, BM_SETCHECK, 1, 0);
            return TRUE;

        case WM_USER+0x1742:                        
            if ( (result = DialogBoxIndirect(GetModuleHandle(NULL),
                                             (LPDLGTEMPLATE)
                                             mConfCustomDialog,
                                             hwnd,
                                             CustomDialogProc))
                 == -1 )
                Error("Custom dialog box failed");
            if ( result )
                printf("Custom: OK\n");
            else
                printf("Custom: Cancel\n");
            return TRUE;

        case WM_COMMAND:
            switch ( LOWORD(wparam) )
            {
                case IDOK:
                    EndDialog(hwnd, 1);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;

                case IDC_MEDIUMQUALITY:
                    if ( HIWORD(wparam) == BN_CLICKED )
                    {
                        printf("Normal\n");
                    }
                    return TRUE;

                case IDC_CUSTOMQUALITY:                    
                    if ( HIWORD(wparam) == BN_CLICKED )
                    {
                        if ( SendDlgItemMessage(hwnd, IDC_CUSTOMQUALITY,
                                                BM_GETCHECK, 0, 0) )
                        {
                            printf("Custom clicked\n");
                            if ( !PostMessage(hwnd, WM_USER+0x1742, 17, 42) )
                                Error("PostMessage failed");
                        }
                    }
                    return TRUE;
            }
    }

    return FALSE;
}


int main(void)
{
    int result;
    
    printf("Setup dialog test\n");

    if ( (result = DialogBoxIndirect(GetModuleHandle(NULL),
                                     (LPDLGTEMPLATE) mConfDialog,
                                     NULL, ConfDialogProc)) == -1 )
        Error("Dialog box failed");
    if ( result )
        printf("Quit: OK\n");
    else
        printf("Quit: Cancel\n");

    return 0;
}


/*
 * $Log: dlgtest.c,v $
 * Revision 1.1  1997/07/08 19:23:46  pekangas
 * Initial revision
 *
*/
