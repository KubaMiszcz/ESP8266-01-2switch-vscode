// MqttRelay.h

#ifndef _MQTTRELAY_h
#define _MQTTRELAY_h

#include "Arduino.h"
#include <RelayButton.h>

class MqttRelay
{
protected:
	int pin;
	bool state;

public:
	int number;
	String name;
	const char *inTopic;
	const char *outTopic;
	int eepromAddress;
	RelayButton button;

	MqttRelay();
	MqttRelay(int ppin, int nnumber, String nname, const char *iinTopic, const char *ooutTopic, int eeepromAddress);
	void init();
	void turnOn();
	void turnOff();
	bool IsOn();
	bool IsOff();
	bool getState();
	void setState(bool newState);
	void toggle();
	void attachButton(RelayButton bbutton);
};

#endif
