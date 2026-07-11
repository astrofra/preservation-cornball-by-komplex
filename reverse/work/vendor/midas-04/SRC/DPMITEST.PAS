uses Errors, DPMI;



var
    selector : word;
    error, i : integer;
    b : byte;
    base : longint;



BEGIN
    error := dpmiAllocDescriptor(@selector);
    if error <> OK then
        errErrorExit(errorMsg[error]);
    writeln('Allocated selector ', selector);

    error := dpmiSetSegmentBase(selector, $B8000);
    if error <> OK then
        errErrorExit(errorMsg[error]);
    writeln('Selector base set to 0B8000h');

    error := dpmiSetSegmentLimit(selector, 160);
    if error <> OK then
        errErrorExit(errorMsg[error]);
    writeln('Selector limit set to 160');

    writeln('Reading memory within selector limit');
    for i := 0 to 159 do
        b := mem[selector:i];

    writeln('Writing to memory within selector limit');
    for i := 0 to 159 do
        mem[selector:i] := 1;

    error := dpmiGetSegmentBase(selector, @base);
    if error <> OK then
        errErrorExit(errorMsg[error]);
    writeln('DPMI reports selector base address as ', base);

    writeln('Reading memory past selector limit');
    b := mem[selector:161];

    writeln('Writing to memory past selector limit');
    mem[selector:161] := 1;

    error := dpmiFreeDescriptor(selector);
    if error <> OK then
        errErrorExit(errorMsg[error]);
    writeln('Descriptor deallocated');
END.
