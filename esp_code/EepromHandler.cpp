// EepromHandler.cpp

#include "EepromHandler.h"

bool EepromHandler::clearEeprom() {
	for (int i = 0; i < (_ssid_space + _password_space); ++i) {
		EEPROM.write(i, 0);
	}

	//Serial.println("Eeprom cleared");

	return true;
}

bool EepromHandler::writeWifiParameters(String ssid, String password) {
  if (ssid.length() > _ssid_space) {
    //Serial.println("Not enough space to store ssid!");
    return false;
  }

  if (password.length() > _password_space) {
    //Serial.println("Not enough space to store password!");
    return false;
  }

	clearEeprom();

	for (size_t i = 0; i < ssid.length(); ++i) {
		EEPROM.write(i, ssid[i]);
	}

	for (size_t i = 0; i < password.length(); ++i)	{
		EEPROM.write(_ssid_space + i, password[i]);
	}

	bool success = EEPROM.commit();

	//Serial.println("Saved to eeprom");
	//Serial.println(success);

	return success;
}

bool EepromHandler::readWiFiParameters() {
  _ssid = "";
  _password = "";

	//Serial.println("Starting eeprom read");

	for (size_t i = 0; i < _ssid_space; ++i) {
    char tmp = char(EEPROM.read(i));
    if (tmp == 0) break;
		_ssid += tmp;
	}
	for (size_t i = 0; i < _password_space; ++i) {
    char tmp = char(EEPROM.read(_ssid_space + i));
    if (tmp == 0) break;
		_password += tmp;
	}

	//Serial.println(_ssid);
	//Serial.println(_password);
	//Serial.println("Readed from eeprom");

	return true;
}

void EepromHandler::init() {
	EEPROM.begin(_init_eeprom_size);
}

void EepromHandler::end() {
	EEPROM.end();
}
