#include <Wire.h>
#include "ds1307.h"
#include "LiquidCrystal.h"
LiquidCrystal lcd(6, 7, 8, 9, 10, 11);

rtcData new_time = {
  0,//second
  15,//minute
  6,//hour
  1,//weekday
  21,//date
  3,//month
  21//year
};
rtcData time;
void setup() {
  Wire.begin();
  RTC.set(new_time);
  Serial.begin(115200);
  lcd.begin(20, 4);
}

void loop() {
//  lcd.setCursor(0,0);
//  lcd.print(RTC.readString(time));
  Serial.println(RTC.readString(time));
  delay(1000);
}
