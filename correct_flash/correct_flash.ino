/*
  Flash a LED the correct way
  
  Author: Paul Hamilton
  Date: 5-May-2023
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c
*/

#define LED_PIN     13

void ledOn() {
  digitalWrite(LED_PIN, HIGH);
}

void ledOff() {
  digitalWrite(LED_PIN, LOW);
}

// At the start
void setup() {

  // Setup the LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

int state = 0;
int lasttime = 0;

// Go around and around
void loop() {
  if (!lasttime) {
    lasttime = millis();
    return;
  }
  int now = millis();
  int diff = now - lasttime;
  if (state == 0 && diff > 1000) {
    ledOn();
    state = 1;
  }
  else if (state == 1 && diff > 100) {
    ledOff();
    state = 0;
  }
  else {
    return;
  }
  lasttime = millis();
}
