/*
  rbtests.cpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 6-May-2023
  
  Ring Buffer tests.
  
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

// make sure reads and writes work in the simple case where the buffer doesn't wrap around

BOOST_AUTO_TEST_CASE( simple )
{
  cout << "=== simple ===" << endl;
  
  RingBuffer buffer;
  BOOST_CHECK_EQUAL(buffer.length(), 0);
  
  // write some
  buffer.write('A');
  BOOST_CHECK_EQUAL(buffer.length(), 1);
  buffer.write('B');
  BOOST_CHECK_EQUAL(buffer.length(), 2);
  
  // read 1 byte from it
  BOOST_CHECK_EQUAL(buffer.read(), 'A');
  BOOST_CHECK_EQUAL(buffer.length(), 1);
  
  // write some more
  buffer.write('C');
  BOOST_CHECK_EQUAL(buffer.length(), 2);
  buffer.write('D');
  BOOST_CHECK_EQUAL(buffer.length(), 3);

  // read 1 byte from it
  BOOST_CHECK_EQUAL(buffer.read(), 'B');
  BOOST_CHECK_EQUAL(buffer.length(), 2);
  
  // read 1 byte from it
  BOOST_CHECK_EQUAL(buffer.read(), 'C');
  BOOST_CHECK_EQUAL(buffer.length(), 1);

  // read 1 byte from it
  BOOST_CHECK_EQUAL(buffer.read(), 'D');
  BOOST_CHECK_EQUAL(buffer.length(), 0);

}

// the same as simple, but use strings instead for testing and implementation ease.

BOOST_AUTO_TEST_CASE( stringSimple )
{
  cout << "=== stringSimple ===" << endl;
  
  RingBuffer buffer;
  BOOST_CHECK_EQUAL(buffer.length(), 0);
  
  // write some
  buffer.write("AB");
  BOOST_CHECK_EQUAL(buffer.length(), 2);
  
  // read 1 byte from it
  BOOST_CHECK_EQUAL(buffer.read(), 'A');
  BOOST_CHECK_EQUAL(buffer.length(), 1);
  
  // write some more
  buffer.write("CD");
  BOOST_CHECK_EQUAL(buffer.length(), 3);

  // read string from it
  char s[5];
  buffer.read(s, sizeof(s));
  BOOST_CHECK_EQUAL(s, "BCD");
  BOOST_CHECK_EQUAL(buffer.length(), 0);

}

// test overfilling

BOOST_AUTO_TEST_CASE( overfillKeepUp )
{
  cout << "=== overfillKeepUp ===" << endl;
  
  RingBuffer buffer;
  BOOST_CHECK_EQUAL(buffer.length(), 0);
  
  // write some
  buffer.write("ABCDEFGH");
  BOOST_CHECK_EQUAL(buffer.length(), 8);
  
  // read a few
  BOOST_CHECK_EQUAL(buffer.read(), 'A');
  BOOST_CHECK_EQUAL(buffer.length(), 7);
  BOOST_CHECK_EQUAL(buffer.read(), 'B');
  BOOST_CHECK_EQUAL(buffer.length(), 6);
  
  // write some more to wrap it.
  buffer.write("IJ");
  BOOST_CHECK_EQUAL(buffer.length(), 8);
    
  // read the remainder
  char s[8];
  BOOST_CHECK_EQUAL(buffer.readAppend(s, sizeof(s), 0), 8);
  BOOST_CHECK_EQUAL(s, "CDEFGHIJ");
  BOOST_CHECK_EQUAL(buffer.length(), 0);

}

// test overfilling but not keeping up

BOOST_AUTO_TEST_CASE( overfillDidntKeepUp )
{
  cout << "=== overfillDidntKeepUp ===" << endl;
  
  RingBuffer buffer;
  BOOST_CHECK_EQUAL(buffer.length(), 0);
  
  // write some
  buffer.write("ABCDEFGH");
  BOOST_CHECK_EQUAL(buffer.length(), 8);
  
  // read 2 of them.
  BOOST_CHECK_EQUAL(buffer.read(), 'A');
  BOOST_CHECK_EQUAL(buffer.length(), 7);
  BOOST_CHECK_EQUAL(buffer.read(), 'B');
  BOOST_CHECK_EQUAL(buffer.length(), 6);
  
  // write a a whole heap to wrap it right around.
  buffer.write("IJKLMN");
  
  // this is wrong but ok
  BOOST_CHECK_EQUAL(buffer.length(), 3);
    
  // read string from it
  // wrong but can work.
  char s[8];
  buffer.read(s, sizeof(s));
  BOOST_CHECK_EQUAL(s, "LMN");
  BOOST_CHECK_EQUAL(buffer.length(), 0);

}


