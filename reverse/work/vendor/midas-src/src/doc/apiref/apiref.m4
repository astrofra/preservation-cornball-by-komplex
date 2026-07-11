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
\title{MIDAS Digital Audio System\\ \ \\ API Reference}

\begin{document}

\maketitle
\pagenumbering{roman}
\tableofcontents

\newpage
\pagenumbering{arabic}

% ***************************************************************************
% *
% *     INTRODUCTION
% *
% ***************************************************************************

\chapter{Introduction}


% ---------------------------------------------------------------------------
%       About this document
% ---------------------------------------------------------------------------

\section{About this document}

This document contains a full programmer's reference for the MIDAS
Application Programming interface. It includes complete descriptions of all
constants, data structure and functions available in the API, plus examples
on how to use them. It is not intended to be a tutorial on using MIDAS ---
for that kind of information see
\htmladdnormallink{MIDAS Programmer's Guide}{../prgguide/prgguide.html}.


% ---------------------------------------------------------------------------
%       Document organization
% ---------------------------------------------------------------------------

\section{Document organization}

The document itself is divided into seven different chapters, according to
different functional groups. In addition to this introduction, the chapters
cover configuration and initialization, overall system control, module
playback, sample playback, stream playback, and miscellaneous system
functions. Each chapter is further divided into three sections: constants,
data types and functions.




% ***************************************************************************
% *
% *     CONFIGURATION, INITIALIZATION AND CONTROL
% *
% ***************************************************************************

\chapter{Configuration, initialization and control}


% ---------------------------------------------------------------------------
%       Constants
% ---------------------------------------------------------------------------

\section{Constants}

This section describes all constants used in MIDAS initialization and
configuration. They are grouped according to the \mResWord{enum} used to
define them.


%----------------------------- MIDASdsoundModes
\mDocEntry{MIDASdsoundModes} %K:MIDAS_OPTION_DSOUND_MODE;DirectSound;\
                             %K:DirectSound,Modes;Modes;Modes,DirectSound
\begin{verbatim}
enum MIDASdsoundModes
\end{verbatim}

\subsubsection*{Description}

These constants are used to describe different MIDAS DirectSound usage modes.
By default MIDAS does not use DirectSound at all, and DirectSound usage can
be enabled by setting
\mDocRefLabel{MIDAS\_{}OPTION\_{}DSOUND\_{}MODE}{MIDASoptions}. Note that
\mDocRefLabel{MIDAS\_{}OPTION\_{}DSOUND\_{}HWND}{MIDASoptions} needs to
be set when using DirectSound. A complete discussion of using DirectSound with
MIDAS is available at \htmladdnormallink{MIDAS Programmer's
Guide}{../prgguide/prgguide.html}.

\subsubsection*{Values}

\begin{description}
    \item [MIDAS\_{}DSOUND\_{}DISABLED] DirectSound usage is disabled
    \item [MIDAS\_{}DSOUND\_{}STREAM] DirectSound is used in stream mode --
        MIDAS will play to a DirectSound stream buffer. DirectSound usage is
        disabled if DirectSound runs in emulation mode.
    \item [MIDAS\_{}DSOUND\_{}PRIMARY] DirectSound is used in primary buffer
        mode if possible -- MIDAS will play directly to DirectSound primary
        buffer. If primary buffer is not available for writing, this mode
        behaves like MIDAS\_{}DSOUND\_{}STREAM.
    \item [MIDAS\_{}DSOUND\_{}FORCE\_{}STREAM] Behaves like
        MIDAS\_{}DSOUND\_{}STREAM, except that DirectSound is used always,
        even in emulation mode.
\end{description}




%----------------------------- MIDASfilterModes
\mDocEntry{MIDASfilterModes} %K:MIDAS_OPTION_FILTER_MODE;Filter;\
                             %K:Filter,Modes;Modes;Modes,Filter
\begin{verbatim}
enum MIDASfilterModes
\end{verbatim}

\subsubsection*{Description}

These constants are used to describe different MIDAS output filter modes.
By default, MIDAS Digital Audio System will select the most appropriate filter
for the mixing mode automatically, but in some cases manually overriding the
selection can result in better sound. Also, if all sounds are played exactly
at the mixing rate, the output filtering should be disabled to get better
sound quality. Finally, filtering is not normally used with high-quality
mixing, and is therefore disabled with it by default.

The filter mode can be set by changing the option 
\mDocRefLabel{MIDAS\_{}OPTION\_{}FILTER\_{}MODE}{MIDASoptions} with
the function \mDocRef{MIDASsetOption}.


\subsubsection*{Values}

\begin{description}
    \item [MIDAS\_{}FILTER\_{}NONE] No filtering
    \item [MIDAS\_{}FILTER\_{}LESS] Some filtering
    \item [MIDAS\_{}FILTER\_{}MORE] More filtering
    \item [MIDAS\_{}FILTER\_{}AUTO] Automatic filter selection (default)
\end{description}




%----------------------------- MIDASmixingModes
\mDocEntry{MIDASmixingModes} %K:MIDAS_OPTION_MIXING_MODE;Mixing modes;\
                             %K:Modes;Modes,Mixing
\begin{verbatim}
enum MIDASmixingModes
\end{verbatim}

\subsubsection*{Description}

These constants define the different MIDAS Digital Audio System mixing modes
available. The high-quality mixing mode yields better sound quality, with
virtually no aliasing noise, but uses much more CPU power than normal mixing,
and is mainly intended for stand-alone module players and similar
applications. By default, MIDAS uses the normal mixing quality.

The mixing mode can be set by changing the option 
\mDocRefLabel{MIDAS\_{}OPTION\_{}MIXING\_{}MODE}{MIDASoptions} with
the function \mDocRef{MIDASsetOption}.


\subsubsection*{Values}

\begin{description}
    \item [MIDAS\_{}MIX\_{}NORMAL\_{}QUALITY] Normal quality mixing
    \item [MIDAS\_{}MIX\_{}HIGH\_{}QUALITY] High-quality interpolating mixing
\end{description}




%----------------------------- MIDASmodes
\mDocEntry{MIDASmodes} %K:MIDAS_OPTION_OUTPUTMODE;Output modes;Modes;Modes,Output
\begin{verbatim}
enum MIDASmodes
\end{verbatim}

\subsubsection*{Description}

These constants are used to describe different MIDAS output modes. They are
used with the function \mDocRef{MIDASsetOption}, when changing the setting
\mDocRefLabel{MIDAS\_{}OPTION\_{}OUTPUTMODE}{MIDASoptions}.

\subsubsection*{Values}

\begin{description}
    \item [MIDAS\_{}MODE\_{}8BIT\_{}MONO] 8-bit mono output
    \item [MIDAS\_{}MODE\_{}16BIT\_{}MONO] 16-bit mono output
    \item [MIDAS\_{}MODE\_{}8BIT\_{}STEREO] 8-bit stereo output
    \item [MIDAS\_{}MODE\_{}16BIT\_{}STEREO] 16-bit stereo output
\end{description}




%----------------------------- MIDASoptions
\mDocEntry{MIDASoptions} %K:Options;MIDAS_OPTION_MIXRATE;MIDAS_OPTION_OUTPUTMODE;MIDAS_OPTION_MIXBUFLEN;MIDAS_OPTION_MIXBUFBLOCKS;MIDAS_OPTION_DSOUND_MODE;MIDAS_OPTION_DSOUND_HWND;MIDAS_OPTION_DSOUND_OBJECT;MIDAS_OPTION_DSOUND_BUFLEN;MIDAS_OPTION_FILTER_MODE;MIDAS_OPTION_MIXING_MODE;MIDAS_OPTION_DEFAULT_STEREO_SEPARATION;MIDAS_OPTION_FORCE_NO_SOUND
\begin{verbatim}
enum MIDASoptions
\end{verbatim}

\subsubsection*{Description}

These constants are used with the function \mDocRef{MIDASsetOption}
to change different MIDAS configuration options, and \mDocRef{MIDASgetOption}
to query their current settings.

\subsubsection*{Values}

\begin{description}
    \item [MIDAS\_{}OPTION\_{}MIXRATE] Output mixing rate
    \item [MIDAS\_{}OPTION\_{}OUTPUTMODE] Output mode, see \mResWord{enum}
        \mDocRef{MIDASmodes}
    \item [MIDAS\_{}OPTION\_{}MIXBUFLEN] Mixing buffer length, in
        milliseconds
    \item [MIDAS\_{}OPTION\_{}MIXBUFBLOCKS] The number of blocks the buffer
        should be divided into
    \item [MIDAS\_{}OPTION\_{}DSOUND\_{}MODE] DirectSound mode to use, see
        \mResWord{enum} \mDocRef{MIDASdsoundModes}
    \item [MIDAS\_{}OPTION\_{}DSOUND\_{}HWND] Window handle for DirectSound
        support. The window handle is used by DirectSound to determine which
        window has the focus. The window handle has to be set when using
        DirectSound.
    \item [MIDAS\_{}OPTION\_{}DSOUND\_{}OBJECT] The DirectSound object that
        should be used. Setting this option forces DirectSound support on.
    \item [MIDAS\_{}OPTION\_{}DSOUND\_{}BUFLEN] Output buffer
        length fot DirectSound, in milliseconds.
        This option is used instead of MIDAS\_{}OPTION\_{}MIXBUFLEN
        when using DirectSound without emulation.
    \item [MIDAS\_{}OPTION\_{}16BIT\_{}ULAW\_{}AUTOCONVERT] Controls
        whether 16-bit samples will be automatically converted to u-law or
        not. Enabled by default. The autoconversion only applies to
        Sound Devices which can natively play u-law format data, and will only
        be used if it results in smaller CPU use.
    \item [MIDAS\_{}OPTION\_{}FILTER\_{}MODE] Output filter mode, see
        \mResWord{enum} \mDocRef{MIDASfilterModes}. The filter is selected
        automatically by default.
    \item [MIDAS\_{}OPTION\_{}MIXING\_{}MODE] Mixing mode, affects the output
        sound quality. See \mResWord{enum} \mDocRef{MIDASmixingModes}.
    \item [MIDAS\_{}OPTION\_{}DEFAULT\_{}STEREO\_{}SEPARATION] Controls the
        default stereo separation for modules with no panning information
        (MODs and old S3Ms). 64 is maximum stereo separation, 0 none. Default
        64.
    \item [MIDAS\_{}OPTION\_{}FORCE\_{}NO\_{}SOUND] Forces the No Sound Sound
        Device to be used for playback. Useful for trying to run MIDAS with no
        sound if \mDocRef{MIDASinit} fails.
\end{description}



% ---------------------------------------------------------------------------
%       Data types
% ---------------------------------------------------------------------------

\newpage
\section{Data types}

This section describes all data types used in MIDAS initialization and
configuration.




% ---------------------------------------------------------------------------
%       Functions
% ---------------------------------------------------------------------------

\newpage
\section{Functions}

This section describes all functions available for MIDAS initialization
and configuration.




%----------------------------- MIDASclose
\mDocEntry{MIDASclose}
\begin{verbatim}
BOOL MIDASclose(void)
\end{verbatim}

Uninitializes MIDAS Digital Audio System.

\subsubsection*{Input}

None.

\subsubsection*{Description}

This function uninitializes all MIDAS Digital Audio System components,
deallocates all resources allocated, and shuts down all MIDAS processing. This
function must always be called before exiting under MS-DOS and is also
strongly recommended under other operating systems. After this function has
been called, no MIDAS function may be called unless MIDAS is initialized
again.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

All

\subsubsection*{See also}

\mDocRef{MIDASinit}




%----------------------------- MIDASconfig
\mDocEntry{MIDASconfig} %K:Setup;Config
\begin{verbatim}
BOOL MIDASconfig(void)
\end{verbatim}

Runs manual MIDAS setup.

\subsubsection*{Input}

None.


\subsubsection*{Description}

This function runs the manual MIDAS Digital Audio System configuration. It
queries the sound card to use and desired sound quality and output mode from
the user. The setup entered can be saved do disk with
\mDocRef{MIDASsaveConfig} and loaded back with \mDocRef{MIDASloadConfig}, or
written to registry with \mDocRef{MIDASwriteConfigRegistry} and read back with
\mDocRef{MIDASreadConfigRegistry}.  These functions can be used to create a
simple external setup program, or just save the settings between two runs of
the program. After this function has been called, \mDocRef{MIDASsetOption} can
be used to change the output mode options, to, for example, force mono output.

This function returns TRUE if the setup was completed successfully, FALSE if
not. The setup can fail for two reasons: either the user aborted it by
pressing escape or clicking Cancel, or an error occured. As errors during the
setup are extremely unlikely, it is safe to simply exit the program if this
function returns FALSE. \mDocRef{MIDASgetLastError} can be used to check if an
error occured --- if the return value is zero, the user just pressed cancelled
the setup.

This function must be called before \mDocRef{MIDASinit}, but after
\mDocRef{MIDASstartup}. 

\subsubsection*{Return value}

TRUE if successful, FALSE if not (the user cancelled the configuration, or an
error occurred)

\subsubsection*{Operating systems}

MS-DOS, Win32

\subsubsection*{See also}

\mDocRef{MIDASsaveConfig}, \mDocRef{MIDASloadConfig},
\mDocRef{MIDASsetOption}, \mDocRef{MIDASgetOption}, \mDocRef{MIDASinit},
\mDocRef{MIDASwriteConfigRegistry}, \mDocRef{MIDASreadConfigRegistry}




%----------------------------- MIDASdetectSoundCard
\mDocEntry{MIDASdetectSoundCard} %K:Detection;Autodetection
\begin{verbatim}
BOOL MIDASdetectSoundCard(void)
\end{verbatim}

Attempts to detect the sound card to use.

\subsubsection*{Input}

None.

\subsubsection*{Description}

[MS-DOS only]

This function attempts to detect the sound card that should be used. It
will set up MIDAS to use the detected card, and return TRUE if a sound
card was found, FALSE if not. If this function returns FALSE, you should
run \mDocRef{MIDASconfig} to let the user manually select the sound
card. Note that you {\bf can} use MIDAS even if no sound card has been
selected - MIDAS will just not play sound in that case.

If no sound card has been manually set up, \mDocRef{MIDASinit} will
automatically detect it, or use No Sound if none is available. Therefore
this function does not have to be called if manual setup will not be
used.

Note that, as there is no way to safely autodetect the Windows Sound
System cards under MS-DOS, MIDAS will not attempt to detect them at all. If
you do not provide a manual setup possibility to your program (via
\mDocRef{MIDASconfig}), WSS users will not be able to get any sound. The
computer may also have several sound cards, and the user may wish not to
use the one automatically detected by MIDAS. Therefore it is a very good
idea to include an optional manual sound setup to all programs.

This discussion naturally applies to MS-DOS only, under Win32 and Linux
MIDAS uses the sound card through the system audio devices, and no sound
card selection or setup is necessary.

\subsubsection*{Return value}

TRUE if a sound card was detected, FALSE if not.

\subsubsection*{Operating systems}

MS-DOS

\subsubsection*{See also}

\mDocRef{MIDASconfig}, \mDocRef{MIDASinit}



%----------------------------- MIDASgetDisplayRefreshRate
\mDocEntry{MIDASgetDisplayRefreshRate} %K:Timer;Timer,MIDASgetDisplayRefreshRate
\begin{verbatim}
DWORD MIDASgetDisplayRefreshRate(void)
\end{verbatim}

Gets the current display refresh rate.

\subsubsection*{Input}

None.

\subsubsection*{Description}

This function tries to determine the current display refresh rate. It is
used with \mDocRef{MIDASsetTimerCallbacks} to set a display-synchronized
timer callback. It returns the current display refresh rate in
milliHertz (ie. 1000*Hz, 50Hz becomes 50000, 70Hz 70000 etc), or 0 if it
could not determine the refresh rate. The refresh rate may be
unavailable when running under Win95 or a similar OS, or when the VGA
card does return Vertical Retraces correctly (as some SVGA cards do in
SVGA modes). Therefore it is important to check the return value, and
substitute some default value if it is zero.

Unlike most other MIDAS functions, this function must be called {\bf
before} \mDocRef{MIDASinit} is called, as the MIDAS timer may interfere
with the measurements.

Note that the display refresh rate is {\bf display mode
specific}. Therefore you need to set up the display mode with which you
want to use display-synchronized timer callbacks {\bf before} calling
this function. Also, if your application uses several display modes, you
must get the display refresh rate for each mode separately, and remove
and restart the display-synchronized timer callbacks at each mode
change. 

This function is only available in MS-DOS.

\subsubsection*{Return value}

The current display refresh rate, in milliHertz, or 0 if unavailable.

\subsubsection*{Operating systems}

MS-DOS

\subsubsection*{See also}

\mDocRef{MIDASsetTimerCallbacks}



%----------------------------- MIDASgetOption
\mDocEntry{MIDASgetOption} %K:Options;Initialization
\begin{verbatim}
DWORD MIDASgetOption(int option)
\end{verbatim}

Gets a MIDAS option.

\subsubsection*{Input}

\begin{description}
    \item [option] Option number (see \mResWord{enum}
        \mDocRef{MIDASoptions} above)
\end{description}


\subsubsection*{Description}

This function reads the current value of a MIDAS option. The different number
codes for different options are described above.

\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

All

\subsubsection*{See also}

\mDocRef{MIDASsetOption}




%----------------------------- MIDASinit
\mDocEntry{MIDASinit} %K:Initialization
\begin{verbatim}
BOOL MIDASinit(void)
\end{verbatim}

Initializes MIDAS Digital Audio System.

\subsubsection*{Input}

None.

\subsubsection*{Description}

This function initializes all MIDAS Digital Audio System components, and sets
up the API. Apart from configuration functions, this function must be called
before any other MIDAS functions are used.

If this function fails with no apparent reason, it is very probable that some
other application is using the sound card hardware or the user has badly
mis-configured MIDAS. Therefore, if this function fails, it is recommended to
give the user a possibility to close other applications and retry, continue
with no sound (set option
\mDocRefLabel{MIDAS\_{}OPTION\_{}FORCE\_{}NO\_{}SOUND}{MIDASoptions}), or
possibly re-configure MIDAS.

\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

All

\subsubsection*{See also}

\mDocRef{MIDASsetOption}, \mDocRef{MIDASclose}



%----------------------------- MIDASloadConfig
\mDocEntry{MIDASloadConfig} %K:Setup;Config
\begin{verbatim}
BOOL MIDASloadConfig(char *fileName)
\end{verbatim}

Load MIDAS setup from disk.

\subsubsection*{Input}

\begin{description}
    \item [fileName] Setup file name
\end{description}


\subsubsection*{Description}

This function loads MIDAS setup from disk. The setup must have been
saved with \mDocRef{MIDASsaveConfig}. \mDocRef{MIDASsetOption} can be
used afterwards to change, for example, the output mode.

This function must be called before \mDocRef{MIDASinit}, but after
\mDocRef{MIDASstartup}.

\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

MS-DOS, Win32

\subsubsection*{See also}

\mDocRef{MIDASconfig}, \mDocRef{MIDASsaveConfig}





%----------------------------- MIDASreadConfigRegistry
\mDocEntry{MIDASreadConfigRegistry} %K:Setup;Config
\begin{verbatim}
BOOL MIDASreadConfigRegistry(DWORD key, char *subKey);
\end{verbatim}

Reads MIDAS configuration from a registry key.

\subsubsection*{Input}

\begin{description}
    \item [key] A currently open registry key, for example 
        \mVariable{H\_{}KEY\_{}CURRENT\_{}USER}
    \item [subKey] The name of the subkey of \mVariable{key} that contains the
        configuration
\end{description}


\subsubsection*{Description}

This function reads the MIDAS configuration from a registry key. The
configuration must have been written there with
\mDocRef{MIDASwriteConfigRegistry}. \mDocRef{MIDASsetOption} can be used
afterwards to chance, for example, the output mode, and \mDocRef{MIDASconfig}
to prompt the user for new settings.

This function must be called before \mDocRef{MIDASinit}, but after
\mDocRef{MIDASstartup}.

\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

Win32

\subsubsection*{See also}

\mDocRef{MIDASconfig}, \mDocRef{MIDASwriteConfigRegistry}





%----------------------------- MIDASsaveConfig
\mDocEntry{MIDASsaveConfig} %K:Setup;Config
\begin{verbatim}
BOOL MIDASsaveConfig(char *fileName)
\end{verbatim}

Saves the MIDAS setup to a file.

\subsubsection*{Input}

\begin{description}
    \item [fileName] Setup file name
\end{description}


\subsubsection*{Description}

This function saves the MIDAS setup entered with \mDocRef{MIDASconfig}
to a file on disk. It can be then loaded with \mDocRef{MIDASloadConfig}.

\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

MS-DOS, Win32

\subsubsection*{See also}

\mDocRef{MIDASconfig}, \mDocRef{MIDASloadConfig}




%----------------------------- MIDASsetOption
\mDocEntry{MIDASsetOption} %K:Options;Initialization
\begin{verbatim}
BOOL MIDASsetOption(int option, DWORD value)
\end{verbatim}

Sets a MIDAS option.

\subsubsection*{Input}

\begin{description}
    \item [option] Option number (see \mResWord{enum}
        \mDocRef{MIDASoptions} above)
    \item [value] New value for option
\end{description}


\subsubsection*{Description}

This function sets a value to a MIDAS option. The different number codes for
different options are described above. All MIDAS configuration options must
be set with this function {\bf before} \mDocRef{MIDASinit} is called.

\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

All

\subsubsection*{See also}

\mDocRef{MIDASinit}




%----------------------------- MIDASstartup
\mDocEntry{MIDASstartup} %K:Initialization
\begin{verbatim}
BOOL MIDASstartup(void)
\end{verbatim}

Prepares MIDAS Digital Audio System for initialization and use.

\subsubsection*{Input}

None.

\subsubsection*{Description}

This function sets all MIDAS Digital Audio System configuration variables to
default values and prepares MIDAS for use. It must be called before any other
MIDAS function, including \mDocRef{MIDASinit} and \mDocRef{MIDASsetOption}, is
called. After this function has been called, \mDocRef{MIDASclose} can be
safely called at any point and any number of times, regardless of whether
MIDAS has been initialized or not. After calling this function, you can use
\mDocRef{MIDASsetOption} to change MIDAS configuration before initializing
MIDAS with \mDocRef{MIDASinit}, or call \mDocRef{MIDASconfig} to prompt the
user for configuration options.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

All

\subsubsection*{See also}

\mDocRef{MIDASsetOption}, \mDocRef{MIDASinit}, \mDocRef{MIDASclose}




%----------------------------- MIDASwriteConfigRegistry
\mDocEntry{MIDASwriteConfigRegistry} %K:Setup;Config
\begin{verbatim}
BOOL MIDASwriteConfigRegistry(DWORD key, char *subKey);
\end{verbatim}

Writes the MIDAS configuration to a registry key.

\subsubsection*{Input}

\begin{description}
    \item [key] A currently open registry key, for example 
        \mVariable{H\_{}KEY\_{}CURRENT\_{}USER}
    \item [subKey] The name of the subkey of \mVariable{key} where the
        configuration will be written. They key does not need to exist.
\end{description}


\subsubsection*{Description}

This function saves the current MIDAS configuration, usually entered with
\mDocRef{MIDASconfig} to a registry key. The configuration can then be read
with \mDocRef{MIDASreadConfigRegistry}.

\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

Win32

\subsubsection*{See also}

\mDocRef{MIDASconfig}, \mDocRef{MIDASreadConfigRegistry}






% ***************************************************************************
% *
% *     SYSTEM CONTROL
% *
% ***************************************************************************

\chapter{System control}


% ---------------------------------------------------------------------------
%       Constants
% ---------------------------------------------------------------------------

\section{Constants}

This section describes all constants used in MIDAS system control. 
They are grouped according to the \mResWord{enum} used to
define them.


% ---------------------------------------------------------------------------
%       Data types
% ---------------------------------------------------------------------------

\newpage
\section{Data types}

This section describes all data types used in MIDAS system control.




% ---------------------------------------------------------------------------
%       Functions
% ---------------------------------------------------------------------------

\newpage
\section{Functions}

This section describes all functions available for MIDAS system
control. This includes error handling.




%----------------------------- MIDASallocateChannel
\mDocEntry{MIDASallocateChannel} %K:Channels;Channels,MIDASallocateChannels
\begin{verbatim}
DWORD MIDASallocateChannel(void)
\end{verbatim}

Allocates a single Sound Device channel.


\subsubsection*{Input}

None.


\subsubsection*{Description}

This function allocates a single Sound Device channel, and returns its
number. Sound Device are used for all sound playback, but most functions
take care of allocating and deallocating channels automatically. If you
wish to play a sample on a specific channel, to ensure it won't be
replaced by other samples, you'll need to pass \mDocRef{MIDASplaySample}
a specific channel number, and this function is used to allocate those
channels. 

Channels allocated with this function need to be deallocated with
\mDocRef{MIDASfreeChannel}. Before any channels can be allocated, some
sound channels need to be opened with \mDocRef{MIDASopenChannels}.


\subsubsection*{Return value}

Channel number for the allocated channel, or MIDAS\_{}ILLEGAL\_{}CHANNEL
if failed.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASopenChannels}, \mDocRef{MIDASfreeChannel}.




%----------------------------- MIDAScloseChannels
\mDocEntry{MIDAScloseChannels} %K:Channels;Channels,MIDAScloseChannels
\begin{verbatim}
BOOL MIDAScloseChannels(void)
\end{verbatim}

Closes Sound Device channels opened with \mDocRef{MIDASopenChannels}.


\subsubsection*{Input}

None.


\subsubsection*{Description}

This function closes Sound Device channels that were opened with
\mDocRef{MIDASopenChannels}. Note that you may {\bf not} use this function to
close channels that were opened by \mDocRef{MIDASplayModule} ---
\mDocRef{MIDASstopModule} will do that automatically.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASopenChannels}, \mDocRef{MIDASplayModule},
\mDocRef{MIDASstopModule}




%----------------------------- MIDASfreeChannel
\mDocEntry{MIDASfreeChannel} %K:Channels;Channels,MIDASfreeChannel
\begin{verbatim}
BOOL MIDASfreeChannel(DWORD channel)
\end{verbatim}

Deallocates a single Sound Device channel.


\subsubsection*{Input}

\begin{description}
    \item [channel] The channel number to deallocate, from
        \mDocRef{MIDASallocateChannel} 
\end{description}


\subsubsection*{Description}

This function deallocates a single Sound Device channel that has
previously been allocated with \mDocRef{MIDASallocateChannel}. Any sound
playback on the channel should be stopped before deallocating it.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASallocateChannel}




%----------------------------- MIDASgetErrorMessage
\mDocEntry{MIDASgetErrorMessage} %K:Errors;Errors,MIDASgetErrorMessage
\begin{verbatim}
char *MIDASgetErrorMessage(int errorCode)
\end{verbatim}

Gets an error message corresponding to an error code.


\subsubsection*{Input}

\begin{description}
    \item [errorCode] The error code from \mDocRef{MIDASgetLastError}
\end{description}


\subsubsection*{Description}

This function returns a textual error message corresponding to a MIDAS error
code. It can be used for displaying an error message to the user. Use
\mDocRef{MIDASgetLastError} to determine the error code.

This function can be called at any point after \mDocRef{MIDASstartup} has
been called.


\subsubsection*{Return value}

Error message string corresponding to the error code.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASgetLastError}




%----------------------------- MIDASgetLastError
\mDocEntry{MIDASgetLastError} %K:Errors;Errors,MIDASgetLastError
\begin{verbatim}
int MIDASgetLastError(void)
\end{verbatim}

Gets the MIDAS error code for last error.


\subsubsection*{Input}

None.

\subsubsection*{Description}

This function can be used to read the error code for most recent failure.
When a MIDAS API function returns an error condition, this function can be
used to determine the actual cause of the error, and this error can then be
reported to the user or ignored, depending on the kind of response needed.
Use \mDocRef{MIDASgetErrorMessage} to get a textual message corresponding to
an error code.

This function can be called at any point after \mDocRef{MIDASstartup} has
been called.


\subsubsection*{Return value}

MIDAS error code for the most recent error.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASgetErrorMessage}




%----------------------------- MIDASgetVersionString
\mDocEntry{MIDASgetVersionString}
\begin{verbatim}
char *MIDASgetVersionString(void)
\end{verbatim}

Gets the current MIDAS version as a string.


\subsubsection*{Input}

None.


\subsubsection*{Description}

This function can be used to determine the MIDAS version being loaded. It
returns a text string description of the version. Version numbers are usually
of form ``x.y.z'', where ``x'' is the major version number, ``y'' minor
version number and ``z'' patch level. In some occasions, ``z'' can be
replaced with a textual message such as ``rc1'' for Release Candidate 1. All
MIDAS versions with the major and minor version numbers equal have a
compatible DLL API, and can be used interchangeably.


\subsubsection*{Return value}

MIDAS Digital Audio System version number as a string.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}




%----------------------------- MIDASopenChannels
\mDocEntry{MIDASopenChannels} %K:Channels;Channels,MIDASopenChannels
\begin{verbatim}
BOOL MIDASopenChannels(int numChans)
\end{verbatim}

Opens Sound Device channels for sound and music output.

\subsubsection*{Input}

\begin{description}
    \item [numChans] Number of channels to open
\end{description}


\subsubsection*{Description}

This function opens a specified number of channels for digital sound and
music output. The channels opened can be used for playing streams, samples
and modules.

If this function has not been called before \mDocRef{MIDASplayModule} is
called, \mDocRef{MIDASplayModule} will open the channels it needs for
module playback. However, if this function has been called, but the number of
available channels is inadequate for the module,
\mDocRef{MIDASplayModule} will return an error and refuse to play the
module. 


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDAScloseChannels}, \mDocRef{MIDASplayModule}



%----------------------------- MIDASpoll
\mDocEntry{MIDASpoll}
\begin{verbatim}
BOOL MIDASpoll(void)
\end{verbatim}

Polls the MIDAS sound and music player.


\subsubsection*{Input}

None.


\subsubsection*{Description}

This function can be used to poll MIDAS sound and music player manually. It
plays music forward, mixes sound data, and sends it to output. When using
manual polling, make sure you call \mDocRef{MIDASpoll} often enough to make
sure there are no breaks in sound output --- at least two times during buffer
length, preferably four times or more. Under multitasking operating systems
such as Win32 and Linux, this may be difficult, so very short buffer sizes
can't be used reliably.

Also note that {\bf currently this function is not available under MS-DOS.}
Under MS-DOS, playback is always done in background using the system timer
(IRQ 0).


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}

\mDocRef{MIDASstartBackgroundPlay}, \mDocRef{MIDASstopBackgroundPlay}




%----------------------------- MIDASremoveTimerCallbacks
\mDocEntry{MIDASremoveTimerCallbacks} %K:Timer;Timer,MIDASremoveTimerCallbacks
\begin{verbatim}
BOOL MIDASremoveTimerCallbacks(void)
\end{verbatim}

Removes the user timer callbacks.

\subsubsection*{Input}

None.


\subsubsection*{Description}

This function removes the user timer callbacks set with
\mDocRef{MIDASsetTimerCallbacks}. The callback functions will no longer
be called. This function {\bf may not} be called if
\mDocRef{MIDASsetTimerCallbacks} has not been called before.

It is not necessary to call this function without exiting even if the
callbacks have been used --- \mDocRef{MIDASclose} will remove the
callbacks automatically. On the other hand, if the callback functions or
rate are changed with \mDocRef{MIDASsetTimerCallbacks}, this function
must be called to remove the previous ones first.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

MS-DOS


\subsubsection*{See also}

\mDocRef{MIDASsetTimerCallbacks}




%----------------------------- MIDASresume
\mDocEntry{MIDASresume}
\begin{verbatim}
BOOL MIDASresume(void)
\end{verbatim}

Resumes MIDAS sound playback.

\subsubsection*{Input}

None.

\subsubsection*{Description}

This function re-allocates the system audio device to MIDAS, and resumes
sound playback, after being suspended with \mDocRef{MIDASsuspend}. See
\mDocRef{MIDASsuspend} documentation for more information about suspending
MIDAS.

Note that this function may fail, if another application has captured the
sound output device while MIDAS was suspended.

\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

Win32

\subsubsection*{See also}

\mDocRef{MIDASsuspend}




%----------------------------- MIDASsetAmplification
\mDocEntry{MIDASsetAmplification}
\begin{verbatim}
BOOL MIDASsetAmplification(DWORD amplification)
\end{verbatim}

Sets sound output amplification level.

\subsubsection*{Input}

\begin{description}
    \item [amplification] New output amplification level
\end{description}


\subsubsection*{Description}

This function changes the output amplification level. Amplification can be
used to boost the volume of the music, if the sounds played are unusually
quiet, or lower it if the output seems distorted.. The amplification level is
given as a percentage --- 100 stands for no amplification, 200 for double
volume, 400 for quadruple volue, 50 for half volume etc.

MIDAS has some build-in amplification, but the default amplification is
designed for situations where most channels have data played at moderate
volumes (eg. module playback). If a lot of the channels are empty, or sounds
are played at low volumes, adding amplification with this function can help
to get the total sound output at a reasonable level. The amplification set
with this function acts on top of the default MIDAS amplification, so nothing
will be overridden.

This function can be called at any point after \mDocRef{MIDASstartup}.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}





%----------------------------- MIDASsetTimerCallbacks
\mDocEntry{MIDASsetTimerCallbacks} %K:Timer;Timer,MIDASsetTimerCallbacks;Callbacks
\begin{verbatim}
BOOL MIDASsetTimerCallbacks(DWORD rate, BOOL displaySync,
    void (MIDAS_CALL *preVR)(), void (MIDAS_CALL *immVR)(),
    void (MIDAS_CALL *inVR)());
\end{verbatim}

Sets the user timer callback functions and their rate.

\subsubsection*{Input}

\begin{description}
    \item [rate] Timer callback rate, in milliHertz (1000*Hz, 100Hz
        becomes 100000 etc)
    \item [displaySync] TRUE if the callbacks should be synchronized to
        display refresh, FALSE if not.
    \item [preVR] preVR callback function pointer or NULL
    \item [immVR] immVR callback function pointer or NULL
    \item [inVR] inVR callback function pointer or NULL
\end{description}


\subsubsection*{Description}

This function sets the user timer callback functions and their call
rate. The functions will be called periodically by the MIDAS timer
interrupt, one after another. Any of the callback function pointers may
be set to NULL --- the callback is then ignored.

If \mVariable{displaySync} is TRUE, the timer system attempts to
synchronize the callbacks to the display refresh. In that case,
\mVariable{preVR} is called just before the Vertical Retrace starts,
\mVariable{immVR} immediately after it has started, and \mVariable{inVR}
later during retrace. \mVariable{preVR} can then be used for changing
the display start address, for example. If display synchronization is
used, \mVariable{rate} has to be set to the value returned by
\mDocRef{MIDASgetDisplayRefreshRate}.

If \mVariable{displaySync} is FALSE, or the timer system is unable to
synchronize to display refresh (running under Win95, for example), the
functions are simply called one after another: first \mVariable{preVR},
then \mVariable{immVR} and last \mVariable{inVR}. Note that display
synchronization is not always possible, and this may happen even if
\mVariable{displaySync} is set to 1. 

In either case, both the \mVariable{preVR} and \mVariable{immVR}
functions have to be kept as short as possible, to prevent timing
problems. They should not do more than update a few counters, or set a
couple of hardware registers. \mVariable{inVR} can take somewhat longer
time, and be used for, for example, setting the VGA palette. It should
not take more than one quarter of the time between callbacks though.

The most common use for the timer callback functions is to use them for
controlling the program speed. There one of the callbacks, usually
\mVariable{preVR} is simply used for incrementing a counter. This
counter then can be used to determine when to display a new frame of
graphics, for example, or how many frames of movement needs to be
skipped to maintain correct speed.

Note that this function may cause a small break to music playback with
some sound cards. Therefore it should not be called more often than
necessary. Also, if the application changes display modes, any
display-synchronized timer callbacks {\bf must} be resetted, and a
separate refresh rate must be read for each display mode used.

\mResWord{MIDAS\_{}CALL} is the calling convention used for the callback
functions --- \mResWord{\_{}\_{}cdecl} for Watcom C, empty (default) 
for DJGPP. As the functions will be called from an interrupt, the module
containing the callback functions must be compiled with the ``SS==DS''
setting disabled (command line argument ``-zu'' for Watcom C, default 
setting for DJGPP).


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

MS-DOS


\subsubsection*{See also}

\mDocRef{MIDASremoveTimerCallbacks},
\mDocRef{MIDASgetDisplayRefreshRate}




%----------------------------- MIDASstartBackgroundPlay
\mDocEntry{MIDASstartBackgroundPlay}
\begin{verbatim}
BOOL MIDASstartBackgroundPlay(DWORD pollRate)
\end{verbatim}

Starts playing music and sound in the background.

\subsubsection*{Input}

\begin{description}
    \item [pollRate] Polling rate (number of polls per second), 0 for default
\end{description}


\subsubsection*{Description}

This function starts playing sound and music in the background.
\mVariable{pollRate} controls the target polling rate --- number of polls per
second. Polling might not be done at actually the set rate, although usually
it will be faster. Under Win32 and Linux, a new thread will be created for
playing. {\bf Under MS-DOS this function is currently ignored, and background
playback starts immediately when MIDAS is initialized.}


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All, but see MS-DOS note above.


\subsubsection*{See also}

\mDocRef{MIDASstopBackgroundPlay}, \mDocRef{MIDASpoll}




%----------------------------- MIDASstopBackgroundPlay
\mDocEntry{MIDASstopBackgroundPlay}
\begin{verbatim}
BOOL MIDASstopBackgroundPlay(void)
\end{verbatim}

Stops playing sound and music in the background.


\subsubsection*{Input}

None.


\subsubsection*{Description}

This function stops music and sound background playback that has been started
with \mDocRef{MIDASstartBackgroundPlay}. Under Win32 and Linux, this function
also destroys the thread created for playback. {\bf Under MS-DOS this
function is currently ignored, and background playback starts immediately
when MIDAS is initialized.}

If background playback has been started with
\mDocRef{MIDASstartBackgroundPlay}, this function {bf must} be called before
the program exits.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All, but see MS-DOS note above.


\subsubsection*{See also}

\mDocRef{MIDASstartBackgroundPlay}, \mDocRef{MIDASpoll}




%----------------------------- MIDASsuspend
\mDocEntry{MIDASsuspend}
\begin{verbatim}
BOOL MIDASsuspend(void)
\end{verbatim}

Suspends MIDAS Digital Audio System.

\subsubsection*{Input}

None.

\subsubsection*{Description}

This function suspends all MIDAS Digital Audio System output, and releases the
system audio device to other programs. Playback can be resumed with
\mDocRef{MIDASresume}. Suspending and resuming MIDAS can be used to change
some of the initial configuration options (set with \mDocRef{MIDASsetOption})
on the fly. In particular, the DirectSound mode and DirectSound window handle
can be changed while MIDAS is suspended, and the new values take effect when
MDIAS is restarted. Buffer size can also be changed, although this is not
recommended. Output mode and mixing rate cannot be changed without completely
uninitializing MIDAS.

While MIDAS is suspended, all MIDAS functions can be called normally --- the
sound simply is not played. Also, stream, module and sample playback
positions do not change while MIDAS is suspended.

Note that \mDocRef{MIDASsuspend} and \mDocRef{MIDASresume} are only available
in Win32 systems at the moment.

\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

Win32

\subsubsection*{See also}

\mDocRef{MIDASresume}




% ***************************************************************************
% *
% *     MODULE PLAYBACK
% *
% ***************************************************************************

\chapter{Module playback}


% ---------------------------------------------------------------------------
%       Constants
% ---------------------------------------------------------------------------

\section{Constants}

This section describes all constants used in MIDAS module playback. They are
grouped according to the \mResWord{enum} used to define them.




% ---------------------------------------------------------------------------
%       Data types
% ---------------------------------------------------------------------------

\newpage
\section{Data types}

This section describes all data types used in MIDAS module playback.



%----------------------------- MIDASinstrumentInfo
\mDocEntry{MIDASinstrumentInfo} %K:Module;Module information;Module information,MIDASinstrumentInfo
\begin{verbatim}
typedef struct
{
    char        instName[32];
} MIDASinstrumentInfo;
\end{verbatim}

Instrument information structure.


\subsubsection*{Members}

\begin{description}
    \item [instName] Instrument name, an ASCIIZ string
\end{description}


\subsubsection*{Description}

This structure is used with the function \mDocRef{MIDASgetInstrumentInfo} to
query information about an instrument in a module.
\mDocRef{MIDASgetInstrumentInfo} fills a \mDocRef{MIDASinstrumentInfo}
structure with the information.




%----------------------------- MIDASmodule
\mDocEntry{MIDASmodule} %K:Module;Module playback;Module playback,MIDASmodule
\begin{verbatim}
typedef ... MIDASmodule;
\end{verbatim}

\subsubsection*{Description}

\mDocRef{MIDASmodule} is a module handle that defines a module that has been
loaded into memory.



%----------------------------- MIDASmoduleInfo
\mDocEntry{MIDASmoduleInfo} %K:Module;Module information;Module information,MIDASmoduleInfo
\begin{verbatim}
typedef struct
{
    char        songName[32];
    unsigned    songLength;
    unsigned    numPatterns;
    unsigned    numInstruments;
    unsigned    numChannels;
} MIDASmoduleInfo;
\end{verbatim}

Module information structure.


\subsubsection*{Members}

\begin{description}
    \item [songName] Module song name, an ASCIIZ string
    \item [songLength] Module song length in number of positions
    \item [numPatterns] Number of patterns in module
    \item [numInstruments] Number of instruments in module
    \item [numChannels] The number of channels the module uses
\end{description}


\subsubsection*{Description}

This structure is used with the function \mDocRef{MIDASgetModuleInfo} to
query information about an module. \mDocRef{MIDASgetModuleInfo} fills a
\mDocRef{MIDASmoduleInfo} structure with the information.




%----------------------------- MIDASmodulePlayHandle
\mDocEntry{MIDASmodulePlayHandle} %K:Module;Module playback;Module playback,MIDASmodulePlayHandle
\begin{verbatim}
typedef ... MIDASmodulePlayHandle;
\end{verbatim}

\subsubsection*{Description}

\mDocRef{MIDASmodulePlayHandle} is a module playback handle that defines
a module or module section that is being played. One module can be
played several times simultaneously.




%----------------------------- MIDASplayStatus
\mDocEntry{MIDASplayStatus} %K:Status;Status,MIDASplayStatus
\begin{verbatim}
typedef struct
{
    unsigned    position;
    unsigned    pattern;
    unsigned    row;
    int         syncInfo;
    unsigned    songLoopCount;
} MIDASplayStatus;
\end{verbatim}

Module status information structure.


\subsubsection*{Members}

\begin{description}
    \item [position] Current playback position number
    \item [pattern] Current playback pattern number
    \item [row] Current playback row number
    \item [syncInfo] Latest synchronization command infobyte, -1 if no
        synchronization command has been encountered yet.
    \item [songLoopCount] Module song loop counter --- incremented by 1
        every time the song loops around. Module looping can be detected
        by checking if this field is nonzero.
\end{description}


\subsubsection*{Description}

This structure is used with the function \mDocRef{MIDASgetPlayStatus} to
query the current module playback status. \mDocRef{MIDASgetPlayStatus} fills
a \mDocRef{MIDASplayStatus} structure with the information.

Some more information about the synchronization commands: In FastTracker 2 and
Scream Tracker 3 modules, the command \mResWord{Wxx} is used as a music
synchronization command. The infobyte of this command is available as the
music synchronization command infobyte above.




% ---------------------------------------------------------------------------
%       Functions
% ---------------------------------------------------------------------------

\newpage
\section{Functions}

This section describes all functions available for MIDAS module playback.




%----------------------------- MIDASfadeMusicChannel
\mDocEntry{MIDASfadeMusicChannel} %K:Module;Module playback;Module playback,MIDASfadeMusicChannel
\begin{verbatim}
BOOL MIDASfadeMusicChannel(MIDASmodulePlayHandle playHandle,
    unsigned channel, unsigned fade)
\end{verbatim}

Fades a music channel.


\subsubsection*{Input}

\begin{description}
    \item [playHandle] Module playback handle for the music
    \item [channel] Module channel number to control
    \item [fade] Channel fade value: 64 is normal volume, 0 silence
\end{description}


\subsubsection*{Description}

This function is used to control the ``fade'' of a music
channel. Channel fade acts as a channel-specific master volume: it can
be used to quiet down the general volume of the sounds played on the
channel, while any volume changes in the music still take effect.

\mVariable{channel} if the {\bf module} channel number for the sounds to
control, not a Sound Device channel number. Module channels are numbered
from zero upwards.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASsetMusicVolume}




%----------------------------- MIDASfreeModule
\mDocEntry{MIDASfreeModule} %K:Module;Module playback;Module playback,MIDASfreeModule
\begin{verbatim}
BOOL MIDASfreeModule(MIDASmodule module)
\end{verbatim}

Deallocates a module.


\subsubsection*{Input}

\begin{description}
    \item [module] Module that should be deallocated
\end{description}


\subsubsection*{Description}

This function deallocates a module loaded with \mDocRef{MIDASloadModule}. It
should be called to free unused modules from memory, after they are no longer
being played, to avoid memory leaks.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASloadModule}




%----------------------------- MIDASgetInstrumentInfo
\mDocEntry{MIDASgetInstrumentInfo} %K:Module;Module information;Module information,MIDASgetInstrumentInfo
\begin{verbatim}
BOOL MIDASgetInstrumentInfo(MIDASmodule module,
    int instNum, MIDASinstrumentInfo *info)
\end{verbatim}

Gets information about an instrument in a module.


\subsubsection*{Input}

\begin{description}
    \item [module] Module handle for the module
    \item [instNum] Instrument number
    \item [info] Pointer to an instrument info structure where the information
        willl be written
\end{description}


\subsubsection*{Description}

This function returns information about an instrument in a module, including
the instrument name. The user needs to pass it a valid pointer to a
\mDocRef{MIDASinstrumentInfo} structure (\mVariable{*info}), where the
information will be written. You should ensure that \mVariable{instNum} is a
valid instrument number. Instrument numbers start from 0, although trackers
traditionally number them from 1, and you can use\mDocRef{MIDASgetModuleInfo}
to get the number of instruments available in a module.


\subsubsection*{Return value}

TRUE if successful, FALSE if not. The instrument information is written to
\mVariable{*info}.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplayModule}, \mDocRef{MIDASgetModuleInfo},
\mDocRef{MIDASmoduleInfo}




%----------------------------- MIDASgetModuleInfo
\mDocEntry{MIDASgetModuleInfo} %K:Module;Module information;Module information,MIDASgetModuleInfo
\begin{verbatim}
BOOL MIDASgetModuleInfo(MIDASmodule module,
    MIDASmoduleInfo *info)
\end{verbatim}

Gets information about a module.


\subsubsection*{Input}

\begin{description}
    \item [module] Module handle for the module
    \item [info] Pointer to a module info structure where the information
        will be written
\end{description}


\subsubsection*{Description}

This function returns information about a module, including the module name
and the number of channels used. The user needs to pass it a valid pointer to
a \mDocRef{MIDASmoduleInfo} structure (\mVariable{*info}), where the
information will be written.


\subsubsection*{Return value}

TRUE if successful, FALSE if not. The module information is written to
\mVariable{*info}.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplayModule}, \mDocRef{MIDASmoduleInfo}




%----------------------------- MIDASgetPlayStatus
\mDocEntry{MIDASgetPlayStatus} %K:Status;Status,MIDASgetPlayStatus
\begin{verbatim}
BOOL MIDASgetPlayStatus(MIDASmodulePlayHandle playHandle,
    MIDASplayStatus *status)
\end{verbatim}

Gets module playback status.


\subsubsection*{Input}

\begin{description}
    \item [playHandle] Module playback handle for the music
    \item [status] Pointer to playback status structure where the status will
        be written.
\end{description}


\subsubsection*{Description}

This function reads the current module playback status, and writes it to
\mVariable{*status}. The user needs to pass it a valid pointer to a
\mDocRef{MIDASplayStatus} structure, which will be updated.


\subsubsection*{Return value}

TRUE if successful, FALSE if not. The current playback status is written to
\mVariable{*status}.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplayModule}, \mDocRef{MIDASplayStatus}




%----------------------------- MIDASloadModule
\mDocEntry{MIDASloadModule} %K:Module;Module playback;Module playback,MIDASloadModule
\begin{verbatim}
MIDASmodule MIDASloadModule(char *fileName)
\end{verbatim}

Loads a module file into memory.

\subsubsection*{Input}

\begin{description}
    \item [fileName] Module file name
\end{description}


\subsubsection*{Description}

This function loads a module file into memory. It checks the module format
based on the module file header, and invokes the correct loader to load the
module into memory in GMPlayer internal format. The module can then be played
using \mDocRef{MIDASplayModule}, and deallocated from memory with
\mDocRef{MIDASfreeModule}.


\subsubsection*{Return value}

Module handle if successful, NULL if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplayModule}, \mDocRef{MIDASfreeModule}




%----------------------------- MIDASplayModule
\mDocEntry{MIDASplayModule} %K:Module;Module playback;Module playback,MIDASplayModule
\begin{verbatim}
MIDASmodulePlayHandle MIDASplayModule(MIDASmodule module, 
    BOOL loopSong)
\end{verbatim}

Starts playing a module.

\subsubsection*{Input}

\begin{description}
    \item [module] Module to be played
    \item [loopSong] TRUE if the song should be looped, FALSE if not
\end{description}


\subsubsection*{Description}

This functions starts playing a module that has been previously loaded with
\mDocRef{MIDASloadModule}. If channels have not been previously opened using
\mDocRef{MIDASopenChannels}, this function opens the channels necessary to
play the module. This function plays the complete module --- to play
just a section of the song data, use \mDocRef{MIDASplayModuleSection}.

{\bf Note!} Currently, when multiple modules or module sections are
played simultaneously, all modules should have the same (BPM)
{\bf tempo}. Otherwise some modules may be played at the wrong tempo. All
modules can have different {\bf speed} setting though.


\subsubsection*{Return value}

MIDAS module playback handle for the module, or 0 if failed.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASloadModule}, \mDocRef{MIDASstopModule},
\mDocRef{MIDASplayModuleSection}




%----------------------------- MIDASplayModuleSection
\mDocEntry{MIDASplayModuleSection} %K:Module;Module playback;Module playback, MIDASplayModuleSection;Module section
\begin{verbatim}
MIDASmodulePlayHandle MIDASplayModuleSection(MIDASmodule module, 
    unsigned startPos, unsigned endPos, unsigned restartPos,
    BOOL loopSong)
\end{verbatim}

Starts playing a module section.

\subsubsection*{Input}

\begin{description}
    \item [module] Module to be played
    \item [startPos] Start song position for the section to play
    \item [endPos] End song position for the section to play
    \item [restartPos] Restart position to use when the section loops
    \item [loopSong] TRUE if the playback should be looped, FALSE if not
\end{description}


\subsubsection*{Description}

This module starts playing a section of a module that has been
previously loaded with \mDocRef{MIDASloadModule}. If channels have not
been previously opened using
\mDocRef{MIDASopenChannels}, this function opens the channels necessary to
play the module. Playback will start from the pattern at position
\mVariable{startPos}, and after the pattern at position
\mVariable{endPos} has been played, playback will resume from the pattern at
\mVariable{restartPos}. This function can thus be used to play a section
of a module, and a single module can be used to store several songs.

{\bf Note!} Currently, when multiple modules or module sections are
played simultaneously, all modules should have the same (BPM)
{\bf tempo}. Otherwise some modules may be played at the wrong tempo. All
modules can have different {\bf speed} setting though.


\subsubsection*{Return value}

MIDAS module playback handle for the module, or 0 if failed.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASloadModule}, \mDocRef{MIDASstopModule},
\mDocRef{MIDASplayModule}




%----------------------------- MIDASsetMusicSyncCallback
\mDocEntry{MIDASsetMusicSyncCallback} %K:Callbacks;Module;Module playback;Module playback,MIDASsetMusicSyncCallback
\begin{verbatim}
BOOL MIDASsetMusicSyncCallback(MIDASmodulePlayHandle playHandle,
    void (MIDAS_CALL *callback)
    (unsigned syncInfo, unsigned position, unsigned row))
\end{verbatim}

Sets the music synchronization callback.

\subsubsection*{Input}

\begin{description}
    \item [playHandle] Module playback handle for the music
    \item [callback] Pointer to the callback function, NULL to disable
\end{description}


\subsubsection*{Description}

This function sets the music synchronization callback function. It will be
called by the MIDAS music player each time a {\bf Wxx} command is played from
a FastTracker 2 or Scream Tracker 3 module, or a {\bf Zxx} command from an
Impulse Tracker module. The function will receive as its arguments the
synchronization command infobyte (xx), the current playback position and the
current playback row. Setting \mVariable{callback} to NULL disables it.

\mResWord{MIDAS\_{}CALL} is the calling convention used for the callback
function --- \mResWord{\_{}\_{}cdecl} for Watcom and Visual C/C++, empty
(default) for GCC. Under MS-DOS the function will be called from the
MIDAS timer interrupt, so the module
containing the callback function must be compiled with the ``SS==DS''
setting disabled (command line argument ``-zu'' for Watcom C, default 
setting for DJGPP). Under Win32 and Linux the function will be called
in the context of the MIDAS player thread.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}





%----------------------------- MIDASsetMusicVolume
\mDocEntry{MIDASsetMusicVolume} %K:Volume;Module;Module playback;Module playback,MIDASsetMusicVolume
\begin{verbatim}
BOOL MIDASsetMusicVolume(MIDASmodulePlayHandle playHandle, 
    unsigned volume)
\end{verbatim}

Changes music playback volume.


\subsubsection*{Input}

\begin{description}
    \item [playHandle] Module playback handle for the music
    \item [volume] New music playback volume (0--64)
\end{description}


\subsubsection*{Description}

This function changes the music playback master volume. It can be used, for
example, for fading music in or out smoothly, or for adjusting the music
volume relative to sound effects. The volume change only affects the song
that is currently being played --- if a new song is started, the volume is
reset. The default music volume is 64 (the maximum).


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}




%----------------------------- MIDASsetPosition
\mDocEntry{MIDASsetPosition} %K:Position;Module;Module playback;Module playback,MIDASsetPosition
\begin{verbatim}
BOOL MIDASsetPosition(MIDASmodulePlayHandle playHandle,
    int newPosition)
\end{verbatim}

Changes module playback position.


\subsubsection*{Input}

\begin{description}
    \item [playHandle] Module playback handle for the music
    \item [newPosition] New playback position
\end{description}


\subsubsection*{Description}

This function changes the current module playback position. The song starts
at position 0, and the length is available in the \mDocRef{MIDASmoduleInfo}
structure. You should make sure that \mVariable{position} lies inside those
limits. To skip backward or forward a single position, first read the current
position with \mDocRef{MIDASgetPlayStatus}, and substract or add one to the
current position.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplayModule}, \mDocRef{MIDASgetPlayStatus},
\mDocRef{MIDASgetModuleInfo}




%----------------------------- MIDASstopModule
\mDocEntry{MIDASstopModule} %K:Module;Module playback;Module playback,MIDASstopModule

\begin{verbatim}
BOOL MIDASstopModule(MIDASmodulePlayHandle playHandle)
\end{verbatim}

Stops playing a module.

\subsubsection*{Input}

\begin{description}
    \item [playHandle] Module playback handle for the music that should be
        stopped. 
\end{description}


\subsubsection*{Description}

This function stops playing a module that has been played with
\mDocRef{MIDASplayModule}. If the channels were opened automatically by
\mDocRef{MIDASplayModule}, this function will close them, but if they were
opened manually with \mDocRef{MIDASopenChannels}, they will be left open.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.

\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplayModule}, \mDocRef{MIDASopenChannels}






% ***************************************************************************
% *
% *     SAMPLE PLAYBACK
% *
% ***************************************************************************

\chapter{Sample playback}


% ---------------------------------------------------------------------------
%       Constants
% ---------------------------------------------------------------------------

\section{Constants}

This section describes all constants used in MIDAS sample playback. They are
grouped according to the \mResWord{enum} used to define them.


%----------------------------- MIDASchannels
\mDocEntry{MIDASchannels} %K:Channels,Numbers;Channels,MIDASchannels;MIDAS_CHANNEL_AUTO;MIDAS_ILLEGAL_CHANNEL
\begin{verbatim}
enum MIDASchannels
\end{verbatim}

\subsubsection*{Description}

These constants are used to indicate the channel number a sound should be
played on. Legal channel numbers range from 0 upwards, depending on the
number of open channels. In addition, MIDAS\_{}CHANNEL\_{}AUTO can be used
with \mDocRef{MIDASplaySample}.


\subsubsection*{Values}

\begin{description}
    \item [MIDAS\_{}CHANNEL\_{}AUTO] Select channel automatically, used with
        \mDocRef{MIDASplaySample}
    \item [MIDAS\_{}ILLEGAL\_{}CHANNEL] Illegal channel number, returned
        by \mDocRef{MIDASallocateChannel} as an error code.
\end{description}




%----------------------------- MIDASloop
\mDocEntry{MIDASloop} %K:Looping;Sample;Sample,Loop types;Sample,MIDASloop;MIDAS_LOOP_NO;MIDAS_LOOP_YES
\begin{verbatim}
enum MIDASloop
\end{verbatim}

\subsubsection*{Description}

These constants are used to indicate the loop type of a sample or stream.

\subsubsection*{Values}

\begin{description}
    \item [MIDAS\_{}LOOP\_{}NO] Sample or stream does not loop
    \item [MIDAS\_{}LOOP\_{}YES] Sample or stream loops
\end{description}




%----------------------------- MIDASpanning
\mDocEntry{MIDASpanning} %K:Panning;Sample;Sample,Panning;Sample,MIDASpanning;Stream;Stream,Panning;Stream,MIDASpanning;Channels;Channels,Panning;MIDAS_PAN_LEFT;MIDAS_PAN_MIDDLE;MIDAS_PAN_RIGHT;MIDAS_PAN_SURROUND
\begin{verbatim}
enum MIDASpanning
\end{verbatim}

\subsubsection*{Description}

These constants are used to describe the panning position of a sound. Legal
panning positions range from -64 (extreme left) to 64 (extreme right),
inclusive, plus MIDAS\_{}PAN\_{}SURROUND for surround sound.


\subsubsection*{Values}

\begin{description}
    \item [MIDAS\_{}PAN\_{}LEFT] Panning position full left
    \item [MIDAS\_{}PAN\_{}MIDDLE] Panning position middle
    \item [MIDAS\_{}PAN\_{}RIGHT] Panning position full right
    \item [MIDAS\_{}PAN\_{}SURROUND] Surround sound
\end{description}




%----------------------------- MIDASsamplePlayStatus
\mDocEntry{MIDASsamplePlayStatus} %K:Sample;Sample,Playing status;\
                                  %K:Sample,MIDASpanning;MIDAS_SAMPLE_STOPPED;\
                                  %K:MIDAS_SAMPLE_PLAYING
\begin{verbatim}
enum MIDASsamplePlayStatus
\end{verbatim}

\subsubsection*{Description}

These constants describe the possible sample playing status values returned by
\mDocRef{MIDASgetSamplePlayStatus}. 


\subsubsection*{Values}

\begin{description}
    \item [MIDAS\_{}SAMPLE\_{}STOPPED] The sample has stopped playing. Either
        the sample has ended, or another sample has taken its place (with
        automatic effect channels)
    \item [MIDAS\_{}SAMPLE\_{}PLAYING] The sample is playing
\end{description}




%----------------------------- MIDASsampleTypes
\mDocEntry{MIDASsampleTypes} %K:Sample;Sample,Types;Sample,MIDASsampleTypes;Stream;Stream,Types;Stream,MIDASsampleTypes;MIDAS_SAMPLE_8BIT_MONO;MIDAS_SAMPLE_8BIT_STEREO;MIDAS_SAMPLE_16BIT_MONO;MIDAS_SAMPLE_16BIT_STEREO;MIDAS_SAMPLE_ADPCM_MONO;MIDAS_SAMPLE_ADPCM_STEREO;
\begin{verbatim}
enum MIDASsampleTypes
\end{verbatim}

\subsubsection*{Description}

These constants identify different sample types. They are used with the
functions \mDocRef{MIDASloadRawSample}, \mDocRef{MIDASplayStreamFile} and
\mDocRef{MIDASplayStreamPolling} to indicate the format of the sample data.
The byte order of the sample data is always the system native order (little
endian for Intel x86 systems).


\subsubsection*{Values}

\begin{description}
    \item [MIDAS\_{}SAMPLE\_{}8BIT\_{}MONO] 8-bit mono sample, unsigned
    \item [MIDAS\_{}SAMPLE\_{}8BIT\_{}STEREO] 8-bit stereo sample, unsigned
    \item [MIDAS\_{}SAMPLE\_{}16BIT\_{}MONO] 16-bit mono sample, signed
    \item [MIDAS\_{}SAMPLE\_{}16BIT\_{}STEREO] 16-bit stereo sample, signed
    \item [MIDAS\_{}SAMPLE\_{}ADPCM\_{}MONO] 4-bit ADPCM mono sample 
        (streams only)
    \item [MIDAS\_{}SAMPLE\_{}ADPCM\_{}STEREO] 4-bit ADPCM stereo sample
        (streams only)
    \item [MIDAS\_{}SAMPLE\_{}ULAW\_{}MONO] 8-bit $mu$-law mono sample
    \item [MIDAS\_{}SAMPLE\_{}ULAW\_{}STEREO] 8-bit $mu$-law stereo sample
\end{description}




% ---------------------------------------------------------------------------
%       Data types
% ---------------------------------------------------------------------------

\newpage
\section{Data types}

This section describes all data types used in MIDAS sample playback.


%----------------------------- MIDASsample
\mDocEntry{MIDASsample} %K:Sample;Sample,MIDASsample
\begin{verbatim}
typedef ... MIDASsample;
\end{verbatim}

\subsubsection*{Description}

\mDocRef{MIDASsample} is a sample handle that defines a sample that has been
loaded into memory. The sample handle is used for identifying the sample when
playing or deallocating it.




%----------------------------- MIDASsamplePlayHandle
\mDocEntry{MIDASsamplePlayHandle} %K:Sample;Sample,MIDASsamplePlayHandle
\begin{verbatim}
typedef ... MIDASsamplePlayHandle;
\end{verbatim}

\subsubsection*{Description}

\mDocRef{MIDASsamplePlayHandle} is a sample playing handle. It describes a
sample sound that is being played. The sample playing handle is used for
controlling the attributes of the sound, such as panning or volume, and for
stopping the sound.




% ---------------------------------------------------------------------------
%       Functions
% ---------------------------------------------------------------------------

\newpage
\section{Functions}

This section describes all functions available for MIDAS sample playback.




%----------------------------- MIDASallocAutoEffectChannels
\mDocEntry{MIDASallocAutoEffectChannels} %K:Channels;Channels,Automatic effect channels;Channels,MIDASallocAutoEffectChannels;Sample;Sample,Automatic effect channels;Sample,MIDASallocAutoEffectChannels
\begin{verbatim}
BOOL MIDASallocAutoEffectChannels(unsigned numChannels)
\end{verbatim}

Allocates a number of channels for use as automatic effect channels.


\subsubsection*{Input}

\begin{description}
    \item [numChannels] Number of channels to use
\end{description}


\subsubsection*{Description}

This function allocates a number of channels that can be used as
automatic effect channels by \mDocRef{MIDASplaySample}. When
\mDocRef{MIDASplaySample} is passed MIDAS\_{}CHANNEL\_{}AUTO as the channel
number, it will use one of these automatic channels to play the
sound. The channels allocated can be deallocated with
 \mDocRef{MIDASfreeAutoEffectChannels}.



\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASfreeAutoEffectChannels}, \mDocRef{MIDASplaySample}




%----------------------------- MIDASfreeAutoEffectChannels
\mDocEntry{MIDASfreeAutoEffectChannels} %K:Channels;Channels,Automatic effect channels;Channels,MIDASfreeAutoEffectChannels;Sample;Sample,Automatic effect channels;Sample,MIDASfreeAutoEffectChannels
\begin{verbatim}
BOOL MIDASfreeAutoEffectChannels(void)
\end{verbatim}

Deallocates the channels allocated for automatic effect channels.


\subsubsection*{Input}

None.


\subsubsection*{Description}

This function deallocates the channels allocated by
\mDocRef{MIDASallocAutoEffectChannels} for use as automatic sound effect
channels.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASallocAutoEffectChannels}




%----------------------------- MIDASfreeSample
\mDocEntry{MIDASfreeSample} %K:Sample;Sample,MIDASfreeSample
\begin{verbatim}
BOOL MIDASfreeSample(MIDASsample sample)
\end{verbatim}

Deallocates a sound effect sample.


\subsubsection*{Input}

\begin{description}
    \item [sample] Sample to be deallocated
\end{description}


\subsubsection*{Description}

This function deallocates a sound effect sample that has been previously
loaded with \mDocRef{MIDASloadRawSample} or \mDocRef{MIDASloadWaveSample}. 
It removes the sample from the
Sound Device and deallocates the memory used. This function may not be called
if the sample is still being played.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASloadRawSample}, \mDocRef{MIDASloadWaveSample}




%----------------------------- MIDASgetSamplePlayStatus
\mDocEntry{MIDASgetSamplePlayStatus} %K:Sample;Sample,Playing status;
                                     %K:Sample,MIDASgetSamplePlayStatus
\begin{verbatim}
DWORD MIDASgetSamplePlayStatus(MIDASsamplePlayHandle sample);
\end{verbatim}

Gets the sample playing status.


\subsubsection*{Input}

\begin{description}
    \item [sample] The sample playing handle
\end{description}


\subsubsection*{Description}

This function returns the playing handle for the sample with playing handle
\mVariable{sample}. The playing status can be used to determine if the sample
has stopped, and if another sample should be played in its place. Normally
manually checking for the sample playing status is unnecessary, however, as
new samples can simply be played on top of the old ones.


\subsubsection*{Return value}

Playing status for the sample, see \mResWord{enum}
\mDocRef{MIDASsamplePlayStatus}. 


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplaySample}, \mDocRef{MIDASstopSample}




%----------------------------- MIDASloadRawSample
\mDocEntry{MIDASloadRawSample} %K:Sample;Sample,MIDASloadRawSample
\begin{verbatim}
MIDASsample MIDASloadRawSample(char *filename, int sampleType,
    int loopSample)
\end{verbatim}

Loads a raw sound effect sample.


\subsubsection*{Input}

\begin{description}
    \item [filename] Sample file name
    \item [sampleType] Sample type, see \mResWord{enum}
        \mDocRef{MIDASsampleTypes}
    \item [loopSample] Sample loop type, see \mResWord{enum}
        \mDocRef{MIDASloop}
\end{description}


\subsubsection*{Description}

This function loads a sound effect sample into memory and adds it to the
Sound Device. The sample file must contain just the raw sample data, and all
of it will be loaded into memory. If \mVariable{loopSample} is
MIDAS\_{}LOOP\_{}YES, the whole sample will be looped. After the sample has
been loaded, it can be played using \mDocRef{MIDASplaySample}, and it
should be deallocated with \mDocRef{MIDASfreeSample} after it is no
longer used.


\subsubsection*{Return value}

Sample handle if successful, NULL if failed.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplaySample}, \mDocRef{MIDASfreeSample}




%----------------------------- MIDASloadWaveSample
\mDocEntry{MIDASloadWaveSample} %K:Sample;Sample,MIDASloadWaveSample;\
                                %K:WAVE;WAVE,MIDASloadWaveSample
\begin{verbatim}
MIDASsample MIDASloadWaveSample(char *filename, int loopSample)
\end{verbatim}

Loads a RIFF WAVE sound effect sample.


\subsubsection*{Input}

\begin{description}
    \item [filename] Sample file name
    \item [loopSample] Sample loop type, see \mResWord{enum}
        \mDocRef{MIDASloop}
\end{description}


\subsubsection*{Description}

This function loads a sound effect sample into memory and adds it to the
Sound Device. The sample file must be a standard RIFF WAVE
(\mFileName{.wav}) sound file, containing raw PCM sound data --- 
compressed WAVE files are not supported. If \mVariable{loopSample} is
MIDAS\_{}LOOP\_{}YES, the whole sample will be looped. After the sample has
been loaded, it can be played using \mDocRef{MIDASplaySample}, and it
should be deallocated with \mDocRef{MIDASfreeSample} after it is no
longer used.


\subsubsection*{Return value}

Sample handle if successful, NULL if failed.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplaySample}, \mDocRef{MIDASfreeSample}




%----------------------------- MIDASplaySample
\mDocEntry{MIDASplaySample} %K:Sample;Sample,MIDASplaySample
\begin{verbatim}
MIDASsamplePlayHandle MIDASplaySample(MIDASsample sample,
    unsigned channel, int priority, unsigned rate,
    unsigned volume, int panning)
\end{verbatim}

Plays a sound effect sample.


\subsubsection*{Input}

\begin{description}
    \item [sample] The sample that will be played
    \item [channel] The channel number that is used to play the
        sample. Use MIDAS\_{}CHANNEL\_{}AUTO to let \mDocRef{MIDASplaySample}
        select the channel automatically. See \mResWord{enum}
        \mDocRef{MIDASchannels}.
    \item [priority] Sample playing priority. The higher the value the
        more important the sample is considered.
    \item [rate] Initial sample rate for the sample
    \item [volume] Initial volume for the sample
    \item [panning] Initial panning position for the sample. See
        \mResWord{enum} \mDocRef{MIDASpanning}.
\end{description}


\subsubsection*{Description}

This function is used to play a sound effect sample on a given channel. The
sample will receive as initial parameters the values passed as arguments, and
playing the sample will be started. If \mVariable{channel} is
MIDAS\_{}CHANNEL\_{}AUTO, the channel will be selected automatically. The
sample playing priority is used to choose the channel the sample will be
played on in this case. Otherwise a channel needs to be allocated with
\mDocRef{MIDASallocateChannel} before the sample can be played.

This function returns a sample playing handle, that can later be used to stop
the sample or change its parameters. This makes it possible to refer to
samples without knowing the exact channel they are played on.


\subsubsection*{Return value}

Sample playing handle if successful, NULL if failed.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASstopSample}, \mDocRef{MIDASallocAutoEffectChannels}




%----------------------------- MIDASsetSamplePanning
\mDocEntry{MIDASsetSamplePanning} %K:Sample;Sample,Panning;Sample,MIDASsetSamplePanning;Panning
\begin{verbatim}
BOOL MIDASsetSamplePanning(MIDASsamplePlayHandle sample,
    int panning)
\end{verbatim}

Changes the panning position of a sound effect sample.


\subsubsection*{Input}

\begin{description}
    \item [sample] Sample to be changed
    \item [panning] New panning position for the sample (see
        \mResWord{enum} \mDocRef{MIDASpanning})
\end{description}


\subsubsection*{Description}

This function changes the panning position of a sound effect sample that is
being played. See description of \mResWord{enum} \mDocRef{MIDASpanning} for
information about the panning position values.



\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplaySample}




%----------------------------- MIDASsetSamplePriority
\mDocEntry{MIDASsetSamplePriority} %K:Sample;Sample,MIDASsetSamplePriority
\begin{verbatim}
BOOL MIDASsetSamplePriority(MIDASsamplePlayHandle sample,
    int priority)
\end{verbatim}

Changes the playing priority of a sound effect sample.


\subsubsection*{Input}

\begin{description}
    \item [sample] Sample to be changed
    \item [priority] New playing priority for the sample
\end{description}


\subsubsection*{Description}

This function changes the playing priority a sound effect sample
that is being played.



\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplaySample}




%----------------------------- MIDASsetSampleRate
\mDocEntry{MIDASsetSampleRate} %K:Sample;Sample,MIDASsetSampleRate
\begin{verbatim}
BOOL MIDASsetSampleRate(MIDASsamplePlayHandle sample,
    unsigned rate)
\end{verbatim}

Changes the sample rate for a sound effect sample.


\subsubsection*{Input}

\begin{description}
    \item [sample] Sample to be changed
    \item [rate] New sample rate for the sample
\end{description}


\subsubsection*{Description}

This function changes the sample rate for a sound effect sample that is being
played.



\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplaySample}




%----------------------------- MIDASsetSampleVolume
\mDocEntry{MIDASsetSampleVolume} %K:Sample;Sample,MIDASsetSampleVolume;Volume
\begin{verbatim}
BOOL MIDASsetSampleVolume(MIDASsamplePlayHandle sample,
    unsigned volume)
\end{verbatim}

Changes the volume for a sound effect sample.


\subsubsection*{Input}

\begin{description}
    \item [sample] Sample to be changed
    \item [rate] New volume for the sample (0--64)
\end{description}


\subsubsection*{Description}

This function changes the volume for a sound effect sample that is being
played.



\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplaySample}




%----------------------------- MIDASstopSample
\mDocEntry{MIDASstopSample} %K:Sample;Sample,MIDASstopSample
\begin{verbatim}
BOOL MIDASstopSample(MIDASsamplePlayHandle sample)
\end{verbatim}

Stops playing a sample.


\subsubsection*{Input}

\begin{description}
    \item [sample] Sample to be stopped
\end{description}


\subsubsection*{Description}

This function stops playing a sound effect sample started with
\mDocRef{MIDASplaySample}. Playing the sound will stop, and the channel is
freed for other samples to use. Note that \mVariable{sample} is
the sample playing handle returned by \mDocRef{MIDASplaySample}.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASplaySample}





% ***************************************************************************
% *
% *     STREAM PLAYBACK
% *
% ***************************************************************************

\chapter{Stream playback}


% ---------------------------------------------------------------------------
%       Constants
% ---------------------------------------------------------------------------

\section{Constants}

This section describes all constants used in MIDAS stream playback. They are
grouped according to the \mResWord{enum} used to define them. Note that
stream playback properties, such as volume and panning, are controlled
similarily those of samples.




% ---------------------------------------------------------------------------
%       Data types
% ---------------------------------------------------------------------------

\newpage
\section{Data types}

This section describes all data types used in MIDAS stream playback.


%----------------------------- MIDASstreamHandle
\mDocEntry{MIDASstreamHandle} %K:Stream;Stream,MIDASstreamHandle
\begin{verbatim}
typedef ... MIDASstreamHandle;
\end{verbatim}

\subsubsection*{Description}

\mDocRef{MIDASstreamHandle} is a stream handle that defines a digital audio
stream that is being played. Streams only exist in the system when they are
being played, so there is no separate ``playing handle'' data type.





% ---------------------------------------------------------------------------
%       Functions
% ---------------------------------------------------------------------------

\newpage
\section{Functions}

This section describes all functions available for MIDAS stream playback.




%----------------------------- MIDASfeedStreamData
\mDocEntry{MIDASfeedStreamData} %K:Stream;Stream,MIDASfeedStreamData;Stream,Polling
\begin{verbatim}
unsigned MIDASfeedStreamData(MIDASstreamHandle stream,
    unsigned char *data, unsigned numBytes, BOOL feedAll);
\end{verbatim}

Feeds sound data to a digital audio stream buffer.


\subsubsection*{Input}

\begin{description}
    \item [stream] The stream that will play the data
    \item [data] Pointer to sound data
    \item [numBytes] Number of bytes of sound data available
    \item [feedAll] TRUE if the function should block until all sound data can
        be fed
\end{description}


\subsubsection*{Description}

This function is used to feed sample data to a stream that has been started
with \mDocRef{MIDASplayStreamPolling}. Up to \mVariable{numBytes} bytes of
new sample data from \mVariable{*data} will be copied to the stream
buffer, and the stream buffer write position is updated accordingly. The
function returns the number of bytes of sound data actually used. If
\mVariable{feedAll} is TRUE, the function will block the current thread
of execution until all sound data is used.


\subsubsection*{Return value}

The number of bytes of sound data actually used.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}

\mDocRef{MIDASplayStreamPolling}




%----------------------------- MIDASgetStreamBytesBuffered
\mDocEntry{MIDASgetStreamBytesBuffered} %K:Stream;Stream,MIDASgetStreamBytesBuffered;Stream,Buffer
\begin{verbatim}
DWORD MIDASgetStreamBytesBuffered(MIDASstreamHandle stream)
\end{verbatim}

Gets the number of bytes of stream currently buffered.


\subsubsection*{Input}

\begin{description}
    \item [stream] The stream handle
\end{description}


\subsubsection*{Description}

This function returns the number of bytes of sound data currently stored
in the stream buffer. It can be used to monitor the stream playback, and
possibly prepare to feed extra data if the figure gets too low.



\subsubsection*{Return value}

The number of bytes of sound data currently buffered.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}

\mDocRef{MIDASfeedStreamData}




%----------------------------- MIDASpauseStream
\mDocEntry{MIDASpauseStream} %K:Stream;Stream,MIDASpauseStream
\begin{verbatim}
BOOL MIDASpauseStream(MIDASstreamHandle stream)
\end{verbatim}

Pauses stream playback.


\subsubsection*{Input}

\begin{description}
    \item [stream] The stream to pause
\end{description}


\subsubsection*{Description}

This function pauses the playback of a stream. When a stream is paused,
stream data can be fed normally with \mDocRef{MIDASfeedStreamData}, but
nothing will actually be played. Playback can be resumed
with \mDocRef{MIDASresumeStream}.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}

\mDocRef{MIDASresumeStream}, \mDocRef{MIDASfeedStreamData}




%----------------------------- MIDASplayStreamFile
\mDocEntry{MIDASplayStreamFile} %K:Stream;Stream,MIDASplayStreamFile;Stream,File
\begin{verbatim}
MIDASstreamHandle MIDASplayStreamFile(char *fileName, 
    unsigned sampleType, unsigned sampleRate,
    unsigned bufferLength, int loopStream)
\end{verbatim}

Starts playing a digital audio stream from a file.


\subsubsection*{Input}

\begin{description}
    \item [fileName] Stream file name
    \item [sampleType] Stream sample type, see \mResWord{enum}
        \mDocRef{MIDASsampleTypes}
    \item [sampleRate] Stream sample rate
    \item [bufferLength] Stream playback buffer length in milliseconds
    \item [loopStream] 1 if the stream should be looped, 0 if not
\end{description}


\subsubsection*{Description}

This function starts playing a digital audio stream from a file. The
file must contain raw audio data with no headers --- to play WAVE files,
use \mDocRef{MIDASplayStreamWaveFile}. The function allocates
the stream buffer, creates a new thread that will read sample data from the
file to the stream buffer, and starts the Sound Device to play the stream.
The stream will continue playing until it is stopped with
\mDocRef{MIDASstopStream}. A Sound Device channel will be automatically
allocated for the stream.

The stream buffer length should be at least around 500ms if the stream file
is being read from a disc, to avoid breaks in stream playback


\subsubsection*{Return value}

MIDAS stream handle if successful, NULL if failed.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}

\mDocRef{MIDASplayStreamWaveFile}, \mDocRef{MIDASstopStream}




%----------------------------- MIDASplayStreamWaveFile
\mDocEntry{MIDASplayStreamWaveFile} %K:Stream;Stream,MIDASplayStreamWaveFile;\
                                    %K:Stream,File;WAVE;\
                                    %K:WAVE,MIDASplayStreamWaveFile
\begin{verbatim}
MIDASstreamHandle MIDASplayStreamWaveFile(char *fileName, 
    unsigned bufferLength, int loopStream)
\end{verbatim}

Starts playing a digital audio stream from a RIFF WAVE file.


\subsubsection*{Input}

\begin{description}
    \item [fileName] Stream file name
    \item [bufferLength] Stream playback buffer length in milliseconds
    \item [loopStream] 1 if the stream should be looped, 0 if not
\end{description}


\subsubsection*{Description}

This function starts playing a digital audio stream from a file. The
file must be a standard RIFF WAVE (\mFileName{.wav}) sound file
containing raw PCM sound data --- compressed WAVE files are not
supported. The function allocates
the stream buffer, creates a new thread that will read sample data from the
file to the stream buffer, and starts the Sound Device to play the stream.
The stream will continue playing until it is stopped with
\mDocRef{MIDASstopStream}. A Sound Device channel will be automatically
allocated for the stream.

The stream buffer length should be at least around 500ms if the stream file
is being read from a disk, to avoid breaks in stream playback


\subsubsection*{Return value}

MIDAS stream handle if successful, NULL if failed.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}

\mDocRef{MIDASplayStreamFile}, \mDocRef{MIDASstopStream}




%----------------------------- MIDASplayStreamPolling
\mDocEntry{MIDASplayStreamPolling} %K:Stream;Stream,MIDASplayStreamPolling;Stream,Polling
\begin{verbatim}
MIDASstreamHandle MIDASplayStreamPolling(unsigned sampleType, 
    unsigned sampleRate, unsigned bufferLength)
\end{verbatim}

Starts playing a digital audio stream in polling mode.


\subsubsection*{Input}

\begin{description}
    \item [sampleType] Stream sample type, see \mResWord{enum}
        \mDocRef{MIDASsampleTypes}
    \item [sampleRate] Stream sample rate
    \item [bufferLength] Stream playback buffer length in milliseconds
\end{description}


\subsubsection*{Description}

This function starts playing a digital audio stream in polling mode. It
allocates and empty stream buffer, and starts the Sound Device to play the
stream. Sample data can be fed to the stream buffer with
\mDocRef{MIDASfeedStreamData}. The stream will continue playing until it is
stopped with \mDocRef{MIDASstopStream}. This function will automatically
allocate a Sound Device channel for the stream.

To avoid breaks in playback, the stream buffer size should be at least twice
the expected polling period. That is, if you will be feeding data 5 times
per second (every 200ms), the buffer should be at least 400ms long.


\subsubsection*{Return value}

MIDAS stream handle if successful, NULL if failed.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}

\mDocRef{MIDASstopStream}, \mDocRef{MIDASfeedStreamData}




%----------------------------- MIDASresumeStream
\mDocEntry{MIDASresumeStream} %K:Stream;Stream,MIDASresumeStream
\begin{verbatim}
BOOL MIDASresumeStream(MIDASstreamHandle stream)
\end{verbatim}

Resumes stream playback after pause.


\subsubsection*{Input}

\begin{description}
    \item [stream] The stream to resume
\end{description}


\subsubsection*{Description}

This function resumes the playback of a stream that has been paused with
\mDocRef{MIDASpauseStream}.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}

\mDocRef{MIDASpauseStream}




%----------------------------- MIDASsetStreamPanning
\mDocEntry{MIDASsetStreamPanning} %K:Stream;Stream,MIDASsetStreamPanning;Panning
\begin{verbatim}
BOOL MIDASsetStreamPanning(MIDASstreamHandle stream,
    int panning);
\end{verbatim}

Changes stream panning position.


\subsubsection*{Input}

\begin{description}
    \item [stream] Stream handle for the stream
    \item [panning] New panning position for the stream
\end{description}


\subsubsection*{Description}

This function changes the panning position for a stream that is being played.
The initial volume is 0 (center). See description of \mResWord{enum}
\mDocRef{MIDASpanning} for information about the panning position values.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}

\mDocRef{MIDASsetStreamVolume}, \mDocRef{MIDASsetStreamRate}




%----------------------------- MIDASsetStreamRate
\mDocEntry{MIDASsetStreamRate} %K:Stream;Stream,MIDASsetStreamRate
\begin{verbatim}
BOOL MIDASsetStreamRate(MIDASstreamHandle stream,
    unsigned rate);
\end{verbatim}

Changes stream playback sample rate.


\subsubsection*{Input}

\begin{description}
    \item [stream] Stream handle for the stream
    \item [rate] New playback sample rate for the stream, in Hertz.
\end{description}


\subsubsection*{Description}

This function changes the playback sample rate for a stream that is being
played. The initial sample rate is given as an argument to the function that
starts stream playback.

Note that the stream playback buffer size is calculated based on the initial
sample rate, so the stream sample rate should not be changed very far from
that figure. In particular, playback sample rates over two times the initial
value may cause breaks in stream playback. Too low rates, on the other hand,
will increase latency.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}

\mDocRef{MIDASsetStreamVolume}, \mDocRef{MIDASsetStreamPanning}




%----------------------------- MIDASsetStreamVolume
\mDocEntry{MIDASsetStreamVolume} %K:Stream;Stream,MIDASsetStreamVolume;Volume
\begin{verbatim}
BOOL MIDASsetStreamVolume(MIDASstreamHandle stream,
    unsigned volume);
\end{verbatim}

Changes stream playback volume.


\subsubsection*{Input}

\begin{description}
    \item [stream] Stream handle for the stream
    \item [volume] New volume for the stream, 0--64.
\end{description}


\subsubsection*{Description}

This function changes the playback volume for a stream that is being
played. The initial volume is 64 (maximum).


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}

\mDocRef{MIDASsetStreamRate}, \mDocRef{MIDASsetStreamPanning}




%----------------------------- MIDASstopStream
\mDocEntry{MIDASstopStream} %K:Stream;Stream,MIDASstopStream
\begin{verbatim}
BOOL MIDASstopStream(MIDASstreamHandle stream)
\end{verbatim}

Stops playing a digital audio stream.


\subsubsection*{Input}

\begin{description}
    \item [stream] The stream that will be stopped
\end{description}


\subsubsection*{Description}

This function stops playing a digital audio stream. It stops the stream
player thread, deallocates the stream buffer, closes the stream file (if
playing from a file) and deallocates the stream playback channel.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

Win32, Linux


\subsubsection*{See also}

\mDocRef{MIDASplayStreamFile}, \mDocRef{MIDASplayStreamPolling}




% ***************************************************************************
% *
% *     MISCELLANEOUS
% *
% ***************************************************************************

\chapter{Miscellaneous}


% ---------------------------------------------------------------------------
%       Constants
% ---------------------------------------------------------------------------

\section{Constants}

This section describes all constants used with miscellaneous MIDAS
functions. They are grouped according to the \mResWord{enum} used to define
them.


%----------------------------- MIDASechoFilterTypes
\mDocEntry{MIDASechoFilterTypes} %K:Echo effects;Echo effects,Filters
\begin{verbatim}
enum MIDASechoFilterTypes
\end{verbatim}

\subsubsection*{Description}

These constants define different echo filter types for MIDAS Digital Audio
System echo effects. Each echo in an echo effect has a separate filter. If the
echo effect contains more than a couple of echoes, the echoes should usually
sound best with low-pass filtering.

\subsubsection*{Values}

\begin{description}
    \item [MIDAS\_{}ECHO\_{}FILTER\_{}NONE] No filtering
    \item [MIDAS\_{}ECHO\_{}FILTER\_{}LOWPASS] Low-pass filtering, higher
        frequencies are filtered out.
    \item [MIDAS\_{}ECHO\_{}FILTER\_{}HIGHPASS] High-pass filtering, lower
        frequencies are filtered out.
\end{description}



%----------------------------- MIDASpostProcPosition
\mDocEntry{MIDASpostProcPosition} %K:Post processors;Post processors,position
\begin{verbatim}
enum MIDASpostProcPosition
\end{verbatim}

\subsubsection*{Description}

These constants define the position of a new post-processor in the system
post-processor list when it is being added with
\mDocRef{MIDASaddPostProcessor}. The order of the post-processors in the list
determines the order in which they are applied to the sound output. 

MIDAS internal echo effects are always inserted to the beginning of the list,
and filters to the end. Most user post-processors should be inserted to the
beginning as well.

\subsubsection*{Values}

\begin{description}
    \item [MIDAS\_{}POST\_{}PROC\_{}FIRST] The post-processor is inserted to
        the beginning of the list
    \item [MIDAS\_{}POST\_{}PROC\_{}LAST] The post-processor is inserted to
        the end of the list
\end{description}




\section{Data types}

This section describes all data types used with miscellaneous MIDAS functions.



%----------------------------- MIDASecho
\mDocEntry{MIDASecho} %K:Echo effects;Echo effects,MIDASecho
\begin{verbatim}
typedef struct
{
    unsigned    delay;
    int         gain;
    int         reverseChannels;
    unsigned    filterType;
} MIDASecho;
\end{verbatim}

MIDAS echo effect echo structure.


\subsubsection*{Members}

\begin{description}
    \item [delay] Echo delay in milliseconds, in 16.16 fixed point
    \item [gain] Echo gain, in 16.16 fixed point
    \item [reverseChannels] 1 if the left and right channels should be
        reversed in the echo
    \item [filterType] Echo filter type, see \mResWord{enum}
        \mDocRef{MIDASechoFilterTypes}. 
\end{description}


\subsubsection*{Description}

This structure defines a single echo of a MIDAS Digital Audio System echo
effect. One or more of echoes form an echo set (\mDocRef{MIDASechoSet}), which
can then be activated with \mDocRef{MIDASaddEchoEffect}.

The \mVariable{delay} and \mVariable{gain} values are given in 16.16 fixed
point, which essentially means multiplying the value with 65536. Thus, a delay
of 32 milliseconds becomes 2097152 (0x200000), and a gain of 0.5 32768
(0x8000). If \mVariable{reverseChannels} is 1, data from the left channel is
echoed on the right one and vice versa.



%----------------------------- MIDASechoHandle
\mDocEntry{MIDASechoHandle} %K:Echo effects;Echo effects,MIDASechoHandle
\begin{verbatim}
typedef ... MIDASechoHandle;
\end{verbatim}

\subsubsection*{Description}

\mDocRef{MIDASechoHandle} is an echo handle that defines an echo effect that
is being used.



%----------------------------- MIDASechoSet
\mDocEntry{MIDASechoSet} %K:Echo effects;Echo effects,MIDASechoSet
\begin{verbatim}
typedef struct
{
    int         feedback;
    int         gain;
    unsigned    numEchoes;
    MIDASecho   *echoes;
} MIDASechoSet;
\end{verbatim}

MIDAS echo effect echo set structure.


\subsubsection*{Members}

\begin{description}
    \item [feedback] Echo effect feedback, in 16.16 fixed point
    \item [gain] Echo effect total gain, in 16.16 fixed point
    \item [numEchoes] Total number of echoes in the echo effect
    \item [echoes] Pointer to the echoes
\end{description}


\subsubsection*{Description}

This structure defines a MIDAS Digital Audio System echo set, that can be used
with \mDocRef{MIDASaddEchoEffect}. The echo set consists of one or more echoes
(\mDocRef{MIDASecho}), plus two controlling variables: \mVariable{feedback}
controls the amount of feedback for the effect, ie. the amount of echoed data
that is recycled back to the echo delay line, and \mVariable{gain} controls
the total gain for the effect.

The \mVariable{feedback} and \mVariable{gain} values are given in 16.16 fixed
point, which essentially means multiplying the value with 65536. Thus, a
feedback of 0.8 becomes 52428, and a gain of 0.25 16384
(0x1000). \mVariable{feedback} typically controls the strength of the echo
effect, and is kept at a value below 1, while \mVariable{gain} can be used to
decrease the total volume from the effect to reduce distortion by setting it
to a value below 1. Values above 1 are also possible for both
\mVariable{feedback} and \mVariable{gain}, but should be used with care.



%----------------------------- MIDASpostProcessor
\mDocEntry{MIDASpostProcessor} %K:Post processors;\
        %K:Post processors,structure;Post processors,MIDASpostProcessor

\begin{verbatim}
typedef struct
{
    struct _MIDASpostProcessor *next, *prev;
    void *userData;
    MIDASpostProcFunction floatMono;
    MIDASpostProcFunction floatStereo;
    MIDASpostProcFunction intMono;
    MIDASpostProcFunction intStereo;
} MIDASpostProcessor;
\end{verbatim}

MIDAS user post-processor structure.


\subsubsection*{Members}

\begin{description}
    \item [next, prev, userData] Reserved for MIDAS use
    \item [floatMono] Pointer to the floating-point mono post-processor
        function, see \mApiRef{MIDASpostProcFunction}
    \item [floatStereo] Pointer to the floating-point stereo post-processor
        function, see \mApiRef{MIDASpostProcFunction}
    \item [intMono] Pointer to the integer mono post-processor
        function, see \mApiRef{MIDASpostProcFunction}
    \item [intStereo] Pointer to the integer stereo post-processor
        function, see \mApiRef{MIDASpostProcFunction}
\end{description}


\subsubsection*{Description}

This structure describes a MIDAS Digital Audio System user post-processor,
used with \mDocRef{MIDASaddPostProcessor} and
\mDocRef{MIDASremovePostProcessor}. The structure consists of four function
pointers, for all possible post-processing functions, plus three reserved
members. If any of the post-processor function pointers is NULL, it is simply
ignored.

The functions actually used depend on the mixing and output mode, but to be
safe all of them should be implemented. The floating-point functions will
receive 32-bit floating-point data (C \mResWord{float}), and the integer ones
32-bit integers (C \mResWord{int}). The sample data range is normal signed
16-bit range, -32768--32767, but the data has not been clipped yet, so smaller
and larger values are also possible --- the user should clip them if necessary.
The data needs to be modified in place.

Note that it is not necessary to use the post-processing functions to actually
modify the data. They could also be used, for example, to gather statistics
about the output sample data for spectrum analyzers. However, remember that
the post-processing functions are called in the context of the MIDAS player
thread or interrupt. Therefore they may not call MIDAS functions, and should
be kept as simple and short as possible.



%----------------------------- MIDASpostProcFunction
\mDocEntry{MIDASpostProcFunction} %K:Post processors;\
        %K:Post processors,function;Post processors,MIDASpostProcFunction

\begin{verbatim}
typedef void (MIDAS_CALL *MIDASpostProcFunction)(void *data,
    unsigned numSamples, void *user);
\end{verbatim}


\subsubsection*{Description}

A MIDAS user post-processing function, used with in
\mDocRef{MIDASpostProcessor} structures. The function receives three
arguments: pointer to the sample data to be processed, the number of samples
of data to process, and an user pointer passed to
\mDocRef{MIDASaddPostProcessor}. 



% ---------------------------------------------------------------------------
%       Functions
% ---------------------------------------------------------------------------

\newpage
\section{Functions}

This section describes all available miscellaneous MIDAS Digital Audio System
functions.



%----------------------------- MIDASaddEchoEffect
\mDocEntry{MIDASaddEchoEffect} %K:Echo effects;Echo effects,MIDASaddEchoEffect
\begin{verbatim}
MIDASechoHandle MIDASaddEchoEffect(MIDASechoSet *echoSet);
\end{verbatim}

Adds an echo effect to the output.


\subsubsection*{Input}

\begin{description}
    \item [echoSet] The echo set that describes the effect, see
        \mDocRef{MIDASechoSet} 
\end{description}


\subsubsection*{Description}

This function adds an echo effect to the output. An echo effect is described
by a \mDocRef{MIDASechoSet} structure, and can consist of one or more
echoes. Any number of echo effects can be active simultaneously, the effects
added last are processed first. After this function returns, the echo set
structure is no longer used by MIDAS and may be deallocated.

Note that echo effects can use large amounts of CPU time, and may even be
unusable on 486-class and slower computers.


\subsubsection*{Return value}

MIDAS echo handle for the echo effect if successful, NULL if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASremoveEchoEffect}




%----------------------------- MIDASaddPostProcessor
\mDocEntry{MIDASaddPostProcessor} %K:Post processors;\
                                  %K:Post processors,MIDASaddPostProcessor
\begin{verbatim}
BOOL MIDASaddPostProcessor(MIDASpostProcessor *postProc,
    unsigned procPos, void *userData);
\end{verbatim}

Adds a user post-processor to the output.


\subsubsection*{Input}

\begin{description}
    \item [postProc] A pointer to a \mDocRef{MIDASpostProcessor} structure
        that describes the post-processor.
    \item [procPos] The post-processor position in the post-processor list,
        see \mResWord{enum} \mApiRef{MIDASpostProcPosition}.
    \item [userData] An user data pointer that will be passed to the
        post-processing functions.
\end{description}


\subsubsection*{Description}

This function adds a user post-processor to the output. The post-processor can
be used to alter the output sound data in interesting ways, or to gather
information about the data for graphical displays and such. See the
\mDocRef{MIDASpostProcessor} structure documentation for more information.

The \mVariable{postProc} structure may {\bf not} be deallocated or re-used
after this function returns, as it is used by MIDAS Digital Audio System
internally. The post-processor can be removed from the output with
\mDocRef{MIDASremovePostProcessor}, after which the structure can be
deallocated. 


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASremovePostProcessor}




%----------------------------- MIDASremoveEchoEffect
\mDocEntry{MIDASremoveEchoEffect} %K:Echo effects;\
                                  %K:Echo effects,MIDASremoveEchoEffect
\begin{verbatim}
BOOL MIDASremoveEchoEffect(MIDASechoHandle echoHandle);
\end{verbatim}

Removes an echo effect from the output.


\subsubsection*{Input}

\begin{description}
    \item [echoHandle] Echo handle for the effect that will be removed
\end{description}


\subsubsection*{Description}

This function removes an echo effect, added with \mDocRef{MIDASaddEchoEffect},
from the output. The echo handle will no longer be valid after this function
has been called. Echo effects do not need to be removed in the same order they
were added in, but can be added and removed in any order.


\subsubsection*{Return value}

TRUE if successful, FALSE if not


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASaddEchoEffect}




%----------------------------- MIDASremovePostProcessor
\mDocEntry{MIDASremovePostProcessor} %K:Post processors;\
                                     %K:Post processors,MIDASremovePostProcessor
\begin{verbatim}
BOOL MIDASremovePostProcessor(MIDASpostProcessor *postProc);
\end{verbatim}

Removes a user post-processor from the output.


\subsubsection*{Input}

\begin{description}
    \item [postProc] A pointer to a \mDocRef{MIDASpostProcessor} structure
        that describes the post-processor.
\end{description}


\subsubsection*{Description}

This function removes a user post-processor added with
\mApiRef{MIDASaddPostProcessor} from the output. The post-processing functions
will no longer be called, and the post-processor structure may be deallocated.


\subsubsection*{Return value}

TRUE if successful, FALSE if not.


\subsubsection*{Operating systems}

All


\subsubsection*{See also}

\mDocRef{MIDASaddPostProcessor}




\end{document}
%eq




