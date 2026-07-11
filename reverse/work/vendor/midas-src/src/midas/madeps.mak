#*      madeps.mak
#*
#* Defines make dependencies for MIDAS Digital Audio System assembelr files
#* Note! You need to include mincs.mak first!
#*
#* $Id: madeps.mak,v 1.14.2.3 1997/09/06 17:17:31 pekangas Exp $
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

$(T)dpmi.$(O) :     	dpmi.asm lang.inc errors.inc dpmi.inc

$(T)gus.$(O) :		gus.asm lang.inc errors.inc sdevice.inc mmem.inc \
			mglobals.inc mutils.inc

$(T)pas.$(O) :      	pas.asm pas.inc lang.inc errors.inc sdevice.inc \
			dsm.inc mixsd.inc dpmi.inc

$(T)postproc.$(O) : 	postproc.asm lang.inc

$(T)sb.$(O) :       	sb.asm lang.inc errors.inc sdevice.inc dsm.inc \
			dma.inc mixsd.inc mutils.inc

$(T)timer.$(O) :    	timer.asm lang.inc mglobals.inc errors.inc timer.inc \
			sdevice.inc

$(T)vgatext.$(O) :  	vgatext.asm lang.inc vgatext.inc

$(T)wss.$(O) :      	wss.asm lang.inc errors.inc sdevice.inc mixsd.inc \
			dsm.inc


$(T)amix_mf.$(O) :	amix_mf.nas mnasm.inc dsmmix.inc mprof.inc

$(T)amix_sf.$(O) :	amix_sf.nas mnasm.inc dsmmix.inc mprof.inc

$(T)amix_mfi.$(O) :	amix_mfi.nas mnasm.inc dsmmix.inc mprof.inc

$(T)amix_sfi.$(O) :	amix_sfi.nas mnasm.inc dsmmix.inc mprof.inc

$(T)adsmmix.$(O) :	adsmmix.nas mnasm.inc dsmmix.inc mprof.inc

$(T)adsmfilt.$(O) :	adsmfilt.nas mnasm.inc dsmmix.inc

$(T)amix_mi.$(O) :	amix_mi.nas mnasm.inc dsmmix.inc mprof.inc

$(T)amix_si.$(O) :	amix_si.nas mnasm.inc dsmmix.inc mprof.inc


$(T)profasm.$(O) :	mprofasm.nas mnasm.inc mprof.inc


$(T)adpcmasm.$(O) :	adpcmasm.nas mnasm.inc

$(T)cputype.$(O) :	cputype.nas mnasm.inc


#* $Log: madeps.mak,v $
#* Revision 1.14.2.3  1997/09/06 17:17:31  pekangas
#* Added cputype.nas
#*
#* Revision 1.14.2.2  1997/08/21 20:27:06  pekangas
#* Added assembler optimized ADPCM decoding routines
#*
#* Revision 1.14.2.1  1997/08/20 12:48:04  pekangas
#* Manually merged fixes from the main trunk (oops)
#*
#* Revision 1.15  1997/08/19 17:22:01  pekangas
#* Added assembler versions of the default filtering routines
#*
#* Revision 1.14  1997/07/31 10:56:44  pekangas
#* Renamed from MIDAS Sound System to MIDAS Digital Audio System
#*
#* Revision 1.13  1997/07/27 17:28:17  jpaana
#* Fixed for DJGPP
#*
#* Revision 1.12  1997/07/24 14:55:04  pekangas
#* Added stereo integer mixing routines
#*
#* Revision 1.11  1997/07/23 17:27:57  pekangas
#* Added profiling support
#*
#* Revision 1.10  1997/07/22 13:46:18  pekangas
#* Added support for keeping object modules in a different directory
#* from the source
#*
#* Revision 1.9  1997/07/17 09:55:46  pekangas
#* Added support for integer mono mixing
#*
#* Revision 1.8  1997/06/26 19:14:01  pekangas
#* Added amix_sfi
#*
#* Revision 1.7  1997/06/26 14:33:31  pekangas
#* Added amix_mfi.nas
#*
#* Revision 1.6  1997/06/10 19:54:23  jpaana
#* Made amix_sf.nas to depend on dsmmix.inc
#*
#* Revision 1.5  1997/06/04 15:24:43  pekangas
#* Added assembler mixing routines
#*
#* Revision 1.4  1997/05/30 18:26:58  pekangas
#* Removed dsmmix.asm
#*
#* Revision 1.3  1997/01/16 18:41:59  pekangas
#* Changed copyright messages to Housemarque
#*
#* Revision 1.2  1996/07/29 19:31:27  pekangas
#* .
#*
#* Revision 1.1  1996/05/24 21:24:00  pekangas
#* Initial revision
#*
#* Revision 1.1  1996/05/24 19:04:50  pekangas
#* Initial revision
#*
