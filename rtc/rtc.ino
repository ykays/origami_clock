#include <Wire.h>
#include <DS3231.h>

RTClib rtc;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);

  DateTime now = rtc.now();
  String nowString = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
  Serial.println("Time: " + nowString);

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
