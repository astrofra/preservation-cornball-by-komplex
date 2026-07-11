;*      ERRORS.ASM
;*
;* MIDAS Sound System error codes, error message strings and error handling
;* routines for pure assembler.
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*

IDEAL
P386
JUMPS

INCLUDE "lang.inc"
INCLUDE "errors.inc"



DATASEG


; Error message string pointers:
errorMsg        DD      far ptr _0, far ptr _1, far ptr _2, far ptr _3
                DD      far ptr _4, far ptr _5, far ptr _6, far ptr _7
                DD      far ptr _8, far ptr _9, far ptr _10, far ptr _11
                DD      far ptr _12, far ptr _13, far ptr _14, far ptr _15
                DD      far ptr _16, far ptr _17, far ptr _18, far ptr _19
                DD      far ptr _20, far ptr _21, far ptr _22, far ptr _23
                DD      far ptr _24, far ptr _25, far ptr _26, far ptr _27
                DD      far ptr _28, far ptr _29, far ptr _30, far ptr _31
                DD      far ptr _32, far ptr _33, far ptr _34, far ptr _35
                DD      far ptr _36, far ptr _37


; Error messages: (maximum length 40 characters)

_0              DB      "OK", 0
_1              DB      "Undefined error", 0
_2              DB      "Out of conventional memory", 0
_3              DB      "Conventional memory heap corrupted", 0
_4              DB      "Invalid conventional memory block", 0
_5              DB      "Out of EMS memory", 0
_6              DB      "EMS memory heap corrupted", 0
_7              DB      "Invalid EMS memory block", 0
_8              DB      "Expanded Memory Manager failure", 0
_9              DB      "Out of soundcard memory", 0
_10             DB      "Soundcard memory heap corrupted", 0
_11             DB      "Invalid soundcard memory block", 0
_12             DB      "Out of instrument handles", 0
_13             DB      "Unable to open file", 0
_14             DB      "Unable to read file", 0
_15             DB      "Invalid module file", 0
_16             DB      "Invalid instrument in module", 0
_17             DB      "Invalid pattern data in module", 0
_18             DB      "Invalid channel number", 0
_19             DB      "Invalid instrument handle", 0
_20             DB      "Sound Device channels not open", 0
_21             DB      "Sound Device hardware failure", 0
_22             DB      "Invalid function arguments", 0
_23             DB      "File does not exist", 0
_24             DB      "Invalid file handle", 0
_25             DB      "Access denied", 0
_26             DB      "File exists", 0
_27             DB      "Too many open files", 0
_28             DB      "Disk full", 0
_29             DB      "Unexpected end of file", 0
_30             DB      "Invalid path", 0
_31             DB      "Unable to write file", 0
_32             DB      "Unable to lock Virtual DMA buffer", 0
_33             DB      "Unable to use Virtual DMA Services", 0
_34             DB      "Invalid Virtual DMA Service version", 0
_35             DB      "DPMI failure", 0
_36             DB      "Invalid segment descriptor", 0
_37             DB      "Out of system resources", 0


CRLF            DB      13, 10, 0

IFDEF DEBUG

errorList       errRecord       ?               ; error list
                errRecord (MAXERRORS-1) DUP (?)
numErrors       DW      0                       ; number of errors in list

        ; error list heading:
errListHead     DB      "MIDAS Error List:", 13, 10, 0

        ; error list element pieces:
errList1        DB      ": <", 0
errList2        DB      ", ", 0
errList3        DB      "> - ", 0
errList4        DB      " at ", 0


ENDIF


errPrintBuffer  DB      80 DUP (?)      ; message printing buffer



CODESEG



;/***************************************************************************\
;*
;* Function:    WriteASCIIZ
;*
;* Description: Writes an ASCIIZ string to screen (using DOS function 9)
;*
;* Input:       ds:bx                   point to string
;*
;* Destroys:    ax, bx, dx, es
;*
;\***************************************************************************/

PROC NOLANGUAGE WriteASCIIZ     NEAR

        push    di

        ; Point es:di to printing buffer
        mov     ax,seg errPrintBuffer
        mov     es,ax
        mov     di,offset errPrintBuffer

        ; Copy string to buffer until the terminating '\0' and append a '$':
@@clp:
        mov     al,[ds:bx]
        test    al,al
        jz      @@zero
        mov     [es:di],al
        inc     bx
        inc     di
        jmp     @@clp

@@zero:
        mov     [byte es:di],'$'

        ; Print string using DOS function 9:
        push    ds
        mov     ax,es
        mov     ds,ax
        mov     dx,offset errPrintBuffer
        mov     ax,0900h
        int     21h
        pop     ds

        pop     di

        ret
ENDP




;/***************************************************************************\
;*
;* Function:    WriteAX
;*
;* Description: Writes an integer to screen
;*
;* Input:       ax                      integer to be written
;*
;* Destroys:    ax, bx, cx, dx, es
;*
;\***************************************************************************/

PROC NOLANGUAGE WriteAX         NEAR

        push    di

        mov     cx,ax                   ; cx = integer

        ; Point es:di to printing buffer, at the end of the number (least
        ; significant digit)
        mov     ax,seg errPrintBuffer
        mov     es,ax
        mov     di,offset errPrintBuffer
        add     di,6

        ; Place terminating '$' after the string
        mov     [byte es:di],'$'

@@numloop:
        dec     di

        ; Get current integer mod 10 to dx:
        xor     dx,dx
        mov     ax,cx
        mov     bx,10
        div     bx

        add     dl,'0'                  ; dl = character for current digit
        mov     [es:di],dl              ; write digit

        mov     ax,cx
        mov     bx,10                   ; cx = number / 10
        xor     dx,dx
        div     bx
        mov     cx,ax
        test    cx,cx                   ; if number still nonzero, continue
        jnz     @@numloop               ; writing

        ; Write number to screen using DOS function 9:
        push    ds
        mov     ax,es
        mov     ds,ax
        mov     dx,di
        mov     ax,0900h
        int     21h
        pop     ds

        pop     di

        ret
ENDP




IFDEF DEBUG


;/***************************************************************************\
;*
;* Function:	 void errAdd(int errorCode, unsigned functID);
;*
;* Description:  Add an error to error list
;*
;* Input:	 int errorCode		 error code
;*		 unsigned functID	 ID for function that caused the error
;*
;\***************************************************************************/

PROC    errAdd          FAR     errorCode : word, functID : word

        ; Make sure that error list does not overflow:
        cmp     [numErrors],MAXERRORS
        jae     @@done

        mov     ax,SIZE errRecord
        mul     [numErrors]                     ; ax = offset to error list
        mov     bx,offset errorList
        add     bx,ax                           ; point ds:bx to list element

        mov     ax,[errorCode]                  ; store error code in list
        mov     [ds:bx+errRecord.errorCode],ax

        mov     ax,[functID]                    ; store function ID in list
        mov     [ds:bx+errRecord.functID],ax

        inc     [numErrors]
@@done:
        ret
ENDP




;/***************************************************************************\
;*
;* Function:    void errClearList(void)
;*
;* Description: Clears the error list. Can be called if a error has been
;*              handled without exiting the program to avoid filling the
;*              error list with handled errors.
;*
;\***************************************************************************/

PROC    errClearList    FAR

        mov     [numErrors],0
        ret
ENDP




;/***************************************************************************\
;*
;* Function:	 void errPrintList(void);
;*
;* Description:  Prints the error list to stderr
;*
;\***************************************************************************/

PROC    errPrintList    FAR
USES    si,di

        cmp     [numErrors],0           ; do nothing if list is empty
        je      @@done

        mov     bx,offset errListHead
        call    WriteASCIIZ

        xor     di,di
        mov     si,offset errorList

@@printloop:
        ; Write error number:
        mov     ax,di
        call    WriteAX

        ; Write ": <":
        mov     bx,offset errList1
        call    WriteASCIIZ

        ; Write error code:
        mov     ax,[ds:si+errRecord.errorCode]
        call    WriteAX

        ; Write ", ":
        mov     bx,offset errList2
        call    WriteASCIIZ

        ; Write function ID number:
        mov     ax,[ds:si+errRecord.functID]
        call    WriteAX

        ; Write "> - ":
        mov     bx,offset errList3
        call    WriteASCIIZ

        ; Write error message string:
        mov     bx,[ds:si+errRecord.errorCode]
        shl     bx,2                    ; offset to error message pointer tbl
        mov     bx,[word errorMsg+bx]   ; error message offset in data segment
        call    WriteASCIIZ

        ; Write " at ":
        mov     bx,offset errList4
        call    WriteASCIIZ

        ; Write function ID number:
        mov     ax,[ds:si+errRecord.functID]
        call    WriteAX

        ; Write CR/LF:
        mov     bx,offset CRLF
        call    WriteASCIIZ

        add     si,SIZE errRecord       ; next error list record
        inc     di                      ; next message
        cmp     di,[numErrors]
        jb      @@printloop

@@done:
        ret
ENDP



ENDIF   ; IFDEF DEBUG



;/***************************************************************************\
;*
;* Function:     void errErrorExit(char *msg)
;*
;* Description:  Set up standard text mode, print an error message and exit
;*
;* Input:        char *msg               pointer to error message, ASCIIZ
;*
;\***************************************************************************/

PROC    errErrorExit    FAR     msg : dword

        ; Set up standard 80x25 text mode:
        mov     ax,0003h
        int     10h

        ; Print error message string:
        push    ds
        lds     bx,[msg]
        call    WriteASCIIZ
        pop     ds

        ; Print CR/LF:
        mov     bx,offset CRLF
        call    WriteASCIIZ

IFDEF DEBUG
        call    errPrintList LANG
ENDIF

        mov     ax,4C01h                ; exit to DOS with error code 1
        int     21h
ENDP



END
