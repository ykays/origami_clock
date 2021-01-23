int LEDS[12] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

struct Time {
  int hour;
  int minute;
};

Time getTimeFromUser();
int parseHour(char* timeInput);
int parseMinute(char* timeInput);
void setupLeds();
void turnAllOff();
void setHour(int hour);
void setMinute(int minute);

void setup() {
  setupLeds();
  Serial.begin(9600);
}

void loop() {
  // wait for input from the user with the time.  Time must be formatted as HH:mm.  Eventually we'll wait for the photoresistor and check the time from the RTC module.
  Time currentTime = getTimeFromUser();

  // turn everything off
  turnAllOff();

  // light up the hour
  setHour(currentTime.hour);

  // light up the minute
  setMinute(currentTime.minute);
}

void setupLeds() {
  for (int pin = 0; pin < 12; pin++) {
    pinMode(LEDS[pin], OUTPUT);
  }
  turnAllOff();
  for (int pin = 0; pin < 12; pin++) {
    digitalWrite(LEDS[pin], HIGH);
    delay(250);
    digitalWrite(LEDS[pin], LOW);
  }
}

int parseHour(String timeInput) {
  String hour = timeInput.substring(0, 2);
  return hour.toInt();
}

int parseMinute(String timeInput) {
  String minute = timeInput.substring(3, 5);
  return minute.toInt();
}

/** Waits for the user to send a time over serial.
 *  - The time should be in the format HH:mm.
 *  - The function will block and the return the string.
 */
Time getTimeFromUser() {
  Serial.println("Please enter the time to display (HH:mm): ");
  while (!Serial.available()) {} // Do nothing until there's serial data available.
  String timeInput = Serial.readString();
  struct Time parsedTime;
  parsedTime.hour = parseHour(timeInput);
  parsedTime.minute = parseMinute(timeInput);
  Serial.print("Selected time: This is the hour: ");
  Serial.print(parsedTime.hour);
  Serial.print(" and this is the minute: ");
  Serial.println(parsedTime.minute);
  return parsedTime;
}

/** Turns all of the digital pins off. */
void turnAllOff() {
  for (int pin = 0; pin < 12; pin++) {
    digitalWrite(LEDS[pin], LOW);
  }
}

void setHour(int hour) {
  digitalWrite(LEDS[hour % 12], HIGH);
}

void setMinute(int minute) {
  int pin = minute / 5;
  digitalWrite(LEDS[pin], HIGH);
}
