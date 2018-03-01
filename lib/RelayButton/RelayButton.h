// MqttRelay.h

#ifndef _RELAYBUTTON_h
#define _RELAYBUTTON_h

#include "Arduino.h"
#include <Bounce2.h>

class RelayButton
{
protected:
	int pin;
	bool state;

public:
	Bounce debouncer;
	RelayButton();
	RelayButton(int ppin, int debouncerInterval);
};

#endif
