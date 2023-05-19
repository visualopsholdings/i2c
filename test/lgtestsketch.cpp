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
#include <thread>
#include <chrono>

#include "../logo.hpp"

#include <iostream>
#include <vector>
#include <strstream>

using namespace std;
using namespace boost::posix_time;

vector<string> gCmds;

//#define PRINT_RESULT

#include "nulltimeprovider.hpp"

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

class RealTimeProvider: public LogoTimeProvider {

public:

  // LogoTimeProvider
  virtual void schedule(short ms);
  virtual bool next();
  
private:
  ptime _lasttime;
  short _time;
};

void RealTimeProvider::schedule(short ms) {
  if (_lasttime == ptime(not_a_date_time)) {
    _lasttime = microsec_clock::local_time();
  }
  if (_time == 0) {
    _time = ms;
  }
  else {
    _time += ms;
  }
}

bool RealTimeProvider::next() {
  if (_time == 0) {
    return true;
  }
  ptime now = microsec_clock::local_time();
  time_duration diff =  now - _lasttime;
  if (diff.total_milliseconds() > _time) {
    _lasttime = now;
    _time = 0;
    return true;
  }
  this_thread::sleep_for(chrono::milliseconds(100));
  return false;

}

#if defined(HAS_FOREVER)

BOOST_AUTO_TEST_CASE( bigSketch )
{
  cout << "=== bigSketch ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
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

  gCmds.clear();
//   BOOST_CHECK_EQUAL(logo.run(), 0);
 DEBUG_STEP_DUMP(1000, false);
  for (int i=0; i<100; i++) {
    BOOST_CHECK_EQUAL(logo.step(), 0);
  }
  BOOST_CHECK_EQUAL(gCmds.size(), 49);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "WAIT 1000");
  BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[3], "WAIT 2000");
    
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
  };
//  RealTimeProvider time;
  NullTimeProvider time;
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


