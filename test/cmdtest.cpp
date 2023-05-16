/*
  cmdtests.cpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 6-May-2023
  
  Cmd code tests.
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c
*/

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "../ringbuffer.hpp"
#include "../cmd.hpp"

#include <iostream>

using namespace std;

BOOST_AUTO_TEST_CASE( simple )
{
  cout << "=== simple ===" << endl;
  
  RingBuffer buffer;
  Cmd cmd;

  buffer.write("ON;");
  cmd.accept(&buffer);
  
  BOOST_CHECK(cmd.ready());

  char c[20];
  cmd.read(c, sizeof(c));
  BOOST_CHECK_EQUAL(c, "ON");

  BOOST_CHECK(!cmd.ready());
}

BOOST_AUTO_TEST_CASE( partial )
{
  cout << "=== partial ===" << endl;
  
  RingBuffer buffer;
  Cmd cmd;

  buffer.write("O");
  cmd.accept(&buffer);
  
  BOOST_CHECK(!cmd.ready());

  buffer.write("N;");
  cmd.accept(&buffer);

  BOOST_CHECK(cmd.ready());

  char c[20];
  cmd.read(c, sizeof(c));
  BOOST_CHECK_EQUAL(c, "ON");

  BOOST_CHECK(!cmd.ready());
}

BOOST_AUTO_TEST_CASE( multiple )
{
  cout << "=== multiple ===" << endl;
  
  RingBuffer buffer;
  Cmd cmd;

  buffer.write("O");
  cmd.accept(&buffer);
  
  BOOST_CHECK(!cmd.ready());

  buffer.write("N OFF ");
  cmd.accept(&buffer);

  BOOST_CHECK(cmd.ready());

  char c[20];
  cmd.read(c, sizeof(c));
  BOOST_CHECK_EQUAL(c, "ON");

  BOOST_CHECK(cmd.ready());

  cmd.read(c, sizeof(c));
  BOOST_CHECK_EQUAL(c, "OFF");

  BOOST_CHECK(!cmd.ready());
}

BOOST_AUTO_TEST_CASE( multipleRead )
{
  cout << "=== multipleRead ===" << endl;
  
  RingBuffer buffer;
  Cmd cmd;

  buffer.write("ON;");
  cmd.accept(&buffer);
  
  BOOST_CHECK(cmd.ready());

  char c[20];
  cmd.read(c, sizeof(c));
  BOOST_CHECK_EQUAL(c, "ON");

  BOOST_CHECK(!cmd.ready());

  buffer.write("OFF ");
  cmd.accept(&buffer);

  BOOST_CHECK(cmd.ready());

  cmd.read(c, sizeof(c));
  BOOST_CHECK_EQUAL(c, "OFF");

  BOOST_CHECK(!cmd.ready());
}

BOOST_AUTO_TEST_CASE( newline )
{
  cout << "=== newline ===" << endl;
  
  RingBuffer buffer;
  Cmd cmd;

  buffer.write("ON\n");
  cmd.accept(&buffer);
  
  BOOST_CHECK(cmd.ready());

  char c[20];
  cmd.read(c, sizeof(c));
  BOOST_CHECK_EQUAL(c, "ON");

}

BOOST_AUTO_TEST_CASE( tab )
{
  cout << "=== tab ===" << endl;
  
  RingBuffer buffer;
  Cmd cmd;

  buffer.write("ON\t");
  cmd.accept(&buffer);
  
  BOOST_CHECK(cmd.ready());

  char c[20];
  cmd.read(c, sizeof(c));
  BOOST_CHECK_EQUAL(c, "ON");

}




