#include "Lamp.h"
#include "MqttHandler.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

Lamp::Lamp(const int read_pin, const int write_pin) : read_pin(read_pin), write_pin(write_pin) {
	pinMode(read_pin, INPUT);
	pinMode(write_pin, OUTPUT);

	digitalWrite(write_pin, LOW);
}

int Lamp::readLevel() {
	// int current_level = random(0xffff) % 4;
	// return  current_level;
  int pwm_value = pulseIn(read_pin, HIGH, 2000);
  if (pwm_value == 0) {
    if (digitalRead(read_pin)) {
      return 3;
    } else {
      return 0;
    }
  }

  int d1 = abs(pwm_value - 40);
  int d2 = abs(pwm_value - 170);

  if (d1 < d2) {
    return 1;
  }
  return 2;  
 

	// const int samples = 1000;
	// int cnt = 0;
	// for (int i = 0; i < samples; i++) {
	// 	cnt += digitalRead(read_pin);
	// }

	// int levels[] = {int(samples * 0.0f), int(samples * 0.1f), int(samples * 0.4f), int(samples * 1.0f)};
	// int min_dist = samples * 2;
	// int min_idx = -1;
	// MqttHandler::updateLevel2(cnt);
	// for (int i = 0; i < (sizeof(levels) / sizeof(int)); i++) {
	// 	int dist = abs(levels[i] - cnt);
	// 	if (dist < min_dist) {
	// 		min_dist = dist;
	// 		min_idx = i;
	// 	}
	// }

	// return min_idx;
}

void Lamp::handle(bool force) {
  static int last_level = 0;
  static unsigned long lastCheckTime = 0;
  if ((millis() - lastCheckTime > 5000) || force) {
	  int current_level = readLevel();
	if (last_level != current_level) {
		if (callback) {
			callback(current_level);
		}
	}
	last_level = current_level;
	lastCheckTime = millis();
  }
}

void Lamp::toggle() {
	int next_level = (readLevel() + 1 ) % 4;
	setLevel(next_level);
}

void Lamp::_toggle() {
	digitalWrite(write_pin, HIGH);
	delay(80);
	digitalWrite(write_pin, LOW);
	delay(80);
}

void Lamp::setLevel(int level) {
	if (level < 0 || level > 3) return;

	while (level != readLevel()) {
		_toggle();
	}
}

void Lamp::setCallback(LAMP_CALLBACK_SIGNATURE) {
	this->callback = callback;
}
