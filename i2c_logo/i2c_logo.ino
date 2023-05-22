/*
  Flash a LED with logo code sent from I2C
  
  Author: Paul Hamilton
  Date: 5-May-2023
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c
*/

#include "ringbuffer.hpp"
#include "cmd.hpp"
#include "logo.hpp"
#include "arduinotimeprovider.hpp"

#include <Wire.h>

#define I2C_ADDRESS 8
#define LED_PIN     13

void ledOn() {
  digitalWrite(LED_PIN, HIGH);
}

void ledOff() {
  digitalWrite(LED_PIN, LOW);
}

RingBuffer buffer;
Cmd cmd;
char cmdbuf[64];

LogoBuiltinWord builtins[] = {
  { "ON", &ledOn },
  { "OFF", &ledOff },
};
ArduinoTimeProvider time;
Logo logo(builtins, sizeof(builtins), &time, Logo::core);

void flashErr(int mode, int n) {
  // it's ok to tie up the device with delays here.
  for (int i=0; i<mode; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  delay(500);
  for (int i=0; i<n; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

// At the start
void setup() {

  // Setup the LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Setup I2C bus address
  Wire.begin(I2C_ADDRESS);
  
  // Register handler for when data comes in
  Wire.onReceive(receiveEvent);

  // Compile a little program into the LOGO interpreter :-)
  logo.compile("TO FLASH; ON WAIT 100 OFF WAIT 1000; END;");
  logo.compile("TO GO; FOREVER FLASH; END;");
  logo.compile("TO STOP; OFF; END;");
  int err = logo.geterr();
  if (err) {
    flashErr(1, err + 2);
  }
 
  // this would make it just run straight away
//  logo.compile("GO");
}

// recieve data on I2C and write it into the buffer
void receiveEvent(int howMany) {
  while (Wire.available()) {
    buffer.write(Wire.read());
  }
}

// Go around and around
void loop() {

  // The buffer is filled in an interrupt as it comes in

  // accept the buffer into the command parser
  cmd.accept(&buffer);
  
  // when there is a valid command
  if (cmd.ready()) {
  
    // read it in
    cmd.read(cmdbuf, sizeof(cmdbuf));
    
    // reset the code but keep all our words we have defined.
    logo.resetcode();
    
    // compile whatever it is into the LOGO interpreter and if there's
    // a compile error flash the LED
    logo.compile(cmdbuf);
    int err = logo.geterr();
    if (err) {
      flashErr(1, err + 2);
    }
  }
  
  // just execute each LOGO word
  int err = logo.step();
  
  // And if there was an error doing that, apart from STOP (at the end)
  // flash the LED
  if (err && err != LG_STOP) {
    flashErr(2, err);
  }

}
