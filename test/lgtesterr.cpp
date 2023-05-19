/*
  lgtesterr.cpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 6-May-2023
  
  Tiny Logo error tests.
  
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

#include "nulltimeprovider.hpp"

BOOST_AUTO_TEST_CASE( unknownWord )
{
  cout << "=== unknownWord ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  NullTimeProvider time;
  Logo logo(empty, 0, &time);

  logo.compile("XXXX");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

}

BOOST_AUTO_TEST_CASE( unknownWordInWord )
{
  cout << "=== unknownWordInWord ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  NullTimeProvider time;
  Logo logo(empty, 0, &time);

  logo.compile("TO TEST; ON; END;");
  DEBUG_DUMP(false);
  BOOST_CHECK_EQUAL(logo.geterr(), 0);

}

BOOST_AUTO_TEST_CASE( tooManyWords )
{
  cout << "=== tooManyWords ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  NullTimeProvider time;
  Logo logo(empty, 0, &time);

  for (short i=1; i<MAX_WORDS+2; i++) {
    strstream str;
    str << "TO WORD" << i << "; ON; END;";
    logo.compile(str.str());
  }
  DEBUG_DUMP();
  BOOST_CHECK(logo.haserr(LG_TOO_MANY_WORDS));
}

BOOST_AUTO_TEST_CASE( outOfStrings )
{
  cout << "=== outOfStrings ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  NullTimeProvider time;
  Logo logo(empty, 0, &time);
  
  short segn = 16;
  short seg = STRING_POOL_SIZE/segn;
  strstream str;
  for (short i=0; i<(segn + 2); i++) {
    for (short j=0; j<seg; j++) {
      str << "A";
    }
    str << ";";
  }

  logo.compile(str.str());
  DEBUG_DUMP(false);
  
  BOOST_CHECK_EQUAL(logo.geterr(), LG_OUT_OF_STRINGS);
  
}

BOOST_AUTO_TEST_CASE( lineTooLong )
{
  cout << "=== lineTooLong ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  NullTimeProvider time;
  Logo logo(empty, 0, &time);
  
  strstream str;
  for (short i=0; i<20; i++) {
    str << "A";
  }
  str << "; ";
  for (short i=0; i<LINE_LEN; i++) {
    str << "B";
  }
  str << "TOOBIG; ";
  
  logo.compile(str.str());
  DEBUG_DUMP(false);
  BOOST_CHECK_EQUAL(logo.geterr(), LG_LINE_TOO_LONG);
  
}

BOOST_AUTO_TEST_CASE( wordTooLong )
{
  cout << "=== wordTooLong ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  NullTimeProvider time;
  Logo logo(empty, 0, &time);
  
  strstream str;
  for (short i=0; i<WORD_LEN+1; i++) {
    str << "A";
  }
  str << "; ";
  
  logo.compile(str.str());
  DEBUG_DUMP(false);
  BOOST_CHECK_EQUAL(logo.geterr(), LG_WORD_TOO_LONG);
 
}

BOOST_AUTO_TEST_CASE( outOfCode )
{
  cout << "=== outOfCode ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  NullTimeProvider time;
  Logo logo(empty, 0, &time);
  
  strstream str;
  for (short i=0; i<MAX_CODE+2; i++) {
    str << "A;";
  }
  logo.compile(str.str());
  DEBUG_DUMP(false);
  BOOST_CHECK_EQUAL(logo.geterr(), LG_OUT_OF_CODE);
  
}

void nevercalled(Logo &logo) {
  BOOST_FAIL( "was called!" );
}

#ifdef HAS_SENTENCES

BOOST_AUTO_TEST_CASE( stackOverflow )
{
  cout << "=== stackOverflow ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "NEVER", &nevercalled, MAX_STACK+2 }
  };
  NullTimeProvider time;
  Logo logo(builtins, sizeof(builtins), &time);
  
  strstream str;
  str << "TO LOTS; ";
  for (short i=0; i<10; i++) {
    str << "A ";
  }
  str << "; END;";
  logo.compile(str.str());
  logo.compile("NEVER [LOTS LOTS LOTS LOTS LOTS LOTS LOTS]");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  DEBUG_STEP_DUMP(20, false);
  BOOST_CHECK_EQUAL(logo.run(), LG_STACK_OVERFLOW);
  
}

#endif // HAS_SENTENCES

#ifdef HAS_VARIABLES

BOOST_AUTO_TEST_CASE( tooManyVariables )
{
  cout << "=== tooManyVariables ===" << endl;
  
  LogoBuiltinWord empty[] = {};
  NullTimeProvider time;
  Logo logo(empty, 0, &time, Logo::core);

  for (short i=1; i<MAX_VARS+2; i++) {
    strstream str;
    str << "MAKE \"v" << i << " " << i;
    logo.compile(str.str());
  }
  DEBUG_DUMP(false);
  BOOST_CHECK_EQUAL(logo.run(), LG_TOO_MANY_VARS);
  
}

#endif // HAS_VARIABLES

#ifdef HAS_IFELSE

void condret(Logo &logo) {
  logo.condreturn(0);
}

void wantsstring(Logo &logo) {
  
  DEBUG_DUMP(false);
  tStrPool s, l;
  logo.codetostring(1, &s, &l);

}

BOOST_AUTO_TEST_CASE( notString )
{
  cout << "=== notString ===" << endl;
  
  LogoBuiltinWord builtins[] = {
    { "WANTSSTRING", &wantsstring, 0 }
  };
  NullTimeProvider time;
  Logo logo(builtins, sizeof(builtins), &time);
  
  logo.compile("WANTSSTRING 1");
  BOOST_CHECK_EQUAL(logo.geterr(), 0);
  DEBUG_DUMP(false);

  BOOST_CHECK_EQUAL(logo.run(), LG_NOT_STRING);

}

#endif // HAS_IFELSE


