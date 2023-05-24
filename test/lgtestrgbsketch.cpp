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
//#define REAL_TIME

#include "testtimeprovider.hpp"

void rgb(Logo &logo, const char *color) {

  strstream str;
  str << color << " " << logo.popint();
  gCmds.push_back(str.str());
#ifdef PRINT_RESULT
  cout << gCmds.back() << endl;
#endif  

}

void red(Logo &logo) {
  rgb(logo, "RED");
}

void green(Logo &logo) {
  rgb(logo, "GREEN");
}

void blue(Logo &logo) {
  rgb(logo, "BLUE");
}

#ifdef REAL_TIME
class RealTimeProvider: public LogoTimeProvider {

public:

  unsigned long currentms() {
    std::time_t t = 1118158776;
    ptime first = from_time_t(t);
    ptime nowt = microsec_clock::local_time();
    time_duration start = nowt - first;
    return start.total_milliseconds();
  }
  void delayms(unsigned long ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
  }
  bool testing(short ms) { 
    cout << "WAIT " << ms << endl;
    return false; 
  };
  
};
#endif

BOOST_AUTO_TEST_CASE( rgbSketch )
{
  cout << "=== rgbSketch ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "RED", &red, 1 },
    { "GREEN", &green, 1 },
    { "BLUE", &blue, 1 },
  };
#ifdef REAL_TIME
  RealTimeProvider time;
#else
  TestTimeProvider time;
#endif
  Logo logo(builtins, sizeof(builtins), &time, Logo::core);

  logo.compile("TO C :C; 100 - :C / 100  * 255; END;");
  logo.compile("TO R :N; RED C :N; END;");
  logo.compile("TO G :N; GREEN C :N; END;");
  logo.compile("TO B :N; BLUE C :N; END;");
  logo.compile("TO A; R 100 G 75 B 0; END;");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);
  
  logo.resetcode();
  logo.compile("A");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  DEBUG_STEP_DUMP(20, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  
  BOOST_CHECK_EQUAL(gCmds.size(), 3);
  BOOST_CHECK_EQUAL(gCmds[0], "RED 0");
  BOOST_CHECK_EQUAL(gCmds[1], "GREEN 63");
  BOOST_CHECK_EQUAL(gCmds[2], "BLUE 255");

}
