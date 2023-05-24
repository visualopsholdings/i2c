
# Using I2C to control an Arduino

## Context

### The Problem

I2C is an excellent and simple protocol with builtin tools to use from most
SBC's running Linux and other Arduinos.

There are many great libraries in Python, Node (JavaScript), C for sending and receiving 
from and to the I2C bus.

Unfortunately there are some things that complicate getting it to work for most people beyond
just simple binary commands. The reason for this is that in an Arduino, the "Wire" library
uses an interrupt to receive bytes and you can't do very much in an interrupt beyond save 
away what you get and return.

You certainly can't use delay() or send to serial etc.

This is because there isn't really any internal buffering in the i2c library, and why
would there be :-)

This code solves this problem by introducing a mechanism for quickly saving what comes in
within the event handler and then using that usefully in the main loop of the program.

### Naive attempt at a non solution

Here is very simple code to receive a single word of data from I2C and try to do something
useful with it.

```
void setup() {
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
}
void receiveEvent(int howMany) {
  if (Wire.available()) {
    if (Wire.read() == 1) {
      digitalWrite(7, HIGH);
      delay(1000);
      ... do some other code we want
    }
    else {
      digitalWrite(7, LOW);
      delay(1000);
      ... do some other code we want
    }
  }
}
void loop() {
}
```

This will not work :-) The reason is that while your delaying that second you are dropping 
data from the I2C port :-(

### Naive solution

Here is how you might rewrite it.

```
volatile int gotit = 0; // note the volatile here <- that's why it's complicated.
void setup() {
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
}
void receiveEvent(int howMany) {
  if (Wire.available()) {
    gotit = Wire.read();
  }
}
void loop() {
  if (gotit == 1) {
    digitalWrite(7, HIGH);
    delay(1000);
    ... do some other code we want
  }
  else {
      digitalWrite(7, LOW);
      delay(1000);
      ... do some other code we want
  }
}
```

Now you happily send a 1 or a 0 and whatever "gotit" had in it at the time will
be used.

Obviously not ideal still. What happens if we want to store more data as it comes in
and use commands that are not just a single word.

For one thing, where do you put the data :-) You can't just write it into a global string
buffer and then read it out because the writer might simply overwrite it as it goes.

The solution for this is a data structure call a Ring Buffer.

### Ring buffer

A Ring Buffer can be written to AND read from at the same time without the reader and
writer sharing any thing between them apart from the actual buffer.

As long as the reader keeps up with the writer, as the writer get's to the end and wraps around
the reader can happily consume the buffer.

The C++ class in ringbuffer.hpp and ringbuffer.cpp achieves this. To use it, just
include the header and then declare one and use it.

Now your code above can look like this:

```
RingBuffer buffer;

void setup() {
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
}
void receiveEvent(int howMany) {
  while (Wire.available()) {
    buffer.write(Wire.read());
  }
}
void loop() {
  if (buffer.length() > 0) {
    if (buffer.read() == 1) {
      digitalWrite(7, HIGH);
    }
    else {
      digitalWrite(7, LOW);
    }
  }
}
```

OK! So now we can send various I2C words and it will happily do all the different
things sending a 1 or a 0 can do to an LED :-)

But how about more complicated things like string data. The thing is that you can can
happily send multiple bytes to the device but you need to tell it, that's it I'm finished
NOW go and look at that thing.

This is what the Cmd class is for!

### The command (Cmd) getter

Cmd consumed the data in the buffer and waits till it finds a ;, tab, newline or space
and THEN you can get the data out and use it.

So you can use it like this:

```
RingBuffer buffer;
Cmd cmd;

void setup() {
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
}
void receiveEvent(int howMany) {
  while (Wire.available()) {
    buffer.write(Wire.read());
  }
}
void loop() {
  cmd.accept(&buffer);
  if (cmd.ready()) {
    char c[20];
    cmd.read(c, sizeof(c));
    if (strcmp(cmd) == "ON") {
      digitalWrite(7, HIGH);
    }
    else if (strcmp(cmd) == "OFF") {
      digitalWrite(7, LOW);
    }
    else {
      // flash the LED we got an error.
    }
  }
}
```
  
## Sending I2C from a PI

To actually send i2c commands to your Arduino, you can use a raspberry PI and then wire
up the various pins between your Arduino and the PI like this:

Power:
- 5v pin on the PI to the 5v pin on your Arduino
- GRound pin on the PI to (you guessed it) the ground pin on the Arduino

I2C:
- Wire the SDA and SCL ins together

On a PI, all of these pins are right up the end AWAY from the USB/Ethernet ports in a tight
cluster so easy to make a simple little 4 pin jumper for that end.

Looking down at the PI header it's this:

| <!-- --> | <!-- --> | <!-- -->  | <!-- --> |
| ------ | ------ | ------ | ------ |
|        | SCL    | SDA    |        |
|        | GROUND | VIN    |        |
| <!-- --> | <!-- --> | <!-- -->  | <!-- --> |

And on an Arduino, all the pins are usually labelled so that's easy, but there are 2 sets
of pins usually. The SDA and SCL are right at the end of the long header and the power 
is at the start of the second short header on an Arduino.

While you program your Arduino, unplug the power from the PI to the Arduino while it's
connected to your computer and then unplug your Arduino from your computer and plug it
into the PI and yoru good to go.

On the PI, you can do this:

```
$ sudo apt-get install -y i2c-tools
```

And then you can see your Arduino with:

```
$ i2cdetect -y 1
```

It draws a nice graphical map of the i2c bus, yours will be number 8 on the first one.

To send it an "ON;"

```
$ i2ctransfer -y 1 w3@0x08 0x4F 0x4E 0x3B
```

"OFF;"
```
$ i2ctransfer -y 1 w4@0x08 0x4F 0x46 0x46 0x3B
```

"GO;"
```
$ i2ctransfer -y 1 w3@0x08 0x47 0x4F 0x3B
```

You can work out those codes in here:

https://www.rapidtables.com/convert/number/ascii-to-hex.html

## Long delay() considered harmful

For simplicity I just used delays between led flashes (sort of to prove my point), but
this is considered bad practice on an arduino since long delays can stop anything else
happening.

Here is the correct code you would use to flash an LED with varying delays between on and 
off.

```
void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

int state = 0;
int lasttime = 0;

void loop() {
  if (!lasttime) {
    lasttime = millis();
    return;
  }
  int now = millis();
  int diff = now - lasttime;
  if (state == 0 && diff > 1000) {
    ledOn();
    state = 1;
  }
  else if (state == 1 && diff > 100) {
    ledOff();
    state = 0;
  }
  else {
    return;
  }
  lasttime = millis();
}

```

## Using the examples

Each example is named appropriately for what it does, and to get them all to build, put this
whole i2c folder into the "libraries" folder in your Arduion Sketch Directory. You can find
this in Preferences, or Settings on the macintosh.

## Tiny LOGO

Ok. So you've written all this code, you can send various commands to your Arduino to
do interesting things, but how nice would it be to be able to SEND AND RUN ACTUAL CODE
to the Arduino AFTER you've left it on the Moon!

That's where the whole "TinyLogo" project that is a part of this is for. It's only in
early days with very simple syntax and capabilities, but just for a teaser take
a look at the source for i2c.ino and search down to "#idef I2C_LOGO" and you
can see some cool code that takes this LOGO program:

```
TO FLASH
  ON WAIT 100 OFF WAIT 1000
END
TO GO
  FOREVER FLASH
END
TO STOP
  OFF
END
```

And then when you get an I2C string that says "GO;" it will flash the LEDs on and off.

You can send it any of that code ON THE FLY, so even word definitions.

So You could send it:

```
TO FLASH; ON WAIT 20 OFF WAIT 200; END;
```

To make it flash faster, or even completely write a new thing like a morse code
tapper etc without reflashing your Arduino :-)

And that code above fits happily on a Leonardo with 32k of memory (it about fills it though).

After you compile an example with LOGO in it, make sure you have at least 100 bytes for local
variables or it just won't work!!

To free more space, go into the top of logo.hpp and change the sizes of various data structures
until you free more up.

Open "i2c.ino" and you can flash that example right onto your Arduino and try it out.

There are about 5 different examples in there of various other things to get you started using
the various ways you might use this stuff.

One nice feature of the LOGO code is that implements WAIT the correct way without using delay().

You can read more about logo here https://en.wikipedia.org/wiki/Logo_(programming_language)

### LOGO syntax

#### word definitions

Words need to be defined on 3 lines like this:


```
TO wordname; BLOCK; END
```

#### Sentences

Sentences can be used and they are implemented as actual "unnamed words", like anonymous
closures or lambas.

```
[WORD WORD WORD]
```

#### Arguments

Argumemts to functions work BUT it is just a lexical trick, not scoped so recursion won't
work.

```
TO MULT :A :B; :A * :B; END;
MULT 10 20
```

Is functionally equivalent to:

```
TO MULT; :A * :B; END;
MAKE \"A 10 MAKE \"B 20 MULT
```

### LOGO words

## MAKE 

Make a variable. The syntax is:

```
MAKE "VARNAME VALUE
VALUE := [number | string] 
```

## FOREVER

Repeat the next word forever. The syntax is:

```
REPEAT WORD
WORD := [wordname | SENTENCE]
SENTENCE := "[" LITWORD ... "]"
LITWORD := the name of a word
```

## REPEAT

Repeat the next word a number of times. The syntax is:

```
REPEAT NUM WORD
NUM := [number | VAR]
VAR := :varname
WORD := [wordname | SENTENCE | LITERAL]
LITERAL := [number | string } VAR] 
VAR := :varname
SENTENCE := "[" LITWORD ... "]"
LITWORD := the name of a word
```

## IFELSE

Test an expression, it true put the first word on the stack else the second. The syntax is:

```
IFELSE EXPR THEN ELSE
EXPR := [wordname | SENTENCE | LITERAL]
LITERAL := [number | string } VAR] 
VAR := :varname
THEN := [wordname | SENTENCE | LITERAL]
ELSE := [wordname | SENTENCE | LITERAL]
SENTENCE := "[" LITWORD ... "]"
LITWORD := the name of a word
```

## =

Push a 1 on the stack if the operands are the same. The syntax is:

```
WORD = WORD
WORD := [wordname | SENTENCE | LITERAL]
SENTENCE := "[" LITWORD ... "]"
LITWORD := the name of a word
```

## WAIT (not working yet so not in current build)

Wait for milliseconds before executing the next word. The syntax is:

```
WAIT NUM WORD
NUM := [number | VAR]
WORD := [wordname | SENTENCE]
SENTENCE := "[" LITWORD ... "]"
LITWORD := the name of a word
```

## Arithmetic

+, -, * and / all work as usual but no parenthesis and just left to right.

## Development

The development process for all of this code used a normal Linux environment with the BOOST
libraries and a C++ compiler to write and run unit tests to test all of the various
things and then use that code in the Arduino IDE to flash the Arduino.

It's written in a subset of C++ that happily builds on an Arduino:

  - No dynamic memory (no new, malloc etc. Everything just static)
  - No inheritence (see above)
  - No STL (see above)

BUT for the actual development, including a fairly complex debugger for the LOGO stuff,
we use all of these things and just don't build that bit for the Arduino.

So on your Linux (or mac using Homebrew etc), get all you need:

```
$ sudo apt-get -y install g++ gcc make cmake boost
```

And then inside the "test" folder on your machine, run:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make test
```

And then you can go in and fiddle and change the code in lgtestsketch.cpp to change what
your sketch might look like on in your .ino and then do

```
$ make && ./LGTestSketch
```

And it will just run your test in your Dev environment! Just stub out all your builtin words
like "ledOn()" etc to output a string.

For an M1 or M2 mac, use this command for cmake

```
$ cmake -DCMAKE_OSX_ARCHITECTURES="arm64" ..
```

To turn on all the debugging for the various things, each header has something like:

```
//#define LOGO_DEBUG
//#define CMD_DEBUG
//#define RINGBUF_DEBUG
```

That you can comment out BUT while it's commented out the code won't build on the Arduino.

This stuff is just for the development environment.
  
## Related discussions

https://forum.arduino.cc/t/how-to-properly-use-wire-onreceive/891195/12

## Current development focus

### OUTPUT from the Arduino to the PI

## Change Log

24 May 2023
- Changed Cmd so that commands can have spaces in them so that it works properly with the
LOGO code. Tab, Newline and ";" all will mark the end of a command.
- Lot's of LOGO work including an RGB LED example that shows off the Maths.