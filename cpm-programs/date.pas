program TDate;

type
  TTimeStamp = record
    { http://www.cpm.z80.de/manuals/cpm3-pgr.pdf - page 3-85, 3-86 (PDF page 158, 159) }
    { https://www.seasip.info/Cpm/bdos.html#105 }
    { BDOS 105 / 69h; Entered with C=69h, DE=address of time stamp. Returns A=seconds (packed BCD). }
    Day   : Integer; { DW; Day 1 is 1 January 1978 }
    Hour  : Byte;    { DB; Packed BCD }
    Minute: Byte;    { DB; Packed BCD }
    Second: Byte;    { DB; Packed BCD }
  end;

  TDate = record
    Day  : Integer;
    Month: Integer;
    Year : Integer;
  end;

const
  { Count of Days in a Month               J,  F,   M,  A,  M,  J,  J,  A,  S,  O,  N,  D}
  DaysInMonth: array[1..12] of Integer = (31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31);

function BcdToInt(B: Byte): Integer;
begin
  BcdToInt := (B shr 4) * 10 + (B and $0F);
end;

function LeadingZero(Value: Byte): String;
var
  R: string[2];
begin
  R[0] := Chr(2); { string length }
  R[1] := Chr(Ord('0') + (Value div 10));
  R[2] := Chr(Ord('0') + (Value mod 10));

  LeadingZero := R;
end;

function IsLeapYear(Year: Integer): Boolean;
begin
  IsLeapYear := ((Year mod 4) = 0) and (((Year mod 100) <> 0) or ((Year mod 400) = 0));
end;

function DaysInYear(Year: Integer): Integer;
begin
  if IsLeapYear(Year)
  then
    DaysInYear := 366
  else
    DaysInYear := 365;
end;

function DaysToDate(Days: Integer): TDate;
var
  Date: TDate;
  M: Integer;
  Offset: Integer;
begin
  Date.Year := 1978;
  while Days >= DaysInYear(Date.Year) do
  begin
    Days := Days - DaysInYear(Date.Year);
    Inc(Date.Year);
  end;

  { Day of year to month-day conversion }
  Date.Day := Days;
  Date.Month := 1;
  for M := 1 to 12 do
  begin
    Offset := 0;
    if (M = 2) and IsLeapYear(Date.Year) then
      Offset := 1; { Leap-Year variant of February has one more day }

    if Date.Day < (DaysInMonth[M] + Offset) then
      Break;
    Date.Day := Date.Day - DaysInMonth[M] - Offset;
    Inc(Date.Month);
  end;

  DaysToDate := Date;
end;

var
  SecA: Byte;
  TS  : TTimeStamp;
  Date: TDate;
begin
  SecA := BDos(105, Addr(TS));
  TS.Second := BcdToInt(SecA);
  TS.Minute := BcdToInt(TS.Minute);
  TS.Hour := BcdToInt(TS.Hour);
  Date := DaysToDate(TS.Day);
  
  Writeln(LeadingZero(Date.Day), '.', LeadingZero(Date.Month), '.', Date.Year, ' ', LeadingZero(TS.Hour), ':', LeadingZero(TS.Minute), ':', LeadingZero(TS.Second));
end.
