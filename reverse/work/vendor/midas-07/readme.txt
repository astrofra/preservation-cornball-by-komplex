        MIDAS Sound System 0.7 Beta 1, release notes
        --------------------------------------------

A Sahara Surfers production.

Copyright 1996, 1997 Housemarque Inc.

Written by Petteri Kangaslampi and Jarno Paananen
(a.k.a. Alfred & Guru / S2)

This file is part of the MIDAS Sound System, and may only be used,
modified and distributed under the terms of the MIDAS Sound System
license, LICENSE.TXT. By continuing to use, modify or distribute this
file you indicate that you have read the license and understand and
accept it fully.



1. Introduction and news
------------------------

This file accompanies MIDAS Sound System 0.7 Beta 1, a MS-DOS only beta
version of MIDAS 0.7. If all goes as planned, this beta is all that will
be released of MIDAS 0.7, since 1.0 is only a few fixes and some
documentation updates away. Unfortunately one of those "few fixes" is a
complete rewrite of the MIDAS mixing routines, as the current assembler
mess is practically nonmaintainable, and thus the first beta of MIDAS
1.0 is probably about a month away.

This version includes proper 16-bit sample support for GUS, a much
updated GUS Sound Device otherwise too, partly rewritten and much
friendlier timer, stable (hopefully) DJGPP support and some other small
fixes and updates. As essentially all of the changes only affect MS-DOS,
this is a MS-DOS only release.

Note that this release is still somewhat experimental, due to the large
number of changes made. In addition, DOS is a very unfriendly
environment for developers, and DOS programs would require testing on a
much wider range of different computers than Win32 or Linux ones.

Due to the limitations of MS-DOS, and especially the GUS GF1 support,
this version does not contain stream support. In addition, only 8- and
16-bit mono samples are guaranteed to work.

Note to DJGPP users: As there apparently is no reasonable way to lock
all memory used by a part of a program (such as MIDAS), MIDAS low simply
locks ALL memory by setting _crt0_startup_flags to
_CRT0_FLAG_LOCK_MEMORY. This effectively disables all virtual memory for
the program, and might not be a good idea in all cases. However, MIDAS
makes a heavy use of hardware interrupts (mainly the timer), and just
does not work reliably without this. This setting is done in source file
"midas.c", so you can change it if necessary.

Also the MIDAS development source tree has been reorganized. We changed
version control system from RCS to CVS (client-server works beautifully
between NT and Linux), changed the zillion different and inconsistent
makefiles to a single GNU Make -based build system, and did some general
cleanup. You can get GNU Make for DOS with the DJGPP distribution. Note
that GNU Make is _NOT_ necessary for using MIDAS, only for rebuilding
it. Normal MIDAS retail library can be build with "make TARGET=dos
BUILD=retail lib install". For more details on the build system, see
files in directory src/bldopts.



2. Brief introduction to MIDAS
------------------------------

So what is MIDAS Sound System anyway?

In brief, MIDAS is a multichannel digital sound and music system,
capable of playing an unlimited number of channels of digital sound on
all supported platforms. It can play music modules, individual samples,
and digital audio streams, in any combination.

MIDAS supports the following module formats:
        - 4-channel Protracker modules plus 1-32-channel variants (.MOD)
        - 1-32 -channel Scream Tracker 3 modules (.S3M)
        - 2-32 -channel FastTracker 2 modules (.XM)

Under Win32 and Linux, MIDAS plays sound through the system sound API,
so all cards that have drivers installed are supported. Under DOS,
MIDAS supports the following sound cards:
        - Creative Labs Sound Blaster series (1.0, 1.5, 2.0, Pro, 16)
        - Media Vision Pro Audio Spectrum series (regular, Plus, 16)
        - Windows Sound System and compatible sound cards
        - Gravis Ultrasound (regular, max, PnP)

This release of MIDAS can be used for free for free programs, and full
source code is included. Licenses for commercial purposes are also
available, contact "midas@housemarque.fi" for details.



3. What's New?
--------------

Since the old 16-bit MIDAS 0.40a release, MIDAS has been rewritten
almost completely. A few hightlights of the changes include:
        - MIDAS now supports 32-bit environments. 16-bit support is
          removed.
        - MIDAS now plays FastTracker 2 (XM) modules
        - All individual Module Players are removed, a single Generic
          Module Player takes care of all module playback
        - MIDAS has been ported to Win32 and Linux
        - A new DLL API is available, with documentation

Changes since MIDAS 0.6.1 include:
	- Updated GUS support
	- Working GUS Software Mixing Sound Device
	- Partly rewritten and updated timer, with friendlier user
	callbacks
	- Updated API for DOS, also timer functions are now available at
	the API level
	- Updated documentation
	- Proper 16-bit sample support in XMs
	- Better DJGPP support, now integrated to the main MIDAS source
	tree
	- Source code directories re-organized, with a new GNU Make
	based build system


4. Supported platforms
----------------------

MIDAS supports the following platforms and compilers:
        - 32-bit MS-DOS under DOS/4GW, with Watcom C/C++
	- 32-bit MS-DOS under any DPMI host (such as CWSDPMI) with DJGPP
        - Windows NT/95 with Watcom C/C++
        - Windows NT/95 with Visual C/C++
        - Windows NT/95 with Borland Delphi
        - Linux with GNU C/C++

In addition, under Win32 MIDAS can be used with any programming
environment that supports DLLs. Direct examples are provided only for
Watcom and Visual C and Borland Delphi though.

Under Linux background and stream playing needs a pthreads compatible
package, preferably the LinuxThreads-package available at:

ftp://ftp.inria.fr/INRIA/Projects/cristal/Xavier.Leroy/linuxthreads.tar.gz



5. Using MIDAS
--------------

Before trying to use MIDAS, make sure you read all documentation
available in the "doc" directory. The documentation is included in
several formats: LaTeX source (*.tex, produced by m4), PostScript
(*.ps), Adobe PDF (*.pdf) and HTML (separate directories). PostScript or
PDF is recommended for printing, and HTML for reading online. If you
prefer, you can also use Adobe Acrobat Reader for reading the PDF
documents, or read the LaTeX source code directly (mostly ASCII) if you
don't have access to a web browser. At the moment the HTML documentation
requires long filenames, so it is not very usable from plain DOS.

After at least browsing through the documentation, you should check of
the examples in the "samples" directory. The directory "samples/common"
should be especially useful, as it contains several small programs that
demonstrate the new MIDAS API. DOS programmer's should also check
"samples/dos". "samples/midpnt" contains the source code for MIDAS
Module Player for Windows NT, a slightly bigger and more complex
program.

Some of the examples might require a bit tweaking, depending your
target system. In particular, you may need to edit a few lines at the
beginning of the Makefiles in the directories. However, the code itself
should compile cleanly on all supported platforms.

When you are ready to use MIDAS in your own programs, it is probably
useful to use one of the examples as a model. Remember that you are
allowed to use MIDAS for free programs as is, but for commercial usage
you will need to negotiate a license with us. In addition, your program
HAS to include proper credits for MIDAS, either in the program itself
or its documentation. This is even a requirement in the MIDAS license,
and if you fail to include the credits you are breaking it.



6. Contact Information
----------------------

Comments? Bug reports? Want more information about MIDAS or MIDAS
licensing? Contact us! E-mail is naturally preferred.

For technical questions, contact Petteri Kangaslampi:

        E-Mail:         pekangas@sci.fi (preferred)
                        k153997@cs.tut.fi (slow, use as backup if scifi
                                fails)

        Snail mail: (please don't)
                        Petteri Kangaslampi
                        Insinoorinkatu 60 A 49
                        FIN-33720 Tampere
                        Finland
                        Europe

        Phone:          +358-3-3172204 (note the new area code)


Linux-specific questions should go to Jarno Paananen:

        E-Mail:         jpaana@iki.fi


For MIDAS licensing information:

        E-Mail:         midas@housemarque.fi


The latest version is always available at our WWW site:
"http://kalahari.ton.tut.fi/s2/midas/". You can also find all MIDP
versions there plus all latest MIDAS related news.

In addition, there is a mailing list available for MIDAS-related
announcements. Send mail to "majordomo@kalahari.ton.tut.fi", with the
words "subscribe midas" in the BODY of the message. This is a standard
Majordomo list, so all normal Majordomo commands apply.



7. Getting MIDAS
----------------

The best and fastest way to get MIDAS is the Internet. The latest MIDAS
version is always at "http://kalahari.ton.tut.fi/s2/midas.html". In
addition, you can find there the latest MIDAS news, MIDP releases, and
general information.

In addition, MIDAS is available at the following fine BBSes:

        Edge Of Delight
                Node 1: +32-2-3755651   Boca 28.8K (V34/VFAST)
                Node 2: +32-2-3758923   ZyXEL 19.2K (V32terbo)
                Node 3: +32-2-3721089   ISDN 64K (X75)
                Chaos Managers: Cobra, aCceSs & Fredy

        The Underworld BBS
                Node 1: +41-22-9600621  USR Courier V.34+, ISDN Analog
                Node 2: +41-22-9600622  USR Courier V.34+, ISDN Analog
                Node 3: +41-22-9600623  ZyXEL Elite 2864I 64kb ISDN,
                                                ISDN Digital
                Sysop: Synoptic

        WarmBoot BBS
                Node 1: +55-194-261993
                Sysop: Warmbooter

        South of Heaven BBS
                Node 1: +1-916-567-1090 (2x28.8k)
                Sysop: Pantera

        Psycho Beaver BBS
                Node 1: +972-9-8320175 (2x28.8k)
                Sysop: riff raff

	Splatter Punk BBS
		 Node 1: +49-2378-2627 (28.8k, 20:00-06:00 CET)
		 Sysop: Teasy

        Apologies for any boards we forgot - email us with your current
        information and you are back on the list. I have to confess I
        haven't kept very good track on our distribution sites, and now
        deliberately left out everybody I haven't heard from in a year
        or so...


Go code.	

-- Petteri Kangaslampi, 29 Mar 1997







