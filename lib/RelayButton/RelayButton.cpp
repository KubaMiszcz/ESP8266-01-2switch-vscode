#include "RelayButton.h"

RelayButton::RelayButton() {}

RelayButton::RelayButton(int ppin, int debouncerInterval = 100)
{
	pin = ppin;
	pinMode(pin, INPUT_PULLUP);
	debouncer = Bounce();
	debouncer.attach(pin);
	debouncer.interval(debouncerInterval);
}
