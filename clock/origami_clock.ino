#include <Wire.h>
#include <DS3231.h>

int LEDS[12] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
int CENTER_LED = A2;
int POT = A0;
int PHOTO = A1;
int IS_CHILD_PIN = A3;

byte INVALID = -1;

DS3231 rtc;

struct Time {
  byte hour;
  byte minute;
};

Time currentTime = {
  .hour = INVALID,
  .minute = INVALID,
};

Time getTimeFromUser();
Time getTimeFromRtc();
byte parseHour(char* timeInput);
byte parseMinute(char* timeInput);
byte parseSecond(char* timeInput);
void setupLeds();
void turnAllOff();
void lightHour(byte hour);
void lightMinute(byte minute);
bool isLaserTouching();
bool clockOn = true;
bool needsUpdate = false;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  setupLeds();

  delay(1000);
  Serial.println("Please enter the time to display (HH:mm:ss): ");
}

void loop() {
  // Check whether the laser is turning the clock on/off.
  if (isLaserTouching()) {
    clockOn = !clockOn;
    needsUpdate = true;
  }

  // Check whether the time has changed in the rtc module.
  Time rtcTime = getTimeFromRtc();
  if (rtcTime.hour != currentTime.hour || rtcTime.minute != currentTime.minute) {
    currentTime = rtcTime;
    needsUpdate = true;
  }

  // Check whether the user entered a new time over serial.
  Time userTime = getTimeFromUser();
  if (userTime.hour != currentTime.hour || userTime.minute != currentTime.minute) {
    currentTime = userTime;
    needsUpdate = true;
  }

  if (!needsUpdate) return;

  delay(50);

  turnAllOff();

  if (clockOn) {
    // Tell any Serial listeners to set their minute hand.
    Serial.println("--:" + String(currentTime.minute) + ":00");

    // light up the hour
    lightHour(currentTime.hour);
    // light up the minute
    lightMinute(currentTime.minute);
  }

  needsUpdate = false;
  delay(3000); // Wait a few seconds before updating again.
}

void setupLeds() {
  for (int pin = 0; pin < 12; pin++) {
    pinMode(LEDS[pin], OUTPUT);
  }
  pinMode(CENTER_LED, OUTPUT);

  turnAllOff();

  digitalWrite(CENTER_LED, HIGH);
  delay(250);
  digitalWrite(CENTER_LED, LOW);
  for (int pin = 0; pin < 12; pin++) {
    digitalWrite(LEDS[pin], HIGH);
    delay(250);
    digitalWrite(LEDS[pin], LOW);
  }
}

/** Returns INVALID if the input is not a number. */
byte parseHour(String timeInput) {
  String hour = timeInput.substring(0, 2);
  if (!isDigit(hour.charAt(0)) && !isDigit(hour.charAt(1))) return INVALID;
  return (byte)(hour.toInt());
}

/** Returns -1 if the input is not a number. */
byte parseMinute(String timeInput) {
  String minute = timeInput.substring(3, 5);
  if (!isDigit(minute.charAt(0)) && !isDigit(minute.charAt(1))) return INVALID;
  return (byte)(minute.toInt());
}

/** Returns -1 if the input is not a number. */
byte parseSecond(String timeInput) {
  String second = timeInput.substring(6, 8);
  if (!isDigit(second.charAt(0)) && !isDigit(second.charAt(1))) return INVALID;
  return (byte)(second.toInt());
}

Time getTimeFromRtc() {
  bool isChild = digitalRead(IS_CHILD_PIN);
  if (isChild) return currentTime;  // Children only get time from parent rings (serial).
  bool h12;
  bool pmTime;
  Time rtcTime = {
    .hour = rtc.getHour(h12, pmTime),
    .minute = rtc.getMinute(),
  };
  return rtcTime;
}

/** Waits for the user to send a time over serial.
 *  - The time should be in the format HH:mm:ss.
 *  - The function will block and the return the string.
 */
Time getTimeFromUser() {
  if (!Serial.available()) return currentTime;

  String timeInput = Serial.readStringUntil('\n');
  struct Time parsedTime;
  parsedTime.hour = parseHour(timeInput);
  parsedTime.minute = parseMinute(timeInput);
  byte parsedSecond = parseSecond(timeInput);
  if (parsedTime.hour == INVALID && parsedTime.minute == INVALID) {
    Serial.println("Ignoring non-time: " + timeInput);
    return currentTime;
  }
  if (parsedTime.hour != INVALID) {
    rtc.setHour(parsedTime.hour);
  }
  if (parsedTime.minute != INVALID) {
    rtc.setMinute(parsedTime.minute);
  }
  if (parsedSecond != INVALID) {
    rtc.setSecond(parsedSecond);
  }
  Serial.println("Selected time: " + String(parsedTime.hour) + ":" + String(parsedTime.minute) + ":" + String(parsedSecond));
  return parsedTime;
}

/** Turns all of the digital pins off. */
void turnAllOff() {
  for (int pin = 0; pin < 12; pin++) {
    digitalWrite(LEDS[pin], LOW);
  }
  digitalWrite(CENTER_LED, LOW);
}

void lightHour(byte hour) {
  if (hour == INVALID) return; // Ignore this, it's not a valid hour.
  digitalWrite(LEDS[hour % 12], HIGH);
  digitalWrite(CENTER_LED, HIGH);
}

void lightMinute(byte minute) {
  if (minute == INVALID) return; // Ignore this, it's not a valid minute.
  int pin = minute / 5;
  digitalWrite(LEDS[pin], HIGH);
}

bool isLaserTouching() {
  int potVal = analogRead(POT);
  int photoVal = analogRead(PHOTO);
  return photoVal >= potVal;
}
