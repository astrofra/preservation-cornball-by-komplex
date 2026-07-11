#*      mincs.mak
#*
#* Defines make macros dependencies for MIDAS Digital Audio System include files
#*
#* $Id: mincs.mak,v 1.17 1997/08/18 14:01:18 pekangas Exp $
#*
#* Copyright 1996,1997 Housemarque Inc.
#*
#* This file is part of MIDAS Digital Audio System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Digital Audio System license, "license.txt". By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*

dma_h =         dma.h
errors_h =      errors.h
gmplayer_h =    gmplayer.h
lang_h =        lang.h
mconfig_h =     mconfig.h
mglobals_h =    mglobals.h
midas_h =       midas.h
mixsd_h =       mixsd.h
mmem_h =        mmem.h
mtypes_h =      mtypes.h
mutils_h =      mutils.h
rawfile_h =     rawfile.h
sdevice_h =     sdevice.h
timer_h =       timer.h
vgatext_h =     vgatext.h
vu_h =          vu.h
xm_h =          xm.h
it_h =          it.h
mpoll_h =       mpoll.h
midasfx_h =     midasfx.h
midasstr_h =    midasstr.h
midasdll_h =    midasdll.h
midastrk_h =    midastrk.h
dsm_h =         dsm.h $(sdevice_h)
file_h =        file.h $(rawfile_h)
madpcm_h =	madpcm.h
mulaw_h =	mulaw.h
waveread_h =    waveread.h
win32dlg_h =	win32dlg.h
mecho_h =       mecho.h
midasrc_h =     midasrc.h
mprof_h =       mprof.h

midas_h =       midas.h $(lang_h) $(mtypes_h) $(errors_h) $(mglobals_h) \
                $(mmem_h) $(file_h) $(sdevice_h) $(gmplayer_h) $(timer_h) \
                $(dma_h) $(mutils_h) $(mpoll_h) $(midasfx_h) \
                $(midasstr_h) $(madpcm_h) $(waveread_h)

#* $Log: mincs.mak,v $
#* Revision 1.17  1997/08/18 14:01:18  pekangas
#* Removed dsm.h from midas.h
#*
#* Revision 1.16  1997/07/31 10:56:51  pekangas
#* Renamed from MIDAS Sound System to MIDAS Digital Audio System
#*
#* Revision 1.15  1997/07/23 17:27:58  pekangas
#* Added profiling support
#*
#* Revision 1.14  1997/07/15 18:24:07  jpaana
#* Added midasrc.*
#*
#* Revision 1.13  1997/07/10 18:40:23  pekangas
#* Added echo effect support
#*
#* Revision 1.12  1997/07/08 19:16:44  pekangas
#* Added Win32 setup functions, save/restore setup from registry, and
#* fixed WinWave to ignore buffer blocks -setting to be compatible with the
#* new setup.
#*
#* Revision 1.11  1997/06/10 19:51:24  jpaana
#* Added IT-loader
#*
#* Revision 1.10  1997/05/20 20:36:47  pekangas
#* Added RIFF WAVE header and updated some dependencies
#*
#* Revision 1.9  1997/05/20 10:16:29  pekangas
#* Added preliminary tracker mode support
#*
#* Revision 1.8  1997/02/20 19:49:31  pekangas
#* Added u-law module
#*
#* Revision 1.7  1997/02/12 16:27:47  pekangas
#* Added adpcm.h to midas.h
#*
#* Revision 1.6  1997/02/11 11:07:26  pekangas
#* Added madpcm.h
#*
#* Revision 1.5  1997/01/16 18:41:59  pekangas
#* Changed copyright messages to Housemarque
#*
#* Revision 1.4  1996/09/25 16:29:32  pekangas
#* Added midasdll.h
#*
#* Revision 1.3  1996/09/22 23:17:05  pekangas
#* Added midasfx.h and midasstr.h
#*
#* Revision 1.2  1996/08/06 20:35:21  pekangas
#* Added mpoll.h
#*
#* Revision 1.1  1996/05/24 19:05:24  pekangas
#* Initial revision
#*
#* Revision 1.1  1996/05/24 19:01:59  pekangas
#* Initial revision
#*
