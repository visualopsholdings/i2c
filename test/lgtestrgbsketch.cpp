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

#define PRINT_RESULT
//#define REAL_TIME

#include "testtimeprovider.hpp"

void rgb(Logo &logo, const char *color, int num) {

  strstream str;
  str << color << num << " " << logo.popint();
  gCmds.push_back(str.str());
#ifdef PRINT_RESULT
  cout << gCmds.back() << endl;
#endif  

}

void red1(Logo &logo) {
  rgb(logo, "RED", 1);
}

void green1(Logo &logo) {
  rgb(logo, "GREEN", 1);
}

void blue1(Logo &logo) {
  rgb(logo, "BLUE", 1);
}

void red2(Logo &logo) {
  rgb(logo, "RED", 2);
}

void green2(Logo &logo) {
  rgb(logo, "GREEN", 2);
}

void blue2(Logo &logo) {
  rgb(logo, "BLUE", 2);
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
    { "RED1", &red1, 1 },
    { "GREEN1", &green1, 1 },
    { "BLUE1", &blue1, 1 },
    { "RED2", &red2, 1 },
    { "GREEN2", &green2, 1 },
    { "BLUE2", &blue2, 1 },
  };
#ifdef REAL_TIME
  RealTimeProvider time;
#else
  TestTimeProvider time;
#endif
  Logo logo(builtins, sizeof(builtins), &time, Logo::core);

  logo.compile("TO CCLR :CLR; 255 - :CLR; END;");
  logo.compile("TO CRED1 :R; RED1 CCLR :R; END;");
  logo.compile("TO CBLUE1 :R; BLUE1 CCLR :R; END;");
  logo.compile("TO CGREEN1 :R; GREEN1 CCLR :R; END;");
  logo.compile("TO CRED2 :R; RED2 CCLR :R; END;");
  logo.compile("TO CBLUE2 :R; BLUE2 CCLR :R; END;");
  logo.compile("TO CGREEN2 :R; GREEN2 CCLR :R; END;");
  logo.compile("TO AMBER1; CRED1 255 CGREEN1 191 CBLUE1 0; END;");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);
  
  logo.resetcode();
  logo.compile("AMBER1");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  DEBUG_STEP_DUMP(20, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  
  BOOST_CHECK_EQUAL(gCmds.size(), 3);
  BOOST_CHECK_EQUAL(gCmds[0], "RED1 0");
  BOOST_CHECK_EQUAL(gCmds[1], "GREEN1 64");
  BOOST_CHECK_EQUAL(gCmds[2], "BLUE1 255");

}
