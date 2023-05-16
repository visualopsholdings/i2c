/*
  cmd.hpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 5-May-2023

  A command
  
  Keep feeding it data from the buffer and then it works out
  what the next command is if there is one ready to go
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c
*/

#ifndef H_cmd
#define H_cmd

//#define CMD_DEBUG

class RingBuffer;

#ifndef CMD_SIZE
#define CMD_SIZE  128
#endif

class Cmd {

public:
  Cmd();
  ~Cmd();
  
  void accept(RingBuffer *buffer);
  bool ready();
  void read(char *s, int l); // char s[3]; read(s, sizeof(s));
  
#ifdef CMD_DEBUG
  void outstate() const;
#endif

private:
  char _buffer[CMD_SIZE];
  int _len;
  
  int findNext();
  
};

#endif // H_cmd
