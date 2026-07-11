#*      wcopts.mak
#*
#* Gnu C -specific macros
#*
#* $Id: gcopts.mak,v 1.5 1997/02/27 16:22:37 pekangas Exp $
#*
#* Copyright 1997 Housemarque Inc.
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*

# Define the programs:
CC = gcc
ifdef _DOS
  CPP = gxx
else
  CPP = g++
endif
LINK = gcc
RC = 

# Object file extension:
O = o

# Library file extension:
_LIB = a

# Dynamic Link Library extension:
_DLL = so

# Executable extension:
ifdef _DOS
  _EXE = .exe
else
  _EXE =
endif

# Generic compiler options:
CCOPTS = -c -Wall -Werror -m486 -fpack-struct -D_REENTRANT
        # -c            compile only
        # -Wall         maximum warnings
        # -Werror       warnings are errors
        # -fpack-struct pack all structure members together
        # -shared       produce a shared object
        # -m486         optimize for 486 (no Pentium GCC yet)
	# -D_REENTRANT  use thread-safe libc

CPPOPTS = $(CCOPTS)

LINKOPTS =
DLLLINKOPTS = -shared 
RCOPTS = 



# Add necessary options for building DLLs:
ifdef _DLL_TARGET
  CCOPTS += -fpic
        # -fpic		generate position-independent code
endif


# Add optimization for retail build:
ifdef _RETAIL
  CCOPTS += -O2 -fomit-frame-pointer
        # -O2		reasonable speed optimizations
        # -fomit-frame-pointer don't keep the frame pointer in a register for functions that don't use it
endif


# Add debug information and debug code for debug build:
ifdef _DEBUG
  CCOPTS += -g -DDEBUG
        # -g            generate debugging information
  LINKOPTS += -g
  DLLLINKOPTS += -g
endif


# If additional libraries have been given, add them to the linker directives:
ifdef FIXLIBS
  LINKOPTS += $(patsubst %,-l%,$(FIXLIBS))
  DLLLINKOPTS += $(patsubst %,-l%,$(FIXLIBS))
endif

ifdef FIXLIBPATH
  LINKOPTS += $(patsubst %,-L%,$(GCCDOSLIBPATH))
  DLLLINKOPTS += $(patsubst %,-L%,$(GCCDOSLIBPATH))
endif


# Commands for building a library and a DLL:
BUILDLIBRARY = ar r $@ $^

# Although DOS has a 126(?) character command line length limit, programs
# compiled with DJGPP (such as make and gcc) can use longer command lines
# between each other, so a fix is not necessary.


# If an include directory has been given, add it to the command line options:
ifdef FIXINCPATH
  CCOPTS += $(patsubst %,-I%,$(FIXINCPATH))
endif


# If command line macro defines have been given, add them to the command line
# options:
ifdef DEFINES
  _DEFINES := $(patsubst %,-D%,$(DEFINES))
  CCOPTS += $(_DEFINES)
endif


#* $Log: gcopts.mak,v $
#* Revision 1.5  1997/02/27 16:22:37  pekangas
#* Added DJGPP support. Changed INCDIR to INCPATH. Other minor fixes.
#*
#* Revision 1.4  1997/02/22 18:44:42  jpaana
#* Changed the command line parameters to the "normal" gcc
#*
#* Revision 1.3  1997/02/11 18:15:44  pekangas
#* Added executable extension
#*
#* Revision 1.2  1997/02/08 13:35:53  jpaana
#* Added some explanations to options
#*
#* Revision 1.1  1997/02/07 15:16:17  pekangas
#* Initial revision
#*




