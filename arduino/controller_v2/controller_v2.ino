#include "LiquidCrystal.h"
#include "Wire.h"
#include "extensions.h"
#include "RTClib.h"
const byte    decButtonPin      = 4;
const byte    setButtonPin      = 5;
const byte    incButtonPin      = 3;
const byte    modeButtonPin     = 2;
const byte    relayPin          = 12;
const byte    buzzerPin         = 13;
const byte    tempPin           = A0;//lm35
const byte    currentPin        = A1;//ct
const byte    voltagePin        = A3;

RTC_DS1307 rtc;
LiquidCrystal lcd(6, 7, 8, 9, 10, 11);

const char    types[7][8]       = {"Coupler", "Elbow", "Tee", "Reducer", "Saddle", "Endcap", "Other" };
const int     diameters[20]     = {20, 25, 32, 40, 50, 63, 75, 90, 110, 125, 140, 160, 180, 200, 225, 250, 280, 315, 355, 400};
const float   voltages[3]       = {40.0, 39.5, 39.0};
//  lcd config mode

/**screens
   intro      : 0
   volt       : 1
   language   : 2
   product no : 3
   time & temp: 4
   process    : 5
   config     : 6
   confirm    : 7
   started    : 8
   complete   : 9
   error      : 99
   disconnect : 10
  **/
float         temp              = 0.0;
float         voltage           = 0;
float         current           = 0.0;
byte          error             = 0;
byte          screen            = 0;
byte          pipe_type_select  = 0;
byte          pipe_di_select    = 0;
byte          voltage_select    = 0;
unsigned int  time_in_sec       = 20;  // 0 to 999 // to hold last time
unsigned int  timer             = 0;  // 0 to 999 // actual timer
byte          selection_index   = 0;  // 0 : pipe type, 1 : diameter, 2 : sec(time)
byte          blink_counter     = 0;
unsigned long last_millis       = 0; // for counter

void setup() {
  Wire.begin();
  pinMode(modeButtonPin, INPUT);
  pinMode(setButtonPin, INPUT);
  pinMode(incButtonPin, INPUT);
  pinMode(decButtonPin, INPUT);

  pinMode(relayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  Serial.begin(9600);
  lcd.begin(20, 4);//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
//    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
//  RTC.set(new_time);
  delay(1000);
  monitor();
  monitor_voltage();
  for(int i = 0; i < 6; i++){
    screen = i;
    screen_display();                      // change screen
    delay(1000);
  }
}

void loop() {
  screen_handle();
  screen_display();                      // change screen
  Serial.println(screen);
}
void screen_handle() {
  switch (screen) {
    case 0 :{ //intro
      if(onSetButton()) screen = 5;
      break;
    }
    case 5 :{ // process select
      if (onDecButton()) ++screen;
      break;
    }
    case 6 :{ // pipe type / pipe dia / time / settings
      blink_counter = blink_counter < 3 ? blink_counter + 1 : 0;
      delay(250);
      if (onSetButton()) {
        if (selection_index < 4) ++selection_index; //parameter selection
        else {
          ++screen;
          timer = time_in_sec;
        }
      }
      else if (onDecButton()) {
        if (selection_index == 0)      pipe_type_select = pipe_type_select > 0 ? pipe_type_select - 1 : 6  ;
        else if (selection_index == 1) pipe_di_select   = pipe_di_select   > 0 ? pipe_di_select   - 1 : 19 ;
        else if (selection_index == 2) voltage_select   = voltage_select   > 0 ? voltage_select   - 1 : 2  ;
        else if (selection_index == 3) {
          long lastMillis = millis();
          while(digitalRead(decButtonPin))
          if(millis() - lastMillis > 1000) {
            time_in_sec      = time_in_sec      > 0 ? time_in_sec      - 1 : 999,
            lcd.setCursor(0, 3);
            lcd.print(F("Sec  : "));
            lcd.print(time_in_sec);
            lcd.print(F("         "));
            delay(20);
          }
          
          time_in_sec      = time_in_sec      > 0 ? time_in_sec      - 1 : 999;
        }
      }
      else if (onIncButton()) {
        if (selection_index == 0)      pipe_type_select = pipe_type_select < 6   ? pipe_type_select + 1 : 0;
        else if (selection_index == 1) pipe_di_select   = pipe_di_select   < 19  ? pipe_di_select   + 1 : 0;
        else if (selection_index == 2) voltage_select   = voltage_select   < 2   ? voltage_select   + 1 : 0;
        else if (selection_index == 3) {
          long lastMillis = millis();
          while(digitalRead(incButtonPin))
          if(millis() - lastMillis > 1000) {
            time_in_sec      = time_in_sec      < 999 ? time_in_sec      + 1 : 0;      
            lcd.setCursor(0, 3);
            lcd.print(F("Sec  : "));
            lcd.print(time_in_sec);
            lcd.print(F("         "));
            delay(20);
          }
          time_in_sec      = time_in_sec      < 999 ? time_in_sec      + 1 : 0;
        }
      }
      else if(onModeButton())--screen;
      break;
    }
    case 7 :{ //confirm
      if(onModeButton()){
        selection_index = 0;
        blink_counter = 0;
        --screen;
      } else if (onSetButton()) {
        monitor_voltage();
        if(error > 0) {
          screen = 99;
          break;
        }
        last_millis = millis();
        digitalWrite(relayPin, HIGH);
        delay(500);
        monitor();
        if(error > 0) {
          screen = 99;
          break;
        }
        screen = 8;
      }
      break;
    }
    case 8 :{ // process started
      monitor();
      if(error > 0) screen = 99;
      delay(500);
      if (millis() > last_millis + 1000) {
        last_millis = millis();
        Serial.print("t: ");
        Serial.println(timer);
        if (timer > 0) {
          timer -= 1;
          buzz(30);
        }
        else {
          digitalWrite(relayPin, LOW);
          screen = 9;
          buzz(1000);
        }
      } else if(onModeButton()){
        digitalWrite(relayPin, LOW);
        buzz(1000);
        error = 22;
        screen = 99;
      }
      break;
    }
    case 9 :{ //complete
      if (onSetButton()) {
        digitalWrite(relayPin, LOW);
        selection_index = 0;
        blink_counter = 0;
        screen = 0;
      };
      break;
    }
    case 99:{ // error display
      digitalWrite(relayPin, LOW);
      selection_index = 0;
      blink_counter = 0;
      if(onSetButton()) screen = 5, error = 0;
      break;
    }
    default:{
      nextScreen();
      break;
    }
  }
}
void screen_display() {
  switch (screen) {
    case 1 : {// voltage
      lcd.setCursor(0, 0);
      lcd.print(F("Voltage : "));
      lcd.print(voltage);
      lcd.print(F(" VAC   "));
      lcd.setCursor(0, 1);
      lcd.print(F("                    "));
      lcd.setCursor(0, 2);
      lcd.print(F("                    "));
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
      break;
    }
    case 2 : {// language
      lcd.setCursor(0, 0);
      lcd.print(F("Language: English   "));
      lcd.setCursor(0, 1);
      lcd.print(F("                    "));
      lcd.setCursor(0, 2);
      lcd.print(F("                    "));
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
      break;
    }
    case 3 : {// product
      lcd.setCursor(0, 0);
      lcd.print(F("Product Serial : "));
      lcd.print(F("101"));
      lcd.setCursor(0, 1);
      lcd.print(F("                    "));
      lcd.setCursor(0, 2);
      lcd.print(F("Product Version: "));
      lcd.print(F("1.0"));
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
      break;
    }
    case 4 : {// time and temp
      DateTime now = rtc.now();
      char ti[21];
      sprintf(ti, "%.2d/%.2d/%.2d %.2d:%.2d:%.2d m", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());
      now.hour() >= 12 ? ti[19] = 'p' : 'a';
      
      lcd.setCursor(0, 0);
      lcd.print(ti);//19 char
      lcd.print(" ");
      lcd.setCursor(0, 1);
      lcd.print(F("                    "));
      lcd.setCursor(0, 2);
      lcd.print(F("Ambient Temp:"));
      lcd.print(temp, 1);
      lcd.print(F(" "));
      lcd.print((char)0xdf);
      lcd.print(F("C "));
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
      break;
    }
    case 5 : {// process
      lcd.setCursor(0, 0);
      lcd.print(F("Barcode    (+)      "));
      lcd.setCursor(0, 1);
      lcd.print(F("                    "));
      lcd.setCursor(0, 2);
      lcd.print(F("Manual     (-)      "));
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
      break;
    }
    case 6 : {// config
      lcd.setCursor(0, 0);
      lcd.print(F("Type : "));
      lcd.print(blink_counter % 2 && selection_index == 0 ? "        " : types[pipe_type_select]);
      lcd.print(F("     "));
      lcd.setCursor(0, 1);
      lcd.print(F("Dia  : "));
      lcd.print(blink_counter % 2 && selection_index == 1 ? "   " : String(diameters[pipe_di_select]));
      lcd.print(F("          "));
      lcd.setCursor(0, 2);
      lcd.print(F("Vol  : "));
      lcd.print(blink_counter % 2 && selection_index == 2 ? "    " : String(voltages[voltage_select]));
      // lcd.print(F("40.0"));
      lcd.print(F("        "));
      lcd.setCursor(0, 3);
      lcd.print(F("Sec  : "));
      lcd.print((blink_counter % 2 && selection_index == 3) ? "   " : String(time_in_sec));
      lcd.print(F("          "));
      break;
    }
    case 7 : {// confirm
      lcd.setCursor(0, 0);
      lcd.print(F("Pipe Ready?         "));
      lcd.setCursor(0, 1);
      lcd.print(F("Press start         "));
      lcd.setCursor(0, 2);
      lcd.print(F("                    "));
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
      break;
    }
    case 8 : {// fusion started
      lcd.setCursor(0, 0);
      lcd.print(F("Fusion started      "));
      lcd.setCursor(0, 1);
      lcd.print(F("Vol  : 40V          "));
      lcd.setCursor(0, 2);
      lcd.print(F("Sec  : "));//7
      lcd.print(timer);
      lcd.print(timer > 99 ? F("") : timer > 9 ? F(" ") : F("  "));//adjust space
      lcd.print(F("          "));
      lcd.setCursor(0, 3);
      lcd.print(F("Temp : ")); //7
      lcd.print(temp, 1);//max7//min4
      lcd.print(F(" ")); //1
      lcd.print((char)0xdf); //1
      lcd.print(F("C")); //1
      lcd.print(temp > 999 ? F("") : temp > 99 ? F(" ") : temp > 9 ? F("  ") : F("   "));//space adjust
      lcd.print(F("   "));
      break;
    }
    case 9 : {// complete
      lcd.setCursor(0, 0);
      lcd.print(F("    FUSION CYCLE    "));
      lcd.setCursor(0, 1);
      lcd.print(F("      COMPLETE      "));
      lcd.setCursor(0, 2);
      lcd.print(F("                    "));
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
      break;
    }
    case 10: {// disconnect
      lcd.setCursor(0, 0);
      lcd.print(F(" DISCONNECT FITTING "));
      lcd.setCursor(0, 1);
      lcd.print(F("      PRESS OK      "));
      lcd.setCursor(0, 2);
      lcd.print(F("                    "));
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
      break;
    }
    case 99: {// error display
      lcd.setCursor(0, 0);
      lcd.print("ERROR ");
      lcd.print(error);
      lcd.print("             ");
      lcd.setCursor(0, 1);
      lcd.print(error_message());
      lcd.setCursor(0, 2);
//      lcd.print(voltage);
      lcd.print(F("                    "));
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
      delay(500);
      monitor();
      break;
    }
    default: {// intro
      lcd.setCursor(0, 0);
      lcd.print(F("    O+ FITTINGS     "));
      lcd.setCursor(0, 1);
      lcd.print(F("                    "));
      lcd.setCursor(0, 2);
      lcd.print(F("    www.hdpe.in     "));
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
      break;
    }
  };
}


String error_message(){
//  Serial.print("error: ");
//  Serial.println(error);
  return error == 1  ? "AMBIENT TEMP LOW    " :
  error == 2  ? "AMBIENT TEMP HIGH   " :
  error == 4  ? "CURRENT OUT OF RANGE" :
  error == 5  ? "FUSION OPEN CIRCUIT " :
  error == 22 ? "FUSION CYCLE STOPPED" :
  error == 23 ? "MAINS VOLTAGE LOW   " :
  error == 24 ? "MAINS VOLTAGE HIGH  " : "                ";
}
void monitor() {
  delay(50);
  temp = read_temp();
  
//  if(screen == 8) voltage *= 1.126;
  current = read_current();
   
  if     (temp    < -10.0)  error = 1;
  else if(temp    > 45.0 )  error = 2;
  else if(screen == 7 && current < 1) error = 5; // on start check  // open loop
  else if(screen == 8 && current < 1) error = 4; // process started // fittings broken
//  else error = 0;
}
void monitor_voltage(){
  voltage = (int) read_voltage();
  if(voltage < 220.0)  error = 23;
  else if(voltage > 250.0)  error = 24;
}
float read_voltage(){
  long val = 0;
  for(int i = 0; i < 1000; i++) val += analogRead(voltagePin);
  val /= 1000;
  //
//  return val * 0.34575260804769001490312965722802
  return val/1.72;
}
float read_temp(){
  long val = 0;
  for(int i = 0; i < 100; i++) val += analogRead(tempPin);
  val /= 100;
  float _temp = val * 500.0 / 1023.0;
  return _temp * 0.76726342710997442455242966751918;
}
float getVPP()
{
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  long int avgVal = 0;
  uint32_t start_time = millis();
  int i = 0;
  while ((millis() - start_time) < 100){
      readValue = analogRead(currentPin);
      avgVal = avgVal + readValue;
      if (readValue > maxValue){
        maxValue = readValue;
      }
      i++;

  }
  maxValue = (maxValue - (avgVal / i));
  result = (maxValue * 5.0) / 1024.0;
  return result;
}

float read_current()
{
  float nVPP = getVPP();
  nVPP = (nVPP / 2) * 0.707 * 1000.0;
  float nCurrThruResistorPP = (nVPP) / 56.0;
  float nCurrThruResistorRMS = (nCurrThruResistorPP); //-8.06
  float nCurrentThruWire = nCurrThruResistorRMS * 1000;

  if (nCurrThruResistorRMS < 1.0){
    nCurrThruResistorRMS = 0;
  }
  return (nCurrThruResistorPP);
}


bool onModeButton() {
  if (digitalRead(modeButtonPin)) {
    delay(10);
    while (digitalRead(modeButtonPin));
    buzz(30);
    return true;
  }
  else return false;
}
bool onSetButton() {
  if (digitalRead(setButtonPin)) {
    delay(10);
    while (digitalRead(setButtonPin));
    buzz(30);
    return true;
  }
  else return false;
}
bool onDecButton() {
  if (digitalRead(decButtonPin)) {
    delay(10);
    buzz(30);
    return true;
  }
  else return false;
}
bool onIncButton() {
  if (digitalRead(incButtonPin)) {
    delay(10);
    buzz(30);
    return true;
  }
  else return false;
}


void nextScreen() {
  if (onSetButton() && screen < 9) ++screen;
  last_millis = millis();
}
void buzz(int _delay){
    digitalWrite(buzzerPin, HIGH);
    delay(_delay);
    digitalWrite(buzzerPin, LOW);
}
