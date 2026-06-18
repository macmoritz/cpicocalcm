program KeyboardBell;

begin
    while True do
    begin
        ReadKey;        // Read input blocking
        Write(Chr(7));  // Write the ASCII bell character to the console
    end;
end.
