#*      DBPPASPM.MAK
#*
#* MIDAS Pascal Protected Mode debug module player make script
#*
#* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*



# Compiler directories:

BCDIR = c:\bc
LIBDIR = $(BCDIR)\lib


# Compilers and options:

CC = bcc
CCOPTS = -c -3 -G -v -ml -l-v-c -I$(BCDIR)\include -D__BPPROT__ -DDEBUG \
        -DREALVUMETERS
PC = bpc
PCOPTS = /$G+ /$X+ /V /CP /GS /Q /D__BPPROT__ /DDEBUG /DREALVUMETERS #/DNOTIMER
ASM = tasm
ASMOPTS = -UT310 -ml -zi -m9 -dDEBUG -d__BPPROT__ -p


all : ccopts asmopts dbp.exe

ccopts :
        echo $(CCOPTS) > ccopts

asmopts :
        echo $(ASMOPTS) > asmopts


dbp.exe : dbp.pas MIDAS.tpp mParser.tpp Errors.tpp mGlobals.tpp mMem.tpp \
        SDevice.tpp MPlayer.tpp DMA.tpp DSM.tpp VU.tpp mConfig.tpp
        $(PC) dbp.pas $(PCOPTS)


MIDAS.tpp : MIDAS.pas Errors.tpp mGlobals.tpp mMem.tpp mFile.tpp \
        SDevice.tpp MPlayer.tpp S3M.tpp MODP.tpp MTM.tpp Timer.tpp
        $(PC) MIDAS.pas $(PCOPTS)

mParser.tpp : mParser.pas mParser.obj MIDAS.tpp mUtils.tpp mMem.tpp \
        mGlobals.tpp Errors.tpp
        $(PC) mParser.pas $(PCOPTS)

mparser.obj : mparser.c midas.h
        $(CC) @ccopts mparser.c

mConfig.tpp : mConfig.pas mConfig.obj MIDAS.tpp SDevice.tpp mUtils.tpp \
        vgaText.tpp mFile.tpp mMem.tpp
        $(PC) mConfig.pas $(PCOPTS)

mconfig.obj : mconfig.c midas.h
        $(CC) @ccopts mconfig.c



mUtils.tpp : mUtils.pas mUtils.obj
        $(PC) mUtils.pas $(PCOPTS)

mutils.obj : mutils.asm lang.inc mutils.inc
        $(ASM) @asmopts mutils.asm



Errors.tpp : Errors.pas
        $(PC) Errors.pas $(PCOPTS)

mGlobals.tpp : mGlobals.pas
        $(PC) mGlobals.pas $(PCOPTS)

mMem.tpp : mMem.pas Errors.tpp
        $(PC) mMem.pas $(PCOPTS)


DPMI.tpp : DPMI.pas Errors.tpp dpmi.obj
        $(PC) DPMI.pas $(PCOPTS)

dpmi.obj : dpmi.asm lang.inc errors.inc dpmi.inc
        $(ASM) dpmi.asm @asmopts


DMA.tpp : DMA.pas dma.obj mMem.tpp Errors.tpp mGlobals.tpp DPMI.tpp
        $(PC) DMA.pas $(PCOPTS)

dma.obj : dma.asm lang.inc errors.inc dma.inc mmem.inc
        $(ASM) @asmopts dma.asm


SDevice.tpp : SDevice.pas Errors.tpp mMem.tpp mUtils.tpp mGlobals.tpp \
        DSM.tpp DMA.tpp DPMI.tpp gus.obj pas.obj wss.obj sb.obj nsnd.obj
        $(PC) SDevice.pas $(PCOPTS)

GUS.obj : gus.asm lang.inc mglobals.inc sdevice.inc mmem.inc mglobals.inc \
        errors.inc mutils.inc
        $(ASM) @asmopts gus.asm

PAS.obj : pas.asm pas.inc lang.inc errors.inc sdevice.inc dsm.inc dma.inc
        $(ASM) @asmopts pas.asm

WSS.obj : wss.asm lang.inc errors.inc sdevice.inc dsm.inc dma.inc
        $(ASM) @asmopts wss.asm

SB.obj : sb.asm lang.inc errors.inc sdevice.inc dsm.inc dma.inc
        $(ASM) @asmopts sb.asm

NSND.obj : nsnd.asm lang.inc errors.inc sdevice.inc mmem.inc mglobals.inc
        $(ASM) @asmopts nsnd.asm


DSM.tpp : DSM.pas DSM.obj Errors.tpp mGlobals.tpp mMem.tpp DPMI.tpp
        $(PC) DSM.pas $(PCOPTS)

DSM.obj : dsm.asm lang.inc errors.inc mglobals.inc dsm.inc mmem.inc \
        sdevice.inc
        $(ASM) @asmopts dsm.asm


Timer.tpp : Timer.pas timer.obj Errors.tpp mGlobals.tpp
        $(PC) Timer.pas $(PCOPTS)

timer.obj : timer.asm errors.inc mglobals.inc lang.inc timer.inc \
        sdevice.inc
        $(ASM) @asmopts timer.asm


MPlayer.tpp : MPlayer.pas
        $(PC) MPlayer.pas $(PCOPTS)


S3M.tpp : S3M.pas S3M.obj S3MLoad.obj Errors.tpp mGlobals.tpp mMem.tpp \
        MPlayer.tpp SDevice.tpp mFile.tpp mUtils.tpp
        $(PC) S3M.pas $(PCOPTS)

s3m.obj : s3m.asm lang.inc errors.inc mglobals.inc s3m.inc mplayer.inc \
        sdevice.inc mmem.inc
        $(ASM) @asmopts s3m.asm

s3mload.obj : s3mload.c mtypes.h errors.h mglobals.h mmem.h sdevice.h \
        mplayer.h s3m.h lang.h file.h mutils.h
        $(CC) @ccopts s3mload.c


MODP.tpp : MODP.pas MOD.obj MODLoad.obj Errors.tpp mGlobals.tpp mMem.tpp \
        MPlayer.tpp SDevice.tpp mFile.tpp mUtils.tpp
        $(PC) MODP.pas $(PCOPTS)

mod.obj : mod.asm lang.inc errors.inc mglobals.inc mod.inc mplayer.inc \
        sdevice.inc mmem.inc
        $(ASM) @asmopts mod.asm

modload.obj : modload.c mtypes.h errors.h mglobals.h mmem.h sdevice.h \
        mplayer.h mod.h lang.h file.h mutils.h
        $(CC) @ccopts modload.c


MTM.tpp : MTM.pas MTM.obj MTMLoad.obj Errors.tpp mGlobals.tpp mMem.tpp \
        MPlayer.tpp SDevice.tpp mFile.tpp mUtils.tpp
        $(PC) MTM.pas $(PCOPTS)

mtm.obj : mtm.asm lang.inc errors.inc mglobals.inc mtm.inc mplayer.inc \
        sdevice.inc mmem.inc
        $(ASM) @asmopts mtm.asm

mtmload.obj : mtmload.c mtypes.h errors.h mglobals.h mmem.h sdevice.h \
        mplayer.h mtm.h lang.h file.h mutils.h
        $(CC) @ccopts mtmload.c


mFile.tpp : mFile.pas file.obj Errors.tpp mMem.tpp rawFile.tpp
        $(PC) mFile.pas $(PCOPTS)

file.obj : file.c lang.h mtypes.h errors.h mmem.h file.h rawfile.h
        $(CC) @ccopts file.c


rawFile.tpp : rawFile.pas asmrfile.obj Errors.tpp mMem.tpp
        $(PC) rawFile.pas $(PCOPTS)

asmrfile.obj : asmrfile.asm lang.inc errors.inc rawfile.inc mmem.inc
        $(ASM) @asmopts asmrfile.asm


VU.tpp : VU.pas VU.obj Errors.tpp mMem.tpp
        $(PC) VU.pas $(PCOPTS)

vu.obj : vu.asm lang.inc errors.inc vu.inc mmem.inc sdevice.inc
        $(ASM) @asmopts vu.asm


vgaText.tpp : vgaText.pas vgaText.obj
        $(PC) vgaText.pas $(PCOPTS)

vgatext.obj : vgatext.asm lang.inc vgatext.inc
        $(ASM) @asmopts vgatext.asm
