#include "Lamp.h"

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
	const int samples = 1000;
	int cnt = 0;
	for (int i = 0; i < samples; i++) {
		cnt += digitalRead(read_pin);
	}

	int levels[] = {int(samples * 0.0f), int(samples * 0.1f), int(samples * 0.4f), int(samples * 1.0f)};
	int min_dist = samples * 2;
	int min_idx = -1;

	for (int i = 0; i < (sizeof(levels) / sizeof(int)); i++) {
		int dist = abs(levels[i] - cnt);
		if (dist < min_dist) {
			min_dist = dist;
			min_idx = i;
		}
	}

	return min_idx;
}

void Lamp::toggle() {
	digitalWrite(write_pin, HIGH);
	delay(80);
	digitalWrite(write_pin, LOW);
	delay(80);
}

void Lamp::setLevel(int level) {
	if (level < 0 || level > 3) return;

	while (level != readLevel()) {
		toggle();
	}
}
