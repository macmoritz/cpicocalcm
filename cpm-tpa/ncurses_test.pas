program Colors;

const
  LineBreak = #13#10; // Carriage Return + Line Feed
  ESC = #27; // Escape character

var
  A, B, C: Integer;
  Input: Char;
  CursorVisible: Boolean;
begin
  ClrScr;
  for B := 0 to 7 do
  begin
    TextBackground(B);
    for C := 0 to 7 do
    begin
      TextColor(C);
      GotoXY(1 + C * 4, 1 + B * 2);
      Write(' # ');
    end;
  end;
  Write(ESC, 'm');
  WriteLn(LineBreak, 'Normal', LineBreak);
  WriteLn(ESC, 'c', 'Alternative Font', #27, 'd', LineBreak);
  WriteLn(ESC, 'e', 'Bold', #27, 'f', LineBreak);
  WriteLn(ESC, 'g', 'Underline', #27, 'h', LineBreak);
  WriteLn(ESC, 'i', 'Inverted', #27, 'j', LineBreak);
  WriteLn(ESC, 'k', 'Blinking', #27, 'l', LineBreak);
  WriteLn(ESC, 'p', 'Standout', #27, 'q', LineBreak);
  WriteLn(ESC, 'i', #27, 'g', #27, 'k', 'Blinking & Underline & Inverted', #27, 'm');

  repeat until KeyPressed;
  Write(ESC, 'E'); // E: Clear Screen; Cursor home

  CursorOff;
  WriteLn('Cursor Movement Test', LineBreak,
        'Cursor should be blinking', LineBreak,
        'Arrow:     Move', LineBreak,
        'Enter:     Insert blank line', LineBreak,
        'Backspace: Delete line', LineBreak,
        'd:         Clear to end of line', LineBreak,
        'D:         Clear to end of screen', LineBreak,
        'c:         Toggle cursor visibility', LineBreak,
        'q:         Exit', LineBreak);
  CursorOn;

  for A := 1 to 31 do
    Writeln('Lorem ipsum dolor sit amet, consetetur s');

  CursorVisible := True;
  // Write(ESC, 'b'); // b: Restore Cursor
  Input := ReadKey;
  while Input <> 'q' do
  begin
    if Input = Chr(5) then
      Write(ESC, 'A') // A: Cursor Up
    else if Input = Chr(24) then
      Write(ESC, 'B') // B: Cursor Down
    else if Input = Chr(19) then
      Write(ESC, 'D') // D: Cursor Left
    else if Input = Chr(4) then
      Write(ESC, 'C') // C: Cursor Right
    else if Input = 'd' then
      ClrEol
    else if Input = 'D' then
      ClrEos
    else if Input = Chr(10) then // Chr(10) = Line Feed (LF) = Enter key
      InsLine
    else if Input = Chr(8) then // Chr(8) = Backspace
      DelLine
    else if Input = 'c' then
    begin
      if CursorVisible then
      begin
        Write(ESC, 'a'); // a: Cursor Off
        // CursorOff;
        CursorVisible := False;
      end
      else
      begin
        Write(ESC, 'b'); // b: Cursor On
        // CursorOn;
        CursorVisible := True;
      end
    end;
    Input := ReadKey;
  end;

  repeat until KeyPressed;
  // Write(ESC, 'E'); // E: Clear Screen; Cursor home
  //
  // WriteLn(ESC, 'k');
  // for A := 1 to 15 do
  //   Writeln('Lorem ipsum dolor sit amet, consetetur s');
  // WriteLn(#27, 'l', LineBreak);

end.
