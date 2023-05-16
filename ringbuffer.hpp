/*
  ringbuffer.hpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 5-May-2023

  A safe ring buffer 
  
  A ring buffer that can be written and read to without a chance that the 
  memory allocator will run, can be written and read to at the same time by 
  multiple threads or processors and written by an interrupt and read from
  a main loop.
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c
*/

#ifndef H_ringbuffer
#define H_ringbuffer

//#define RINGBUF_DEBUG

#ifndef RINGBUF_SIZE_MINUS_1
#define RINGBUF_SIZE_MINUS_1  256
#endif

class RingBuffer {

public:
  RingBuffer();
  ~RingBuffer();
  
  // return the length remaining in the buffer
  int length() const;
  
  // write and read single chars and strings
  void write(unsigned char c);
  void write(const char *s);

  unsigned char read();
  void read(char *s, int size); // char s[3]; read(s, sizeof(s));
  int readAppend(char *s, int size, int start); // char s[3]; s[0] = 0; readAppend(s, sizeof(s));
  
#ifdef RINGBUF_DEBUG
  void outstate() const;
  void checkWrite();
#endif

private:
  unsigned char _buffer[RINGBUF_SIZE_MINUS_1+1];
  int _writep;
  int _readp;
  
};

#endif // H_ringbuffer
