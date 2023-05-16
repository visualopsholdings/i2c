/*
  logo.hpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 5-May-2023
  
  Tiniest Logo Intepreter
  
  Keep feeding it commands and it will execute them.
  
  Use it like this:
  
  void ledOn() {
   ... turn a LED on
  }

  void ledOff() {
    ... turn an LED off
  }

  void wait(Logo &logo) {
    int time = logo.popint();
    ... wait for this time
  }
  
  LogoBuiltinWord builtins[] = {
    { "ON", &ledOn },
    { "OFF", &ledOff },
    { "WAIT", &wait, 1 },
  };
  Logo logo(builtins, sizeof(builtins), &Logo::core);
  logo.compile("TO GO; FOREVER [ON WAIT 100 OFF WAIT 1000]; END;");

  ... then on some trigger
  logo.compile("GO");
  int err = logo.geterr();
  if (err) {
    ... do something with the error number
  }
  
  ... and to run it, call this pretty often
  int err = logo.step();
  if (err && err != LG_STOP) {
    ... do something with the error number
  }
  
  ... or just run it till it ends (the above one doesn't)
  int err = logo.run();
  if (err) {
    ... do something with the error number
  }
  
  ... at any time you can start from the top again with
  logo.restart();
  
  ... or completely reset the machines code
  logo.reset();
  
  If your code is so simple it only uses YOUR builtins, you can make 
  the runtime smaller by NOT including the core words (at the moment
  this will only save about 8 bytes):
  
  Logo logo(builtins, sizeof(builtins));
  
  ....
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c
*/

#ifndef H_logo
#define H_logo

//#define LOGO_DEBUG

// if you don't need variables, save about 1.5k bytes
#define HAS_VARIABLES

// if you don't need repeat or forever, you can save 452 bytes
#define HAS_FOREVER

// if you don't need IFELSE and all it needs you can save around 1.3k bytes
#define HAS_IFELSE

// if you don't need [] sentences and all it needs you can save aroun 628 bytes
#define HAS_SENTENCES

// about 15.2k with all code, 10.2k bare bones with everything off.
#define STRING_POOL_SIZE  256       // these number of bytes
#define LINE_LEN          64        // these number of bytes
#define WORD_LEN          32        // these number of bytes
#define MAX_WORDS         16        // 6 bytes each
#define MAX_CODE          100       // 6 bytes each
#define MAX_STACK         24        // 6 bytes each
#ifdef HAS_VARIABLES
#define MAX_VARS          8        // 10 bytes each
#endif

#include <string.h>
#include <stdio.h>

// success is 0
#define LG_STOP               1
#define LG_UNHANDLED_OP_TYPE  2
#define LG_TOO_MANY_WORDS     3
#define LG_OUT_OF_STRINGS     4
#define LG_LINE_TOO_LONG      5
#define LG_WORD_TOO_LONG      6
#define LG_OUT_OF_CODE        7
#define LG_STACK_OVERFLOW     8
#define LG_WORD_NOT_FOUND     9
#define LG_NO_RETURN_ADDRESS  10
#define LG_TOO_MANY_VARS      11
#define LG_NOT_NUM            12
#define LG_NOT_STRING         13
#define LG_NOT_BUILTIN        14

class Logo;

typedef void (*LogoFp)(Logo &logo);

typedef struct {
  const char *_name;
  LogoFp      _code;
  short         _arity;
} LogoBuiltinWord;

typedef struct {
  short _name;
  short _namelen;
  short _jump;
} LogoWord;

typedef struct {
  short _optype;
  short _op;
  short _opand;
} LogoInstruction;

#ifdef HAS_VARIABLES

#define TYPE_NUM      0
#define TYPE_STRING   1

typedef struct {
  short             _name;
  short             _namelen;
  LogoInstruction _value;
} LogoVar;

#endif

#define OPTYPE_NOOP       0 //
#define OPTYPE_RETURN     1 //
#define OPTYPE_HALT       2 //
#define OPTYPE_BUILTIN    3 // _op = index of builtin, _opand = category 0 = builtin, 1 = core 
#define OPTYPE_ERR        4 // _op = error
#define OPTYPE_WORD       5 // _op = index of word
#define OPTYPE_STRING     6 // _op = index of string, _opand = length of string
#define OPTYPE_NUM        7 // _op = literal number
#ifdef HAS_VARIABLES
#define OPTYPE_REF        8 // _op = index of string with a var in it, _opand = length of string
#endif

// only on the stack
#define SOP_START         100
#define SOPTYPE_ARITY     SOP_START + 1 // _op = the arity of the builtin function
#define SOPTYPE_RETADDR   SOP_START + 2 // _op = the return address. These are just on the stack
#ifdef HAS_FOREVER
#define SOPTYPE_MRETADDR  SOP_START + 3 // _op = the offset to modify by
#endif
#ifdef HAS_IFELSE
#define SOPTYPE_CONDRET   SOP_START + 4 // _op = the return address of if true, otherwise _op + 1
#define SOPTYPE_SKIP      SOP_START + 5 // skip the next instruction if on the stack under a return
#endif
class Logo;

class LogoWords {

public:

  static void err(Logo &logo);
  static short errArity;
  
#ifdef HAS_IFELSE
  static void ifelse(Logo &logo);
  static short ifelseArity;
#endif
  
#ifdef HAS_FOREVER
  static void repeat(Logo &logo);
  static short repeatArity;
  
  static void forever(Logo &logo);
  static short foreverArity;
#endif

  static void eq(Logo &logo);
  static short eqArity;

#ifdef HAS_VARIABLES
  static void make(Logo &logo);
  static short makeArity;
#endif

private:

#ifdef HAS_IFELSE
  static bool pushliterals(Logo &logo, short rel);
#endif

};

class Logo {

public:
  Logo(LogoBuiltinWord *builtins, short size, LogoBuiltinWord *core=0);
  ~Logo() {}
  
  // the compiler.
  void compile(const char *code) {
      compile(code, strlen(code));
  }
  
  // find any errors in the code.
  short geterr();
   
  // main execution
  short step();
  short run();
  void restart(); // run from the top
  void reset(); // reset all the code, everything.
  void fail(short err);
  
  // dealing with the stack
  bool stackempty();
  short popint();
  void pushint(short n);
  void popstring(char *s, short len);  
  void pushstring(short n, short len);  
  bool pop();
#ifdef HAS_VARIABLES
  void defineintvar(char *s, short i);
#endif
  

  // logo words can be self modifying code but be careful!
#ifdef HAS_FOREVER
  void modifyreturn(short rel, short n);
#endif
#ifdef HAS_IFELSE
  bool codeisnum(short rel);
  bool codeisstring(short rel);
  short codetonum(short rel);
  void codetostring(short rel, short *s, short *len);
  void jumpskip(short rel);
  void jump(short rel);
  void condreturn(short rel);
#endif

#ifdef LOGO_DEBUG
  void outstate() const;
  void dump(const char *msg, bool all=true) const;
  void dump(bool all=true) const;
  void dumpwords() const;
  void dumpvars() const;
  void dumpcode(bool all=true) const;
  void dumpstack(bool all=true) const;
  short stepdump(short n, bool all=true);
#endif

  static LogoBuiltinWord core[];

private:
  
  // the state variables for defining new words
  bool _inword;
  short _defining;
  short _defininglen;
  short _jump;
  
  // the pool of all strings
  char _strings[STRING_POOL_SIZE];
  short _nextstring;
  
  // the builtin words.
  LogoBuiltinWord *_builtins;
  short _builtincount;
  
  // the core words if needed.
  LogoBuiltinWord *_core;
  short _corecount;
  
  // the word dictionary;
  LogoWord _words[MAX_WORDS];
  short _wordcount;
  
  // various buffers to hold data
  char _linebuf[LINE_LEN];
  char _wordbuf[WORD_LEN];
  char _findwordbuf[LINE_LEN];
  char _tmpbuf[LINE_LEN];
#ifdef HAS_SENTENCES
  char _tmpbuf2[LINE_LEN];
#endif
   
  // the code
  LogoInstruction _code[MAX_CODE];
  short _pc;
  short _nextcode;
  short _nextjcode; // for allocating new jumps
  short _startjcode; // where we started making the jumps.
  
  // the stack
  LogoInstruction _stack[MAX_STACK];
  short _tos;
  
#ifdef HAS_VARIABLES
  // the variables
  LogoVar _variables[MAX_VARS];
  short _varcount;

  short findvariable(const char *word) const;
  short getvarfromref(const LogoInstruction &entry);
#endif
  
#ifdef HAS_SENTENCES
  // sentences
  short _sentencecount;

  void dosentences(char *buf, short len, const char *start);
#endif
  
  // parser
  bool dodefine(const char *word);
  void error(short error);
  void compile(const char *code, short len);
  void compilewords(const char *buf, short len, bool define);
  short scanfor(char *s, short size, const char *str, short len, short start, bool newline);
  void parseword(LogoInstruction *entry, const char *word, short len);
  bool istoken(char c, bool newline);
  bool isnum(const char *word, short len);
  bool parsestring(const LogoInstruction &entry, char *s, short len);
  void addop(short *next, short type, short op=0, short opand=0);

  // words
  void compileword(short *next, const char *word, short op);
  void finishword(short word, short wordlen, short jump);
  short findword(const char *word) const;
  
  // strings
  short addstring(const char *s, short len);
  void getstring(char *buf, short buflen, short str, short len) const;

  // builtins
  const LogoBuiltinWord *getbuiltin(const LogoInstruction &entry) const;
  
  // the machine
  bool push(const LogoInstruction &entry);
  short parseint(const LogoInstruction &entry);
  short doreturn();
  short dobuiltin();
  short doarity();
  bool call(const LogoWord &word);
  
#ifdef LOGO_DEBUG
  void markword(short jump) const;
  void dump(short ident, const LogoInstruction &entry) const;
  void printword(const LogoWord &word) const;
  void entab(short indent) const;
  void mark(short i, short mark, const char *name) const;
#ifdef HAS_VARIABLES
  void printvar(const LogoVar &var) const;
#endif
#endif // LOGO_DEBUG

};

#ifdef LOGO_DEBUG
#define DEBUG_DUMP(all)                                     logo.dump(all)
#define DEBUG_DUMP_MSG(msg, all)                            logo.dump(msg, all)
#define DEBUG_STEP_DUMP(count, all)                         BOOST_CHECK_EQUAL(logo.stepdump(count, all), 0)
#else
#define DEBUG_DUMP(all)
#define DEBUG_DUMP_MSG(msg, all)
#define DEBUG_STEP_DUMP(count, all)
#endif

#endif // H_logo
