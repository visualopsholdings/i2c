/*
  lgtestwords.cpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 6-May-2023
  
  Tiny Logo sketch builtin word tests.
  
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

#include "testtimeprovider.hpp"

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

#ifdef HAS_VARIABLES
BOOST_AUTO_TEST_CASE( make )
{
  cout << "=== make ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("MAKE \"VAR 10");
  logo.compile(":VAR");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(logo.popint(), 10);
  BOOST_CHECK(logo.stackempty());
  DEBUG_DUMP(false);
  
}

BOOST_AUTO_TEST_CASE( makeChange )
{
  cout << "=== makeChange ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  TestTimeProvider time;
  Logo logo(empty, 0, &time, Logo::core);

  logo.compile("MAKE \"num 100");
  logo.compile("MAKE \"num 200");
  logo.compile("WAIT :num");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  DEBUG_DUMP(false);
  BOOST_CHECK_EQUAL(gCmds.size(), 1);
  BOOST_CHECK_EQUAL(gCmds[0], "WAIT 200");
  
}
#endif

#if defined(HAS_FOREVER) && defined(HAS_SENTENCES)

BOOST_AUTO_TEST_CASE( forever )
{
  cout << "=== forever ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
  };
  TestTimeProvider time;
  Logo logo(builtins, sizeof(builtins), &time, Logo::core);

  logo.compile("1 2 FOREVER [ON WAIT 10 OFF WAIT 20];");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  gCmds.clear();
//  DEBUG_STEP_DUMP(100, false);
  for (int i=0; i<100; i++) {
    BOOST_CHECK_EQUAL(logo.step(), 0);
  }
  BOOST_CHECK_EQUAL(gCmds.size(), 48);
  BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
  BOOST_CHECK_EQUAL(gCmds[1], "WAIT 10");
  BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
  BOOST_CHECK_EQUAL(gCmds[3], "WAIT 20");
  BOOST_CHECK_EQUAL(gCmds[4], "LED ON");
   
}

BOOST_AUTO_TEST_CASE( repeat )
{
  cout << "=== repeat ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
  };
  TestTimeProvider time;
  Logo logo(builtins, sizeof(builtins), &time, Logo::core);

  logo.compile("REPEAT 3 [ON WAIT 10 OFF WAIT 20];");
  DEBUG_DUMP(false);
  BOOST_CHECK_EQUAL(logo.geterr(), 0);

  gCmds.clear();
  DEBUG_STEP_DUMP(20, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  
  BOOST_CHECK_EQUAL(gCmds.size(), 12);
  for (int i=0; i<10; i++) {
    BOOST_CHECK_EQUAL(gCmds[0], "LED ON");
    BOOST_CHECK_EQUAL(gCmds[1], "WAIT 10");
    BOOST_CHECK_EQUAL(gCmds[2], "LED OFF");
    BOOST_CHECK_EQUAL(gCmds[3], "WAIT 20");
  }
  
}

#endif // defined(HAS_FOREVER) && defined(HAS_SENTENCES)

BOOST_AUTO_TEST_CASE( eq )
{
  cout << "=== eq ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("1 = 3");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(logo.popint(), 0);
  
  logo.reset();
  logo.compile("10 = 10");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);
  
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(logo.popint(), 1);
  
  BOOST_CHECK(logo.stackempty());
}

#ifdef HAS_IFELSE

BOOST_AUTO_TEST_CASE( ifelseFalse )
{
  cout << "=== ifelseFalse ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("TO TEST; 0; END");
  logo.compile("TO THEN; 2; END");
  logo.compile("TO ELSE; 3; END");
  logo.compile("IFELSE TEST THEN ELSE");
//  logo.compile("IFELSE [1 = 1] [THIS] [THAT]");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(logo.popint(), 3);
  BOOST_CHECK(logo.stackempty());

}

BOOST_AUTO_TEST_CASE( ifelseTrue )
{
  cout << "=== ifelseTrue ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("TO TEST; 1; END");
  logo.compile("TO THEN; 2; END");
  logo.compile("TO ELSE; 3; END");
  logo.compile("IFELSE TEST THEN ELSE");
//  logo.compile("IFELSE [1 = 1] [THIS] [THAT]");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(logo.popint(), 2);
  BOOST_CHECK(logo.stackempty());

}

#ifdef HAS_SENTENCES

BOOST_AUTO_TEST_CASE( ifelseCond )
{
  cout << "=== ifelseCond ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("IFELSE [1 = 0] [2] [3]");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(logo.popint(), 3);
  BOOST_CHECK(logo.stackempty());

}

BOOST_AUTO_TEST_CASE( ifelseSentences )
{
  cout << "=== ifelseSentences ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("IFELSE [1] [2] [3]");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(logo.popint(), 2);
  BOOST_CHECK(logo.stackempty());

}

BOOST_AUTO_TEST_CASE( ifelseLiteralTrueCond )
{
  cout << "=== ifelseLiteralTrueCond ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("IFELSE 1 [2] [3]");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(logo.popint(), 2);
  BOOST_CHECK(logo.stackempty());

}

BOOST_AUTO_TEST_CASE( ifelseLiteralFalseCond )
{
  cout << "=== ifelseLiteralFalseCond ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("IFELSE 0 [2] [3]");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(logo.popint(), 3);
  BOOST_CHECK(logo.stackempty());

}

#endif // HAS_SENTENCES

BOOST_AUTO_TEST_CASE( ifelseTrueLiteralNumBranches )
{
  cout << "=== ifelseTrueLiteralNumBranches ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("IFELSE 1 2 3");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(logo.popint(), 2);
  BOOST_CHECK(logo.stackempty());

}

BOOST_AUTO_TEST_CASE( ifelseFalseLiteralNumBranches )
{
  cout << "=== ifelseFalseLiteralNumBranches ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("IFELSE 0 2 3");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(logo.popint(), 3);
  BOOST_CHECK(logo.stackempty());

}

BOOST_AUTO_TEST_CASE( ifelseTrueLiteralStringBranches )
{
  cout << "=== ifelseTrueLiteralStringBranches ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("IFELSE 1 \"A \"B");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  char str[32];
  logo.popstring(str, sizeof(str));  
  BOOST_CHECK_EQUAL(str, "A");
  BOOST_CHECK(logo.stackempty());

}

BOOST_AUTO_TEST_CASE( ifelseFalseLiteralStringBranches )
{
  cout << "=== ifelseTrueLiteralStringBranches ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("IFELSE 0 \"A \"B");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  char str[32];
  logo.popstring(str, sizeof(str));  
  BOOST_CHECK_EQUAL(str, "B");
  BOOST_CHECK(logo.stackempty());

}

#ifdef HAS_VARIABLES

BOOST_AUTO_TEST_CASE( ifelseVarRef )
{
  cout << "=== ifelseVarRef ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("MAKE \"VAR 1");
  logo.compile("IFELSE :VAR \"A \"B");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  char str[32];
  logo.popstring(str, sizeof(str));  
  BOOST_CHECK_EQUAL(str, "A");
  BOOST_CHECK(logo.stackempty());

}

BOOST_AUTO_TEST_CASE( ifelseMissingVar )
{
  cout << "=== ifelseMissingVar ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  Logo logo(empty, 0, 0, Logo::core);

  logo.compile("IFELSE :VAR \"A \"B");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  DEBUG_STEP_DUMP(10, false);
  BOOST_CHECK_EQUAL(logo.run(), 0);
  char str[32];
  logo.popstring(str, sizeof(str));  
  BOOST_CHECK_EQUAL(str, "B");
  BOOST_CHECK(logo.stackempty());

}

#endif // HAS_VARIABLES

#endif // HAS_IFELSE

class TestWordTimeProvider: public LogoTimeProvider {

public:

  unsigned long currentms() {
    return _time;
  }
  void delayms(unsigned long ms) {
    _time += ms;
  }
  bool testing(short ms) {
    strstream str;
    str << "WAIT " << ms;
    gCmds.push_back(str.str());
#ifdef PRINT_RESULT
    cout << gCmds.back() << endl;
#endif
    return false;
  }
  void settime(unsigned long time) {
    _time = time;
  }
  
private:
  unsigned long _time;
  
};

BOOST_AUTO_TEST_CASE( waitWordFired )
{
  cout << "=== waitWordFired ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
  };
  TestWordTimeProvider time;
  Logo logo(builtins, sizeof(builtins), &time, Logo::core);
 
  logo.compile("WAIT 1000 ON");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  time.settime(900);
  
  gCmds.clear();
  BOOST_CHECK_EQUAL(logo.run(), 0);
  BOOST_CHECK_EQUAL(gCmds.size(), 2);

}

BOOST_AUTO_TEST_CASE( waitWordNotReady )
{
  cout << "=== waitWordNotReady ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
  };
  TestWordTimeProvider time;
  Logo logo(builtins, sizeof(builtins), &time, Logo::core);
 
  logo.compile("WAIT 1000 ON");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  time.settime(900);
  
  gCmds.clear();
  for (int i=0; i<10; i++) {
    BOOST_CHECK_EQUAL(logo.step(), 0);
  }
  BOOST_CHECK_EQUAL(gCmds.size(), 1);

}
