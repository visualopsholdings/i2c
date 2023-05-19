/*
  nulltimeprovider.hpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 19-May-2023
    
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c
*/

// include this right after gCmds

class NullTimeProvider: public LogoTimeProvider {

public:
  
  // LogoTimeProvider
  virtual void schedule(short ms) {
    strstream str;
    str << "WAIT " << ms;
    gCmds.push_back(str.str());
  #ifdef PRINT_RESULT
    cout << gCmds.back() << endl;
  #endif
  }
  virtual bool next() { return true; }
  
};
