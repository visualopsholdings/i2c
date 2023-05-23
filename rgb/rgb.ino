/*
  Control 2 RGB LEDs in LOGO over serial or i2c
  
  Author: Paul Hamilton
  Date: 22-May-2023
  
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
#define BLUE1_PIN   2
#define RED1_PIN  3
#define GREEN1_PIN    4

#define BLUE2_PIN   9
#define RED2_PIN  10
#define GREEN2_PIN    11

void red1(Logo &logo) {
  analogWrite(RED1_PIN, logo.popint());
}

void green1(Logo &logo) {
  analogWrite(GREEN1_PIN, logo.popint());
}

void blue1(Logo &logo) {
  analogWrite(BLUE1_PIN, logo.popint());
}

void red2(Logo &logo) {
  analogWrite(RED2_PIN, logo.popint());
}

void green2(Logo &logo) {
  analogWrite(GREEN2_PIN, logo.popint());
}

void blue2(Logo &logo) {
  analogWrite(BLUE2_PIN, logo.popint());
}

RingBuffer buffer;
Cmd cmd;
char cmdbuf[64];

LogoBuiltinWord builtins[] = {
  { "RED1", &red1, 1 },
  { "GREEN1", &green1, 1 },
  { "BLUE1", &blue1, 1 },
  { "RED2", &red2, 1 },
  { "GREEN2", &green2, 1 },
  { "BLUE2", &blue2, 1 },
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

  // the RGB pins
  for (int i=0, j=BLUE1_PIN; i<3; i++, j++) {
    pinMode(j, OUTPUT);
    analogWrite(j, 255);
  }
 for (int i=0, j=BLUE2_PIN; i<3; i++, j++) {
    pinMode(j, OUTPUT);
    analogWrite(j, 255);
  }

  // Setup I2C bus address
  Wire.begin(I2C_ADDRESS);
  
  // Register handler for when data comes in
  Wire.onReceive(receiveEvent);

  // Compile a little program into the LOGO interpreter :-)
  logo.compile("TO CCLR :CLR; 255 - :CLR; END;");
  logo.compile("TO CRED1 :R; RED1 CCLR :R; END;");
  logo.compile("TO CBLUE1 :R; BLUE1 CCLR :R; END;");
  logo.compile("TO CGREEN1 :R; GREEN1 CCLR :R; END;");
  logo.compile("TO CRED2 :R; RED2 CCLR :R; END;");
  logo.compile("TO CBLUE2 :R; BLUE2 CCLR :R; END;");
  logo.compile("TO CGREEN2 :R; GREEN2 CCLR :R; END;");
  logo.compile("TO AMBER1; CRED1 255 CGREEN1 191 CBLUE1 0; END;");
  int err = logo.geterr();
  if (err) {
    flashErr(1, err + 2);
  }
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
    
    // reset the code but keep all our words we have defined.
    logo.resetcode();
    
    // compile whatever it is into the LOGO interpreter and if there's
    // a compile error flash the LED
    logo.compile(cmdbuf);
    int err = logo.geterr();
    if (err) {
      flashErr(1, err);
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

