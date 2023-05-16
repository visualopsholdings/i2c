/*
  logo.cpp
    
  Author: Paul Hamilton (paul@visualops.com)
  Date: 5-May-2023
  
  This work is licensed under the Creative Commons Attribution 4.0 International License. 
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
  send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

  https://github.com/visualopsholdings/i2c
*/

#include "logo.hpp"

#ifdef LOGO_DEBUG
#include "test/debug.hpp"
void Logo::outstate() const {
//  cout << " _inword " << _inword << endl;
//  cout << " _pc " << _pc << endl;
  cout << endl;
}
#else
#include "nodebug.hpp"
#endif

#include <stdlib.h>
#include <ctype.h>

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

// all the core words are here. To make it smaller you can comment them
// in and out here.
LogoBuiltinWord Logo::core[] = {
  { "ERR", LogoWords::err, LogoWords::errArity },
#ifdef HAS_VARIABLES
  { "MAKE", LogoWords::make, LogoWords::makeArity },
#endif
#ifdef HAS_FOREVER
  { "FOREVER", LogoWords::forever, LogoWords::foreverArity },
  { "REPEAT", LogoWords::repeat, LogoWords::repeatArity },
#endif
#ifdef HAS_IFELSE
  { "IFELSE", LogoWords::ifelse, LogoWords::ifelseArity },
#endif
  { "=", LogoWords::eq, LogoWords::eqArity },
};

Logo::Logo(LogoBuiltinWord *builtins, short size, LogoBuiltinWord *core) : 
  _inword(false), _defining(-1), _defininglen(-1),
  _jump(-1), _nextcode(0), 
  _builtins(builtins), 
  _core(core), 
  _nextstring(0), _wordcount(0), 
  _pc(0), _tos(0) {
  
#ifdef HAS_VARIABLES
  _varcount = 0;
#endif

#ifdef HAS_SENTENCES
  _sentencecount = 0;
#endif

  _builtincount = size / sizeof(LogoBuiltinWord);
  _corecount = _core ? sizeof(Logo::core) / sizeof(LogoBuiltinWord) : 0;
  _startjcode = _nextjcode = MAX_CODE/2;

  reset();
  
}

short Logo::run() {
  
  DEBUG_IN(Logo, "run");
  
  short err = 0;
  do {
    err = step();
  }
  while (!err);
  
  if (err == LG_STOP) {
    err = 0;
  }
  
  return err;
}

void Logo::reset() {

  DEBUG_IN(Logo, "reset");
  
  for (short i=0; i<MAX_CODE; i++) {
    _code[i]._optype = OPTYPE_NOOP;
  }
  _code[_startjcode-1]._optype = OPTYPE_HALT;
  restart();
  
}

void Logo::restart() {

  DEBUG_IN(Logo, "restart");
  
  _pc = 0;
  _tos = 0;
  for (short i=0; i<MAX_STACK; i++) {
    _stack[i]._optype = OPTYPE_NOOP;
  }
  
}

short Logo::parseint(const LogoInstruction &entry) {

  switch (entry._optype) {
  
  case OPTYPE_STRING:
    getstring(_tmpbuf, sizeof(_tmpbuf), entry._op, entry._opand);
    return atoi(_tmpbuf);
    
  case OPTYPE_NUM:
    return entry._op;
    
  default:
    break;
    
  }
  
  return -1;
  
}

bool Logo::stackempty() {
  return _tos == 0;
}

#ifdef HAS_VARIABLES
short Logo::getvarfromref(const LogoInstruction &entry) {

  getstring(_tmpbuf, sizeof(_tmpbuf), entry._op, entry._opand);
  return findvariable(_tmpbuf);

}
#endif

short Logo::popint() {

  DEBUG_IN(Logo, "popint");
  
  if (!pop()) {
    error(LG_STACK_OVERFLOW);
    return 0;
  }
  short i = parseint(_stack[_tos]);
  if (i >= 0) {
    return i;
  }
#ifdef HAS_VARIABLES
  if (_stack[_tos]._optype == OPTYPE_REF) {
    short var = getvarfromref(_stack[_tos]);
    if (var >= 0) {
      i = parseint(_variables[var]._value);
      if (i >= 0) {
        return i;
      }
    }
  }
#endif  
  return 0;
}

void Logo::pushint(short n) {

  DEBUG_IN(Logo, "pushint");
  
  if (_tos >= MAX_STACK) {
    error(LG_STACK_OVERFLOW);
    return;
  }
  
  _stack[_tos]._optype = OPTYPE_NUM;
  _stack[_tos]._op = n;
  _stack[_tos]._opand = 0;
  _tos++;

}

void Logo::pushstring(short n, short len) {

  DEBUG_IN(Logo, "pushstring");
  
  if (_tos >= MAX_STACK) {
    error(LG_STACK_OVERFLOW);
    return;
  }
  
  _stack[_tos]._optype = OPTYPE_STRING;
  _stack[_tos]._op = n;
  _stack[_tos]._opand = len;
  _tos++;

}

void Logo::fail(short err) {
  
  DEBUG_IN_ARGS(Logo, "fail", "%i", err);
  
  _pc = 0;
  _code[0]._optype = OPTYPE_ERR;
  _code[0]._op = err;
  _code[0]._opand = 0;
  
}

#ifdef HAS_FOREVER
void Logo::modifyreturn(short rel, short count) {

  DEBUG_IN_ARGS(Logo, "modifyreturn", "%i%i", rel, _tos);
  
  if (_tos >= MAX_STACK) {
    fail(LG_STACK_OVERFLOW);
    return;
  }

  // push the return adddres
  _stack[_tos]._optype = SOPTYPE_MRETADDR;
  _stack[_tos]._op = rel;
  _stack[_tos]._opand = count;
  _tos++;
  
}
#endif

bool Logo::parsestring(const LogoInstruction &entry, char *s, short len) {

  switch (entry._optype) {
  
  case OPTYPE_STRING:
    getstring(s, len, entry._op, entry._opand);
    return true;
    
  case OPTYPE_NUM:
    snprintf(s, len, "%d", _stack[_tos]._op);
    return true;
    
  default:
    break;
    
  }
  
  return false;
  
}

bool Logo::pop() {

  DEBUG_IN(Logo, "pop");
  
  if (_tos <= 0) {
    return false;
  }
  _tos--;
  return true;
  
}

void Logo::popstring(char *s, short len) {

  DEBUG_IN(Logo, "popstring");
  
  if (!pop()) {
    error(LG_STACK_OVERFLOW);
    return;
  }
  if (!parsestring(_stack[_tos], s, len)) {
#ifdef HAS_VARIABLES
    short var = getvarfromref(_stack[_tos]);
    if (var >= 0) {
      if (!parsestring(_variables[var]._value, s, len))
#endif
        *s = 0;
#ifdef HAS_VARIABLES
    }
#endif  
  }
}

bool Logo::push(const LogoInstruction &entry) {

  DEBUG_IN_ARGS(Logo, "push", "%i%i", entry._optype, entry._op);
  
  if (_tos >= MAX_STACK) {
    return false;
  }
  
  _stack[_tos++] = entry;
  
  return true;
  
}

#ifdef HAS_IFELSE
bool Logo::codeisnum(short rel) {

  DEBUG_IN_ARGS(Logo, "codeisnum", "%i", rel);
  
  bool val;
#ifdef HAS_VARIABLES
  if (_code[_pc+rel]._optype == OPTYPE_REF) {
    short var = getvarfromref(_code[_pc+rel]);
    val = _variables[var]._value._optype == OPTYPE_NUM;
    DEBUG_RETURN(" ref %b", val);
    return val;
  }
#endif 
 
  val = _code[_pc+rel]._optype == OPTYPE_NUM;
  DEBUG_RETURN(" num %b", val);
  return val;
  
}

short Logo::codetonum(short rel) {

  DEBUG_IN_ARGS(Logo, "push", "%i", rel);
  
  short val;
#ifdef HAS_VARIABLES
  if (_code[_pc+rel]._optype == OPTYPE_REF) {
    short var = getvarfromref(_code[_pc+rel]);
    if (_variables[var]._value._optype != OPTYPE_NUM) {
      error(LG_NOT_NUM);
      return 0;
    }
    val = _variables[var]._value._op;
    DEBUG_RETURN(" ref %i", val);
    return val;
  }
#endif
  
  if (_code[_pc+rel]._optype != OPTYPE_NUM) {
    error(LG_NOT_NUM);
    return 0;
  }

  val = _code[_pc+rel]._op;
  DEBUG_RETURN(" num %i", val);
  return val;

}

bool Logo::codeisstring(short rel) {

  return _code[_pc+rel]._optype == OPTYPE_STRING;

}

void Logo::codetostring(short rel, short *s, short *len) {

  if (!codeisstring(rel)) {
    error(LG_NOT_STRING);
    return;
  }
  *s = _code[_pc+rel]._op;
  *len = _code[_pc+rel]._opand;
   
}

void Logo::jumpskip(short rel) {

  if (_tos >= MAX_STACK) {
    error(LG_STACK_OVERFLOW);
    return;
  }
  
  jump(rel);
  
  _stack[_tos]._optype = SOPTYPE_SKIP;
  _stack[_tos]._op = 0;
  _tos++;
  
}

void Logo::jump(short rel) {

  _pc += rel - 1;
  
}

void Logo::condreturn(short rel) {

  DEBUG_IN_ARGS(Logo, "condreturn", "%i", rel);
  
  if (_tos >= MAX_STACK) {
    error(LG_STACK_OVERFLOW);
    return;
  }
  
  // only patch for words.
  if (_code[_pc+1]._optype != OPTYPE_WORD) {
    error(LG_NOT_NUM);
    return;
  }
  
  // push the return adddres
  _stack[_tos]._optype = SOPTYPE_CONDRET;
  _stack[_tos]._op = _pc + rel;
  _stack[_tos]._opand = 0;
  _tos++;
  
}
#endif // HAS_IFELSE

bool Logo::call(const LogoWord &word) {

  DEBUG_IN(Logo, "call");
  
  if (_tos >= MAX_STACK) {
    return false;
  }
  
  // push the return adddres
  _stack[_tos]._optype = SOPTYPE_RETADDR;
  _stack[_tos]._op = _pc + 1;
  _stack[_tos]._opand = 0;
  _tos++;
  
  // and go.
  _pc = word._jump - 1;

  return true;
  
}

short Logo::step() {

//  DEBUG_IN(Logo, "step");
  
  short err = doarity();
  if (err) {
    return err;
  }
  
  // make sure stack ops don't make it onto here.
  if (_code[_pc]._optype >= SOP_START) {
    return LG_UNHANDLED_OP_TYPE;
  }
  switch (_code[_pc]._optype) {
  
  case OPTYPE_HALT:
    return LG_STOP;
    
  case OPTYPE_RETURN:
    return doreturn();
    
  case OPTYPE_NOOP:
    break;
    
  case OPTYPE_BUILTIN:
    err = dobuiltin();
    break;
    
  case OPTYPE_STRING:
  case OPTYPE_NUM:
    // push anything we find onto the stack.
    if (!push(_code[_pc])) {
      err = LG_STACK_OVERFLOW;
    }
    break;
    
#ifdef HAS_VARIABLES
  case OPTYPE_REF:
    {
      short var = getvarfromref(_code[_pc]);
      if (!push(_variables[var]._value)) {
        err = LG_STACK_OVERFLOW;
      }
    }
    break;
#endif
    
  case OPTYPE_WORD:
    if (!call(_words[_code[_pc]._op])) {
      err = LG_STACK_OVERFLOW;
    }
    break;
    
  case OPTYPE_ERR:
    err = _code[_pc]._op;
    break;
    
  default:
    err = LG_UNHANDLED_OP_TYPE;
  }

  _pc++;
  
  return err;
  
}

short Logo::doarity() {

  DEBUG_IN(Logo, "doarity");

  short ar = _tos-1;
  while (ar >= 0 && _stack[ar]._optype != SOPTYPE_RETADDR && _stack[ar]._optype != SOPTYPE_ARITY) {
    ar--;
  }
  if (ar < 0) {
    DEBUG_RETURN(" no arity", 0);
    return 0;
  }
  
  if (_stack[ar]._optype == SOPTYPE_RETADDR) {
    DEBUG_RETURN(" found return address before arity", 0);
    return 0;
  }
  
  if (_stack[ar]._opand > 0) {
    _stack[ar]._opand--;
    DEBUG_RETURN(" going again", 0);
    return 0;
  }

  DEBUG_OUT("finished", 0);

  short pc = _stack[ar]._op;
  if (_code[pc]._optype != OPTYPE_BUILTIN) {
    DEBUG_RETURN(" not a builtin", 0);
    return LG_NOT_BUILTIN;
  }

  memmove(_stack + ar, _stack + ar + 1, (_tos - ar + 1) * sizeof(LogoInstruction));
  _tos--;

  DEBUG_OUT("calling builtin %i", _code[pc]._op);
  getbuiltin(_code[pc])->_code(*this);

  return 0;
  
}

short Logo::dobuiltin() {

  DEBUG_IN(Logo, "dobuiltin");

  short err = 0;
  
  // handle arity by pushing arguments onto the stack
  
  short call = _pc;
  const LogoBuiltinWord *builtin = getbuiltin(_code[call]);
  short arity = builtin->_arity;
  
  if (arity == 0) {
    builtin->_code(*this);
    return 0;
  }
  
  if (_tos >= MAX_STACK) {
    return LG_STACK_OVERFLOW;
  }
  
  // push the arity onto the stack
  _stack[_tos]._optype = SOPTYPE_ARITY;
  _stack[_tos]._op = call;
  _stack[_tos]._opand = arity;
  _tos++;

  return err;
  
}

short Logo::doreturn() {

  DEBUG_IN(Logo, "doreturn");

  // handle returning
  
  if (!pop()) {
    return LG_STACK_OVERFLOW;
  }

  
  // find the return address on the stack
  short ret = _tos;
  while (ret > 0 && _stack[ret]._optype != SOPTYPE_RETADDR) {
//    dump(3, _stack[ret]); cout << endl;
    ret--;
  }

//   dumpstack(false);
//   dump(3, _stack[_tos]); cout << endl;
//   dump(3, _stack[ret]); cout << endl;
  
  if (ret > 0) {
  
#ifdef HAS_IFELSE
    if (_stack[ret-1]._optype == SOPTYPE_CONDRET) {
  
      if (_stack[ret+1]._optype != OPTYPE_NUM) {
        DEBUG_RETURN(" no num", 0);
        return LG_NOT_NUM;
      }
    
      if (_stack[ret+1]._op) {
        // condition was true
        DEBUG_OUT("condition true", 0);
        _pc = _stack[ret-1]._op;
        _tos = ret;
        _stack[_tos-1]._optype = SOPTYPE_SKIP;
        _stack[_tos-1]._op = 0;
      }
      else {
        // condition is false
        DEBUG_OUT("conditional false", 0);
        _pc = _stack[ret-1]._op + 1;
        _tos = ret-1;
      }
      return 0;
    }
  
    if (_stack[ret-1]._optype == SOPTYPE_SKIP) {
  
      DEBUG_OUT("skipping", 0);
      
       _pc = _stack[ret]._op + 1;

     // shuffle the stack down to remove out skip but leave whatever
      // is there after the return
      memmove(_stack + ret - 1, _stack + ret + 1, (_tos - ret + 1) * sizeof(LogoInstruction));
      _tos--;

      return 0;
    }
#endif // HAS_IFELSE
    
#ifdef HAS_FOREVER
    if (_stack[ret-1]._optype == SOPTYPE_MRETADDR) {

      if (_stack[ret-1]._opand == -1) {
        DEBUG_OUT("forever modify return by %i", _stack[ret-1]._op);
        _pc = _stack[ret]._op + _stack[ret-1]._op;
      }
      else if (_stack[ret-1]._opand > 1) {
        _stack[ret-1]._opand--;
        DEBUG_OUT("modify return by %i", _stack[ret-1]._opand);
        _pc = _stack[ret]._op + _stack[ret-1]._op;
      }
      else {
        DEBUG_OUT("finished mod return", 0);
        _pc = _stack[ret]._op;
        _tos--;
      }
      
      return 0;
    }
#endif
  }
  
  // here next
  _pc = _stack[ret]._op;

  // shuffle the stack down so that words can push data
  memmove(_stack + ret, _stack + ret + 1, (_tos - ret) * sizeof(LogoInstruction));
 
  return 0;
    
}

void Logo::addop(short *next, short type, short op, short opand) {

  DEBUG_IN_ARGS(Logo, "addop", "%i%i%i", type, op, opand);

  _code[*next]._optype = type;
  _code[*next]._op = op;
  _code[*next]._opand = opand;
  (*next)++;
  
}

void Logo::compileword(short *next, const char *word, short op) {

  DEBUG_IN_ARGS(Logo, "compileword", "%s%i", word, op);
  
  if (*next >= MAX_CODE) {
  
    // if we run out code, put the error as the FIRST instruction
    // and reset the pc so that we simply fill it up again this
    // will allow people to see what code overflowed.
    *next = 0;
    addop(next, OPTYPE_ERR, LG_OUT_OF_CODE);
    DEBUG_RETURN(" err ", 0);
    return;
  }
  
  short index = -1;
  short category = 0;
  for (short i=0; index < 0 && i<_builtincount; i++) {
    if (strcmp(_builtins[i]._name, word) == 0) {
      index = i;
    }
  }
  for (short i=0; index < 0 && i<_corecount; i++) {
    if (strcmp(_core[i]._name, word) == 0) {
      index = i;
      category = 1;
    }
  }

  if (index >= 0) {
    addop(next, OPTYPE_BUILTIN, index, category);
    DEBUG_RETURN(" builtin ", 0);
    return;
  }

  index = findword(word);
  if (index >= 0) {
    addop(next, OPTYPE_WORD, index);
    DEBUG_RETURN(" word ", 0);
    return;
  }

  short len = strlen(word);
  if (len > 0) {
  
    if (word[0] == '!') {
      addop(next, OPTYPE_ERR, op);
      DEBUG_RETURN(" err ", 0);
      return;
    }
    
    parseword(&_code[*next], word, len);
    (*next)++;
  }

}

void Logo::parseword(LogoInstruction *entry, const char *word, short len) {

  DEBUG_IN_ARGS(Logo, "parseword", "%s%i", word, len);
  
#ifdef HAS_VARIABLES
  if (word[0] == ':') {
    entry->_optype = OPTYPE_REF;
    short len = strlen(word+1);
    short str = addstring(word+1, len);
    if (str < 0) {
      entry->_optype = OPTYPE_ERR;
      entry->_op = LG_OUT_OF_STRINGS;
      entry->_opand = 0;
      DEBUG_RETURN(" mo nore strings %i", 0);
      return;
    }
    entry->_op = str;
    entry->_opand = len;
    return;
  }
#endif
  
  if (isnum(word, len)) {
    entry->_optype = OPTYPE_NUM;
    entry->_op = atoi(word);
    entry->_opand = 0;
    return;
  }

  const char *start = word;
  if (word[0] == '\"') {
    start++;
    len--;
  }
  
  entry->_optype = OPTYPE_STRING;
  
  short op = addstring(start, len);
  if (op < 0) {
    entry->_optype = OPTYPE_ERR;
    op = LG_OUT_OF_STRINGS;
    len = 0;
  }
  
  entry->_op = op;
  entry->_opand = len;
  
}

bool Logo::isnum(const char *word, short len) {

  if (len == 0) {
    return false;
  }
  for (short i=0; i<len; i++) {
    if (!isdigit(word[i])) {
      return false;
    }
  }
  
  return true;
  
}

const LogoBuiltinWord *Logo::getbuiltin(const LogoInstruction &entry) const {

  if (entry._opand == 0) {
    return &_builtins[entry._op];
  }
  else if (entry._opand == 1) {
    return &_core[entry._op];
  }
  else {
    return &Logo::core[0];
  }
  
}

short Logo::addstring(const char *s, short len) {

  DEBUG_IN_ARGS(Logo, "addstring", "%s%i", s, len);
  
  if ((_nextstring + len) > STRING_POOL_SIZE) {
    DEBUG_RETURN(" %i", -1);
    return -1;
  }
  
  short cur = _nextstring;
  memmove(_strings + cur, s, len);
  _nextstring += len;
  
  DEBUG_RETURN(" %i", cur);
  return cur;
}

void Logo::getstring(char *buf, short buflen, short str, short len) const {

  short l = min(buflen, len);
  memmove(buf, _strings + str, l);
  buf[l] = 0;
  
}

#ifdef HAS_VARIABLES
short Logo::findvariable(const char *word) const {

  DEBUG_IN_ARGS(Logo, "findvariable", "%s%i", word, _varcount);
  
  for (short i=0; i<_varcount; i++) {
    char *buf = const_cast<char *>(_findwordbuf);
    getstring(buf, sizeof(_findwordbuf), _variables[i]._name, _variables[i]._namelen);
    if (strcmp(_findwordbuf, word) == 0) {
      return i;
    }
  }
  
  return -1;
}
#endif

short Logo::findword(const char *word) const {

  for (short i=0; i<_wordcount; i++) {
    char *buf = const_cast<char *>(_findwordbuf);
    getstring(buf, sizeof(_findwordbuf), _words[i]._name, _words[i]._namelen);
    if (strcmp(_findwordbuf, word) == 0) {
      return i;
    }
  }
  
  return -1;
}

#ifdef HAS_SENTENCES
void Logo::dosentences(char *buf, short len, const char *start) {

  DEBUG_IN(Logo, "dosentences");
  
  // we preprocess all sentences in a line first. We basically treat
  // sentences like lambdas (anonymous words). Each one has a unique name
  // like &N and we define them and then replace sentence in the line
  // with the word name of the sentence.
  //
  //  so
  //
  //    REPEAT 10 [SOMETHING TO DO]
  //
  //  becomes
  //
  //    REPEAT 10 &0
  //
  //  where
  //
  //    &0 is
  //      string SOMETHING
  //      string TO
  //      string DO
  
  while (1) {
  
    const char *end = strstr(start + 1, "]");
    if (!end) {
      error(LG_OUT_OF_CODE);
      return;
    }
  
    memmove(_tmpbuf, start + 1, end - start - 1);
    _tmpbuf[end - start - 1] = 0;
  
    snprintf(_tmpbuf2, sizeof(_tmpbuf2), "&%d", _sentencecount);
    _sentencecount++;

    short wlen = strlen(_tmpbuf2);
    short word = addstring(_tmpbuf2, wlen);
    if (word < 0) {
      error(LG_OUT_OF_STRINGS);
      return;
    }
    
    // remember where we are before compiling.
    short jump = _nextjcode;
    
    // add all the words etc.
    compilewords(_tmpbuf, strlen(_tmpbuf), false);
    
    // and finish off the word
    finishword(word, wlen, jump);
  
    short endlen = strlen(end);
    
    // replace sentence in the original string.
    memmove((char *)start, _tmpbuf2, wlen);
    memmove((char *)(start + wlen), end + 1, endlen);
    
    if (endlen > 1) {
      start = strstr(buf, "[");
      if (!start) {
        break;
      }
      end = strstr(start + 1, "]");
      if (!end) {
        error(LG_OUT_OF_CODE);
        return;
      }
    }
    else {
      break;
   }
  }
}
#endif

void Logo::compile(const char *code, short len) {

  DEBUG_IN_ARGS(Logo, "compile", "%s%i", code, len);
  
  short nextline = 0;
  while (nextline >= 0) {
    nextline = scanfor(_linebuf, sizeof(_linebuf), code, len, nextline, true);
    
    if (nextline == -2) {
      error(LG_LINE_TOO_LONG);
      break;
    }
    if (!_linebuf[0]) {
      break;
    }
    
#ifdef HAS_SENTENCES
    // build and replace sentences in the line.
    const char *s = strstr(_linebuf, "[");
    if (s) {
      dosentences(_linebuf, strlen(_linebuf), s);
    }
#endif
    
    compilewords( _linebuf, strlen(_linebuf), true);
  }

}

void Logo::compilewords(const char *buf, short len, bool define) {

  DEBUG_IN_ARGS(Logo, "compilewords", "%s%i%b", buf, len, define);
  
  short nextword = 0;
  while (nextword >= 0) {
    nextword = scanfor(_wordbuf, sizeof(_wordbuf), buf, len, nextword, false);
    if (nextword == -2) {
      error(LG_WORD_TOO_LONG);
      return;
    }
    if (!_wordbuf[0]) {
      break;
    }
    if (define) {
      if (!dodefine(_wordbuf)) {
        if (_nextcode >= _startjcode) {
          error(LG_OUT_OF_CODE);
          return;
        }
        compileword(&_nextcode, _wordbuf, 0);
      }
    }
    else {
      if (_nextjcode >= MAX_CODE) {
        error(LG_OUT_OF_CODE);
        return;
      }
      compileword(&_nextjcode, _wordbuf, 0);
    }
  }
  
}

bool Logo::istoken(char c, bool newline) {

//  DEBUG_IN_ARGS(Logo, "istoken", "%c%b", c, newline);
  
  return newline ? c == ';' || c == '\n' : c == ' ';
  
}

short Logo::scanfor(char *s, short size, const char *str, short len, short start, bool newline) {

  DEBUG_IN_ARGS(Logo, "scanfor", "%i%s%i%i%b", size, str, len, start, newline);
  
  // skip ws
  while (start < len && (str[start] == ' ' || str[start] == '\t')) {
    start++;
  }
  
  short end = start;
  
  // find end of line
  for (short i=0; end < len && str[end] && !istoken(str[end], newline); i++) {
    end++;
  }
  
  if ((end - start) > size) {
    DEBUG_RETURN(" too long ", -2);
    return -2;
  }
  
  if (end < len) {
    if (istoken(str[end], newline)) {
      end++;
      strncpy(s, str + start, end-start-1);
      s[end-start-1] = 0;
    }
    else {
      DEBUG_RETURN(" token not found %i", -1);
      return -1;
    }
  }
  else {
    strncpy(s, str + start, end-start);
    s[end-start] = 0;
    DEBUG_RETURN(" end of string %i", -1);
    return -1;
  }
  
  DEBUG_RETURN(" %i", end);
  return end;

}

short Logo::geterr() {

  DEBUG_IN(Logo, "geterr");
  
  // walk through the code looking for errors.
  for (short i=0; i<MAX_CODE; i++) {
    if (_code[i]._optype == OPTYPE_ERR) {
      DEBUG_RETURN(" code %i", i);
      return _code[i]._op;
    }
  }

  // couldn't find any
  DEBUG_RETURN(" %i", 0);
  return 0;
  
}

void Logo::finishword(short word, short wordlen, short jump) {

  DEBUG_IN_ARGS(Logo, "finishword", "%i%i%i", word, wordlen, jump);
  
  addop(&_nextjcode, OPTYPE_RETURN);
      
  if (_wordcount >= MAX_WORDS) {
    error(LG_TOO_MANY_WORDS);
    return;
  }

  _words[_wordcount]._name = word;
  _words[_wordcount]._namelen = wordlen;
  _words[_wordcount]._jump = jump;
  _wordcount++;

}

void Logo::error(short error) {
  compileword(&_nextcode, "!", error);
}

bool Logo::dodefine(const char *word) {

  DEBUG_IN_ARGS(Logo, "dodefine", "%s", word);
  
  if (*word == 0) {
    DEBUG_RETURN(" empty word %b", false);
    return false;
  }
  
  if (strcmp(word, "TO") == 0) {
    _inword = true;
    DEBUG_RETURN(" in define %b", true);
    return true;
  }

  if (word[0] == '[') {
    // all sentences should have been replaced!
    error(LG_WORD_NOT_FOUND);
    return false;
  }

  if (!_inword) {
    DEBUG_RETURN(" %b", false);
    return false;
  }
  
  // the word we are defining
  if (_inword) {
  
    if (_defining < 0) {
  
      _defininglen = strlen(word);
      _defining = addstring(word, _defininglen);
      if (_defining < 0) {
        error(LG_OUT_OF_STRINGS);
        return true;
      }
    
      DEBUG_RETURN(" word started %b", true);
      return true;
    
    }
    else if (strcmp(word, "END") == 0) {
      // the END token
      _inword = false;
      finishword(_defining, _defininglen, _jump);
      _defining = -1;
      _defininglen = -1;
      _jump = -1;
      DEBUG_RETURN(" finished word %b", true);
      return true;
    }
    
  }
    
  // first word we define in our word, remember where we are.
  if (_jump < 0) {
    _jump = _nextjcode;
  }
  
  if (_nextjcode >= MAX_CODE) {
    error(LG_OUT_OF_CODE);
    return true;
  }
  
  compileword(&_nextjcode, word, 0);
  
  DEBUG_RETURN(" %b", true);
  return true;

}

#ifdef HAS_VARIABLES
void Logo::defineintvar(char *s, short i) {

  DEBUG_IN_ARGS(Logo, "defineintvar", "%s%i", s, i);
  
  short var = findvariable(s);
  if (var < 0) {
    short len = strlen(s);
    short str = addstring(s, len);
    if (str < 0) {
      error(LG_OUT_OF_STRINGS);
      return;
    }
    if (_varcount >= MAX_VARS) {
      error(LG_TOO_MANY_VARS);
      return;
    }
    var = _varcount;
    _variables[_varcount]._name = str;
    _variables[_varcount]._namelen = len;
    _varcount++;
  }
  
  _variables[var]._value._optype = OPTYPE_NUM;
  _variables[var]._value._op = i;
  _variables[var]._value._opand = 0;
    
}
#endif

#ifdef LOGO_DEBUG
void Logo::entab(short indent) const {
  for (short i=0; i<indent; i++) {
    cout << "\t";
  }
}

void Logo::printword(const LogoWord &word) const {
  char name[32];
  getstring(name, sizeof(name), word._name, word._namelen);
  cout << name;
}

void Logo::dump(short indent, const LogoInstruction &entry) const {

  entab(indent);
  char str[128];
  switch (entry._optype) {
    case OPTYPE_NOOP:
      cout << "noop";
      break;
    case OPTYPE_RETURN:
      cout << "ret";
      break;
    case OPTYPE_HALT:
      cout << "halt";
      break;
    case OPTYPE_BUILTIN:
      cout << "builtin " << getbuiltin(entry)->_name;
      break;
    case OPTYPE_WORD:
      printword(_words[entry._op]);
      break;
    case OPTYPE_STRING:
      getstring(str, sizeof(str), entry._op, entry._opand);
      cout << "string " << str;
      break;
    case OPTYPE_NUM:
      cout << "num " << entry._op;
      break;
#ifdef HAS_VARIABLES
    case OPTYPE_REF:
      getstring(str, sizeof(str), entry._op, entry._opand);
      cout << "ref " << str;
      break;
#endif
    case OPTYPE_ERR:
      cout << "err " << entry._op;
      break;
    case SOPTYPE_ARITY:
      cout << "(stack) arity pc " << entry._op << " to go " << entry._opand;
      break;
#ifdef HAS_FOREVER
    case SOPTYPE_MRETADDR:
      cout << "(stack) mod ret by " << entry._op << " " << entry._opand << " times";
      break;
#endif
    case SOPTYPE_RETADDR:
      cout << "(stack) ret to " << entry._op;
      break;
    case SOPTYPE_CONDRET:
      cout << "(stack) cond ret to " << entry._op;
      break;
    case SOPTYPE_SKIP:
      cout << "(stack) skip ";
      break;
    default:
      cout << "unknown optype " << entry._optype;
  }
}

void Logo::markword(short jump) const {

  for (short i=0; i<_wordcount; i++) {
    char name[WORD_LEN];
    getstring(name, sizeof(name), _words[i]._name, _words[i]._namelen);
    short word = findword(name);
    if (word >= 0) {
      if (_words[word]._jump == jump) {
         cout << name;
      }
    }
  }
  
}

#ifdef HAS_VARIABLES
void Logo::printvar(const LogoVar &var) const {

  char name[32];
  getstring(name, sizeof(name), var._name, var._namelen);
  cout << name;
  
  dump(2, var._value);
  
}
#endif

void Logo::dump(const char *msg, bool all) const {
  cout << msg << endl;
  dump(all);
}

void Logo::dump(bool all) const {

  cout << "------" << endl;
  dumpwords();
  dumpcode(all);
  dumpstack(all);
#ifdef HAS_VARIABLES
  dumpvars();
#endif  
}

#ifdef HAS_VARIABLES
void Logo::dumpvars() const {

  cout << "vars: " << endl;
  
  if (!_varcount) {
    entab(1);
    cout << "empty" << endl;
  }
  
  for (short i=0; i<_varcount; i++) {
    entab(1);
    printvar(_variables[i]);
    cout << endl;
  }
  
}
#endif

void Logo::dumpwords() const {

  cout << "words: " << endl;
  
  if (!_wordcount) {
    entab(1);
    cout << "empty" << endl;
  }
  
  for (short i=0; i<_wordcount; i++) {
    entab(1);
    printword(_words[i]);
    cout << endl;
  }
  
}

void Logo::mark(short i, short mark, const char *name) const {
  if (i == mark) {
    entab(2);
    cout << "(" << name << ")";
  }
}

void Logo::dumpcode(bool all) const {

  cout << "code: pc (" << _pc << ")" << endl;
  
  if (!_nextcode) {
    entab(1);
    cout << "empty" << endl;
  }
  
  for (short i=0; i<(all ? MAX_CODE : _nextcode); i++) {
    markword(i);
    dump(1, _code[i]);
    mark(i,  _pc, "pc");
    if (all) {
      mark(i, _startjcode, "startjcode");
      mark(i, _nextjcode, "nextjcode");
    }
    cout << endl;
  }
  
  if (!all) {
    if (_pc > _nextcode && _pc < _startjcode) {
      for (short i=_nextcode; i<=_pc; i++) {
        dump(1, _code[i]);
        mark(i,  _pc, "pc");
        cout << endl;
      }
    }
    cout << "\t..." << endl;
    for (short i=_startjcode; i<_nextjcode; i++) {
      markword(i);
      dump(1, _code[i]);
      mark(i,  _pc, "pc");
      mark(i, _startjcode, "startjcode");
      cout << endl;
    }
  }
}

void Logo::dumpstack(bool all) const {

  cout << "stack: (" << _tos << ")" << endl;
  
  for (short i=0; i<(all ? MAX_STACK : _tos); i++) {
    dump(1, _stack[i]);
    mark(i,  _tos, "tos");
    cout << endl;
  }
  
}

short Logo::stepdump(short n, bool all) {
  dump(false);
  for (short i=0; i<n; i++) {
    cout << "step " << i << " -----------" << endl;
    short ret = step();
    if (ret) {
      return ret;
    }
    dump(all);
  }
  return 0;
}

#endif

short LogoWords::errArity = 0;

void LogoWords::err(Logo &logo) {

  logo.fail(LG_STOP);
  
}

#ifdef HAS_VARIABLES
short LogoWords::makeArity = 2;

void LogoWords::make(Logo &logo) {

  short n = logo.popint();
  char s[WORD_LEN];
  logo.popstring(s, sizeof(s));
  logo.defineintvar(s, n);
  
}
#endif

#ifdef HAS_FOREVER
short LogoWords::foreverArity = 0;

void LogoWords::forever(Logo &logo) {

  DEBUG_DUMP_MSG("forever", false);

  // change the next return address to be 1 minus what we have 
  logo.modifyreturn(-1, -1);

}

short LogoWords::repeatArity = 1;

void LogoWords::repeat(Logo &logo) {

  DEBUG_DUMP_MSG("repeat", false);

  short n = logo.popint();

  // change the next return address to be 1 minus what we have 
  logo.modifyreturn(-1, n);

}
#endif

short LogoWords::eqArity = 1;

void LogoWords::eq(Logo &logo) {

  logo.pushint(logo.popint() == logo.popint());
  
}

#ifdef HAS_IFELSE
bool LogoWords::pushliterals(Logo &logo, short rel) {

  if (logo.codeisnum(rel)) {
    logo.pushint(logo.codetonum(rel));
    return true;
  }
  else if (logo.codeisstring(rel)) {
    short s, l;
    logo.codetostring(rel, &s, &l);
    if (logo.geterr()) {
      return false;
    }
    logo.pushstring(s, l);
    return true;
  }

  return false;
  
}

short LogoWords::ifelseArity = 0;

void LogoWords::ifelse(Logo &logo) {

//  logo.dump("ifelse", false);
  
  if (logo.codeisnum(1)) {
  
    // handle numeric literal argument.
    if (logo.codetonum(1)) {  
    
      // true
      if (pushliterals(logo, 2)) {
        logo.jump(4);
      }
      else {
        logo.jumpskip(2);
      }
    }
    else {
    
      // false
      if (pushliterals(logo, 3)) {
        logo.jump(4);
      }
      else {
        logo.jump(3);
      }
      
    }
    
    return;
  }
  
  // this is a very special trick to allow us to manipulate the PC
  // based on the value of the stack.
  //
  // our code looks like:
	//  builtin IFELSE		(pc)
	//  TEST
	//  THEN
	//  ELSE
	//
  // we push a "condreturn" onto the stack telling
  // use to go to "2"
  //
  // then when we hit a return for the TEST
  //
  // our stack looks like:
  //
  //  cond return 2 (relative to the )
  //  ret 3 (or whatever the address of THEN is)
  //  num 1
  //
  //  when we return we look back and find the cond return and 
  //  then pop the top of the stack and test if it and
  // if true we go to THEN, otherwise we go to ELSE
  //
  //  Then we clean the stack up
  
//  logo.dump(false);
  logo.condreturn(2);
//  logo.dumpstack(false);
  
}
#endif
