
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

Read: https://github.com/visualopsholdings/ringbuffer


Read: https://github.com/visualopsholdings/cmd

Read: https://github.com/visualopsholdings/tinylogo

## Change Log

24 May 2023
- Moved ringbuffer, cmd and tinylogo to their own repos.