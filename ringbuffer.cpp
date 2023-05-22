/*
  ringbuffer.cpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 5-May-2023
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c
*/

#include "ringbuffer.hpp"

#ifdef RINGBUF_DEBUG
#define EOB '.'
#include "test/debug.hpp"
void RingBuffer::outstate() const {
  cout << " _buffer " << _buffer << ", _readp " << _readp << ", _writep " << _writep << endl;
}
void RingBuffer::checkWrite() {
  if (_writep >= sizeof(_buffer)) {
    cout << "_writep " << _writep << " writing past end" << endl;
  }
}
#else
#define EOB 0xFF
#include "nodebug.hpp"
#endif

RingBuffer::RingBuffer() {
  _readp = 0;
  _writep = 0;
  for (int i=0; i<sizeof(_buffer); i++) {
    _buffer[i] = 0;
  }
}

RingBuffer::~RingBuffer() {
}

void RingBuffer::write(unsigned char c) {

  DEBUG_IN_ARGS(RingBuffer, "write", "%c", c);
  
  // don't write the end of buffer marker. Just fail.
  if (c == EOB) {
    return;
  }
  
  #ifdef RINGBUF_DEBUG
    checkWrite();
  #endif

  _buffer[_writep] = c;
  _writep++;
  if (_writep >= sizeof(_buffer)) {
    _buffer[0] = EOB;
    _writep = 0;
  }
  else {
    _buffer[_writep] = EOB;
  }

}

int RingBuffer::length() const {

  DEBUG_IN(RingBuffer, "length");
  
  // special case initial
  if (_writep == 0 && _buffer[sizeof(_buffer)-1] != EOB) {
    DEBUG_RETURN("%i", 0);
    return 0;
  }
  
  int len = 0;
  for (int i=_readp; i<=(sizeof(_buffer)-1); i++, len++) {
    if (_buffer[i] == EOB) {
      DEBUG_RETURN(" before . %i", len);
      return len;
    }
  }
  
  for (int i=0; i<=(sizeof(_buffer)-1); i++, len++) {
    if (_buffer[i] == EOB) {
      DEBUG_RETURN(" after . %i", len);
      return len;
    }
  }
  
  DEBUG_RETURN("%i", -1);
  return -1; 
}

unsigned char RingBuffer::read() {

  DEBUG_IN(RingBuffer, "read");
  
  unsigned char c = _buffer[_readp];
  _readp++;
  if (_readp >= sizeof(_buffer)) {
    _readp = 0;
  }
   
  DEBUG_RETURN("%c", c);
  return c;
}

void RingBuffer::write(const char *s) {

  DEBUG_IN_ARGS(RingBuffer, "writeS", "%s", s);
  
  for (int i=0; s[i]; i++) {
    write(s[i]);
  }
  
}

void RingBuffer::read(char *s, int size) {

  DEBUG_IN(RingBuffer, "readS");
  
  int i=0;
  while (length() > 0 && i < size) {
    s[i++] = read();
  }
  s[i] = 0;
  
  DEBUG_RETURN("%s", s);
}

int RingBuffer::readAppend(char *s, int size, int start) {

  int i=start;
  while (length() > 0 && i < size) {
    s[i++] = read();
  }
  s[i] = 0;
  
  return i;
}
