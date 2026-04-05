program KeyboardBell;

var
    C: Char;
begin
    while True do
    begin
        C := ReadKey;
        Write(Chr(7));
    end;
end.
