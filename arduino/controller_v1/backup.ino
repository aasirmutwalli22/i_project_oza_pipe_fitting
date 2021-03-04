/*
#include "LiquidCrystal.h"
#include "RTClib.h"
#include "Wire.h"
#include "extensions.h"
const byte    modeButtonPin     = 4;
const byte    setButtonPin      = 9;
const byte    incButtonPin      = 10;
const byte    decButtonPin      = 8;
const byte    relayPin          = 11;
const byte    buzzerPin         = 12;

const byte    tempPin           = A0;//lm35
const byte    currentPin        = A1;//ct

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
RTC_DS1307    rtc;

const char    types[6][4]       = {"CPL", "ELB", "SLD", "TEE", "OTH", "RED"};
const int     diameters[20]     = {20, 25, 32, 40, 50, 63, 75, 90, 110, 125,
140, 160, 180, 200, 225, 250, 280, 315, 355, 400};
//7066/454
//  lcd config mode
byte          config            = 0;
byte          screen            = 0;
byte          pipe_type_select  = 0;
byte          pipe_di_select    = 0;
int           time_in_sec       = 0;  // 0 to 999
float         temp              = 0.0;
byte          selection_index   = 0;  // 0 : pipe type, 1 : diameter, 2 :
sec(time) bool          inConfig          = true; byte          blink_counter =
0;

function      screen_intro,
              screen_voltage,
              screen_language,
              screen_product,
              screen_time,
              screen_process,
              screen_config,
              screen_confirm,
              screen_complete,
              screen_disconnect; //declare functions
function_pointers screens = {
  screen_intro,     //0
  screen_voltage,   //1
  screen_language,  //2
  screen_product,   //3
  screen_time,      //4
  screen_process,   //5
  screen_config,    //6
  screen_confirm,   //7

  screen_complete,  //8
  screen_disconnect // 9
};                                // declare pointers to functions

void setup() {
  pinMode(modeButtonPin, INPUT);
  pinMode(setButtonPin, INPUT);
  pinMode(incButtonPin, INPUT);
  pinMode(decButtonPin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  Serial.begin(9600);
  lcd.begin(16, 2);

  screens[0]();                     //intro
//  if (! rtc.begin()) {
//    Serial.println(F("Couldn't find RTC"));
//    while (1);
//  }
//  if (! rtc.isrunning()) {
//    Serial.println(F("RTC is NOT running!"));
//    rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
//    // This line sets the RTC with an explicit date & time, for example to set
//    // January 21, 2014 at 3am you would call:
//    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
//  }
}

void loop() {
  while (inConfig) {
    if (digitalRead(incButtonPin)) {
      delay(10);
      if (screen == 6) {
        if (selection_index == 0) pipe_type_select = pipe_type_select < 5   ?
pipe_type_select + 1 : 0; else if (selection_index == 1) pipe_di_select   =
pipe_di_select   < 19  ? pipe_di_select   + 1 : 0; else if (selection_index ==
2) time_in_sec      = time_in_sec      < 999 ? time_in_sec      + 1 : 0;
      }
    }
    else if (digitalRead(decButtonPin)) {
      delay(10);
      if (screen == 5) ++screen, selection_index = 0; // manual process select
and restart parameters selection else if (screen == 6) { if (selection_index ==
0) pipe_type_select = pipe_type_select > 0 ? pipe_type_select - 1 : 5  ; else if
(selection_index == 1) pipe_di_select   = pipe_di_select   > 0 ? pipe_di_select
- 1 : 19 ; else if (selection_index == 2) time_in_sec      = time_in_sec      >
0 ? time_in_sec      - 1 : 999;
      }
    }
    else if (digitalRead(setButtonPin)) {
      delay(10);
      while(digitalRead(setButtonPin));
      if(screen == 5);
      else if(screen == 6) {
        if (selection_index < 3) ++selection_index; //parameter selection
        else ++screen;
      }
      else if(screen < 7) ++screen;              // select next screen // until
final screen else inConfig = false;
    }
    else;
    delay(200);
    blink_counter = blink_counter < 3 ? blink_counter + 1 : 0;
    screens[screen]();                      // change screen
  }
  screen = 8;
  screens[screen]();                      // change screen
  Serial.println("config done");
  delay(1000);
}

void screen_intro() {
  lcd.setCursor(0, 0); //col/row
  lcd.print(F("       O+       "));

  lcd.setCursor(0, 1); //col/row
  lcd.print(F("OZA PIPE FITTING"));
}
void screen_voltage() {
  lcd.setCursor(0, 0); //col/row
  lcd.print(F("Frequency: 50Hz "));

  lcd.setCursor(0, 1); //col/row
  lcd.print(F("Voltage  : 40VAC"));
}
void screen_language() {
  lcd.setCursor(0, 0); //col/row
  lcd.print(F("Language: Eng   "));//7924/570

  lcd.setCursor(0, 1); //col/row
  lcd.print(F("                "));//7008/454
}
void screen_product() {
  lcd.setCursor(0, 0); //col/row
  lcd.print(F("Product Sr. No.:"));

  lcd.setCursor(0, 1); //col/row
  lcd.print(F("123             "));
}
void screen_time() {
  //DateTime now = rtc.now();
  lcd.setCursor(0, 0); //col/row
  //lcd.print(now.toString("DD/MM/YYYY hh:mm"));
  lcd.print(F("no rtc          "));

  lcd.setCursor(0, 1); //col/row
  lcd.print(F("Amb. Temp:"));
  lcd.print(round1dec(temp));
  lcd.print(F(" C"));
}
void screen_process() {
  lcd.setCursor(0, 0); //col/row
  lcd.print(F("Barcode    (+)  "));

  lcd.setCursor(0, 1); //col/row
  lcd.print(F("Manual     (-)  "));
}
void screen_config() {
  lcd.setCursor(0, 0); //col/row
  lcd.print(F("Type:"));
  lcd.print(blink_counter % 2 && selection_index == 0 ? "   " :
types[pipe_type_select]); lcd.print(F("  Di:")); lcd.print(blink_counter % 2 &&
selection_index == 1 ? "   " : String(diameters[pipe_di_select]));

  lcd.setCursor(0, 1); //col/row
  lcd.print(F("Vol: 40 Sec: "));
  lcd.print((blink_counter % 2 && selection_index == 2) ? "   " :
String(time_in_sec));
}
void screen_confirm() {
  lcd.setCursor(0, 0); //col/row
  lcd.print(F("Pipe Ready?     "));

  lcd.setCursor(0, 1); //col/row
  lcd.print(F("Press start     "));
}

void screen_complete(){
  lcd.setCursor(0, 0); //col/row
  lcd.print(F("  FUSION CYCLE  "));

  lcd.setCursor(0, 1); //col/row
  lcd.print(F("    COMPLETE    "));
}
void screen_disconnect(){
  lcd.setCursor(0, 0); //col/row
  lcd.print(F("DISCONN. FITTING"));

  lcd.setCursor(0, 1); //col/row
  lcd.print(F("    PRESS OK    "));
}

*/
