{*      DBP.PAS
 *
 * MIDAS debug module player
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*}


{$IFNDEF __BPREAL__}
{$DEFINE NOEMS}
{$ENDIF}


uses crt, dos, MIDAS, mParser, Errors, mGlobals, mMem, SDevice,
   MPlayer, DMA, DSM, S3M, MODp, MTM, mFile, mConfig
{$IFDEF REALVUMETERS}
    , VU
{$ENDIF}
{$IFNDEF NOEMS}
    , EMS
{$ENDIF}
    ;


    { pointers to all Module Players: }
const
    modulePlayers : array[0..(NUMMPLAYERS-1)] of PModulePlayer = (
        @mpS3M, @mpMOD, @mpMTM );




procedure WaitVR; assembler;
asm
        mov     dx,03DAh
@wvr:   in      al,dx
        test    al,8
        jz      @wvr
end;


procedure WaitDE; assembler;
asm
        mov     dx,03DAh
@wde:   in      al,dx
        test    al,1
        jnz     @wde
end;



procedure SetBorder(color : byte); assembler;
asm
        mov     dx,03C0h
        mov     al,31h
        out     dx,al
        mov     al,color
        out     dx,al
end;



var
    free1, free2 : longint;


procedure showheap;
begin
    free2 := MemAvail;
    WriteLn(free2, ' bytes memory free - ', free1-free2, ' bytes used.');
end;




{****************************************************************************\
*
* Function:     toASCIIZ(dest : PChar; str : string);
*
* Description:  Converts a string to ASCIIZ format. (StrPCopy is NOT available
*               in real mode!)
*
* Input:        msg : string            string to be converted
*               dest : PChar            destination buffer
*
\****************************************************************************}

procedure toASCIIZ(dest : PChar; str : string);
var
    spos, slen : integer;
    i : integer;

begin
    spos := 0;                          { string position = 0 }
    slen := ord(str[0]);                { string length }

    { copy string to ASCIIZ conversion buffer: }
    while spos < slen do
    begin
        dest[spos] := str[spos+1];
        spos := spos + 1;
    end;

    dest[spos] := chr(0);               { put terminating 0 to end of string }
end;




var
    error, plMusic : integer;
    module : PmpModule;
    SD : PSoundDevice;
    MP : PModulePlayer;
    key : char;
    meter : word;
    info : PmpInformation;
    i : integer;
    stopPlay : boolean;
    str : array[0..256] of char;
    fname : string;
    chMuted : array[0..31] of integer;
    chNum : integer;


procedure DrawMeters;
var
    i, error : integer;
    meter, pos : word;
    rate : longint;
    chan : PmpChanInfo;

begin
    { do all channels: }
    for i := 0 to (info^.numChannels-1) do
    begin
        { point chan to current channel information: }
        chan := @info^.chans^[i];

        { check that the channel has a valid instrument set }
        if (chan^.instrument > 0) and
            (chan^.instrument <= module^.numInsts) and
            (chMuted[i] = 0) then
        begin
{$IFDEF REALVUMETERS}
            { read channel playing rate: }
            error := SD^.GetRate(i, @rate);
            if error <> OK then
                midasError(error);

            { read channel playing position: }
            error := SD^.GetPosition(i, @pos);
            if error <> OK then
                midasError(error);

            { if there is sound being player, calculate VU-meter value: }
            if rate <> 0 then
            begin
                error := vuMeter(
                    module^.insts^[chan^.instrument-1].sdInstHandle,
                    rate, pos, chan^.volume, @meter);
                if error <> OK then
                    midasError(error);
            end
            else
                { no sound - meter = 0; }
                meter := 0;
{$ELSE}
            meter := chan^.volumebar;
{$ENDIF}
        end
        else
        begin
            { no valid instrument - set meter to zero }
            meter := 0;
        end;

        { Draw the VU-meter: }
asm
        cld
        mov     es,SegB800              { point es to screen segment }

        mov     ax,160
        mul     i                       { i = channel number = y-coordinate }
        mov     di,ax                   { address = 160 * i }
        mov     bx,64                   { bx = total amount to draw }
        mov     cx,meter                { cx = vu meter }
        sub     bx,cx                   { bx = amount left after meter }
        test    cx,cx
        jz      @nometer
        mov     ax,$0BFE                { draw first 'meter' boxes with }
        rep     stosw                   { attribute $0B }

@nometer:
        mov     cx,bx                   { cx = amount to draw after meter }
        test    cx,cx
        jz      @done
        mov     ax,$08FE                { draw the rest of the 64 boxes with }
        rep     stosw                   { attribute $0B }
@done:
end;

    end;
end;



function PlayModule(fileName : Pchar) : PmpModule;
var
    header : pointer;
    f : fileHandle;
    module : PmpModule;
    error, mpNum, recognized : integer;
begin
    { allocate memory for module header: }
    error := memAlloc(MPHDRSIZE, @header);
    if error <> OK then
        midasError(error);

    { open module file: }
    error := fileOpen(fileName, fileOpenRead, @f);
    if error <> OK then
        midasError(error);

    { read MPHDRSIZE bytes of module header: }
    error := fileRead(f, header, MPHDRSIZE);
    if error <> OK then
        midasError(error);

    error := fileClose(f);
    if error <> OK then
        midasError(error);

    { Search through all Module Players to find one that recognizes the
      file header: }
    mpNum := 0;
    MP := NIL;
    while (mpNum < NUMMPLAYERS) and (MP = NIL) do
    begin
        error := modulePlayers[mpNum]^.Identify(header, @recognized);
        if error <> OK then
            midasError(error);
        if recognized = 1 then
            MP := modulePlayers[mpNum];
        mpNum := mpNum + 1;
    end;

    { deallocate module header: }
    error := memFree(header);
    if error <> OK then
        midasError(error);

    if MP = NIL then
    begin
        midasClose;
        errErrorExit('Error: Unknown module format')
    end;

    { load the module file using correct Module Player: }
{$IFDEF REALVUMETERS}
    module := midasLoadModule(fileName, MP, @vuPrepare);
{$ELSE}
    module := midasLoadModule(fileName, MP, NIL);
{$ENDIF

    { play the module: }
    midasPlayModule(module, 0);

    PlayModule := module;
end;



procedure FreeModule(module : PmpModule);
var
    i, error : integer;
    insthdl : word;
begin
{$IFDEF REALVUMETERS}
    { Deallocate VU-meter information for all instruments: }
    for i := 0 to (module^.numInsts-1) do
    begin
        insthdl := module^.insts^[i].sdInstHandle;
        if insthdl <> 0 then
        begin
            error := vuRemove(insthdl);
            if error <> OK then
                midasError(error);
        end
    end;
{$ENDIF}
    midasFreeModule(module);
end;



BEGIN
    if ParamCount < 1 then
    begin
        WriteLn('Usage:  DBP     <filename> [MIDAS options]');
        halt;
    end;

    free1 := MemAvail;
    WriteLn(free1, ' bytes free');

    midasSetDefaults;                       { set MIDAS defaults }
    midasParseEnvironment;                  { parse "MIDAS" environment }
    { midasConfig; }

    ClrScr;
    GotoXY(1, 25);

    { parse MIDAS command line options: }
    for i := 2 to (ParamCount) do
    begin
        toASCIIZ(str, ParamStr(i));
        midasParseOption(@str[1]);
    end;

    for i := 0 to 31 do
        chMuted[i] := 0;

    midasInit;                              { initialize MIDAS }
    SD := midasSD;

{$IFDEF REALVUMETERS}
    { Initialize real VU meters: }
    error := vuInit;
    if error <> OK then
        midasError(error);
{$ENDIF}

    showheap;
    toASCIIZ(str, ParamStr(1));
    (**)

    module := PlayModule(str);              { load and play module }
    MP := midasMP;
    showheap;
    WriteLn('Playing...');
    WriteLn('Keys:');
    WriteLn('        Esc     Exit');
    WriteLn('        N       New module');
    WriteLn('        1-9     Toggle channel on/off');

    (**)

    (*
    showheap;

    module := midasLoadModule(str, @mpS3M);
    midasFreeModule(module);
    midasClose;
    showheap;
    exit;
    *)

    stopPlay := false;
    while not stopPlay do
    begin
        WaitVR;                         { wait for Vertical Retrace }
        WaitDE;                         { wait for Display Enable }
{$IFDEF NOTIMER}

        { If timer is not being used, poll the player manually. Note that this
          should not normally be done, as it changes the tempo when playing
          with GUS, but is here to help debugging. }

{$IFNDEF NOEMS}
            { Set EMS safety flag to optimize EMS mappings: }
            if useEMS = 1 then
            begin
                error := emsSafe;
                if error <> OK then
                    midasError(error);
            end;
{$ENDIF}

        error := SD^.StartPlay;
        if error <> OK then
            midasError(error);

        SetBorder(15);
        if SD^.tempoPoll = 1 then
        begin
            error := SD^.Play(@plMusic);
            if error <> OK then
                midasError(error);
            SetBorder(14);
            error := MP^.Play;
            if error <> OK then
                midasError(error);
        end
        else
        begin
            error := SD^.Play(@plMusic);
            if error <> OK then
                midasError(error);

            while plMusic = 1 do
            begin
                SetBorder(14);
                error := MP^.Play;
                if error <> OK then
                    midasError(error);
                SetBorder(15);
                error := SD^.Play(@plMusic);
                if error <> OK then
                    midasError(error);
            end;
        end;

{$IFNDEF NOEMS}
            { Clear EMS safety flag: }
            if useEMS = 1 then
            begin
                error := emsStopSafe;
                if error <> OK then
                    midasError(error);
            end;
{$ENDIF}

{$ENDIF}
        SetBorder(1);

        { read Module Player information to info^: }
        error := MP^.GetInformation(@info);
        if error <> OK then
            midasError(error);

        { draw VU-meters to top of display: }
        DrawMeters;

        Write('Pos: ', info^.pos, '  Row: ', info^.row, '  ', chr(13));

        SetBorder(0);

        { Handle keypresses: }
        if KeyPressed then
        begin
            key := ReadKey;
            if (key = 'N') or (key = 'n') then
            begin
                { Start playing another module: }
                midasStopModule(module);    { stop playing module }
                FreeModule(module);         { deallocate module }
                showheap;
                WriteLn('Enter new module file name:');
                ReadLn(fname);
                toASCIIZ(str, fname);
                module := PlayModule(str);  { load and play module }
                MP := midasMP;
                showheap;
            end;

            if ord(key) = 27 then
                stopPlay := true;

            if (ord(key) >= ord('1')) and (ord(key) <= ord('9')) then
            begin
                chNum := ord(key) - ord('1');
                chMuted[chNum] := chMuted[chNum] xor 1;
                error := SD^.MuteChannel(chNum, chMuted[chNum]);
                if error <> OK then
                    midasError(error);
            end;
        end

    end;

{    exec(getenv('COMSPEC'), ''); }

    midasStopModule(module);            { stop playing module }
    FreeModule(module);                 { deallocate module }
    showheap;

    { uninitialize real VU meters: }
    error := vuClose;
    if error <> OK then
        midasError(error);

    midasClose;                         { uninitialize MIDAS Sound System }
    showHeap;

{$IFDEF DEBUG}
    errPrintList;
{$ENDIF}
END.
