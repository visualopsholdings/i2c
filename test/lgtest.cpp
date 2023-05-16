/*
  lgtest.cpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 6-May-2023
  
  Tiny Logo general tests.
  
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

BOOST_AUTO_TEST_CASE( builtin )
{
  cout << "=== builtin ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("ON");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);
  
  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.step(), 0);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  DEBUG_DUMP(false);
  
}

BOOST_AUTO_TEST_CASE( defineSimpleWord )
{
  cout << "=== defineSimpleWord ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("TO TURNON; ON; END;");
  logo.compile("TURNON;");
  logo.compile("OFF;");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.step(), 0);
  DEBUG_DUMP(false);
  BOOST_CHECK_EQUAL(logo.step(), 0);
  DEBUG_DUMP(false);
  BOOST_CHECK_EQUAL(logo.step(), 0);
  DEBUG_DUMP(false);
  BOOST_CHECK_EQUAL(logo.step(), 0);
  DEBUG_DUMP(false);
  BOOST_CHECK_EQUAL(gCmds.size(), 2);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "LED OFF");
  
}

BOOST_AUTO_TEST_CASE( compound )
{
  cout << "=== compound ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
    { "WAIT", &wait, 1 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("ON WAIT 100 OFF WAIT 20");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  DEBUG_STEP_DUMP(3, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "WAIT 100");
  BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[3], "WAIT 20");
  
}

BOOST_AUTO_TEST_CASE( defineCompoundWord )
{
  cout << "=== defineCompoundWord ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
    { "WAIT", &wait, 1 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("TO TEST1; ON WAIT 100 OFF WAIT 20; END;");
  logo.compile("TO TEST2; OFF WAIT 30 ON WAIT 40; END;");
  logo.compile("TEST1; TEST2;");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 8);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "WAIT 100");
  BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[3], "WAIT 20");
  BOOST_CHECK_EQUAL(gCmds[4], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[5], "WAIT 30");
  BOOST_CHECK_EQUAL(gCmds[6], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[7], "WAIT 40");
  
}

BOOST_AUTO_TEST_CASE( nestedWord )
{
  cout << "=== nestedWord ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
    { "WAIT", &wait, 1 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("TO TEST1; ON WAIT 100 OFF WAIT 20; END;");
  logo.compile("TO TEST2; TEST1; END;");
  logo.compile("TEST2;");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 4);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "WAIT 100");
  BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[3], "WAIT 20");
  
}

BOOST_AUTO_TEST_CASE( defineCompoundWordRun1 )
{
  cout << "=== defineCompoundWord ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
    { "WAIT", &wait, 1 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("TO TEST1; ON WAIT 10 OFF WAIT 20; END;");
  logo.compile("TO TEST2; OFF WAIT 30 ON WAIT 40; END;");
  logo.compile("TEST1;");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 4);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "WAIT 10");
  BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[3], "WAIT 20");
  
}

#ifdef HAS_SENTENCES

BOOST_AUTO_TEST_CASE( sentence )
{
  cout << "=== sentence ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
    { "WAIT", &wait, 1 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("[ON WAIT 10 OFF WAIT 20];");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 4);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "WAIT 10");
  BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[3], "WAIT 20");
  
}

BOOST_AUTO_TEST_CASE( sentences )
{
  cout << "=== sentences ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
    { "WAIT", &wait, 1 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("[ON] [WAIT 10] [OFF] [WAIT 20];");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
//   DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 4);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "WAIT 10");
  BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[3], "WAIT 20");
  
}

void sarg(Logo &logo) {

  DEBUG_DUMP_MSG("sarg", false);
  
  char s[WORD_LEN];
  logo.popstring(s, sizeof(s));
//  logo.execword(s);

  strstream str;
  str << "SARG " << s;
  gCmds.push_back(str.str());
#ifdef PRINT_RESULT
  cout << gCmds.back() << endl;
#endif  
}

BOOST_AUTO_TEST_CASE( sentenceInWord )
{
  cout << "=== sentenceInWord ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "SARG", &sarg, 1 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("TO TEST; SARG [XXX]; END;");
  logo.compile("TEST;");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  DEBUG_STEP_DUMP(7, 0);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 1);
  BOOST_CHECK_EQUAL(gCmds[0], "SARG XXX");
  
}

#endif

BOOST_AUTO_TEST_CASE( arityLiteral1 )
{
  cout << "=== arityLiteral1 ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "WAIT", &wait, 1 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("WAIT 20");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 1);
  BOOST_CHECK_EQUAL(gCmds[0], "WAIT 20");
  
}

BOOST_AUTO_TEST_CASE( arityWord1 )
{
  cout << "=== arityWord1 ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "WAIT", &wait, 1 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("TO TIME; 20; END");
  logo.compile("WAIT TIME");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  DEBUG_STEP_DUMP(5, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 1);
  BOOST_CHECK_EQUAL(gCmds[0], "WAIT 20");
  
}

void args2(Logo &logo) {
  char s[32];
  logo.popstring(s, sizeof(s));
  int n = logo.popint();
  strstream str;
  str << "ARGS2 " << n << ", " << s;
  gCmds.push_back(str.str());
#ifdef PRINT_RESULT
  cout << gCmds.back() << endl;
#endif
}

BOOST_AUTO_TEST_CASE( arityLiteral )
{
  cout << "=== arityLiteral ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ARGS2", &args2, 2 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("ARGS2 20 XXXX");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 1);
  BOOST_CHECK_EQUAL(gCmds[0], "ARGS2 20, XXXX");
  
}

BOOST_AUTO_TEST_CASE( arityWord )
{
  cout << "=== arityWord ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ARGS2", &args2, 2 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("TO TIME; 20; END");
  logo.compile("TO THING; XXXX; END");
  logo.compile("ARGS2 TIME THING");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 1);
  BOOST_CHECK_EQUAL(gCmds[0], "ARGS2 20, XXXX");
  
}

#ifdef HAS_SENTENCES

BOOST_AUTO_TEST_CASE( seperateLines )
{
  cout << "=== seperateLines ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "SARG", &sarg, 1 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("TO TEST");
  logo.compile("  SARG [XXX]");
  logo.compile("END");
  logo.compile("TEST");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 1);
  BOOST_CHECK_EQUAL(gCmds[0], "SARG XXX");
  
}

BOOST_AUTO_TEST_CASE( newLines )
{
  cout << "=== seperateLines ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "SARG", &sarg, 1 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("TO TEST\n\tSARG [XXX]\nEND\nTEST");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 1);
  BOOST_CHECK_EQUAL(gCmds[0], "SARG XXX");
  
}

BOOST_AUTO_TEST_CASE( reset )
{
  cout << "=== reset ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "SARG", &sarg, 1 }
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("TO TEST\n\tSARG [XXX]\nEND\nTEST");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 1);
  BOOST_CHECK_EQUAL(gCmds[0], "SARG XXX");
  
  gCmds.clear();
  logo.restart();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 1);
  BOOST_CHECK_EQUAL(gCmds[0], "SARG XXX");
  
}

#endif

void infixfn(Logo &logo) {

  strstream str;
  str << "INFIX " << logo.popint() << ", " << logo.popint();
  gCmds.push_back(str.str());
#ifdef PRINT_RESULT
  cout << gCmds.back() << endl;
#endif

}

BOOST_AUTO_TEST_CASE( infix )
{
  cout << "=== infix ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "INFIX", &infixfn, 1 },
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("1 INFIX 5");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  DEBUG_STEP_DUMP(5, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 1);
  BOOST_CHECK_EQUAL(gCmds[0], "INFIX 5, 1");

}

BOOST_AUTO_TEST_CASE( literalsOnStack )
{
  cout << "=== literalsOnStack ===" << endl;
  
  LogoBuiltinWord builtins[] = {
  };

  Logo logo(builtins, sizeof(builtins));

  logo.compile("TO NUM; 1 2 3; END; NUM");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(logo.popint(), 3);
  BOOST_CHECK_EQUAL(logo.popint(), 2);
  BOOST_CHECK_EQUAL(logo.popint(), 1);
  DEBUG_DUMP(false);
  
}


BOOST_AUTO_TEST_CASE( sizes )
{
  cout << "=== sizes ===" << endl;
  
  // half this size on an arduino
  BOOST_CHECK_EQUAL(sizeof(LogoBuiltinWord), 24);
  BOOST_CHECK_EQUAL(sizeof(LogoWord), 6);
  BOOST_CHECK_EQUAL(sizeof(LogoInstruction), 6);
  BOOST_CHECK_EQUAL(sizeof(LogoVar), 10);
  
}
