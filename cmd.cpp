/*
  cmd.cpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 5-May-2023
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c
*/

#include "cmd.hpp"
#include "ringbuffer.hpp"

#include <string.h>

#ifdef CMD_DEBUG
#include "test/debug.hpp"
void Cmd::outstate() const {
  cout << " _buffer " << _buffer << " _len " << _len << endl;
}
#else
#include "nodebug.hpp"
#endif

Cmd::Cmd() {
  _len = 0;
}

Cmd::~Cmd() {
}

void Cmd::accept(RingBuffer *buffer) {
  
  DEBUG_IN(Cmd, "accept");
  
  _len = buffer->readAppend(_buffer, sizeof(_buffer), _len);
  
}

int Cmd::findNext() {

  DEBUG_IN(Cmd, "findNext");
  
  for (int i=0; i<_len; i++) {
    if (_buffer[i] == ';' || _buffer[i] == ' ' || _buffer[i] == '\n' || _buffer[i] == '\t') {
      DEBUG_RETURN("%i", i);
      return i;
    }
  }
  DEBUG_RETURN("%i", -1);
  return -1;
}

bool Cmd::ready() {

  DEBUG_IN(Cmd, "ready");
  
  bool r = findNext() >= 0;
  
  DEBUG_RETURN("%b", r);
  return r;

}

void Cmd::read(char *s, int l) {

  DEBUG_IN(Cmd, "read");
  
  int i = findNext();
  if (i >= 0) {
  
    // copy the cmd out
    memmove(s, _buffer, i);
    s[i] = 0;
    
    // fix the length
    _len -= i + 1;
    
    // shuffle the buffer down
    memmove(_buffer, _buffer + i + 1, _len);
    _buffer[_len] = 0;
    
  }

}
