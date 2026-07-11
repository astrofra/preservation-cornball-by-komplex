;*      dsmmix.inc
;*
;* NASM include file for mixing routines
;*
;* $Id: dsmmix.inc,v 1.10.2.3 1997/09/06 17:16:52 pekangas Exp $
;*
;* Copyright 1996,1997 Housemarque Inc.
;*
;* This file is part of MIDAS Digital Audio System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Digital Audio System license, "license.txt". By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*

%ifndef __dsmmix_inc
%define __dsmmix_inc

%include "mnasm.inc"
%include "mprof.inc"


; The externals:

EXTERNSYMBOL dsmMixSrcPos
EXTERNSYMBOL dsmMixDest
EXTERNSYMBOL dsmMixLeftVol
EXTERNSYMBOL dsmMixRightVol
EXTERNSYMBOL dsmMixLeftVolFloat
EXTERNSYMBOL dsmMixRightVolFloat
EXTERNSYMBOL dsmMixStep
EXTERNSYMBOL dsmMixSample
EXTERNSYMBOL dsmMixBuffer
EXTERNSYMBOL dsmByteFloatTable
EXTERNSYMBOL dsmUlawFloatTable
EXTERNSYMBOL dsmMixLeftVolInt
EXTERNSYMBOL dsmMixRightVolInt
EXTERNSYMBOL dsmMixLeftVolTable
EXTERNSYMBOL dsmMixRightVolTable
EXTERNSYMBOL dsmMixLeftUlawVolTable
EXTERNSYMBOL dsmMixRightUlawVolTable

DSMMIX_PROF_MONO8MONO_F EQU M_PROF_DSM+0x80
DSMMIX_PROF_MONO16MONO_F EQU M_PROF_DSM+0x81
DSMMIX_PROF_MONOULAWMONO_F EQU M_PROF_DSM+0x82
DSMMIX_PROF_STEREO8MONO_F EQU M_PROF_DSM+0x84
DSMMIX_PROF_STEREO16MONO_F EQU M_PROF_DSM+0x85
DSMMIX_PROF_STEREOULAWMONO_F EQU M_PROF_DSM+0x86
DSMMIX_PROF_MONO8MONO_FI EQU M_PROF_DSM+0x88
DSMMIX_PROF_MONO16MONO_FI EQU M_PROF_DSM+0x89
DSMMIX_PROF_MONOULAWMONO_FI EQU M_PROF_DSM+0x8A
DSMMIX_PROF_STEREO8MONO_FI EQU M_PROF_DSM+0x8C
DSMMIX_PROF_STEREO16MONO_FI EQU M_PROF_DSM+0x8D
DSMMIX_PROF_STEREOULAWMONO_FI EQU M_PROF_DSM+0x8E
DSMMIX_PROF_MONO8MONO_I EQU M_PROF_DSM+0x90
DSMMIX_PROF_MONO16MONO_I EQU M_PROF_DSM+0x91
DSMMIX_PROF_MONOULAWMONO_I EQU M_PROF_DSM+0x92
DSMMIX_PROF_STEREO8MONO_I EQU M_PROF_DSM+0x94
DSMMIX_PROF_STEREO16MONO_I EQU M_PROF_DSM+0x95
DSMMIX_PROF_STEREO8MONO_SI EQU M_PROF_DSM+0x96
DSMMIX_PROF_STEREO16MONO_SI EQU M_PROF_DSM+0x97
DSMMIX_PROF_STEREO8MONO_MI EQU M_PROF_DSM+0x98
DSMMIX_PROF_STEREO16MONO_MI EQU M_PROF_DSM+0x99
DSMMIX_PROF_STEREOULAWMONO_I EQU M_PROF_DSM+0x9A
DSMMIX_PROF_STEREOULAWMONO_SI EQU M_PROF_DSM+0x9B
DSMMIX_PROF_STEREOULAWMONO_MI EQU M_PROF_DSM+0x9C
DSMMIX_PROF_STEREO16STEREO_I EQU M_PROF_DSM+0x9D
DSMMIX_PROF_STEREO16STEREO_MI EQU M_PROF_DSM+0x9E
DSMMIX_PROF_STEREO16STEREO_SI EQU M_PROF_DSM+0x9F


; Useful macros for self-modifying code

%macro put_ebp 2
	DB	0x89, 0x2d
  %2:
	DD	%1
%endmacro


%macro put_ebx 2
	DB	0x89, 0x1d
  %2:
	DD	%1
%endmacro


%macro put_ecx 2
	DB	0x89, 0x0d
  %2:
	DD	%1
%endmacro


%macro add_edx 1
	DB	0x81, 0xc2
  %1:
	DD	0xBABEFACE
%endmacro


%macro adc_esi 1
	DB	0x81, 0xd6
  %1:
	DD	0xACDCABBA
%endmacro


%macro cmp_edi 1
	DB	0x81, 0xff
  %1:
	DD	0x1742F000
%endmacro


%macro mov_ebx_4x_eax_plus 1
	DB	0x8B, 0x1C, 0x85
  %1:
	DD	0xFADED00D
%endmacro


%macro mov_ecx_4x_eax_plus 1
	DB	0x8B, 0x0C, 0x85
  %1:
	DD	0xFADED00D
%endmacro


%macro mov_ebp_4x_eax_plus 1
	DB	0x8B, 0x2C, 0x85
  %1:
	DD	0xFADED00D
%endmacro


%macro mov_ebp_4x_ebx_plus 1
	DB	0x8B, 0x2C, 0x9D
  %1:
	DD	0xFADED00D
%endmacro


%macro put_eax 2
	DB	0xA3
  %2:
	DD	%1
%endmacro


%macro fld_4x_eax_plus 1
	DB	0xD9, 0x04, 0x85
  %1:
	DD	0xBEA7F1D0
%endmacro


%macro fld_4x_ebx_plus 1
	DB	0xD9, 0x04, 0x9D
  %1:
	DD	0xBEA7F1D0
%endmacro


%endif

;* $Log: dsmmix.inc,v $
;* Revision 1.10.2.3  1997/09/06 17:16:52  pekangas
;* New macros to support PPro-optimized routines
;*
;* Revision 1.10.2.2  1997/08/20 17:49:26  pekangas
;* Added assembler optimized versions of stereo integer mixing of
;* 16-bit stereo samples with middle and surround panning
;*
;* Revision 1.10.2.1  1997/08/20 14:07:49  pekangas
;* Added an assembler-optimized 16-bit stereo integer mixing routine
;*
;* Revision 1.10  1997/07/31 10:56:39  pekangas
;* Renamed from MIDAS Sound System to MIDAS Digital Audio System
;*
;* Revision 1.9  1997/07/30 19:04:02  pekangas
;* Added optimized mono u-law mixing routines and enabled u-law autoconversion
;* by default
;*
;* Revision 1.8  1997/07/30 13:33:44  pekangas
;* Added optimized integer middle mixing routines
;* Optimized integer surround routines
;*
;* Revision 1.7  1997/07/29 11:44:00  pekangas
;* Cleanup made possible by NASM 0.95 release
;*
;* Revision 1.6  1997/07/24 17:10:01  pekangas
;* Added surround support and support for separate surround and middle mixing
;* routines.
;*
;* Revision 1.5  1997/07/24 14:54:33  pekangas
;* Added stereo integer profiling IDs and some useful macros for
;* self-modifying code
;*
;* Revision 1.4  1997/07/23 17:27:57  pekangas
;* Added profiling support
;*
;* Revision 1.3  1997/07/17 19:35:12  pekangas
;* The same assembler source should now compile to Win32, DOS and Linux
;* without modifications
;*
;* Revision 1.2  1997/07/17 09:55:45  pekangas
;* Added support for integer mono mixing
;*
;* Revision 1.1  1997/06/02 15:22:54  pekangas
;* Initial revision
;*