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

#if !defined(SUPER_TINY) && defined(HAS_VARIABLES) && defined(HAS_FOREVER) && defined(HAS_IFELSE)

BOOST_AUTO_TEST_CASE( bigSketch )
{
  cout << "=== bigSketch ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
    { "WAIT", &wait, 1 },
  };
  Logo logo(builtins, sizeof(builtins), Logo::core);

  logo.compile("TO FLASH; ON WAIT 100 OFF WAIT 1000; END;");
  logo.compile("TO TESTFLASH; IFELSE :RUNNING FLASH []; END;");
  logo.compile("TO RUN; FOREVER TESTFLASH; END;");
  logo.compile("TO GO; MAKE \"RUNNING 1; END;");
  logo.compile("TO STOP; MAKE \"RUNNING 0; END;");
  logo.compile("RUN GO");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);
  
  gCmds.clear();
//  DEBUG_STEP_DUMP(100, false);
  for (int i=0; i<100; i++) {
    BOOST_CHECK_EQUAL(logo.step(), 0);
  }
  BOOST_CHECK_EQUAL(gCmds.size(), 27);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "WAIT 100");
  BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[3], "WAIT 1000");
  BOOST_CHECK_EQUAL(gCmds[4], "LED ON");
    
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
  Logo logo(builtins, sizeof(builtins), Logo::core);

  logo.compile("TO FLASH; ON WAIT 100 OFF WAIT 1000; END;");
  logo.compile("FLASH");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);
  
  gCmds.clear();
//  DEBUG_STEP_DUMP(100, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 4);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "WAIT 100");
  BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[3], "WAIT 1000");
    
}

#endif


