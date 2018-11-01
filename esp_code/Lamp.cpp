#include "Lamp.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "MqttHandler.h"
#include "Parameters.h"

Lamp::Lamp(const int read_pin, const int write_pin) : read_pin(read_pin), write_pin(write_pin)
{
	pinMode(read_pin, INPUT);
	pinMode(write_pin, OUTPUT);

	digitalWrite(write_pin, LOW);
}

int Lamp::readLevel()
{
	int pwm_value = pulseIn(read_pin, HIGH, LAMP_PULSE_IN_TIME);
	if (pwm_value == 0)
	{
		if (digitalRead(read_pin))
		{
			return 3;
		}
		else
		{
			return 0;
		}
	}

	int d1 = abs(pwm_value - LAMP_LEVEL_1_PWM_VALUE);
	int d2 = abs(pwm_value - LAMP_LEVEL_2_PWM_VALUE);

	if (d1 < d2)
	{
		return 1;
	}
	return 2;
}

void Lamp::handle(bool force)
{
	static int last_level = 0;
	static unsigned long lastCheckTime = 0;
	if ((millis() - lastCheckTime > LAMP_HANDLE_LEVEL_CHECK_TIME) || force)
	{
		int current_level = readLevel();
		if (last_level != current_level)
		{
			if (callback)
			{
				callback(current_level);
			}
		}
		last_level = current_level;
		lastCheckTime = millis();
	}
}

void Lamp::toggle()
{
	int next_level = (readLevel() + 1) % 4;
	setLevel(next_level);
}

void Lamp::_toggle()
{
	digitalWrite(write_pin, HIGH);
	delay(80);
	digitalWrite(write_pin, LOW);
	delay(80);
}

void Lamp::setLevel(int level)
{
	if (level < 0 || level > 3)
		return;

	while (level != readLevel())
	{
		_toggle();
	}
}

void Lamp::setCallback(LAMP_CALLBACK_SIGNATURE)
{
	this->callback = callback;
}
