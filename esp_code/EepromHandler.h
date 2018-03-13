// EepromHandler.h

#ifndef EEPROM_HANDLER_H
#define EEPROM_HANDLER_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <EEPROM.h>

class EepromHandler {
public:
	inline String getSsid() {return _ssid;}
	inline String getPassword() {return _password;}
	inline String getMqttHost() {return _mqtt_host;}
	bool clearWifiParameters();
	bool writeWifiParameters(String ssid, String password);
	bool readWiFiParameters();
	bool clearMqttHost();
	bool writeMqttHost(String mqtt_host);
	bool readMqttHost();
	void init();
	void end();

private:
	String _ssid;
	String _password;
	String _mqtt_host;
	const size_t _ssid_space = 64;
	const size_t _password_space = 64;
	const size_t _mqtt_host_space = 64;
	const size_t _init_eeprom_size = 512;
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM_HANDLER)
extern EepromHandler eepromHandler;
#endif

#endif // EEPROM_HANDLER_H
