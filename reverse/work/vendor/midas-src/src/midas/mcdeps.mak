#*      mcdeps.mak
#*
#* Defines make dependencies for MIDAS Digital Audio System C source files
#* Note! You need to include mincs.mak first!
#* Note2! You need to define dpmi_h and vgatext_h if you compile for DOS.
#*
#* $Id: mcdeps.mak,v 1.34.2.2 1997/08/21 18:02:18 pekangas Exp $
#*
#* Copyright 1996,1997 Housemarque Inc.
#*
#* This file is part of MIDAS Digital Audio System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Digital Audio System license, "license.txt". By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*

T:=$(OTARGETDIR)

$(T)dma.$(O) :		dma.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
			$(dma_h) $(dpmi_h)

$(T)dsm.$(O) :		dsm.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
			$(sdevice_h) $(dsm_h) $(mutils_h) $(mglobals_h) \
			$(ems_h) $(dpmi_h) $(mulaw_h)

$(T)dostimer.$(O):  	dostimer.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(timer_h) $(mglobals_h) $(dpmi_h)

$(T)errors.$(O) :	errors.c $(lang_h) $(mtypes_h) $(errors_h)

$(T)file.$(O) :     	file.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
			$(rawfile_h) $(file_h)

$(T)gmpcmds.$(O) :  	gmpcmds.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(gmplayer_h)

$(T)gmplayer.$(O) : 	gmplayer.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(gmplayer_h) $(mmem_h)

$(T)loadmod.$(O) :	loadmod.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(mglobals_h) $(mmem_h) $(file_h) $(sdevice_h) \
			$(gmplayer_h) $(mutils_h)

$(T)loads3m.$(O) :	loads3m.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(mglobals_h) $(mmem_h) $(file_h) $(sdevice_h) \
			$(gmplayer_h) $(mutils_h)

$(T)loadxm.$(O) :	loadxm.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(mglobals_h) $(mmem_h) $(file_h) $(sdevice_h) \
			$(gmplayer_h) $(xm_h)

$(T)loadit.$(O) :	loadit.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(mglobals_h) $(mmem_h) $(file_h) $(sdevice_h) \
			$(gmplayer_h) $(it_h) $(mutils_h)

$(T)mconfig.$(O) :	mconfig.c $(midas_h) $(vgatext_h)

$(T)mglobals.$(O) :	mglobals.c $(lang_h)

$(T)mpoll.$(O) :	mpoll.c $(midas_h)

$(T)midas.$(O) :	midas.c $(midas_h)

$(T)mixsd.$(O) :	mixsd.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
			$(mixsd_h) $(sdevice_h) $(dsm_h) $(dma_h) $(mglobals_h)

$(T)mmem.$(O) :		mmem.c $(lang_h) $(errors_h) $(mmem_h)

$(T)nosound.$(O) :	nosound.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h)

$(T)vu.$(O) :		vu.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
			$(sdevice_h) $(vu_h) $(mutils_h)

$(T)rawfile.$(O) :	rawfile.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
			$(rawfile_h)

$(T)winwave.$(O) :  	winwave.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(mmem_h) $(dsm_h) $(mglobals_h)

$(T)dsmnsnd.$(O):   	dsmnsnd.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(dsm_h)

$(T)rawf_nt.$(O):	rawf_nt.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
			$(rawfile_h)

$(T)midasfx.$(O):	midasfx.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
			$(sdevice_h) $(midasfx_h) $(file_h) $(waveread_h)

$(T)midasstr.$(O):	midasstr.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(mmem_h) $(sdevice_h) $(file_h) $(midasstr_h) \
			$(madpcm_h) $(waveread_h)

$(T)midasdll.$(O):	midasdll.c $(midas_h) $(midasdll_h) $(dsm_h) $(mecho_h)

$(T)midastrk.$(O):	midastrk.c $(midas_h) $(midastrk_h)

$(T)dsound.$(O):	dsound.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(mglobals_h) $(sdevice_h) $(mmem_h) $(dsm_h)

$(T)madpcm.$(O):	madpcm.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
			$(madpcm_h) $(sdevice_h) $(mprof_h)

$(T)mulaw.$(O):		mulaw.c $(lang_h) $(mtypes_h) $(errors_h) $(mulaw_h)

$(T)sbirq.$(O):		sbirq.c $(lang_h) $(mtypes_h) $(errors_h)

$(T)gusmix.$(O):	gusmix.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(mglobals_h) $(dsm_h) $(dma_h) \
			$(mutils_h) $(mixsd_h)

$(T)djtext.$(O):	djtext.c $(lang_h) $(mtypes_h) $(vgatext_h) $(dpmi_h)

$(T)mapiconf.$(O):	mapiconf.c $(midas_h) $(midasdll_h) $(vgatext_h)

$(T)doscnf.$(O):	doscnf.c $(midas_h) $(midasdll_h) $(vgatext_h)

$(T)waveread.$(O):	waveread.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(mmem_h) $(rawfile_h) $(file_h) $(sdevice_h) \
			$(waveread_h) $(mutils_h)

$(T)dsmmix.$(O):	dsmmix.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(dsm_h)

$(T)dsmfilt.$(O):	dsmfilt.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(dsm_h) $(mglobals_h) $(mprof_h)

$(T)mix_mf.$(O):	mix_mf.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(dsm_h)
$(T)mix_sf.$(O):	mix_sf.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(dsm_h)
$(T)mix_mfi.$(O):	mix_mfi.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(dsm_h)
$(T)mix_sfi.$(O):	mix_sfi.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(dsm_h)

$(T)mix_mi.$(O):	mix_mi.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(dsm_h)
$(T)mix_si.$(O):	mix_si.c $(lang_h) $(mtypes_h) $(errors_h) \
			$(sdevice_h) $(dsm_h)

$(T)win32dlg.$(O):	win32dlg.c
$(T)win32cnf.$(O):	win32cnf.c $(midas_h) $(midasdll_h) $(win32dlg_h)

$(T)mecho.$(O):		mecho.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
			$(sdevice_h) $(dsm_h) $(mecho_h)

$(T)midasrc.$(O):	midasrc.c $(midas_h) $(midasdll_h) $(midasrc_h)

$(T)mprof.$(O):		mprof.c $(lang_h) $(mtypes_h) $(errors_h) $(mmem_h) \
			$(mprof_h)

$(T)adpcmtbl.$(O):	adpcmtbl.c




#* $Log: mcdeps.mak,v $
#* Revision 1.34.2.2  1997/08/21 18:02:18  pekangas
#* Added the ADPCM decoding table
#*
#* Revision 1.34.2.1  1997/08/20 12:48:05  pekangas
#* Manually merged fixes from the main trunk (oops)
#*
#* Revision 1.35  1997/08/19 17:22:01  pekangas
#* Added assembler versions of the default filtering routines
#*
#* Revision 1.34  1997/08/18 14:01:17  pekangas
#* Removed dsm.h from midas.h
#*
#* Revision 1.33  1997/07/31 10:56:45  pekangas
#* Renamed from MIDAS Sound System to MIDAS Digital Audio System
#*
#* Revision 1.32  1997/07/28 13:21:19  pekangas
#* Added new dos config module
#*
#* Revision 1.31  1997/07/24 14:55:04  pekangas
#* Added stereo integer mixing routines
#*
#* Revision 1.30  1997/07/23 17:27:57  pekangas
#* Added profiling support
#*
#* Revision 1.29  1997/07/22 13:46:18  pekangas
#* Added support for keeping object modules in a different directory
#* from the source
#*
#* Revision 1.28  1997/07/17 09:55:46  pekangas
#* Added support for integer mono mixing
#*
#* Revision 1.27  1997/07/15 18:24:07  jpaana
#* Added midasrc.*
#*
#* Revision 1.26  1997/07/10 18:40:23  pekangas
#* Added echo effect support
#*
#* Revision 1.25  1997/07/08 19:16:44  pekangas
#* Added Win32 setup functions, save/restore setup from registry, and
#* fixed WinWave to ignore buffer blocks -setting to be compatible with the
#* new setup.
#*
#* Revision 1.24  1997/06/20 10:10:04  pekangas
#* Updated mixsd dependencies
#*
#* Revision 1.23  1997/06/11 15:37:10  pekangas
#* Added interpolating stereo mixing routines
#*
#* Revision 1.22  1997/06/11 14:38:06  pekangas
#* Added filtering files
#*
#* Revision 1.21  1997/06/10 19:51:24  jpaana
#* Added IT-loader
#*
#* Revision 1.20  1997/06/05 20:18:47  pekangas
#* Added preliminary support for interpolating mixing (mono only at the
#* moment)
#*
#* Revision 1.19  1997/05/30 18:25:46  pekangas
#* Added new mixing routines for Win32
#*
#* Revision 1.18  1997/05/20 20:36:47  pekangas
#* Added RIFF WAVE header and updated some dependencies
#*
#* Revision 1.17  1997/05/20 10:16:28  pekangas
#* Added preliminary tracker mode support
#*
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
