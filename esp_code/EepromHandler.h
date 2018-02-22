// EepromHandler.h

#ifndef _EEPROMHANDLER_h
#define _EEPROMHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <EEPROM.h>

class EepromHandler {
public:
	  inline String getSsid() {return _ssid;};
    inline String getPassword() {return _password;};
    bool clearEeprom();
	  bool writeWifiParameters(String ssid, String password);
	  bool readWiFiParameters();
	  void init();
	  void end();

private:
    String _ssid;
    String _password;
    const size_t _ssid_space = 64;
    const size_t _password_space = 64;
    const size_t _init_eeprom_size = 512;
};

#endif
