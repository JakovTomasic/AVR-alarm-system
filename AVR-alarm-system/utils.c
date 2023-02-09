
#include <avr/io.h>

#include "utils.h"

// TODO: make faster using define (and activate pull-up resistor only on init)
uint8_t readMotion(void) {
	return bit_is_set(MOTION_PIN, MOTION_PIN_NUMBER);
}
