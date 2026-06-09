program KeyboardBell;

begin
    while True do
    begin
        ReadKey;
        Write(Chr(7));
    end;
end.
