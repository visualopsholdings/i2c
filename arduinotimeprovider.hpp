/*
  arduinotimeprovider.hpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 22-May-2023

  A provider of time when on an arduino.
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c  
*/

#ifndef H_arduinotimeprovider
#define H_arduinotimeprovider

class ArduinoTimeProvider: public LogoTimeProvider {

public:

  // LogoTimeProvider
  virtual unsigned long currentms();
  virtual void delayms(unsigned long ms);
  virtual bool testing(short ms) { return false; };

};
unsigned long ArduinoTimeProvider::currentms() {
  return millis();
}
void ArduinoTimeProvider::delayms(unsigned long ms) {
  delay(ms);
}

#endif // H_arduinotimeprovider