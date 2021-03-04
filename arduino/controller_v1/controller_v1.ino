#include "LiquidCrystal.h"
#include "RTClib.h"
#include "Wire.h"
#include "extensions.h"
const byte    decButtonPin      = 8;
const byte    setButtonPin      = 9;
const byte    incButtonPin      = 10;
const byte    modeButtonPin     = 11;
const byte    relayPin          = 12;
const byte    buzzerPin         = 13;
const byte    tempPin           = A0;//lm35
const byte    currentPin        = A1;//ct

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
RTC_DS1307    rtc;

const char    types[6][4]       = {"CPL", "ELB", "SLD", "TEE", "OTH", "RED"};
const int     diameters[20]     = {20, 25, 32, 40, 50, 63, 75, 90, 110, 125, 140, 160, 180, 200, 225, 250, 280, 315, 355, 400};
//  lcd config mode

/**screens
  *intro      : 0
  *volt & freq: 1
  *language   : 2
  *product no : 3
  *time & temp: 4
  *process    : 5
  *config     : 6
  *confirm    : 7
  *started    : 8
  *complete   : 9
  *disconnect : 10
  **/
byte          screen            = 0;
byte          pipe_type_select  = 0;
byte          pipe_di_select    = 0;
unsigned int  time_in_sec       = 0;  // 0 to 999 // to hold last time
unsigned int  timer             = 0;  // 0 to 999 // actual timer
float         temp              = 0.0;
byte          selection_index   = 0;  // 0 : pipe type, 1 : diameter, 2 : sec(time)
byte          blink_counter     = 0;
unsigned long last_millis       = 0; // for counter
void setup() {
  pinMode(modeButtonPin, INPUT);
  pinMode(setButtonPin, INPUT);
  pinMode(incButtonPin, INPUT);
  pinMode(decButtonPin, INPUT);
  
  pinMode(relayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  Serial.begin(9600);
  lcd.begin(16, 2);
  
}

void loop() {
  switch (screen) {
    case 5 :  // process select
      if (onDecButton()) ++screen; 
      break;
    case 6 : // pipe type / pipe dia / time / settings
      blink_counter = blink_counter < 3 ? blink_counter + 1 : 0;
      delay(200);
      if(onSetButton()){
        if (selection_index < 3) ++selection_index; //parameter selection
        else {
          ++screen;
          timer = time_in_sec;
        }
      }
      else if(onDecButton()){
        if (selection_index == 0)      pipe_type_select = pipe_type_select > 0 ? pipe_type_select - 1 : 5  ; 
        else if (selection_index == 1) pipe_di_select   = pipe_di_select   > 0 ? pipe_di_select   - 1 : 19 ; 
        else if (selection_index == 2) time_in_sec      = time_in_sec      > 0 ? time_in_sec      - 1 : 999;        
      }
      else if(onIncButton()){
        if (selection_index == 0)      pipe_type_select = pipe_type_select < 5   ? pipe_type_select + 1 : 0; 
        else if (selection_index == 1) pipe_di_select   = pipe_di_select   < 19  ? pipe_di_select   + 1 : 0; 
        else if (selection_index == 2) time_in_sec      = time_in_sec      < 999 ? time_in_sec      + 1 : 0;
      }
      break;
    case 8 : // process started
      if(millis() > last_millis + 1000){  
        last_millis = millis();
        if(timer > 0) timer -= 1;
        else screen = 9;
      }
      break;
    case 9 :
      if(onSetButton()) screen = 0, restart();
      break;
    default: 
      nextScreen();
      break;
  }
  Serial.print(screen);
  screen_display();                      // change screen
}
void screen_display() {
  switch (screen) {
    case 1 : // freq & voltage
      lcd.setCursor(0, 0);
      lcd.print(F("Frequency: 50Hz "));
      lcd.setCursor(0, 1);
      lcd.print(F("Voltage  : 40VAC"));
      break;
    case 2 : // language
      lcd.setCursor(0, 0);
      lcd.print(F("Language: Eng   "));
      lcd.setCursor(0, 1);
      lcd.print(F("                "));
      break;
    case 3 : // product
      lcd.setCursor(0, 0);
      lcd.print(F("Product Sr. No.:"));
      lcd.setCursor(0, 1);
      lcd.print(F("123             "));
      break;
    case 4 : // time and temp
      lcd.setCursor(0, 0);
      if (! rtc.begin()) 
        lcd.print(F("clock not found ")); 
      else if (! rtc.isrunning()) 
        lcd.print(F("clock error     "));
      else {
        DateTime now = rtc.now();
        lcd.print(now.toString("DD/MM/YYYY hh:mm"));
      }
      lcd.setCursor(0, 1);
      lcd.print(F("Amb. Temp:"));
      lcd.print(round1dec(temp));
      lcd.print(F(" C"));
      break;
    case 5 : // process
      lcd.setCursor(0, 0);
      lcd.print(F("Barcode    (+)  "));
      lcd.setCursor(0, 1);
      lcd.print(F("Manual     (-)  "));
      break;
    case 6 : // config
      lcd.setCursor(0, 0);
      lcd.print(F("Type:"));
      lcd.print(blink_counter % 2 && selection_index == 0 ? "   " : types[pipe_type_select]);
      lcd.print(F(" Dia:"));
      lcd.print(blink_counter % 2 && selection_index == 1 ? "   " : String(diameters[pipe_di_select]));
      lcd.setCursor(0, 1);
      lcd.print(F("Vol: 40 Sec: "));
      lcd.print((blink_counter % 2 && selection_index == 2) ? "   " : String(time_in_sec));
      break;
    case 7 : // confirm
      lcd.setCursor(0, 0);
      lcd.print(F("Pipe Ready?     "));
      lcd.setCursor(0, 1);
      lcd.print(F("Press start     "));
      break;
    case 8 : // fusion started
      lcd.setCursor(0, 0);
      lcd.print(F("Fusion started  "));
      lcd.setCursor(0, 1);
      lcd.print(F("sec : "));
      lcd.print(timer);
      lcd.print(F("         "));   
      break;
    case 9 : // complete
      lcd.setCursor(0, 0);
      lcd.print(F("  FUSION CYCLE  "));
      lcd.setCursor(0, 1);
      lcd.print(F("    COMPLETE    "));
      break;
    case 10 : // disconnect
      lcd.setCursor(0, 0);
      lcd.print(F("DISCONN. FITTING"));
      lcd.setCursor(0, 1);
      lcd.print(F("    PRESS OK    "));
      break;
    default:  // intro
      lcd.setCursor(0, 0);
      lcd.print(F("       O+       "));
      lcd.setCursor(0, 1);
      lcd.print(F("OZA PIPE FITTING"));
      break;
  };
}



bool onSetButton(){
  if (digitalRead(setButtonPin)) {
    delay(10);
    while (digitalRead(setButtonPin));
    return true;
  }
  else return false;
}
bool onDecButton(){
  if (digitalRead(decButtonPin)) {
    delay(10);
    return true;
  }
  else return false;
}
bool onIncButton(){
  if (digitalRead(incButtonPin)) {
    delay(10);
    return true;
  }
  else return false;
}

void nextScreen(){
  if(onSetButton() && screen < 9) ++screen;
  last_millis = millis();
}
void restart(){
  selection_index = 0;
  blink_counter = 0;
  screen = 0;  
}
