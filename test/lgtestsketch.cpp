/*
  lgtestsketch.cpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 6-May-2023
  
  Tiny Logo sketch example tests.
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c
*/

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "../logo.hpp"

#include <iostream>
#include <vector>
#include <strstream>

using namespace std;

vector<string> gCmds;

//#define PRINT_RESULT

void ledOn(Logo &logo) {
  gCmds.push_back("LED ON");
#ifdef PRINT_RESULT
  cout << gCmds.back() << endl;
#endif
}

void ledOff(Logo &logo) {
  gCmds.push_back("LED OFF");
#ifdef PRINT_RESULT
  cout << gCmds.back() << endl;
#endif
}

void wait(Logo &logo) {
  strstream str;
  str << "WAIT " << logo.popint();
  gCmds.push_back(str.str());
#ifdef PRINT_RESULT
  cout << gCmds.back() << endl;
#endif
}

using namespace boost::posix_time;

class RealTimeProvider: public LogoTimeProvider {

public:

  // LogoTimeProvider
  virtual void schedule(short ms);
  virtual bool next();
  
private:
  ptime _lasttime;
  ptime _nexttime;
};

void RealTimeProvider::schedule(short ms) {
  if (_lasttime == ptime(not_a_date_time)) {
    _lasttime = microsec_clock::local_time();
    cout << microsec_clock::local_time() << endl;
  }
  if (_nexttime == not_a_date_time) {
    _nexttime = _lasttime + milliseconds(ms);
  }
  else {
    _nexttime = _nexttime + milliseconds(ms);
  }
  cout << _nexttime << endl;
}

bool RealTimeProvider::next() {
  if (_nexttime == not_a_date_time) {
    return true;
  }
  ptime now = microsec_clock::local_time();
  if (now < _nexttime) {
    return false;
  }
  
//  cout << "next " << now << ", " << _nexttime << endl;
  _nexttime = not_a_date_time;
  _lasttime = now;
  return true;

}

class NullTimeProvider: public LogoTimeProvider {

public:
  
  // LogoTimeProvider
  virtual void schedule(short ms) {}
  virtual bool next() { return true; }
  
};

#if defined(HAS_FOREVER)

BOOST_AUTO_TEST_CASE( bigSketch )
{
  cout << "=== bigSketch ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
    { "WAIT", &wait, 1 },
  };
//  RealTimeProvider time;
  NullTimeProvider time;
  Logo logo(builtins, sizeof(builtins), &time, Logo::core);

  logo.compile("TO FLASH; ON WAIT 1000 OFF WAIT 2000; END;");
  logo.compile("TO GO; FOREVER FLASH; END;");
  logo.compile("TO STOP; END;");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);
  
  gCmds.clear();
//   DEBUG_STEP_DUMP(100, false);
  for (int i=0; i<10; i++) {
    BOOST_CHECK_EQUAL(logo.step(), 0);
  }
  BOOST_CHECK_EQUAL(gCmds.size(), 0);

  logo.resetcode();
  logo.compile("GO");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);
  BOOST_CHECK_EQUAL(logo.geterr(), 0);

  gCmds.clear();
//  BOOST_CHECK_EQUAL(logo.run(), 0);
  DEBUG_STEP_DUMP(20, false);
  for (int i=0; i<100; i++) {
    BOOST_CHECK_EQUAL(logo.step(), 0);
  }
  BOOST_CHECK_EQUAL(gCmds.size(), 49);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "WAIT 1000");
  BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[3], "WAIT 2000");
  BOOST_CHECK_EQUAL(gCmds[4], "LED ON");
    
  logo.resetcode();
  logo.compile("STOP");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);
  gCmds.clear();
//  DEBUG_STEP_DUMP(100, false);
  for (int i=0; i<10; i++) {
    BOOST_CHECK_EQUAL(logo.step(), 0);
  }
  BOOST_CHECK_EQUAL(gCmds.size(), 0);
  
}

#else

BOOST_AUTO_TEST_CASE( smallSketch )
{
  cout << "=== sketch ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
    { "WAIT", &wait, 1 },
  };
  RealTimeProvider time;
  Logo logo(builtins, sizeof(builtins), &time, Logo::core);

  logo.compile("TO FLASH; ON WAIT 1000 OFF WAIT 2000; END;");
  logo.compile("FLASH");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);
  
  gCmds.clear();
//  DEBUG_STEP_DUMP(100, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 4);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "WAIT 1000");
  BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[3], "WAIT 2000");
    
}

#endif


