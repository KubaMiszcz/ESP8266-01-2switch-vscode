#include "MqttRelay.h"

MqttRelay::MqttRelay() {}

MqttRelay::MqttRelay(int ppin, int nnumber, String nname, const char *iinTopic, const char *ooutTopic, int eeepromAddress)
{
	pin = ppin;
	state = false;
	number = nnumber;
	name = nname;
	inTopic = iinTopic;
	outTopic = ooutTopic;
	eepromAddress = eeepromAddress;
}

void MqttRelay::init()
{
	pinMode(pin, OUTPUT);
}

void MqttRelay::turnOn()
{
	digitalWrite(pin, state);
}
void MqttRelay::turnOff()
{
	digitalWrite(pin, state);
}

bool MqttRelay::IsOn()
{
	if (state)
		return true;
	else
		return false;
}
bool MqttRelay::IsOff()
{
	return !IsOn();
}

bool MqttRelay::getState()
{
	return state;
}
void MqttRelay::setState(bool newState)
{
	digitalWrite(pin, newState);
}

void MqttRelay::toggle()
{
	digitalWrite(pin, !state);
}

void MqttRelay::attachButton(RelayButton bbutton)
{
	button = bbutton;
}
