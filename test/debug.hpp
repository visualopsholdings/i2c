/*
  debug.hpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 6-May-2023
  
  Simplest debugger
  
  Just declare one at the top of each function.
  
  void Class::function1(const char *arg1, int arg2) {

    DEBUG_IN("function1", "%s%i", arg1, arg2);
  
    ... Some code ...
  
  }
  
  bool Class::function2(const char *arg1, int arg2) {

    DEBUG_IN("function2", "%s%i", arg1, arg2);
  
    ... Some code ...

    ... end places where you return something you want to show
    DEBUG_RETURN("%b", true);
    return true;
  
  }
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c
*/

#ifndef H_debug
#define H_debug

#include <iostream>
#include <sstream>
using namespace std;

template <class T>
class Debug {
public:
  Debug(const T &mainclass, const char *classname, const char *method, const char *fmt, ...) : _mainclass(mainclass), _classname(classname), _method(method) {
    cout << "-> " << _classname << "::" << _method << "()";
    if (fmt) {
      va_list args;
      va_start(args, fmt);
      outargs(cout, fmt, args);
      _mainclass.outstate();
      va_end(args);
      return;
    }
    _mainclass.outstate();
  }
  ~Debug() {
    cout << "<- " << _classname << "::" << _method << "()";
    if (!_return.empty()) {
      cout << _return;
    }
    _mainclass.outstate();
  }
  
  void returning(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    stringstream str;
    outargs(str, fmt, args);
    va_end(args);
    _return = str.str();
  }
  
  void output(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    outargs(cout, fmt, args);
    cout << endl;
    va_end(args);
  }
  
private:
  const T &_mainclass;
  const char *_classname;
  const char *_method;
  string _return;
  
  void outargs(ostream &stream, const char *fmt, va_list &args) {
    for (const char* p = fmt; *p != '\0'; ++p) {
      switch(*p) {
      case '%':
        switch(*++p) {
        case 'i': stream << ", " << va_arg(args, int); continue;
        case 'f': stream << ", " << va_arg(args, double); continue;
        case 's': stream << ", '" << va_arg(args, const char*) << "'"; continue;
        case 'c': stream << ", " << static_cast<char>(va_arg(args, int)); continue;
        case 'b': stream << ", " << (static_cast<bool>(va_arg(args, int)) ? "true" : "false"); continue;
        default: stream << *p;
        }
        continue;
      default: stream << *p;
      }
    }
  }
  
};
#define DEBUG_IN_ARGS(__class__, __name__, __fmt__, ...)    Debug<__class__> debug(*this, #__class__, __name__, __fmt__, __VA_ARGS__)
#define DEBUG_IN(__class__, __name__)                       Debug<__class__> debug(*this, #__class__, __name__, 0)
#define DEBUG_RETURN(__fmt__, ...)                          debug.returning(__fmt__, __VA_ARGS__)
#define DEBUG_OUT(__fmt__, ...)                             debug.output(__fmt__, __VA_ARGS__)

#endif // H_debug
