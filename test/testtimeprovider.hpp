/*
  testtimeprovider.hpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 19-May-2023
  
  https://github.com/visualopsholdings/i2c
*/

// just include any time after defining PRINT_RESULT and gCmds

class TestTimeProvider: public LogoTimeProvider {

public:

  unsigned long currentms() {
    return 0;
  }
  void delayms(unsigned long ms) {
  }
  bool testing(short ms) {
    strstream str;
    str << "WAIT " << ms;
    gCmds.push_back(str.str());
  #ifdef PRINT_RESULT
    cout << gCmds.back() << endl;
  #endif
    return true;
  }
  void settime(unsigned long time) {
  }
  
};
