/*
  arduinodebug.hpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 22-May-2023

  A simple debugger that will output to the serial port entries and exits to
  functions.
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c  
*/

#ifndef H_arduinodebug
#define H_arduinodebug

#include <HardwareSerial.h>

class Debug {
public:
  Debug(const char *classname) {
    Serial.print(classname);
    Serial.print("- ");
  }
  ~Debug() {
    Serial.println(" --");
  }
};

#define DEBUG_IN_ARGS(__class__, __name__, __fmt__, ...)    Debug debug(__name__)
#define DEBUG_IN(__class__, __name__)                       Debug debug(__name__)
#define DEBUG_RETURN(__fmt__, ...)                          
#define DEBUG_OUT(__fmt__, ...)                             

#endif // H_arduinodebug
