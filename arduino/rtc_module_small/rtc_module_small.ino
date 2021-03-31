#include <LiquidCrystal.h>
#include <Wire.h>
#define DS1307_ADDRESS 0x68

LiquidCrystal lcd(6, 7, 8, 9, 10, 11);

boolean MODE = false;

byte Dec_To_BCD(byte dec) {
  return ( (dec / 10 * 16) + (dec % 10) );
}

byte BCD_To_Dec(byte BCD) {
  return ( (BCD / 16 * 10) + (BCD % 16) );
}


void Mode12() {
  MODE = true;
}

void Mode24() {
  MODE = false;
}


void setTime(byte SEC, byte MIN, byte HOUR, byte DOW, byte DOM, byte MONTH, byte YEAR)
{

  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0);
  Wire.write(Dec_To_BCD(SEC));
  Wire.write(Dec_To_BCD(MIN));
  Wire.write(Dec_To_BCD(HOUR));
  Wire.write(Dec_To_BCD(DOW));
  Wire.write(Dec_To_BCD(DOM));
  Wire.write(Dec_To_BCD(MONTH));
  Wire.write(Dec_To_BCD(YEAR));
  Wire.endTransmission();
}



void getTime(byte *SEC, byte *MIN, byte *HOUR)
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 7);

  *SEC   = BCD_To_Dec(Wire.read() & 0x7f);
  *MIN   = BCD_To_Dec(Wire.read());
  *HOUR  = BCD_To_Dec(Wire.read() & 0x3f);
}


void getDate(byte *DOM, byte *MONTH, byte *YEAR)
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(4);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 7);

  *DOM   = BCD_To_Dec(Wire.read());
  *MONTH = BCD_To_Dec(Wire.read());
  *YEAR  = BCD_To_Dec(Wire.read());
}

byte getWeek()
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(3);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 7);

  return (BCD_To_Dec(Wire.read()));
}


String getTimeStr()
{
  String ALL;
  byte SEC, MIN, HOUR, AM_PM;

  getTime(&SEC, &MIN, &HOUR);
  if (!MODE)
  {
    if (HOUR < 10)
      ALL = String('0') + String(HOUR, DEC) + String(':');
    else
      ALL = String(HOUR, DEC) + String(':');
    if (MIN < 10)
      ALL += String('0'); ALL += String(MIN, DEC);
    ALL += String(':');
    if (SEC < 10)
      ALL += String('0'); ALL += String(SEC, DEC);
  }
  else
  {
    AM_PM = HOUR;
    if (HOUR > 12)
      HOUR -= 12;
    if (HOUR < 10)
      ALL = String('0') + String(HOUR, DEC) + String(':');
    else
      ALL = String(HOUR, DEC) + String(':');
    if (MIN < 10)
      ALL += String('0'); ALL += String(MIN, DEC);
    ALL += String(':');
    if (SEC < 10)
      ALL += String('0'); ALL += String(SEC, DEC);
    ALL += String(' ');
    if (AM_PM > 11)ALL += String("PM");
    else ALL += String("AM");
  }

  return (ALL);
}




String getDateStr()
{
  String ALL;
  byte DOM, MONTH, YEAR;

  getDate(&DOM, &MONTH, &YEAR);

  if (DOM < 10)
    ALL += String('0'); ALL += String(DOM, DEC);
  ALL += String('/');
  if (MONTH < 10)
    ALL += String('0'); ALL += String(MONTH, DEC);
  ALL += String('/'); ALL += String(YEAR, DEC);
  ALL += String(' ');

  return (ALL);
}


String getWeekStr()
{
  String ALL;

  switch (getWeek())
  {
    case 1: ALL += String("Sunday"); break;
    case 2: ALL += String("Monday"); break;
    case 3: ALL += String("Tuesday"); break;
    case 4: ALL += String("Wednesday"); break;
    case 5: ALL += String("Thursday"); break;
    case 6: ALL += String("Friday"); break;
    case 7: ALL += String("Saturday"); break;
  }
  return (ALL);
}



void setup()
{
  lcd.begin(16, 2);
  Wire.begin();
  //setTime(00, 56, 11, 4, 17, 03, 21);
}





void loop()
{

  /////  24hr MODE  /////

  //    lcd.clear();
  //    lcd.setCursor(0,0);
  //    Mode24();
  //    lcd.print(getTimeStr());lcd.print(' ');
  //    lcd.setCursor(0,1);
  //    lcd.print(getDateStr());lcd.print(' ');
  //    lcd.print(getWeekStr());
  //    delay(5000);


  /////  12hr MODE  /////
  lcd.clear();
  lcd.setCursor(0, 0);
  Mode12();
  lcd.print(getTimeStr()); lcd.print(' ');
  lcd.setCursor(0, 1);
  lcd.print(getDateStr()); lcd.print(' ');
  lcd.print(getWeekStr());
  delay(1000);


}
