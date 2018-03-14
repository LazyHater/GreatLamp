// Lamp.h

#ifndef LAMP_h
#define LAMP_h

class Lamp {
public:
  Lamp(const int read_pin, const int write_pin);
  int readLevel();
  void toggle();
  void setLevel(int level);

  const int read_pin;
  const int write_pin;
  
private:
  void _toggle();
};

#endif // LAMP_h
