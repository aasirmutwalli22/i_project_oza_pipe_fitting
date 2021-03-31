const int analogInPin = A3;
int voltage = 0;        // value output to the PWM (analog out)

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

void loop() {
  // read the analog in value:
  long sensorValue = 0;
  for(int i = 0; i < 100; i++) sensorValue += analogRead(analogInPin);
  sensorValue /= 100;
  voltage = sensorValue * 0.34575260804769001490312965722802;
  Serial.print("sensor = ");
  Serial.print(sensorValue);
  Serial.print("\t voltage = ");
  Serial.println(voltage);
  delay(20);
}
