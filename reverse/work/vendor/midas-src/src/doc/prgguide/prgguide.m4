% This is m4 source code for the document in LaTeX format. m4 is just used
% for including some files - latex2html doesn't process macros in them
% otherwise. Note changequote below.
changequote(%bq,%eq)
%bq
\documentclass[a4paper,12pt,oneside]{book}
%eq
include(midasdoc.sty)
%bq

\author{Petteri Kangaslampi}
\title{MIDAS Digital Audio System\\ \ \\ Programmer's Guide}


\begin{document}

\maketitle
\pagenumbering{roman}
\tableofcontents

\newpage
\pagenumbering{arabic}
\chapter{Introduction}

\section{Welcome}

Welcome to the exciting world of digital audio! MIDAS Digital Audio
System is the most comprehensive cross-platform digital audio system
today. With features such as an unlimited number of digital channels
on all supported platforms, simultaneous sample, module and stream
playback, and seamless portability across operating systems, MIDAS is
all you need for perfect sound in your applications.

This manual is the Programmer's Guide to the MIDAS Digital Audio
System. It includes descriptions about all aspects of MIDAS, including
initialization, configuration and usage of different system
components. It does not attempt to document all functions and data
structures available in MIDAS, but rather give a good overview on how
you can use MIDAS in your own programs. For complete descriptions of
all function and data structures, see the MIDAS API reference.


\section{What is MIDAS?}

What is MIDAS Digital Audio System anyway?

MIDAS Digital Audio System is a multichannel digital music and sound
engine. It provides you with an unlimited number of channels of
digital audio that you can use to play music, sound effects, speech or
sound streams. MIDAS is portable across a wide range of operating
systems, and provides an identical API in all supported environments,
making it ideal for cross-platform software development.

MIDAS Digital Audio System is free for noncommercial usage, read the
file \mFileName{license.txt} included in the MIDAS distribution for a
detailed license. Commercial licenses are also available.



\chapter{Getting started}

Although MIDAS Digital Audio System is a very powerful sound engine,
it is also extremely easy to use. This chapter contains all the
information necessary to develop simple sound applications using
MIDAS. It describes how to link MIDAS into your own programs, how to
use the MIDAS API functions from your own code, and concludes with a
simple module player program example.


\section{Installing MIDAS} %K:Installing

Installing MIDAS Digital Audio System is very simple: just create a
separate directory for it, and decompress the distribution
archive. MIDAS is normally distributed as one or several
\mFileName{.zip} files, and they all need to be decompressed in the
same directory. If developing Win32 or Linux applications, use an
unzip utility that handles long filenames, such as InfoZip unzip or
WinZip, instead of MS-DOS pkunzip. Linux developers should decompress
the files in Linux, as the archive may contain symbolic links for the
Linux libraries.

{\bf Note!} Make sure your unzip utility decompresses subdirectories
correctly. InfoZip unzip and WinZip should do this by default, but
pkunzip needs the ``\mResWord{-d}'' option to do this.


\section{Compiling with MIDAS} %K:Compiling

For applications using just the MIDAS Digital Audio System API, no
special compilation options are necessary. All MIDAS API definitions
are in the file \mFileName{midasdll.h}, and the modules using
MIDAS functions simply need to \mResWord{\#{}include} this file. No
special macros need to be \mResWord{\#{}define}d, and the data
structures are structure-packing neutral.  \mFileName{midasdll.h} is
located in the \mFileName{include/} subdirectory of the MIDAS
distribution, and you may need to add that directory to your include
file search path.

Under Windows NT/95, the MIDAS API functions use the \mResWord{stdcall}
calling convention, the same as used by the Win32 API. Under DOS, the
functions use the \mResWord{cdecl} calling convention, and under Linux the
default calling convention used by GCC. This is done transparently to the
user, however.

Delphi users can simply use the interface unit \mFileName{midasdll.pas}, and
access the MIDAS API functions through it. Although Delphi syntax is
different from C, the function, structure and constant names are exactly the
same, and all parameters are passed just like in the C versions. Therefore
all information in this document and the API Reference is also valid for
Delphi.

MS-DOS users with Watcom C will need to disable the ``SS==DS'' assumption
from the modules that contain MIDAS callback functions. This can be done
with the ``\mResWord{-zu}'' command line option. Note that this is {\bf
not} necessary for code that just calls MIDAS functions.



\section{Linking with MIDAS} %K:Linking;Library

If your program uses MIDAS Digital Audio System, you naturally need to
link with the MIDAS libraries as well. This section describes how to
do that on each platform supported.

All MIDAS Digital Audio System libraries are stored under the
\mFileName{lib/} subdirectory in the distribution. \mFileName{lib/}
contains a subdirectory for each supported platform, and those in turn
contain directories for each supported compiler. The format of the
compiler directory names is ``<compiler><build>'', where <compiler> is
a two-letter abbreviation for the compiler name, and <build> the
library build type --- retail or debug. Under most circumstances, you
should use the retail versions of the libraries, as they contain
better optimized code. Also, the debug libraries are not included in
all releases.

For example, \mFileName{lib/win32/vcretail/midas.lib} is the retail
build of the Win32 Visual C/C++ static library.


\subsection{Windows NT/95} %K:Linking;Linking,Win32;Library;Library,Win32

Under the Win32 platform, applications can link with MIDAS Digital
Audio System either statically or dynamically. Unless there is a
specific need to link with MIDAS statically, dynamic linking is
recommended. Delphi users need to use dynamic linking always.

When linking with MIDAS statically, simply link with the library file
corresponding to your development platform. For Watcom C/C++, the
library is \mFileName{lib/win32/wcretail/midas.lib}, and for Visual
C/C++ \mFileName{lib/win32/vcretail/midas.lib}.  Depending on your
configuration, you may need to add the library directory to your
``library paths'' list. When MIDAS is linked into the application
statically, the .exe is self-contained and no MIDAS \mFileName{.dll}
files are needed.

Dynamic linking is done by linking with the appropriate MIDAS import
library instead of the static linking library. In addition, the MIDAS
Dynamic Link Library (\mFileName{midasXX.dll}) needs to be placed in a
suitable directory --- either to the same directory with the program
executable, or in some directory in the user's PATH. The import
libraries are stored in the same directory with the static libraries,
but the file name is \mFileName{midasdll.lib}. For example, Visual C
users should link with
\mFileName{lib/win32/vcretail/midasdll.lib}. The MIDAS Dynamic Link
Libraries are stored in \mFileName{lib/win32/retail} and
\mFileName{lib/win32/debug}.

Delphi users do not need a separate import library --- using the interface
unit \mFileName{midasdll.pas} adds the necessary references to the
\mFileName{.dll} automatically. Note that running the program under the
Delphi IDE without the \mFileName{.dll} available can cause strange error
messages.



\subsection{MS-DOS} %K:Linking;Linking,MS-DOS;Library;Library,MS-DOS

As MS-DOS doesn't support dynamic linking, only a static link library is
provided for MS-DOS. You'll simply need to link with the library from the
appropriate subdirectory --- usually \mFileName{lib/dos/gcretail} for GCC
(DJGPP) and \mFileName{lib/dos/wcretail} for Watcom C. The executable is fully
self-contained, and no additional files are needed. DJGPP users also need to
link with the Allegro library, as MIDAS uses some of its functions for IRQ
handling.

Note that some versions of the Watcom Linker are not case-sensitive by
default, and you'll need to use case-sensitive linking with MIDAS. To do that,
simply add \mResWord{option caseexact} to your linker directives.



\subsection{Linux} %K:Linking;Linking,Linux;Library;Library,Linux

For Linux, only a static library is provided for the time being. To link your
program with MIDAS Digital Audio System, add the proper library directory
(usually \mFileName{lib/linux/gcretail}) to your library directory list (gcc
option \mResWord{-L}), and link the library in using the GCC option
\mResWord{-lmidas}.



\section{Using MIDAS from the IDE}

This section contains step-by-step instructions on building applications that
use MIDAS Digital Audio System with the Integrated Developent Environments of
popular compilers.


\subsection{Using MIDAS with Visual C Developer Studio}  %K:Visual C;\
                              %K:Developer Studio;Visual C,Developer Studio

This section contains contains simple step-by-step instructions for
using MIDAS Digital Audio System with Microsoft Developer Studio.

1. Begin the project as usual. If you already have an existing project,
it should need no modifications.

2. Add some simple code for testing MIDAS --- either copy the module
player example below, or just add a call to \mApiRef{MIDASstartup} to
the beginning of the program and \mResWord{\#{}include} 
\mFileName{midasdll.h} at the beginning of the module.

3. Add the MIDAS include directories to the include search path:
Open ``Build/Project Settings'' -dialog, choose the ``C/C++'' tab, 
select ``Preprocessor'' from the Category list, and add the MIDAS
include directory to ``Additional include directories''. For example,
if you installed MIDAS in \mFileName{d:/midas}, add
\mFileName{d:/midas/include}.

4. Add a MIDAS library to the project. In most cases, you should use
the retail import library, and thus link dynamically. Open
``Insert/Files into Project'' -dialog, and select the library file
you want to use, typically
\mFileName{d:/midas/lib/win32/vcretail/midasdll.lib}.

Now you should be able to build the project normally. To be able to 
run the program, you must make sure that the MIDAS DLL is available
either in the same directory with the produced executable, or in some
directory in the system search path. You can simply copy the DLL
from (for example) \mFileName{d:/midas/lib/win32/retail} 
to the project directory.



\subsection{Using MIDAS with Watcom C IDE}  %K:Watcom C;IDE;Watcom C,IDE

This section contains contains simple step-by-step instructions for
using MIDAS Digital Audio System with Watcom C IDE.

1. Begin the project as usual. If you already have an existing project,
it should need no modifications.

2. Add some simple code for testing MIDAS --- either copy the module
player example below, or just add a call to \mApiRef{MIDASstartup} to
the beginning of the program and \mResWord{\#{}include} 
\mFileName{midasdll.h} at the beginning of the module.

3. Add the MIDAS include directories to the include search path:
Open ``Options/C Compiler Switches'' -dialog, choose ``1. File
Option Switches'' from the switches list, and add the MIDAS
include directory to ``Include directories''. For example,
if you installed MIDAS in \mFileName{d:/midas}, add
\mFileName{d:/midas/include}.

4. Add a MIDAS library to the project. In most cases, you should use
the retail import library, and thus link dynamically. Open
``Sources/New Source'' -dialog, and select the library file
you want to use, typically 
\mFileName{d:/midas/lib/win32/wcretail/midasdll.lib}.

Now you should be able to build the project normally. To be able to 
run the program, you must make sure that the MIDAS DLL is available
either in the same directory with the produced executable, or in some
directory in the system search path. You can simply copy the DLL
from (for example) \mFileName{d:/midas/lib/win32/retail} 
to the project directory.



\section{A simple module player example} %K:Example;Module playback;\
                                         %K:Module playback,Example

This section describes a very simple example program that uses MIDAS Digital
Audio System for playing music. First, the complete program source is given in
both C and Delphi format, and after that the operation of the program is
described line by line. To keep the program as short as possible, all error
checking is omitted, and therefore it should not be used as a template for
building real applications --- the other example programs included in the
MIDAS distribution are more suitable for that.

Both versions of the program should be compiled as console applications in
the Win32 environment. Under MS-DOS and Linux the default compiler settings
are fine.


\subsection{C module player example} %K:Example;Example,C;Module playback;\
                                     %K:Module;Module playback,Example
\begin{verbatim}
 1  #include <stdio.h>
 2  #include <conio.h>
 3  #include "midasdll.h"
 4
 5  int main(void)
 6  {
 7      MIDASmodule module;
 8      MIDASmodulePlayHandle playHandle;
 9
10      MIDASstartup();
11      MIDASinit();
12      MIDASstartBackgroundPlay(0);
13
14      module = MIDASloadModule("..\\data\\templsun.xm");
15      playHandle = MIDASplayModule(module, TRUE);
16
17      puts("Playing - press any key");
18      getch();
19
20      MIDASstopModule(playHandle);
21      MIDASfreeModule(module);
22
23      MIDASstopBackgroundPlay();
24      MIDASclose();
25
26      return 0;
27  }
\end{verbatim}



\subsection{Delphi module player example} %K:Example;Example,Delphi;\
                                          %K:Module;Module playback;\
                                          %K:Module playback,Example

\begin{verbatim}
 1  uses midasdll;
 2
 3  var module : MIDASmodule;
 4  var playHandle : MIDASmodulePlayHandle;
 5
 6  BEGIN
 7      MIDASstartup;
 8      MIDASinit;
 9      MIDASstartBackgroundPlay(0)
10
11      module := MIDASloadModule('..\data\templsun.xm');
12      playHandle MIDASplayModule(module, true);
13
14      WriteLn('Playing - Press Enter');
15      ReadLn;
16
17      MIDASstopModule(playHandle);
18      MIDASfreeModule(module);
19
20      MIDASstopBackgroundPlay;
21      MIDASclose;
22  END.
\end{verbatim}


\subsection{Module player example description}

Apart from minor syntax differences, the C and Delphi versions of the program
work nearly identically. This section describes the operation of the programs
line by line. The line numbers below are given in pairs: first for C, second
for Delphi.

\begin{description}
    \item [1-3, 1] Includes necessary system and MIDAS definition files
    \item [7, 3] Defines a variable for the module that will be played
    \item [8, 4] Defines a variable for the module playing handle
    \item [10, 7] Resets the MIDAS internal state --- This needs to be done
        before MIDAS is configured and initialized.
    \item [11, 8] Initializes MIDAS
    \item [12, 9] Starts playing sound in the background
    \item [14, 11] Loads the module file
    \item [15, 12] Starts playing the module we just loaded, looping it
    \item [17-18, 14-15] Simply waits for a keypress
    \item [20, 17] Stops playing the module
    \item [21, 18] Deallocates the module we loaded
    \item [23, 20] Stops playing sound in the background
    \item [24, 21] Uninitializes the MIDAS Digital Audio System
\end{description}




\chapter{Using MIDAS Digital Audio System}

This chapter contains detailed step-by-step instructions on using MIDAS
Digital Audio System. Complete descriptions of the functions, data structures
and constants used here is available in the API reference.


\section{Initializing and configuring MIDAS} %K:Initialization;Configuration\
                                             %K:Setup;Config

MIDAS Digital Audio System initialization consists of five basic steps, which
are outlined below. The last two steps are not necessary in all cases.

1. Call \mApiRef{MIDASstartup}. This should be done as early in the program as
possible, preferably at the very beginning. \mApiRef{MIDASstartup} does not
take a significant amount of time, and does not allocate any memory, it simply
initializes MIDAS to a safe and stable state and resets all configration
options. Calling \mApiRef{MIDASclose} is always safe after
\mApiRef{MIDASstartup} has been called.

2. Configure MIDAS. This can be done by setting different MIDAS options with
\mApiRef{MIDASsetOption}, or by calling \mApiRef{MIDASconfig} to prompt the
user for the settings. The configuration can also be loaded from a file (with
\mApiRef{MIDASloadConfig}) or registry (with
\mApiRef{MIDASreadConfigRegistry}) at this point. Apart from the configuration
functions, no MIDAS functions may be called yet.

3. Initialize MIDAS by calling \mApiRef{MIDASinit}. All MIDAS functions are
usable after this, and the program can fully start using MIDAS. Most MIDAS
configuration options can {\bf not} be changed while MIDAS is initialized, so
to change the options it is necessary to uninitialize MIDAS first.

4. Start background sound playback. Unless you want to poll MIDAS manually
(with \mApiRef{MIDASpoll}), you should now call
\mApiRef{MIDASstartBackgroundPlay} to start background sound
playback. \mApiRef{MIDASstartBackgroundPlay} starts a playback thread (in
multithreaded systems) or a timer, and polls MIDAS automatically from it.

5. Open sound channels with \mApiRef{MIDASopenChannels}. The number of open
sound channel limits the number of sounds that can be played simultaneously
--- one sound channel can play one sound. The number of sound channels needed
depends on the application, typical values might be 8--32 channels for a
module, one channel per stream and 1--8 channels for sound effects. Having
more channels open than necessary will not increase the CPU use, as inactive
channels do not need CPU attention. However, some sound cards may place
limitations on the maximum number of open channels. If you are only playing a
single module, opening the channels manually is not necessary, as
\mApiRef{MIDASplayModule} can open the needed channels automatically.



\section{Uninitializing MIDAS}

MIDAS Digital Audio System uninitialization is essentially the reverse process
of initialization. Actually only the last step is (calling
\mApiRef{MIDASclose}) is absolutely necessary, but it is good practise to
uninitialize everything that has been initialized. In addition, all modules
and samples that have been loaded should be deallocated before uninitializing
MIDAS, as failing to do so may lead to memory leaks. 

Basic MIDAS uninitialization consists of three steps:

1. If sound channels have been opened with \mApiRef{MIDASopenChannels}, close
them by calling \mApiRef{MIDAScloseChannels}. 

2. If background sound playback is used, stop it by calling
\mApiRef{MIDASstopBackgroundPlay}. 

3. Finally, uninitialize the rest of MIDAS Digital Audio System by calling
\mApiRef{MIDASclose}. 




\section{Using modules}

Digital music modules provide an easy to use and space-efficient method for
storing music and more complicated sound effects. MIDAS Digital Audio System
supports Protracker (.mod), Scream Tracker 3 (.s3m), FastTracker 2 (.xm) and
Impulse Tracker (.it) modules. All types of modules are used through the same
simple API functions described in the subsections below.


\subsection{Loading and deallocating modules} %K:Module;Module playback;\
                                              %K:Module playback,Loading

Loading modules with MIDAS Digital Audio System is very simple, just call
\mApiRef{MIDASloadModule}, giving as an argument the name of the module
file. \mApiRef{MIDASloadModule} returns a module handle of type
\mApiRef{MIDASmodule}, which can then be used to refer to the module. After
the module is no longer used, it should be deallocated with
\mApiRef{MIDASfreeModule}.


\subsection{Playing modules} %K:Module;Module playback;Module playback,Playing

Modules that have been loaded into memory can be played by calling
\mApiRef{MIDASplayModule}. \mApiRef{MIDASplayModule} takes as an argument the
module handle for the module, and a boolean flag that indicates whether or not
the module playback should loop or not. It returns a module playback handle of
type \mApiRef{MIDASmodulePlayHandle} that can be used to refer to the module
as it is being played. 

\mApiRef{MIDASplayModuleSection} can be used to play just a portion of the
module. A single module could, for example, contain several different songs,
and \mApiRef{MIDASplayModuleSection} can be used to select which one of them
to play.

MIDAS is also capable of playing several modules simultaneously, or
even the same module several times from different positions. There are some
limitations, however, see \mApiRef{MIDASplayModule} documentation for
details. This can be useful for using module sections as sound effects, or
fading between two modules.

Module playback can be stopped by calling \mApiRef{MIDASstopModule}, passing
it as an argument the module playback handle returned by
\mApiRef{MIDASplayModule}.



\subsection{Controlling module playback} %K:Module;Module playback;\
                                         %K:Module playback,Controlling

Although typically module playback just proceeds without user intervention, it
is also possible to control the playback of the
module. \mApiRef{MIDASsetPosition} can be used to change the current playing
position, \mApiRef{MIDASsetMusicVolume} to set the master volume of the music,
and \mApiRef{MIDASfadeMusicChannel} to fade individual music channels in or
out. All of these functions require as their first argument the module playing
handle from \mApiRef{MIDASplayModule}.


\subsection{Getting module information} %K:Module;Module playback;\
                        %K:Module playback,Information;Module information

In MIDAS Digital Audio System, it is possible to query information about a
module or the status of module playback. This information can be used to
update an user interface, or synchronize the program operation to the music
playback. 

Basic information about the module, such as its name, is available
by calling \mApiRef{MIDASgetModuleInfo}. The function requires as an argument
a module handle returned by \mApiRef{MIDASloadModule}, and a pointer to a
\mApiRef{MIDASmoduleInfo} structure, which it then will fill with the module
information. A similar function, \mApiRef{MIDASgetInstrumentInfo}, is
available for reading information about individual instruments in the
module.

The current status of the playback of a module can be read with
\mApiRef{MIDASgetPlayStatus}. It is passed a module playback handle from
\mApiRef{MIDASplayModule}, and a pointer to a \mApiRef{MIDASplayStatus}
structure, which it will then fill with the playback status information. The
playback status information includes the current module playback position,
pattern number in that position and the current playback row, as well as the
most recent music synchronization command infobyte.

MIDAS Digital Audio System also supports a music synchronization callback
function, which will be called each time the player encounters a music
synchronization command. The synchronization command is {\bf Wxx} for Scream
Tracker 3 and FastTracker 2 modules, and {\bf Zxx} for Impulse Tracker
modules. The callback can be set or removed with the function
\mApiRef{MIDASsetMusicSyncCallback}. As the music synchronization callback is
called in MIDAS player context, it should be kept as short and simple as
possible, and it may not call MIDAS functions.



\section{Using samples} %K:Sample;Sample,Using

Using samples is the easiest way to add sound effects and other miscellaneous
sounds to a program with MIDAS Digital Audio System. This section describes
how samples are used in MIDAS.


\subsection{Loading samples} %K:Sample;Sample,Loading

Before samples can be used, they naturally need to be loaded into
memory. MIDAS Digital Audio System currently supports samples in two formats:
raw audio data files and RIFF WAVE files. Raw sample files can be loaded with
\mApiRef{MIDASloadRawSample}, and RIFF WAVE samples with
\mApiRef{MIDASloadWaveSample}. Both functions require as arguments the name of
the sample file, and sample looping flag --- 1 if the sample should be looped,
0 if not. \mApiRef{MIDASloadRawSample} also requires the sample type as an
argument, \mApiRef{MIDASloadWaveSample} determines it from the file
header. Both functions return a sample handle of type
\mApiRef{MIDASsample} that can be used to refer to the sample.

After the samples are no longer used, they should be deallocated with
\mApiRef{MIDASfreeSample}. You need to make sure, however, that the sample is
no longer being played on any sound channel when it is deallocated.


\subsection{Playing samples} %K:Sample;Sample,Playing

Samples that have been loaded into memory can be played with the function
\mApiRef{MIDASplaySample}. It takes as arguments the sample handle, playback
channel number, and initial values for sample priority, playback rate, volume
and panning. The function returns a MIDAS sample playback handle of type
\mApiRef{MIDASsamplePlayHandle} that can be used to refer to the sample as it
is being played. A single sample can be played any number of times
simultaneously. 

Sample playback can be stopped with \mApiRef{MIDASstopSample}, but this is not
necessary before the sample will be deallocated --- a new sample can simply be
played on the same channel, and it will then replace the previous one. The
sample handles will be recycled as necessary, so there is no danger of memory
leaks.


\subsection{Samples and channels} %K:Sample;Sample,Channels

One sound channel can be used to play a single sample, and therefore
\mApiRef{MIDASplaySample} requires the number of the channel that is used to
play the sample as an argument. The channel number can be set manually, or
MIDAS Digital Audio System can handle the channel selection automatically.

If the channel number for the sample is set manually, the channel used should
be allocated with \mApiRef{MIDASallocateChannel} to ensure that the channel is
not being used for other purposes. If a free channel is available, the
function will return its number that can then be used with
\mApiRef{MIDASplaySample}. After the channel is no longer used, it should be
deallocated with \mApiRef{MIDASfreeChannel}.

Another possibility is to let MIDAS select the channel automatically. A number
of channels can be allocated for use as automatic effect channels with
\mApiRef{MIDASallocAutoEffectChannels}. \mApiRef{MIDASplaySample} can then be
passed {\bf MIDAS\_{}CHANNEL\_{}AUTO} as the channel number, and it will
automatically select the channel that will be used to play the effect. After
the automatic effect channels are no longer used, they should be deallocated
with \mApiRef{MIDASfreeAutoEffectChannels}.


\subsection{Controlling sample playback} %K:Sample;Sample,Controlling playback

Most sample playback properties can be changed while it is being
played. \mApiRef{MIDASsetSampleRate} can be used to change its playback rate,
\mApiRef{MIDASsetSampleVolume} its volume, \mApiRef{MIDASsetSamplePanning} its
panning position and \mApiRef{MIDASsetSamplePriority} its playback
priority. All of these functions take as an argument the sample playback
handle from \mApiRef{MIDASplaySample}, and the new value for the playback
property.

The sample playback properties can be changed at any point after the sample
playback has been started until the sample playback is stopped with
\mApiRef{MIDASstopSample}. If the sample has reached its end, or has been
replaced by another sample with automatic channel selection, the function call
is simply ignored.


\subsection{Getting sample information} %K:Sample;Sample,Getting information

The sample playback status can be monitored with
\mApiRef{MIDASgetSamplePlayStatus}. It takes as an argument the sample
playback handle, and returns the current sample playback status. The playback
status information can be used to determine whether or not a sample has
already reached its end.



\section{Using streams} %K:Stream;Stream,Using

In MIDAS Digital Audio System, streams are continuous flows of sample
data. Unlike samples, they do not need to be loaded completely into memory
before they can be played, but can rather be loaded from disk or generated as
necessary. This section describes how streams are used in MIDAS.


\section{Playing streams} %K:Stream;Stream,Playing

There are two different ways of playing streams in MIDAS Digital Audio System:
stream file playback and polling stream playback. In stream file playback,
MIDAS creates a new thread that will read the stream data from a given file
and plays it automatically on the background. In polling stream playback the
user needs to read or generate the stream data, and feed it to MIDAS.

Stream files are played with \mApiRef{MIDASplayStreamFile} and
\mApiRef{MIDASplayStreamWaveFile}. Both functions require as arguments the
stream file name, stream playback buffer length and stream looping flag. The
files played with \mApiRef{MIDASplayStreamFile} should contain only raw sample
data, and the function will therefore require as arguments also the stream
data sample type and playback rate. \mApiRef{MIDASplayStreamWaveFile} plays
RIFF WAVE files, and can read the information from the file header. The
playback functions will return a stream handle that can be used to refer to
the stream.

Polling stream playback is started with \mApiRef{MIDASplayStreamPolling}. It
requires as its arguments the stream sample type, playback rate and buffer
length. The actual stream playback will not start, however, until some stream
data is fed to the system with \mApiRef{MIDASfeedStreamData}. After the
playback has starter, \mApiRef{MIDASfeedStreamData} needs to be called
periodically to feed the system new stream data to play, otherwise the system
will run out of stream data and stop playback.

The stream playback buffer length controls the amount of stream data buffered
for stream playback. The longer the buffer is, the longer the system can play
the stream when new data is not available until the playback needs to be
stopped. Running out of stream data will result in irritating breaks in the
sound and should be avoided. Longer playback buffers will, however, add delay
to the stream playback, and consume more memory. For stream file playback, a
stream buffer length of 500ms should be suitable. For polling stream
playback, the buffer length should be at least twice the longest expected
delay between two calls to \mApiRef{MIDASfeedStreamData}.

Stream playback is stopped with \mApiRef{MIDASstopStream}, regardless of the
stream type.


\subsection{Controlling stream playback} %K:Stream;Stream,Controlling playback

The stream playback properties can be changed while it is being
played. \mApiRef{MIDASsetStreamRate} can be used to change its playback rate,
\mApiRef{MIDASsetStreamVolume} its volume and \mApiRef{MIDASsetStreamPanning}
its panning position. All of these functions take as an argument the stream
handle from the stream playback function, and the new value for the playback
property.

The playback of the stream can also be paused with \mApiRef{MIDASpauseStream},
and resumed after pausing with \mApiRef{MIDASresumeStream}. This can be useful
if your application knows it will run out of stream data soon, and wishes to
fade the stream out and pause it until more data is available. Stream data
reading and feeding can continue while the stream is paused until the stream
buffer is full.


\subsection{Getting stream information} %K:Stream;Stream,Getting information

The amount of data currently in the stream buffer can be monitored with
\mApiRef{MIDASgetStreamBytesBuffered}. The information could be used to
determine how soon new stream data is needed to continue playback, or whether
or not enough space exists in the stream buffer for a complete new block of
data. 

Note that with ADPCM streams the stream buffer contains the decompressed data,
as 16-bit samples, instead of the compressed ADPCM data.



\section{Using echo effects} %K:Echo effects;Echo effects,Using

The MIDAS Digital Audio System Echo Effects Engine can be used to add
different echo and reverb effects to the sound output. These effects can range
from simple echoes and filtering effects to heavy hall reverbs and stereo
enhancements. This section describes how the echo effects are used


\subsection{Adding echo effects} %K:Echo effects;Echo effects,Adding

Echo effects are added to the sound output with
\mApiRef{MIDASaddEchoEffect}. It takes as an argument a pointer to a filled
\mApiRef{MIDASechoSet} structure (described below), and returns and echo
handle that can then be used to refer to the effect. The echo set structure is
not used after \mApiRef{MIDASaddEchoEffect} returns, and may be
deallocated. Any number of echo effects can be active simultaneously.

After the echo effect is no longer wanted, it can be removed from the output
with \mApiRef{MIDASremoveEchoEffect}. Note that modifying the
\mApiRef{MIDASechoSet} structure of an echo effect that is already being used
has no effect.


\subsection{The echo effect structure} %K:Echo effects;Echo effects,Structure

A MIDAS Digital Audio System echo effect is described by a
\mApiRef{MIDASechoSet} structure. The echo set contains three common fields
plus one or more echoes. The \mVariable{feedback} field controls the amount of
feedback in the echo set, \mVariable{gain} the echo effect total gain, and
\mVariable{numEchoes} simply the number of echoes in the echo set. See the
\mApiRef{MIDASechoSet} description in the API Reference for more details.

The echoes of an echo set are described by an array of \mApiRef{MIDASecho}
structures. Each echo has fields that describe its delay, gain, filtering and
channel reverse status. The delay of an echo controls how far back from the
echo effect delay line the data for the echo is taken --- the greater the
delay, the longer the echo is. Gain controls the strength of the echo, the
echo data is essentially multiplied by the gain. Each echo can optionally be
low-pass or high-pass filtered, in a system with more than a couple of echoes,
low-pass filtering can reduce the build-up of high-frequency noise. Finally,
the left and right channels of the echo can be reversed, producing an
interesting stereo effect in some cases.



\chapter{Advanced topics}

This chapter includes advanced programming information about MIDAS Digital
Audio System. Although the information contained here is not necessary for
being able to use MIDAS, it should be read by everybody who wishes to get
everything out of it. Topics covered here include performance optimization,
sound quality optimization, and working with compressed sample types.


\section{Optimizing MIDAS performance} %K:Optimization;\
        %K:Optimization,Performance

Although MIDAS Digital Audio System has been carefully optimized for maximum
performance, playing multichannel digital audio can still be fairly
time-consuming. To get around the limitations of current PC sound cards, MIDAS
needs to mix the sound channels in software, and this mixing process accounts
for most of CPU usage caused by MIDAS. In many cases tradeoffs can be made
between sound quality and CPU usage, although it is also possible in some
cases to lower CPU usage dramatically without affecting sound quality.

In some cases the opposite is also true: It can be possible to get better
sound quality out of MIDAS without using much additional CPU power. This
section therefore describes how you can get best possible sound quality of
MIDAS while using as little CPU time as possible, and how to find the right
balance between sound quality and CPU usage for your application.


\subsection{The number of channels} %K:Optimization;\
        %K:Optimization,Number of channels

First and foremost, the CPU time used for mixing depends on the number of
active sound channels in use. Each channel needs to mixed to the output
separately, and thus requires CPU time. Note, however, that the total number
of open channels is not very significant, only the number of channels that are
actually playing sound. Also, sounds played at zero volume take very little
CPU time.

On a computer with a 90MHz Pentium CPU, with the default sound quality
settings, the CPU time used is roughly 0.6\%{}--0.9\%{} of the total CPU time per
channel. The number can vary greatly depending on the type of samples used and
other factors, but can be used as a guideline in deciding how many sound
channels to use. Regardless of the output mode settings, however, minimizing
the number of channels used is an easy way to increase MIDAS performance.

Using fewer sound channels does not necessarily mean sacrificing sound quality
or richness. Music modules do not necessarily need to have over 10 channels to
sound good --- talented musicians have composed stunning songs with the Amiga
Protracker program which only supports 4 channels. Unnecessary or very quiet
sound effects could be eliminated, making way for more important
ones. Finally, instead of playing two or more sounds simultaneously, the
sounds could be mixed beforehand with a sample editor into one.


\subsection{Sample types} %K:Optimization;Optimization,Sample types

Another factor that has a great effect on MIDAS Digital Audio System CPU usage
is the type of the samples played. The simplest sample type, and therefore the
fastest to play, is a 8-bit mono sample. 16-bit samples take typically 50\%{}
more CPU time to play than 8-bit samples, and stereo samples more than 50\%{}
than corresponding mono samples. In addition, ADPCM streams take additional
CPU time for decompressing the ADPCM data to 16-bit before it can be played.

However, 16-bit samples do sound much better than 8-bit ones. A good
compromise is to use $mu$-law samples instead. $mu$-law samples have almost
the same sound quality as 16-bit samples, while being as fast to mix as 8-bit
samples. In addition, they only take as much space as 8-bit samples, and thus
lower the memory requirements of the program as well. In some mixing modes
MIDAS will actually automatically convert 16-bit samples to $mu$-law if it is
beneficial. The option
\mApiRefLabel{MIDAS\_{}OPTION\_{}16BIT\_{}ULAW\_{}AUTOCONVERT}{MIDASoptions}
can be used to control this behaviour.

ADPCM compression yields a 1:4 compression on 16-bit sound data, using
effectively only 4 bits per sample, while maintaining sound quality better
than 8-bit samples. As ADPCM sample data cannot easily be played without
decompressing it first, however, MIDAS only supports ADPCM sample data in
streams. Although ADPCM streams take somewhat more CPU time to play than
16-bit streams, as the data needs to be decompressed, they can still turn out
to be faster in practise, as the amount of data that needs to be read from
disk is much smaller.


\subsection{Output settings} %K:Optimization;Optimization,Output settings

Finally perhaps the most important factor in determining MIDAS CPU usage and
sound quality: output sound quality settings. Several different sound quality
settings can be adjusted, and each can be used to adjust the balance of sound
quality and CPU usage.

The most important of all output quality settings is the mixing mode. By
default, MIDAS Digital Audio System uses normal mixing mode, which has very
good performance. High-quality mixing mode with sound interpolation is
available, and it greatly enhances the sound quality in some cases, but also
requires much more CPU power. High-quality mixing can use two to three times
as much CPU time as normal quality mixing, and should be reserved for
applications that only use a few sound channels or do not require the CPU time
for other uses. The mixing mode settings can be changed by setting the option
\mApiRefLabel{MIDAS\_{}OPTION\_{}MIXING\_{}MODE}{MIDASoptions} with
\mApiRef{MIDASsetOption}. 

Another setting that greatly affects CPU usage and sound quality is the output
mixing rate. CPU usage depends almost linearly on the mixing rate, with higher
mixing rates using more CPU power but also sounding better. The default mixing
rate is 44100Hz, but in many cases it can be lowered to 32000Hz or 22050Hz
without too great sound quality loss. In addition, if all of your sounds are
played at the same rate (eg. 22050Hz), the mixing rate should be set at the
same rate --- using a higher rate would not bring any better sound quality and
could actually increase noise in the output. 

Experimenting with different mixing rate and mode combinations can also be
worthwhile, as in some cases using a lower mixing rate with high-quality
mixing can sound better than a higher mixing rate with normal quality.

The last setting to consider is the output mode setting. The output mode
should normally be set to a 16-bit mode, as using 8-bit modes does not
decrease CPU usage, only sound quality. If the sound card used doesn't support
16-bit output, MIDAS will use 8-bit output automatically. Using mono output
instead of stereo, however, can decrease CPU usage by up to 50\%{}. Therefore,
if your application does not use stereo effects in its sound output, consider
using mono output mode instead. The output mode can be changed by setting the
option \mApiRefLabel{MIDAS\_{}OPTION\_{}OUTPUTMODE}{MIDASoptions} with
\mApiRef{MIDASsetOption}. 



\section{Using µ-law compression} %K:µ-law;u-law;mulaw;Sample,µ-law

In MIDAS Digital Audio System, µ-law samples and streams can be used as an
effective compromise between CPU and space usage and sound quality, as they
provide sound quality almost equivalent to 16-bit samples while using only as
much CPU time and space as 8-bit samples. This section describes how µ-law
samples are encoded and used with MIDAS.


\subsection{Encoding µ-law samples} %K:µ-law;u-law;mulaw;µ-law,Encoding;\
                                    %K:Sample,µ-law

Encoding µ-law samples is simple. The \mFileName{tools/} directory in the
MIDAS distribution contains directories for each supported platform, and these
directories contain a program called \mFileName{ulaw}. This program can be
used to encode 16-bit samples into µ-law samples, and decode µ-law samples
back to 16-bit ones. The syntax is simple. To encode a 16-bit sample file to
µ-law, use:

\begin{verbatim}
        ulaw e input-file-name output-file-name
\end{verbatim}

And to decode a µ-law file to a 16-bit one, use:

\begin{verbatim}
        ulaw d input-file-name output-file-name
\end{verbatim}

The files should contain just raw sample data, with no headers. Stereo and
mono samples and streams are processed exactly the same way.


\subsection{Using µ-law samples} %K:µ-law;u-law;mulaw;µ-law,Using;Sample,µ-law

µ-law samples and streams are used just like any other samples and streams in
MIDAS Digital Audio System. Simply pass the playback function
\mApiRefLabel{MIDAS\_{}SAMPLE\_{}ULAW\_{}MONO}{MIDASsampleTypes} or
\mApiRefLabel{MIDAS\_{}SAMPLE\_{}ULAW\_{}STEREO}{MIDASsampleTypes} as the
sample type, and everything will work normally. µ-law sample data can be used
for both samples and streams.



\section{Using ADPCM compression} %K:ADPCM;Stream,ADPCM

ADPCM streams provide a space-effective way of storing long sections of audio
with a fairly good sound quality. Although ADPCM streams have lower sound
quality than uncompressed 16-bit ones, they do sound better than 8-bit ones,
and, as they only use effectively 4-bit samples, they provide 1:4 compression
to the sound. This section descibes how ADPCM streams are encoded and used
with MIDAS Digital Audio System.


\subsection{Encoding ADPCM streams} %K:ADPCM;ADPCM,Encoding;Stream,ADPCM

Encoding ADPCM streams is fairly simple. The \mFileName{tools/} directory in the
MIDAS distribution contains directories for each supported platform, and these
directories contain a program called \mFileName{adpcm}. This program can be
used to encode 16-bit streams into ADPCM ones, and decode ADPCM streams back
to 16-bit. The syntax is similar to the µ-law encoder, although a bit more
complicated. To encode a 16-bit stream into ADPCM, use:

\begin{verbatim}
        adpcm e input-file-name output-file-name channels frame-length
\end{verbatim}

Where \mVariable{channels} is the number of channels in the stream (1 for
mono, 2 for stereo) and \mVariable{frame-length} the ADPCM frame length in
bytes, including the frame header. As ADPCM sample data is adaptative delta
encoded, it is normally impossible to start decoding an ADPCM stream from the
middle. To get around this problem, MIDAS divides the ADPCM stream into
``frames'', and is able to start decoding from the beginning of any frame.

The frame length you should use depends on the needs of your application. If
your application will always play the streams from beginning to end, any value
will do --- 1024 is a reasonable choice. However, if stream playback can start
from the middle of the stream, you should consider how the stream is
accessed. In particular, if the stream is read in blocks of a set number of
bytes, the frame length should be equal to the block size.

If you wish to make the ADPCM frames of a given length of time, remember that
each ADPCM sample is 4 bits. Therefore, one byte of ADPCM data will contain
data for two mono samples or one stereo sample. The ADPCM frame header is 9
bytes long for mono streams and 12 bytes long for stereo ones. Therefore, to
get 59ms long frames for a stereo stream played at 22050Hz, the frame length
should be 453 bytes.

To decode an ADPCM stream back to a 16-bit one, use:

\begin{verbatim}
        adpcm d input-file-name output-file-name channels frame-length
\end{verbatim}

Like with the µ-law encoder/decoder, the files should contain just raw sample
data, with no headers.


\subsection{Using ADPCM streams} %K:ADPCM;ADPCM,Using;Stream,ADPCM

ADPCM streams are used just like other streams in MIDAS Digital Audio
System. Simply pass
\mApiRefLabel{MIDAS\_{}SAMPLE\_{}ADPCM\_{}MONO}{MIDASsampleTypes} or
\mApiRefLabel{MIDAS\_{}SAMPLE\_{}ADPCM\_{}STEREO}{MIDASsampleTypes} as the
sample type to the MIDAS stream playback functions, and everything will work
normally. ADPCM sample data can only be used for streams. If you are feeding
the stream data manually, however, remember that playback can only start from
the beginning of an ADPCM frame.



\chapter{Operating system specific information}

Although the normal MIDAS Digital Audio System APIs are indentical in all
supported platforms, there are some operating system specific points that
should be noted. In particular, the limitations of the MS-DOS operating system
make it somewhat difficult to program under.


\section{Using DirectSound} %K:DirectSound;DirectSound,Using

Beginning from version 0.7, MIDAS Digital Audio System supports DirectSound
for sound output.  Although most of the time this is done completely
transparently to the user, there are some decisions that need to be made in
the initialization phase.


\subsection{Initialization} %K:DirectSound;DirectSound,Initialization

By default, DirectSound support in MIDAS Digital Audio System is disabled. To
enable it, set
\mApiRefLabel{MIDAS\_{}OPTION\_{}\_DSOUND\_{}MODE}{MIDASoptions} to a value
other than \mApiRefLabel{MIDAS\_{}DSOUND\_{}DISABLED}{MIDASdsoundModes}.  The
DirectSound mode you choose depends on the needs of your application, and the
available modes are described in detail in the next section.

In addition to the DirectSound mode, you also need to set the window handle
that MIDAS will in turn pass to DirectSound. DirectSound uses this window
handle to determine the active window, as only the sound played by the active
application will be heard. To set the window handle, simply call
\begin{verbatim}
        MIDASsetOption(MIDAS_OPTION_DSOUND_HWND, (DWORD) hwnd);
\end{verbatim}
where \mVariable{hwnd} is the window handle of your application's main
window.


\subsection{DirectSound modes} %K:DirectSound;DirectSound,Modes

Apart from \mApiRefLabel{MIDAS\_{}DSOUND\_{}DISABLED}{MIDASdsoundModes}, three
different DirectSound modes are available in MIDAS Digital Audio System. This
section describes them in detail.

{\bf MIDAS\_{}DSOUND\_{}STREAM}: DirectSound stream mode. MIDAS will play its
sound to a DirectSound stream buffer, which will then be mixed to the primary
buffer by DirectSound. If the DirectSound object hasn't explicitly been set,
MIDAS will initialize DirectSound and set the primary buffer format to the
same as MIDAS output format. This mode allows arbitrary buffer length, and
possibly smoother playback than primary buffer mode, but has a larger CPU
overhead.

{\bf MIDAS\_{}DSOUND\_{}PRIMARY}: DirectSound primary buffer mode. The sound
data will be played directly to the DirectSound primary buffer. This mode has
the smallest CPU overhead of all available DirectSound modes, and provides
smallest possible latency, but is not without its drawbacks: The primary
buffer size is set by the driver, and cannot be changed, so the buffer size
may be limited.

{\bf MIDAS\_{}DSOUND\_{}FORCE\_{}STREAM}: This mode behaves exactly like the
stream mode, except that DirectSound usage is forced. Normally, MIDAS will
not use DirectSound if it is running in emulation mode (as the standard
Windows WAVE output device will provide better performance), so this mode
must be used to force DirectSound usage. Forcing MIDAS to use DirectSound in
stream mode will also the applications to use DirectSound themselves
simultaneously.

By default, MIDAS an automatical fallback mechanism for DirectSound modes: If
DirectSound support is set to primary mode, but primary buffer is not
available for writing, MIDAS will use stream mode instead. And, if
DirectSound is running in emulation mode, MIDAS will automatically use normal
Win32 audio services instead. This way it is possible to simply set the
desired DirectSound mode, and let MIDAS decide the best of the alternatives
available.


\subsection{Buffer sizes} %K:DirectSound;DirectSound,Buffer sizes;Buffers

When MIDAS Digital Audio System is using DirectSound with proper drivers
(ie. not in emulation mode), much smaller buffer sizes can be used than
normal. Because of this, the DirectSound buffer size is set with a different
option setting ---
\mApiRefLabel{MIDAS\_{}OPTION\_{}DSOUND\_{}BUFLEN}{MIDASoptions} --- from the
normal mixing buffer length.  When playing in emulation mode, MIDAS will use
the normal mixing buffer length, as smaller buffers can't be used as reliably.

Selecting the correct buffer size is a compromise between sound latency and
reliability: the longer the buffer is, the greater latency the sound has, and
the longer it takes the sound to actually reach output, but the smaller the
buffer is made, the more often the music player needs to be called. To ensure
that there are no breaks in sound playback, the music player needs to be
called at least twice, preferably four times, during each buffer length: for
a 100ms buffer, for example, the sound player needs to be called at least
every 50ms, or 20 times per second.

Although the calling frequency requirements don't seem to be very severe, in
practise trying to guarantee that a function gets called even 20 times per
second can be difficult. The realtime capabilities of the Win32 platform,
especially Windows 95, leave a lot to be desired: A 16-bit program or system
service can easily block the system for long periods of time. By default,
MIDAS uses a separate thread for background playback, but although this
thread runs at a higher priority than the rest of the program, you may find
that using manual polling will help you get more consistent and reliable
sound playback.

Unfortunately there is no single buffer size that works for everybody, so
some experimentation will be needed. The default MIDAS DirectSound buffer
size is 100ms, which should be a reasonable compromise for most applications,
but, depending on your applications, buffer sizes at 50ms or below should be
usable.


\subsection{Using other DirectSound services with MIDAS} %K:DirectSound;\
                         %K:DirectSound,Using other DirectSound services


If necessary, it is also possible to use other DirectSound services
simultaneously with MIDAS Digital Audio System. In this case, MIDAS should be
set to use DirectSound in forced stream mode, and the DirectSound object needs
to be explicitly given to MIDAS before initialization:
\begin{verbatim}
        MIDASsetOption{MIDAS_OPTION_DSOUND_OBJECT, (DWORD) ds);
\end{verbatim}
Where \mVariable{ds} is a pointer to the DirectSound object used, returned by
{\em DirectSoundCreate()}. The user is also responsible for setting
DirectSound cooperative level and primary buffer format.

Although this DirectSound usage is not recommended, it can be used, for
example, to play music with MIDAS while using the DirectSound services
directly for playing sound effects.


\subsection{When to use DirectSound?} %K:DirectSound;\
                                      %K:DirectSound,When to use?

Although DirectSound provides a smaller latency than the normal Windows sound
devices, and possibly smaller CPU overhead, it is not suitable of all
applications. This section gives a quick overview on what applications should
use DirectSound, what shouldn't, and which DirectSound mode is most
appropriate.

The most important drawback of DirectSound is, that only the active
application gets its sound played. While this can be useful with games that
run fullscreen, it makes DirectSound completely unusable for applications
such as music players, as background playback is impossible with DirectSound.
Therefore standalone music player programs should never use DirectSound.

Also, if your application does not benefit from the reduced latency that
DirectSound provides, it is safer not to use DirectSound. The DirectSound
drivers currently available are not very mature, and the DirectX setup
included in the DirectX SDK is far from trouble-free. In addition, programs
using DirectSound need to distribute the DirectX runtime with them, making
them considerably larger.

However, if you are writing an interactive high-performance application,
where strict graphics and sound synchronization is essential, DirectSound is
clearly the way to go. For these kind of applications, DirectSound primary
buffer should be the best solution, unless there are clear reasons for using
stream mode.


\subsection{DirectSound and multiple windows} %K:DirectSound;\
                                              %K:DirectSound,Multiple windows

When the application uses DirectSound for sound output, only the sound from
the active window is played. Therefore DirectSound requires a window handle
to be able to determine which window is active. If the application has
multiple windows that it needs to activate separately, however, this can
cause problems. DirectSound provides no documented way to change the window
handle on the fly.

To get around this problem, MIDAS Digital Audio System provides two functions
to suspend and resume playback: \mApiRef{MIDASsuspend} and
\mApiRef{MIDASresume}.  \mApiRef{MIDASsuspend} stops all sound playback,
uninitializes the sound output device, and returns it to the operating system.
\mApiRef{MIDASresume} in turn resumes sound playback after suspension. These
functions can be used to change the DirectSound window handle on the fly:
First call \mApiRef{MIDASsuspend}, set the new window handle, and call
\mApiRef{MIDASresume} to resume playback. This will cause a break to the
sound, and the sound data currently buffered to the sound output device will
be lost.

Depending on the application, it may also be possible to get around the
DirectSound multiple window problem by creating a hidden parent window for
all windows that will be used, and pass the window handle of that parent
window to DirectSound.



\section{MS-DOS timer callbacks} %K:Timer;Timer,MS-DOS timer callbacks

This section describes how MIDAS Digital Audio System uses the MS-DOS system
timer, and how to install user timer callbacks. This information is not
relevant in other operating systems.


\subsection{Introduction} %K:Timer;Timer,MS-DOS timer callbacks;\
                          %K:Callbacks;Callbacks,MS-DOS timer

To be able to play music in the background in MS-DOS, and to keep proper
tempo with all sound cards, MIDAS needs to use the system timer (IRQ 0,
interrupt 8) for music playback. Because of this, user programs may not
access the timer directly, as this would cause conflicts with MIDAS
music playback. As the system timer is often used for controlling
program speed, and
running some tasks in the background, MIDAS provides a separate user
timer callback for these purposes. This callback should used instead of
accessing the timer hardware directly.

The callbacks can be ran at any speed, and can optionally be
synchronized to display refresh. 


\subsection{Using timer callbacks} %K:Timer;Timer,MS-DOS timer callbacks;\
                                   %K:Callbacks;Callbacks,MS-DOS timer

Basic MIDAS timer callback usage is very simple: Simply call 
\mApiRef{MIDASsetTimerCallbacks}, passing it the desired callback rate
and pointers to the callback functions. After that, the callback
functions will be called periodically until 
\mApiRef{MIDASremoveTimerCallbacks} is
called. \mApiRef{MIDASsetTimerCallbacks} takes the callback rate in
milliHertz (one thousandth of a Hertz) units, so to get a callback that 
will be called 70 times per second, set the rate to 70000. The callback
functions need to use \mVariable{MIDAS\_{}CALL} calling convention
(\mResWord{\_{}\_{}cdecl} for Watcom C, empty for DJGPP), take no
arguments and return no value.

For example, this code snippet will use the timer to increment a variable
\mVariable{tickCount} 100 times per second:

\begin{verbatim}
    void MIDAS_CALL TimerCallback(void)
    {
        tickCount++;
    }
    ...
    MIDASinit();
    ...
    MIDASsetTimerCallbacks(100000, FALSE, &TimerCallback, NULL, NULL);
    ...
\end{verbatim}




\subsection{Synchronizing to display refresh} %K:Timer;\
                                   %K:Timer,MS-DOS timer callbacks;\
                                   %K:Callbacks;Callbacks,MS-DOS timer

The MIDAS timer supports synchronizing the user callbacks to display
refresh under some circumstances. Display synchronization does not work
when running under Windows 95 and similar systems, and may fail in SVGA
modes with many SVGA cards. As display synchronization is somewhat
unreliable, and also more difficult to use than normal callbacks, using
it is not recommended if a normal callback is sufficient.

To synchronize the timer callbacks to screen refresh, use the following
procedure: 

1. {\bf BEFORE} MIDAS Digital Audio System is initialized, set up the display
mode you are going to use, and get the display refresh rate corresponding to
that mode using \mApiRef{MIDASgetDisplayRefreshRate}. If your application uses
several different display modes, you will need to set up each of them in turn
and read the refresh rate for each separately. If
\mApiRef{MIDASgetDisplayRefreshRate} returns zero, it was unable to determine
the display refresh rate, and you should use some default value
instead. Display refresh rates, like timer callback rates, are specified in
milliHertz (1000*Hz), so 70Hz refresh rate becomes 70000.

2. Initialize MIDAS Digital Audio System etc.

3. Set up the display mode

4. Start the timer callbacks by calling
\mApiRef{MIDASsetTimerCallbacks}. The first argument is the refresh
rate from step 1, second argument should be set to TRUE (to enable display
synchronization), and the remaining three arguments are pointers to the
\mVariable{preVR}, \mVariable{immVR} and \mVariable{inVR} callback
functions (see descriptions below).

5. When the callbacks are no longer used, remove them by calling 
\mApiRef{MIDASremoveTimerCallbacks}.

When you are changing display modes, you must first remove the existing
timer callbacks, change the display modes, and restart the callbacks
with the correct rate. Please note that synchronizing the timer to
the screen update takes a while, and as the timer is disabled for that time
it may introduce breaks in the music. Therefore we suggest you handle the
timer screen synchronization before you start playing music.

If MIDAS is unable to synchronize the timer to display refresh, it will
simply run the callbacks like normal user callbacks. Therefore there is
no guarantee that the callbacks will actually get synchronized to
display, and your program should not depend on that. For example, you
should not use the timer callbacks for double buffering the display, as
\mVariable{preVR} might not be called at the correct time --- use triple
buffering instead to prevent possible flicker.


\subsection{The callback functions} %K:Timer;Timer,MS-DOS timer callbacks;\
                                   %K:Callbacks;Callbacks,MS-DOS timer

\mApiRef{MIDASsetTimerCallbacks} takes as its three last arguments three
pointers to the timer callback functions. These functions are:

{\em preVR()} --- if the callbacks are synchronized to display refresh,
this function is called immediately {\bf before} Vertical Retrace
starts. It should be kept as short as possible, and can be used for
changing a couple of hardware registers (in particular the display start
address) or updating a counter.

{\em immVR()} --- if the callbacks are synchronized to display refresh,
this function is called immediately after Vertical Retrace starts. As
{\em preVR()}, it should be kept as short as possible.

{\em inVR()} --- if the callbacks are synchronized to display refresh,
this function is called some time later during Vertical Retrace. It can
take a longer time than the two previous functions, and can be used, for
example, for setting the VGA palette. It should not take longer than a
quarter of the time between callbacks though.

If the callbacks are not synchronized to display refresh, the functions
are simply called one after another. The same timing requirements still
hold though.


\subsection{Framerate control} %K:Timer;Timer,MS-DOS timer callbacks;\
                               %K:Callbacks;Callbacks,MS-DOS timer

DOS programs typically control their framerate by checking the Vertical
Retrace from the VGA hardware. If MIDAS is playing music in the
background, this is not a good idea, since the music player can cause
the program to miss retraces. Instead, the program should set up a timer
callback, possibly synchronize it to display refresh, use that callback
to increment a counter, and wait until the counter changes.

For example:

\begin{verbatim}
    volatile unsigned frameCount;
    ...
    void MIDAS_CALL PreVR(void)
    {
        frameCount++;
    }
    ...
    MIDASsetTimerCallbacks(70000, FALSE, &PreVR, NULL, NULL);
    ...
    while ( !quit )
    {
       DoFrame();
       oldCount = frameCount;
       while ( frameCount == oldCount );
    }
\end{verbatim}

Note that \mVariable{frameCount} needs to be declared {\bf volatile},
otherwise the compiler might optimize the wait completely away.

A similar strategy can be used to keep the program run at the same speed
on different computers. You can use the frame counter to determine how
many frames rendering the display takes, and run the movements for all
those frames before rendering the next display.



\end{document}


%eq
