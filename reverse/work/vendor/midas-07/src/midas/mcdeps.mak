#*      mcdeps.mak
#*
#* Defines make dependencies for MIDAS Sound System C source files
#* Note! You need to include mincs.mak first!
#* Note2! You need to define dpmi_h and vgatext_h if you compile for DOS.
#*
#* $Id: mcdeps.mak,v 1.16 1997/03/05 16:51:09 pekangas Exp $
#*
#* Copyright 1996,1997 Housemarque Inc.
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*


dma.$(O) :      dma.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) $(dma_h) \
                $(dpmi_h)

dsm.$(O) :      dsm.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
                $(sdevice_h) $(dsm_h) $(mutils_h) $(mglobals_h) $(ems_h) \
                $(dpmi_h) $(mulaw_h)

dostimer.$(O):  dostimer.c $(lang_h) $(mtypes_h) $(errors_h) $(sdevice_h) \
                $(timer_h) $(mglobals_h) $(dpmi_h)

errors.$(O) :   errors.c $(lang_h) $(mtypes_h) $(errors_h)

file.$(O) :     file.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
                $(rawfile_h) $(file_h)

gmpcmds.$(O) :  gmpcmds.c $(lang_h) $(mtypes_h) $(errors_h) $(sdevice_h) \
                $(gmplayer_h)

gmplayer.$(O) : gmplayer.c $(lang_h) $(mtypes_h) $(errors_h) $(sdevice_h) \
                $(gmplayer_h) $(mmem_h)

loadmod.$(O) :  loadmod.c $(lang_h) $(mtypes_h) $(errors_h) $(mglobals_h) \
                $(mmem_h) $(file_h) $(sdevice_h) $(gmplayer_h) $(mutils_h)

loads3m.$(O) :  loads3m.c $(lang_h) $(mtypes_h) $(errors_h) $(mglobals_h) \
                $(mmem_h) $(file_h) $(sdevice_h) $(gmplayer_h) $(mutils_h)

loadxm.$(O) :   loadxm.c $(lang_h) $(mtypes_h) $(errors_h) $(mglobals_h) \
                $(mmem_h) $(file_h) $(sdevice_h) $(gmplayer_h) $(xm_h) \
                $(mutils_h)

mconfig.$(O) :  mconfig.c $(midas_h) $(vgatext_h)

mglobals.$(O) : mglobals.c $(lang_h)

mpoll.$(O) :    mpoll.c $(midas_h)

midas.$(O) :    midas.c $(midas_h)

mixsd.$(O) :    mixsd.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
                $(mixsd_h) $(sdevice_h) $(dsm_h) $(dma_h)

mmem.$(O) :     mmem.c $(lang_h) $(errors_h) $(mmem_h)

nosound.$(O) :  nosound.c $(lang_h) $(mtypes_h) $(errors_h) $(sdevice_h)

vu.$(O) :       vu.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) $(sdevice_h)\
                $(vu_h) $(mutils_h)

rawfile.$(O) :  rawfile.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
                $(rawfile_h)

winwave.$(O) :  winwave.c $(lang_h) $(mtypes_h) $(errors_h) $(sdevice_h) \
                $(mmem_h) $(dsm_h) $(mglobals_h)

dsmnsnd.$(O):   dsmnsnd.c $(lang_h) $(mtypes_h) $(errors_h) $(sdevice_h) \
                $(dsm_h)

rawf_nt.$(O):   rawf_nt.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
                $(rawfile_h)

midasfx.$(O):   midasfx.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
                $(sdevice_h) $(midasfx_h) $(file_h)

midasstr.$(O):  midasstr.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
                $(sdevice_h) $(file_h) $(midasstr_h) $(madpcm_h)

midasdll.$(O):  midasdll.c $(midas_h) $(midasdll_h)

dsound.$(O):    dsound.c $(lang_h) $(mtypes_h) $(errors_h) $(mglobals_h) \
                $(sdevice_h) $(mmem_h) $(dsm_h)

madpcm.$(O):	madpcm.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
		$(madpcm_h) $(sdevice_h)

mulaw.$(O):	mulaw.c $(lang_h) $(mtypes_h) $(errors_h) $(mulaw_h)

sbirq.$(O):	sbirq.c $(lang_h) $(mtypes_h) $(errors_h)

gusmix.$(O):	gusmix.c $(lang_h) $(mtypes_h) $(errors_h) $(sdevice_h) \
		$(mglobals_h) $(dsm_h) $(dma_h) $(mutils_h) $(mixsd_h)

djtext.$(O):    djtext.c $(lang_h) $(mtypes_h) $(vgatext_h) $(dpmi_h)

mapiconf.$(O):  mapiconf.c $(midas_h) $(midasdll_h) $(vgatext_h)




#* $Log: mcdeps.mak,v $
#* Revision 1.16  1997/03/05 16:51:09  pekangas
#* Added module mapiconf - the new MIDAS API configuration
#*
#* Revision 1.15  1997/02/27 16:02:53  pekangas
#* Added gusmix and sbirq modules
#*
#* Revision 1.14  1997/02/20 19:49:30  pekangas
#* Added u-law module
#*
#* Revision 1.13  1997/02/18 20:21:57  pekangas
#* madpcm.c now depends on sdevice.h
#*
#* Revision 1.12  1997/02/12 16:29:13  pekangas
#* Added madpcm.h to midasstr
#*
#* Revision 1.11  1997/02/11 18:13:41  pekangas
#* Added ADPCM module
#*
#* Revision 1.10  1997/02/06 20:58:19  pekangas
#* Added DirectSound support - new files, errors, and global flags
#*
#* Revision 1.9  1997/01/16 18:41:59  pekangas
#* Changed copyright messages to Housemarque
#*
#* Revision 1.8  1996/09/25 16:29:02  pekangas
#* Added midasdll
#*
#* Revision 1.7  1996/09/22 23:16:56  pekangas
#* Added midasfx and midasstr
#*
#* Revision 1.6  1996/08/13 20:47:54  pekangas
#* Added rawf_nt.obj
#*
#* Revision 1.5  1996/08/06 20:36:15  pekangas
#* Added mpoll.obj
#*
#* Revision 1.4  1996/07/29 19:32:58  pekangas
#* Added dsmnsnd.obj
#*
#* Revision 1.3  1996/06/06 20:32:58  pekangas
#* Added dostimer.
#*
#* Revision 1.2  1996/05/25 09:31:56  pekangas
#* Fixed winwave dependencies
#*
#* Revision 1.1  1996/05/24 21:24:00  pekangas
#* Initial revision
#*
#* Revision 1.1  1996/05/24 19:04:50  pekangas
#* Initial revision
#*
