;*      mnasm.inc
;*
;* MIDAS NASM interfacing definitions
;*
;* $Id: mnasm.inc,v 1.3 1997/07/31 10:56:52 pekangas Exp $
;*
;* Copyright 1996,1997 Housemarque Inc.
;*
;* This file is part of MIDAS Digital Audio System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Digital Audio System license, "license.txt". By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*

%ifndef __mnasm_inc
%define __mnasm_inc

BITS 32


; Segment/section definition macros. 

%ifdef M_TARGET_WATCOM
  %define CODESEG SEGMENT CODE
  %define DATASEG SEGMENT DATA
  SEGMENT CODE PUBLIC ALIGN=16 CLASS=CODE USE32
  SEGMENT DATA PUBLIC ALIGN=16 CLASS=DATA USE32
%else
  %define CODESEG SECTION .text
  %define DATASEG SECTION .data
%endif



; Symbol definition macros - for some reason, we may not put underscores
; to the symbols ourselves in Linux.

%ifdef M_TARGET_LINUX

  %macro EXTERNSYMBOL 1
    EXTERN %1
  %endmacro

  %macro GLOBALLABEL 1
    GLOBAL %1
    %1:
  %endmacro  

%else

  %macro EXTERNSYMBOL 1
    EXTERN _%1
    %define %1 _%1
  %endmacro

  %macro GLOBALLABEL 1
    GLOBAL _%1
    _%1:
    %define %1 _%1
  %endmacro    


%endif




%endif

;* $Log: mnasm.inc,v $
;* Revision 1.3  1997/07/31 10:56:52  pekangas
;* Renamed from MIDAS Sound System to MIDAS Digital Audio System
;*
;* Revision 1.2  1997/07/29 11:44:00  pekangas
;* Cleanup made possible by NASM 0.95 release
;*
;* Revision 1.1  1997/07/23 17:27:26  pekangas
;* Initial revision
;*
