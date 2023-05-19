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

// Uncomment one of these for the example you want to use
//
//#define SIMPLE_FLASH        // flash an LED on and off the normal way. 3.9K
//#define SERIAL_FLASH        // Use serial command "ON" and "OFF" to turn the LED on and off. 4.6k       
//#define I2C_FLASH           // Use I2C command "ON;" and "OFF;" to turn the LED on and off. 5.8k
//#define SERIAL_LOGO         // Use Serial commands to run LOGO code. 13.8k
#define I2C_SERIAL_LOGO     // Use Serial commands and I2c to run LOGO code. 15.2k
//#define I2C_LOGO             // Use I2c to run LOGO code. 14.8k
//#define CORRECT_FLASH         // An example of how you should properly flash an LED

// For the LOGO examples, "GO;" makes it start. "STOP;" makes it stop 
// When sedning through serialm, you don't need the semicolon.

#if defined(SERIAL_LOGO) || defined(I2C_SERIAL_LOGO) || defined(I2C_LOGO)
#define HAS_LOGO
#endif

#if defined(I2C_FLASH) || defined(I2C_SERIAL_LOGO) || defined(I2C_LOGO)
#define HAS_I2C
#endif

#if defined(SERIAL_FLASH) || defined(SERIAL_LOGO) || defined(I2C_SERIAL_LOGO)
#define HAS_SERIAL
#endif

#ifdef HAS_I2C
#include <Wire.h>
#endif

#ifdef HAS_LOGO
#include "logo.hpp"
#endif

#ifdef HAS_I2C
#define I2C_ADDRESS 8
#endif

#define LED_PIN     13

void ledOn() {
  digitalWrite(LED_PIN, HIGH);
}

void ledOff() {
  digitalWrite(LED_PIN, LOW);
}

#ifdef HAS_LOGO
void wait(Logo &logo) {
  delay(logo.popint());
}
#endif

#if defined(HAS_SERIAL) || defined(HAS_I2C)
RingBuffer buffer;
Cmd cmd;
char cmdbuf[64];
#endif

#ifdef HAS_LOGO
LogoBuiltinWord builtins[] = {
  { "ON", &ledOn },
  { "OFF", &ledOff },
  { "WAIT", &wait, 1 },
};
// not currently working.
// class ArduinoTimeProvider: public LogoTimeProvider {

// public:

//   // LogoTimeProvider
//   virtual unsigned long currentms();
//   virtual void delay(unsigned long ms);
//   virtual bool testing(short ms) { return false; };

// };
// unsigned long ArduinoTimeProvider::currentms() {
//   return millis();
// }
// void ArduinoTimeProvider::delay(unsigned long ms) {
//   delay(ms);
// }
// ArduinoTimeProvider time;
// Logo logo(builtins, sizeof(builtins), &time, Logo::core);
Logo logo(builtins, sizeof(builtins), 0, Logo::core);
#endif

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

#ifdef SIMPLE_FLASH

// At the start
void setup() {

  // Setup the LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

// Go around and around
void loop() {

  // flash it
  ledOn();
  delay(100);
  ledOff();
  delay(1000);

}

#endif // SIMPLE_FLASH

#ifdef I2C_FLASH

// At the start
void setup() {

  // Setup the LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Setup I2C bus address
  Wire.begin(I2C_ADDRESS);
  
  // Register handler for when data comes in
  Wire.onReceive(receiveEvent);

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
    
    // Use it to switch the LED on or OFF or if we don't understand give an error
    if (strcmp(cmdbuf, "ON") == 0) {
      ledOn();
    }
    else if (strcmp(cmdbuf, "OFF") == 0) {
      ledOff();
    }
    else {
      flashErr(1, 2);
    }
  }

}

#endif // I2C_FLASH

#ifdef SERIAL_FLASH

// At the start
void setup() {

  // Setup the LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Setup the serial port
  Serial.begin(9600);

}

// Go around and around
void loop() {
  
  // consume the serial data into the buffer as it comes in.
  if (Serial.available()) {
    buffer.write(Serial.read());
  }

  // accept the buffer into the command parser
  cmd.accept(&buffer);

  // when there is a valid command
  if (cmd.ready()) {

    // read it in
    cmd.read(cmdbuf, sizeof(cmdbuf));

    // and use it to turn the LED on or OFF or if we don't
    // understand flash an error
    if (strcmp(cmdbuf, "ON") == 0) {
      ledOn();
    }
    else if (strcmp(cmdbuf, "OFF") == 0) {
      ledOff();
    }
    else {
      flashErr(1, 2);
    }

  }

}

#endif // SERIAL_FLASH

#ifdef SERIAL_LOGO

// At the start
void setup() {

  // Setup the LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Setup the serial port
  Serial.begin(9600);

  // Compile a little program into the LOGO interpreter :-)
  logo.compile("TO FLASH; ON WAIT 100 OFF WAIT 1000; END;");
  logo.compile("TO GO; FOREVER FLASH; END;");
  logo.compile("TO STOP; END;");
  int err = logo.geterr();
  if (err) {
    flashErr(1, err + 2);
    Serial.println(err);
  }
 
  // this would make it just run straight away
 // logo.compile("GO");
}

// Go around and around
void loop() {

  // consume the serial data into the buffer as it comes in.
  while (Serial.available()) {
    buffer.write(Serial.read());
  }
 
  // accept the buffer into the command parser
  cmd.accept(&buffer);

  // when there is a valid command
  if (cmd.ready()) {

    // read it in
    cmd.read(cmdbuf, sizeof(cmdbuf));
    Serial.println(cmdbuf);

    // reset the code but keep all our words we have defined.
    logo.resetcode();
    
    // compile whatever it is into the LOGO interpreter and if there's
    // a compile error flash the LED
    logo.compile(cmdbuf);
    int err = logo.geterr();
    if (err) {
      flashErr(1, err);
      Serial.println(err);
    }
    
    // Restart the machine
    logo.restart();
  }

  // just execute each LOGO word
  int err = logo.step();
  
  // And if there was an error doing that, apart from STOP (at the end)
  // flash the LED
  if (err && err != LG_STOP) {
    flashErr(2, err);
    Serial.println(err);
  }
}

#endif // SERIAL_LOGO

#ifdef I2C_SERIAL_LOGO

// At the start
void setup() {

  // Setup the LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Setup the serial port
  Serial.begin(9600);

  // Setup I2C bus address
  Wire.begin(I2C_ADDRESS);
  
  // Register handler for when data comes in
  Wire.onReceive(receiveEvent);

  // Compile a little program into the LOGO interpreter :-)
  logo.compile("TO FLASH; ON WAIT 100 OFF WAIT 1000; END;");
  logo.compile("TO GO; FOREVER FLASH; END;");
  logo.compile("TO STOP; END;");
  int err = logo.geterr();
  if (err) {
    flashErr(1, err + 2);
    Serial.println(err);
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

  // consume the serial data into the buffer as it comes in.
  while (Serial.available()) {
    buffer.write(Serial.read());
  }
 
  // The buffer is also filled in an interrupt as it comes in

  // accept the buffer into the command parser
  cmd.accept(&buffer);
  
  // when there is a valid command
  if (cmd.ready()) {
  
    // read it in
    cmd.read(cmdbuf, sizeof(cmdbuf));
    Serial.println(cmdbuf);

    // reset the code but keep all our words we have defined.
    logo.resetcode();
    
    // compile whatever it is into the LOGO interpreter and if there's
    // a compile error flash the LED
    logo.compile(cmdbuf);
    int err = logo.geterr();
    if (err) {
      flashErr(1, err + 2);
      Serial.print("compile ");
      Serial.println(err);
    }
  }

  // just execute each LOGO word
  int err = logo.step();

  // And if there was an error doing that, apart from STOP (at the end)
  // flash the LED
  if (err && err != LG_STOP) {
    flashErr(2, err + 2);
    Serial.print("runtime ");
    Serial.println(err);
  }
}

#endif // I2C_SERIAL_LOGO

#ifdef I2C_LOGO

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
  logo.compile("TO STOP; END;");
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
    flashErr(2, err + 2);
  }
}

#endif // I2C_LOGO

#ifdef CORRECT_FLASH

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

#endif // CORRECT_FLASH
