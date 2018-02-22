// Lamp.h

#ifndef _LAMP_h
#define _LAMP_h

class Lamp {
public:
  Lamp(const int read_pin, const int write_pin);
  int readLevel();
  void toggle();
  void setLevel(int level);

  const int read_pin;
  const int write_pin;
};

#endif
