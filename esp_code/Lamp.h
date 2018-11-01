// Lamp.h

#ifndef LAMP_h
#define LAMP_h

#include <Arduino.h>

#define LAMP_CALLBACK_SIGNATURE std::function<void(int)> callback

class Lamp
{
public:
  Lamp(const int read_pin, const int write_pin);
  int readLevel();
  void toggle();
  void setLevel(int level);
  void setCallback(LAMP_CALLBACK_SIGNATURE);
  void handle(bool force = false);

  const int read_pin;
  const int write_pin;

private:
  LAMP_CALLBACK_SIGNATURE = NULL;
  void _toggle();
};

#endif // LAMP_h
