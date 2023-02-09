
#include "cpu_freq.h"

#include <avr/io.h>
#include <util/delay.h>

#include "utils.h"

void initUtils(void) {
	
	// Activate pull-up resistor for motion sensor.
	MOTION_PORT |= _BV(MOTION_PIN_NUMBER);
	// Set buzzer DDR as out
	BUZZER_DDR |= _BV(BUZZER_PIN_NUMBER);
	
	// Set police pins DDR as out
	POLICE_1_DDR |= _BV(POLICE_1_PIN_NUMBER);
	POLICE_2_DDR |= _BV(POLICE_2_PIN_NUMBER);
}

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

void startPolice(void) {
	POLICE_1_PORT |= _BV(POLICE_1_PIN_NUMBER);
	POLICE_2_PORT &= ~_BV(POLICE_2_PIN_NUMBER);
}

void stopPolice(void) {
	POLICE_1_PORT &= ~_BV(POLICE_1_PIN_NUMBER);
	POLICE_2_PORT &= ~_BV(POLICE_2_PIN_NUMBER);
}

void togglePolice(void) {
	POLICE_1_PORT ^= _BV(POLICE_1_PIN_NUMBER);
	POLICE_2_PORT ^= _BV(POLICE_2_PIN_NUMBER);
}
