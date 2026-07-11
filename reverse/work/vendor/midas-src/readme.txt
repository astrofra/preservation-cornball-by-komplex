        MIDAS Digital Audio System 1.1.2 source code release notes
        ----------------------------------------------------------

A Sahara Surfers production.

Copyright 1996-1999 Housemarque Inc.

Written by Petteri Kangaslampi and Jarno Paananen
(a.k.a. Alfred & Guru / S2)

This file is part of MIDAS Digital Audio System, and may only be
used, modified and distributed under the terms of the MIDAS
Digital Audio System license, "license.txt". By continuing to use,
modify or distribute this file you indicate that you have
read the license and understand and accept it fully.



1. Introduction and news
------------------------

This is the long-avaited release of MIDAS Digital Audio System version 1.1.2
source code. It contains exactly the same source code that was used to build
the 1.1.2 release, and as such is not as programmer-friendly as possible. New
snapshots of the MIDAS 1.1 and 1.2 source trees should be available in the
near future, and those releases will include some modifications to make MIDAS
easier to compile and use.

This file is the only documentation currently available regarding the source
code, compiling and using it. Therefore you should read this carefully
through before attempting to compile and use the code -- any questions
already answered in this document will be ignored. Future source code
releases will contain more documentation, and additional support resources
will be set up as well, so if you aren't an experienced programmer you might
want to wait a while.

This release simply contains the src/ subdirectory of the MIDAS source tree
used to build MIDAS release 1.1.2. No documents, examples or precompiled
binaries are included. You should first download the standard 1.1.2 release,
unzip it, and then unzip this source distribution on top of it. Future MIDAS
source release will likely include the whole source tree and be more
self-contained in general.

All code is still under the traditional MIDAS Digital Audio System
License. The license is NOT Open Source compliant and is not compatible with
most existing Open Source licenses, such as the GNU General Public
License. We know that this is a problem, and are working on getting MIDAS
released under an Open Source license. Do NOT e-mail us to complain about the
license. Well-reasoned suggestions are welcome though, currently the most
likely solution is dual-licensing under both the MIDAS license and GPL.

At least for the time being we will only accept contributions to the main
MIDAS Digital Audio System distribution if all copyrights of the contribution
are assigned to Housemarque Games Inc., or the contribution is placed in the
public domain. This is because we want to be able to distribute MIDAS under
different license in the future, and to be able to use it without limitations
in our own work. Note that you can distribute your modifications or modified
MIDAS versions even if you don't agree to this, the code just won't be
included in the official distribution. You'll have to comply with the MIDAS
license though.

Anyway, with all this said, have fun with the code.


2. Requirements
---------------

For compiling MIDAS Digital Audio System, the following tools are needed:

- A supported C compiler (see below)
- GNU Make
- NASM 0.95 (the exact version IS IMPORTANT - no earlier, no later)

In addition, converting the documentation to other formats requires
specialized tools. m4 is required for preprocessing, LaTeX for producing DVI,
GhostScript for producing PostScript and PDF out of DVI, and LaTeX2HTML for
the HTML. WinHelp and plain text is produced by a custom-made Python program
(included in src/tools). The paper and HTML documentation can probably only
be produced under Linux.

At least some of the tools will be made available at the MIDAS web page,
http://www.s2.org/midas/, so check there if you have problems finding them.


2.1 MS-DOS
----------

Under MS-DOS, the supported compilers are Watcom C (10.5 or later should
work) and DJGPP (any 2.x?). The GNU Make version distributed with DJGPP
works, and you might need other DJGPP tools even if you compile with Watcom
C. In addition to those, TASM 4.0 is needed for compiling most of the DOS
Sound Devices and o2c for converting them for use with DJGPP.

The DJGPP version requires Shawn Hargreaves' Allegro library for its IRQ
wrappers. You can get Allegro from http://www.talula.demon.co.uk/allegro/. At
least version 2.11 is known to work, but later versions should do as
well. Note that the Allegro timer functions cannot be used with MIDAS.


2.2 Win32
---------

Under Win95/NT, the supported compilers are Watcom C (10.5 and 10.6, probably
later) and Visual C (4.2, probably others). GNU Make compiles with Visual C
directly from the standard distribution. GNU Make from the Cygnus toolset
should work as well. Cygnus GCC isn't currently supported, but adding support
shouldn't be a major task.


2.3 Linux
---------

Apart from NASM, all the necessary tools should be included in most Linux
distributions. GCC version 2.7.2 or later, including EGCS, works. GNU Make
3.77 at least works, earlier versions are probably fine too. In addition,
you'll need to make sure you have working POSIX threads and a reasonable
sound card driver.



3. Compiling MIDAS
------------------

3.1 Make and options
--------------------

MIDAS Digital Audio System uses a fairly complicated set of Makefiles for
compiling the system. The main build system is in src/bldopts/, and the
makefiles in other directories include files from there. The build system is
controlled by setting variables from the make command line, the most
important ones are listed here:

	  SYSTEM=dos/win32/linux	the host system
	  TARGET=dos/win32/win32win/win32dll/linux	the target system
	  BUILD=debug/retail/profile	build type
	  COMPILER=watcom/visualc/gcc	compiler used

The system tries to autodetect all values, the default BUILD is debug. To
check that the settings are reasonable do "make showinfo" in the
src/-directory, and experiment with different settings. Note that Win95 may
get detected as dos -- this is actually useful, as things tend to work better
that way.

For more information, read the makefiles or the code in
src/bldopts. src/bldopts/bldpts.mak is probably a good place to start.


3.2 Compiling the library
-------------------------

To compile MIDAS, change to src/midas/ and type "make BUILD=buildtype". The
library is placed under the lib/$TARGET/ directory, and the object files
under obj/$TARGET/. There are several useful targets available in the
makefile:

	lib		compiles the library
	dll		compiles the dynamic link library
	clean		cleans the source directory
	cleantarget	cleans the target directories

So, for example, to build a retail library completely from scratch, use
	make BUILD=retail clean cleantarget lib

There are some settings in the beginning of the makefile which you might need
to change, so check them if you have problems compiling.



4. Possible problems
--------------------

Here are answers to some possible problems you might have with MIDAS. If you
mail us asking one of these questions, count on being ignored!


4.1 Delphi (or some other Windows/DOS tool) complains about too long lines
-----
The source code was exported from CVS under Linux, and because of that it
contains Unix-style LF newlines instead of DOS-style CR/LFs


4.2 Funny errors from NASM
-----
The version probably isn't correct. You'll need 0.95, NOT OTHER VERSION WILL
WORK. NASM macros seem to work differently in every release, but we'll try to
support the latest NASM version in the future MIDAS snapshots.


4.3 GNU Make complains about "Missing separator"
-----
The commands need to be indented with a TAB character, not with spaces. Make
sure your editor doesn't mess them! Some GNU Make versions might have
problems with different newlines as well.


4.4. This build system is nuts, why didn't you use automake/autoconf?
-----
The GNU automake and autoconf are fine tools when you want to build software
that is portable across different Unix-like platforms. They aren't very
useful for more portable programs though -- how would you run a configure
script without an Unix shell, for example? Although Unix-like tools are
available to most systems, we'd rather just require the user to download one
GNU Make binary instead of installing a complete development toolset.

Note that you are welcome to change MIDAS to use any build system you want,
just don't expect us to do the same. At least not without considerable
convincing ;-)



5. What to do with the source code?
-----------------------------------

Don't ask me :) Here are some suggestions though. Remember that, at least for
the time being, we will only accept new code to the distribution if the
copyrights are given to Housemarque Inc, or it is placed in the public
domain. If you don't want to do that, you can distribute the modifications by
yourself, but remember that you'll need to apply them to all MIDAS releases
yourself. For larger contributions we might even need to have you sign a
paper, but smaller patches we can take in with less bureaucracy.

Of course, nothing stops someone from making an alternative complete MIDAS
distribution with all kinds of interesting new features.


5.1 Port it to new systems
-----
I would be mostly interested in seeing BeOS, Amiga and Mac support, although
these might require quite a bit of work. Porting to new Unix or Unix-like
systems should be much easier. The code actually has already been ported to
HP/UX, but the current release doesn't support it out of the box. There might
be some alignment or byte order issues in the 1.1 tree as well. In
particular, check the structures in gmplayer.h if nothing works, and the
corresponding module loaders if one format stops working.


5.2 Improve DOS support
-----
We aren't interested in maintaining the DOS support any more, so it is
lagging behind. Most of the Sound Devices are still written in Turbo
Assembler, and should be rewritten in C instead. DJGPP support could also be
improved, especially integration with Allegro. Changing MIDAS to use Allegro
timers and possibly Allegro sound output would make it more useful.


5.3. Fix all bugs
-----
Not that there are any of course...


5.4. Write a new IT player
-----
Impulse Tracker supports a lot of stuff that FT2 and the like don't, and as
the MIDAS module playback code was originally designed with FastTracker 2 and
Protracker in mind, it doesn't play ITs very well. A totally separate IT
player would be a much better idea.



6. Contact Information
----------------------

Comments? Bug reports? Want more information about MIDAS Digital Audio System
licensing?  Contact us! Just select the correct e-mail address:

Technical questions and comments: Petteri Kangaslampi, pekangas@sci.fi
                                  Jarno Paananen, jpaana@iki.fi

Licensing information: Housemarque Inc., midas@housemarque.fi

The updated MIDAS Digital Audio System WWW-pages are now on-line at
http://www.s2.org/midas/. The pages are being moved to www.housemarque.com, at
least partially, but for the time being the most up-to-date information will
be available at s2.org. Mirror sites for either the downloads or the whole
MIDAS web site might be added in the future, contact us if you are
interested.

There are two MIDAS-related mailing lists available:
	midas@s2.org		MIDAS-related annoucements, low volume
	midas-dev@s2.org	MIDAS development discussions, possibly
				higher volume
You can subscribe to the lists by sending mail to "majordomo@s2.org" with the
words "subscribe listname" in the BODY of the message. All other normal
Majordomo commands are available as well.


-- Petteri Kangaslampi & Jarno Paananen, 20 January 1999
