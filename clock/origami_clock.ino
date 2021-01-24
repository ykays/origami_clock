#include <Wire.h>
#include <DS3231.h>

int LEDS[12] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
int CENTER_LED = A2;
int POT = A0;
int PHOTO = A1;

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
byte parseHour(char* timeInput);
byte parseMinute(char* timeInput);
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

  // Wait until we receive a valid time to display (this will eventually happen in the loop, based on the RTC module).
  while (currentTime.hour == INVALID && currentTime.minute == INVALID) {
    // wait for input from the user with the time.  Time must be formatted as HH:mm.  Eventually we'll check the time from the RTC module.
    //currentTime = getTimeFromUser();
    currentTime.hour = 12;
    currentTime.minute = 45;
  }
}

void loop() {
  // Check whether the laser is turning the clock on/off.
  if (isLaserTouching()) {
    clockOn = !clockOn;
    needsUpdate = true;
  }

  // Check whether the time has changed in the rtc module.
  bool h12;
  bool pmTime;
  Time newTime = {
    .hour = rtc.getHour(h12, pmTime),
    .minute = rtc.getMinute(),
  };
  if (newTime.hour != currentTime.hour || newTime.minute != currentTime.minute) {
    currentTime = newTime;
    needsUpdate = true;
  }

  if (!needsUpdate) return;

  turnAllOff();
  if (clockOn) {
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

/** Waits for the user to send a time over serial.
 *  - The time should be in the format HH:mm.
 *  - The function will block and the return the string.
 */
Time getTimeFromUser() {
  Serial.println("Please enter the time to display (HH:mm): ");
  while (!Serial.available()) {} // Do nothing until there's a string available.
  String timeInput = Serial.readStringUntil('\n');
  struct Time parsedTime;
  parsedTime.hour = parseHour(timeInput);
  parsedTime.minute = parseMinute(timeInput);
  if (parsedTime.hour != INVALID) {
    rtc.setHour(parsedTime.hour);
  }
  if (parsedTime.minute != INVALID) {
    rtc.setMinute(parsedTime.minute);
  }
  Serial.println("Selected time: " + String(parsedTime.hour) + ":" + String(parsedTime.minute));
  // Tell any Serial listeners to set their minute hand, only.
  Serial.println("Setting minute to:");
  Serial.println("--:" + String(parsedTime.minute));
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
