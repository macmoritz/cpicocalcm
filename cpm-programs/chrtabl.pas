program CharTable;

const
  LineBreak = #13#10; // Carriage Return + Line Feed
  ESC = #27; // Escape character

procedure PrintCharTable;
var
    X, Y: Integer;
begin
    // Header
    Write('    '); // 4 spaces
    for X := 0 to 9 do
        Write(' 0', X, ' '); 
    WriteLn;

    for Y := 3 to 12 do
    begin
        if Y < 10 then
            Write('0');
        Write(Y, '0 ');
        for X := 0 to 9 do
            if Y * 10 + X < 32 then
               Write('    ') // 4 spaces
            else 
                Write('  ', Chr(Y * 10 + X), ' '); 
        WriteLn;
    end;
end;

var
    C: Char;
begin
    ClrScr;

    WriteLn('*** Normal Charset ***');
    PrintCharTable;
    
    WriteLn('*** Alternative Charset ***');
    Write(ESC, 'F');
    PrintCharTable;

    C := ReadKey;
end.