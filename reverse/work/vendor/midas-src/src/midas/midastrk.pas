{*      midastrk.pas
 *
 * MIDAS tracker API, Delphi interface unit
 *
 * $Id: midastrk.pas,v 1.3.2.1 1997/11/20 21:09:59 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*}

{ You'll probably need to disable range checking }

unit midastrk;


interface


uses wintypes, midasdll;


const
    {* The command numbers: *}
    {enum MTRKcommands}
    MTRKcNone = 0;                       { no command }
    MTRKcArpeggio = 1;                   { arpeggio }
    MTRKcSlideUp = 2;                    { period slide up }
    MTRKcSlideDown = 3;                  { period slide down }
    MTRKcTonePortamento = 4;             { tone portamento }
    MTRKcVibrato = 5;                    { vibrato }
    MTRKcTPortVSlide = 6;                { tone portamento + volume slide }
    MTRKcVibVSlide = 7;                  { vibrato + volume slide }
    MTRKcTremolo = 8;                    { tremolo }
    MTRKcSetPanning = 9;                 { set panning (PT cmd 8) }
    MTRKcSampleOffset = 10;              { set sample offset }
    MTRKcVolumeSlide = 11;               { volume slide }
    MTRKcPositionJump = 12;              { position jump }
    MTRKcSetVolume = 13;                 { set volume }
    MTRKcPatternBreak = 14;              { pattern break (to a row) }
    MTRKcSetSpeed = 15;                  { set speed }
    MTRKcSetTempo = 16;                  { set tempo in BPM }
    MTRKcFineSlideUp = 17;               { fine period slide up }
    MTRKcFineSlideDown = 18;             { fine period slide down }
    MTRKcPatternLoop = 19;               { pattern loop set/loop }
    MTRKcSetPanning16 = 20;              { set 16-point panning value }
    MTRKcPTRetrig = 21;                  { Protracker-style retrig note }
    MTRKcFineVolSlideUp = 22;            { fine volume slide up }
    MTRKcFineVolSlideDown = 23;          { fine volume slide down }
    MTRKcNoteCut = 24;                   { note cut }
    MTRKcNoteDelay = 25;                 { note delay }
    MTRKcPatternDelay = 26;              { pattern delay }
    MTRKcSetMVolume = 27;                { set master volume }
    MTRKcMVolSlide = 28;                 { master volume slide }
    MTRKcS3MRetrig = 29;                 { S3M retrig note }
    MTRKcMusicSync = 30;                 { music synchronization command }
    MTRKcExtraFineSlideUp = 31;          { extra fine period slide up }
    MTRKcExtraFineSlideDown = 32;        { extra fine period slide down }
    MTRKcPanSlide = 33;                  { panning slide }
    MTRKnumCommands = 34;                { total number of commands }

    {* Sample looping modes: *}
    {enum MTRKloopModes}
    MTRKloopNone = 0;                   { no looping }
    MTRKloop1 = 1;                      { 1 loop, release ends note }
    MTRKloop1Rel = 2;                   { 1 loop, sample data after loop is
                                          played when note is released }
    MTRKloop2 = 3;                      { 2 loops, when note is released
                                          playing is continued looping the
                                          second loop }
    MTRKloopAmigaNone = 4;              { Amiga compatible looping, no
                                          loop - ALE logic enabled anyway }
    MTRKloopAmiga = 5;                  { Amiga compatible looping - 1
                                          loop, ALE logic enabled }

    {* Sample loop types: (for a single loop) *}
    {enum MTRKloopTypes}
    {MTRKloopNone = 0,}                 { no looping }
    MTRKloopUnidir = 1;                 { unidirectional loop }
    MTRKloopBidir = 2;                  { bidirectional loop }



type
    Pbyte = ^byte;


{ The sample structure: }
type MTRKsample =
    record
        name : array[0..31] of char;    { sample name }
        sdHandle : dword;               { Sound Device sample handle }
        sample : Pbyte;                 { sample data pointer, NULL if sample
                                          data only in SD memory }
        sampleLength : dword;           { sample length }
        sampleType : byte;              { sample type, see enum
                                          sdSampleType }
        loopMode : byte;                { sample looping mode, see enum
                                          sdLoopMode }
        loop1Type : byte;               { first loop type, see enum
                                          sdLoopType }
        loop2Type : byte;               { second loop type, see enum
                                          sdLoopType }
        loop1Start : dword;             { first loop start }
        loop1End : dword;               { first loop end }
        loop2Start : dword;             { second loop start }
        loop2End : dword;               { second loop end }
        volume : dword;                 { sample volume }
        gvolume : dword;                { sample global volume }
        panning : integer;              { sample panning position }
        baseTune : integer;             { base tune }
        finetune : integer;             { fine tuning value }
        fileOffset : dword;             { sample start offset in module
                                          file }
        vibType : byte;                 { IT auto-vibrato type }
        vibSweep : byte;                { IT auto-vibrato sweep value }
        vibDepth : byte;                { IT auto-vibrato depth }
        vibRate : byte;                 { IT auto-vibrato rate }
    { In tracker mode, the sample pointer is valid. }
    end;
    PMTRKsample = ^MTRKsample;


{ Envelope point structure: }
type MTRKenvelopePoint =
    record
        x : word;
        y : smallint;
    end;
    PMTRKenvelopePoint = ^MTRKenvelopePoint;


{ Instrument envelope structure. }
type MTRKenvelope =
    record
        numPoints : byte;               { number of points in envelope }
        sustain : smallint;             { sustain point number, -1 if no
                                          sustain is used }
        loopStart : smallint;           { envelope loop start point number,
                                          -1 if no loop }
        loopEnd : smallint;             { envelope loop end point number, -1
                                          if no loop }
        susLoopStart : smallint;        { sustain loop start point number, -1
                                          if no loop (IT) }
        susLoopEnd : smallint;          { sustain loop end point number, -1
                                          if no loop (IT) }
        points : array[0..24] of MTRKenvelopePoint;
                                        { envelope points: a maximum of 25
                                          points in (X,Y) coordinate pairs }
    end;
    PMTRKenvelope = ^MTRKenvelope;



{ The instrument structure. In tracker mode, with XMs, the module loader
  always allocates space for 16 sample pointers for each instrument. }
type MTRKinstrument =
    record
        name : array[0..31] of char;    { instrument name }
        numSamples : dword;             { number of samples }
        noteSamples : Pbyte;            { sample numbers for all keyboard
                                          notes, NULL if sample 0 is used
                                          for all (always 96 bytes) }
        volEnvelope : PMTRKenvelope;    { pointer to volume envelope info or
                                          NULL if volume envelope is not
                                          used }
        panEnvelope : PMTRKenvelope;    { pointer to panning envelope info or
                                          NULL if panning envelope is not
                                           used }
        pitchEnvelope : PMTRKenvelope;  { (IT) pointer to pitch envelope info
                                           or NULL if pitch envelope is not
                                           used }        
        volFadeout : integer;           { FT2/IT volume fade-out speed }
        volume : dword;                 { IT instrument volume }
        vibType : byte;                 { FT2 auto-vibrato type }
        vibSweep : byte;                { FT2 auto-vibrato sweep value }
        vibDepth : byte;                { FT2 auto-vibrato depth }
        vibRate : byte;                 { FT2 auto-vibrato rate }
        NNA : byte;                     { IT New Note Action }
        DCT : byte;                     { IT Duplicate note Check Type }
        DCA : byte;                     { IT Duplicate note Check Action }
        PPS : byte;                     { IT Pitch Pan Separation }
        PPC : byte;                     { IT Pitch Pan Center note }
        DfP : byte;                     { IT Default Panning }
        RV : byte;                      { IT Random Volume }
        RP : byte;                      { IT Random Panning }
        
        used : integer;                 { 1 if instrument is used, 0 if not.
                                          Unused instruments are not loaded
                                          or added to Sound Device }
        samples : array [0..0] of PMTRKsample;
                                        { pointers to samples - the numbers
                                          in the noteSamples[] array refer
                                          to these sample numbers in FT2 mode,
                                          and to global sample numbers in IT
                                          mode. }
    end;
    PMTRKinstrument = ^MTRKinstrument;



{ The pattern structure: }
type MTRKpattern =
    record
        length : dword;                 { pattern size in bytes, INCLUDING
                                          this header }
        rows : dword;                   { number of rows in pattern }
        data : array [0..0] of byte;    { pattern data }

    {* Pattern data format:
       The first byte is compression information byte. Bits 0-4 form the
       channel number (0-31) and bits 5-7 compression info:
                bit 5   if 1, note and instrument number bytes follow
                bit 6   if 1, volume column byte follows
                bit 7   if 1, command and infobyte bytes follow
       If the compression infobyte is 0, it marks the end of the current
       row. Possible other data follows the compression information byte in
       the following order: note, instrument, volume, command, infobyte.

       In note numbers the upper nybble is the octave number, and lower
       nybble note number in octave. So, for example, D#2 becomes 0x23.
       0xFE as a note number represents a release note, and 0xFF no new
       note as does 0xFF as an instrument number. Valid instrument numbers
       range from 0 to 254 inclusive.

       Volume column number 0x50 is ignored, like command number 0.

       Pattern data decompressor marks that all data entries have all
       elements. It also allocates space for 256 rows of pattern data.
       "length" is ignored in decompressed patterns. Each pattern data
       row has module.numChannels MTRKpatternDataEntry structures for the
       actual data, plus a 0 byte for the end-of-row marker. Thus the
       length of each row is
       module.numChannels*sizeof(MTRKpatternDataEntry) + 1. *}
    end;
    PMTRKpattern = ^MTRKpattern;




{ Decompressed pattern data entry: }
type MTRKpatternDataEntry =
    record
        compInfo, note, instrument, volume, command, infobyte : byte;
    end;
    PMTRMpatternDataEntry = ^MTRKpatternDataEntry;



{ The MIDAS module structure - MIDASmodule is really a pointer to this.
  In tracker mode, the loaders always allocate room for 256 instrument
  structure pointers, 16384 sample pointers, 256 pattern pointers,
  32 channels of panning positions and 256 song data entries. }
type MTRKmodule =
    record
        name : array[0..31] of char;    { module name }
        playMode : integer;             { module playing mode }
        songLength : dword;             { song length }
        restart : dword;                { song restart position }
        numInsts : dword;               { number of instruments }
        numPatts : dword;               { number of patterns }
        numChannels : byte;             { number of channels in module }
        masterVolume : byte;            { initial master volume }
        speed : byte;                   { initial speed }
        tempo : byte;                   { initial BPM tempo }
        panning : ^integer;             { initial channel panning
                                          positions }
        songData: ^word;                { song data - pattern playing
                                          order }
        instruments : ^PMTRKinstrument; { instrument data }
        samples : ^PMTRKsample;         { sample data }        
        patterns : ^PMTRKpattern;       { pattern data }

        playFlags : dword; {word? how to implement bitfields?}
    (*struct  /* Format-specific playing information flags: */
    {
        int     ptLimits : 1;           /* ST3: Protracker limits */
        int     fastVolSlides : 1;      /* ST3: Fast volume slides */
        int     extOctaves : 1;         /* PT: Extended octaves needed */
        int     linFreqTable : 1;       /* FT2: Linear frequency table used */
    } playFlags;*)
    end;
    PMTRKmodule = ^MTRKmodule;


function MTRKdecompressPattern(module : PMTRKmodule; pattern : dword) :
    boolean; stdcall;
function MTRKcompressPattern(module : PMTRKmodule; pattern : dword) :
    boolean; stdcall;

function MTRKplayModuleInstruments(module : PMTRKmodule) :
    MIDASmodulePlayHandle; stdcall;
function MTRKplayInstrumentNote(playHandle : MIDASmodulePlayHandle;
    channel : dword; instrument : dword; note : dword) : boolean; stdcall;
function MTRKplaySampleNote(playHandle : MIDASmodulePlayHandle;
    channel : dword; instrument : dword; sample : dword; note : dword) :
    boolean; stdcall;
function MTRKreleaseNote(playHandle : MIDASmodulePlayHandle; channel : dword) :
    boolean; stdcall;
function MTRKstopNote(playHandle : MIDASmodulePlayHandle; channel : dword) :
    boolean; stdcall;

function MTRKmemAlloc(numBytes : dword) : pointer; stdcall;
function MTRKmemFree(block : pointer) : boolean; stdcall;

function MTRKresetSample(module : PMTRKmodule; instrument, sample : dword;
    playHandle : MIDASmodulePlayHandle) : boolean; stdcall;
function MTRKremoveInstrument(module : PMTRKmodule; instrument : dword;
    playHandle : MIDASmodulePlayHandle) : boolean; stdcall;
function MTRKremoveSample(module : PMTRKmodule; instrument, sample : dword;
    playHandle : MIDASmodulePlayHandle) : boolean; stdcall;
function MTRKnewInstrument(module : PMTRKmodule; instrument : dword;
    playHandle : MIDASmodulePlayHandle) : boolean; stdcall;
function MTRKnewSample(module : PMTRKmodule; instrument, sample : dword;
    playHandle : MIDASmodulePlayHandle) : boolean; stdcall;

function MTRKsetPlayTempo(playHandle : MIDASmodulePlayHandle; tempo : dword) :
    boolean; stdcall;
function MTRKsetPlaySpeed(playHandle : MIDASmodulePlayHandle; speed : dword) :
    boolean; stdcall;
function MTRKgetPlayTempo(playHandle : MIDASmodulePlayHandle) : dword; stdcall;
function MTRKgetPlaySpeed(playHandle : MIDASmodulePlayHandle) : dword; stdcall;



implementation

function MTRKdecompressPattern(module : PMTRKmodule; pattern : dword) :
    boolean; stdcall; external 'mtrk11.dll' name '_MTRKdecompressPattern@8';
function MTRKcompressPattern(module : PMTRKmodule; pattern : dword) :
    boolean; stdcall; external 'mtrk11.dll' name '_MTRKcompressPattern@8';

function MTRKplayModuleInstruments(module : PMTRKmodule) :
    MIDASmodulePlayHandle;
    stdcall; external 'mtrk11.dll' name '_MTRKplayModuleInstruments@4';
function MTRKplayInstrumentNote(playHandle : MIDASmodulePlayHandle;
    channel : dword; instrument : dword; note : dword) : boolean;
    stdcall; external 'mtrk11.dll' name '_MTRKplayInstrumentNote@16';
function MTRKplaySampleNote(playHandle : MIDASmodulePlayHandle;
    channel : dword; instrument : dword; sample : dword; note : dword) :
    boolean;
    stdcall; external 'mtrk11.dll' name '_MTRKplaySampleNote@20';
function MTRKreleaseNote(playHandle : MIDASmodulePlayHandle; channel : dword) :
    boolean;
    stdcall; external 'mtrk11.dll' name '_MTRKreleaseNote@8';
function MTRKstopNote(playHandle : MIDASmodulePlayHandle; channel : dword) :
    boolean;
    stdcall; external 'mtrk11.dll' name '_MTRKstopNote@8';

function MTRKmemAlloc(numBytes : dword) : pointer;
    stdcall; external 'mtrk11.dll' name '_MTRKmemAlloc@4';
function MTRKmemFree(block : pointer) : boolean;
    stdcall; external 'mtrk11.dll' name '_MTRKmemFree@4';

function MTRKresetSample(module : PMTRKmodule; instrument, sample : dword;
    playHandle : MIDASmodulePlayHandle) : boolean;
stdcall; external 'mtrk11.dll' name '_MTRKresetSample@16';
function MTRKremoveInstrument(module : PMTRKmodule; instrument : dword;
    playHandle : MIDASmodulePlayHandle) : boolean;
    stdcall; external 'mtrk11.dll' name '_MTRKremoveInstrument@12';
function MTRKremoveSample(module : PMTRKmodule; instrument, sample : dword;
    playHandle : MIDASmodulePlayHandle) : boolean;
    stdcall; external 'mtrk11.dll' name '_MTRKremoveSample@14';
function MTRKnewInstrument(module : PMTRKmodule; instrument : dword;
    playHandle : MIDASmodulePlayHandle) : boolean;
    stdcall; external 'mtrk11.dll' name '_MTRKnewInstrument@12';
function MTRKnewSample(module : PMTRKmodule; instrument, sample : dword;
    playHandle : MIDASmodulePlayHandle) : boolean;
    stdcall; external 'mtrk11.dll' name '_MTRKnewSample@16';

function MTRKsetPlayTempo(playHandle : MIDASmodulePlayHandle; tempo : dword) :
    boolean;
    stdcall; external 'mtrk11.dll' name '_MTRKsetPlayTempo@8';
function MTRKsetPlaySpeed(playHandle : MIDASmodulePlayHandle; speed : dword) :
    boolean;
    stdcall; external 'mtrk11.dll' name '_MTRKsetPlaySpeed@8';
function MTRKgetPlayTempo(playHandle : MIDASmodulePlayHandle) : dword;
    stdcall; external 'mtrk11.dll' name '_MTRKgetPlayTempo@4';
function MTRKgetPlaySpeed(playHandle : MIDASmodulePlayHandle) : dword;
    stdcall; external 'mtrk11.dll' name '_MTRKgetPlaySpeed@4';


BEGIN
END.


{*
 * $Log: midastrk.pas,v $
 * Revision 1.3.2.1  1997/11/20 21:09:59  pekangas
 * Brought up to date
 *
 * Revision 1.3  1997/08/13 18:06:48  pekangas
 * Brought up to date
 *
 * Revision 1.2  1997/07/31 10:56:50  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.1  1997/05/26 16:39:59  pekangas
 * Initial revision
 *
*}
