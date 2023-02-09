
#include <avr/io.h>
#include <util/delay.h>

#include "utils.h"

// TODO: make faster using define (and activate pull-up resistor only on init)
uint8_t readMotion(void) {
	return bit_is_set(MOTION_PIN, MOTION_PIN_NUMBER);
}

void buzz(void) {
	BUZZER_PORT |= _BV(BUZZER_PIN_NUMBER);
	_delay_ms(BUZZER_DURATION_MS);
	BUZZER_PORT &= ~(_BV(BUZZER_PIN_NUMBER));
}

void tripleBuzz(void) {
	BUZZER_PORT |= _BV(BUZZER_PIN_NUMBER);
	_delay_ms(BUZZER_TRIPLE_STEP_DURATION_MS);
	BUZZER_PORT &= ~(_BV(BUZZER_PIN_NUMBER));
	_delay_ms(BUZZER_TRIPLE_STEP_DURATION_MS);
	BUZZER_PORT |= _BV(BUZZER_PIN_NUMBER);
	_delay_ms(BUZZER_TRIPLE_STEP_DURATION_MS);
	BUZZER_PORT &= ~(_BV(BUZZER_PIN_NUMBER));
	_delay_ms(BUZZER_TRIPLE_STEP_DURATION_MS);
	BUZZER_PORT |= _BV(BUZZER_PIN_NUMBER);
	_delay_ms(BUZZER_TRIPLE_STEP_DURATION_MS);
	BUZZER_PORT &= ~(_BV(BUZZER_PIN_NUMBER));
}

