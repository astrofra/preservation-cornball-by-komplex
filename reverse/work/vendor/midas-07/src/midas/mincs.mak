#*      mincs.mak
#*
#* Defines make macros dependencies for MIDAS Sound System include files
#*
#* $Id: mincs.mak,v 1.8 1997/02/20 19:49:31 pekangas Exp $
#*
#* Copyright 1996,1997 Housemarque Inc.
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
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
mpoll_h =       mpoll.h
midasfx_h =     midasfx.h
midasstr_h =    midasstr.h
midasdll_h =    midasdll.h
dsm_h =         dsm.h $(sdevice_h)
file_h =        file.h $(rawfile_h)
madpcm_h =	madpcm.h
mulaw_h =	mulaw.h


midas_h =       midas.h $(lang_h) $(mtypes_h) $(errors_h) $(mglobals_h) \
                $(mmem_h) $(file_h) $(sdevice_h) $(gmplayer_h) $(timer_h) \
                $(dma_h) $(dsm_h) $(mutils_h) $(mpoll_h) $(midasfx_h)

#* $Log: mincs.mak,v $
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