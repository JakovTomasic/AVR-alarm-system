
#include <avr/io.h>
#include <avr/eeprom.h>

#include "auth.h"


uint8_t checkEnteredPassword(uint8_t *entered, uint8_t *toCheck) {
	for (uint8_t j = 0; j < PASSWORD_LENGTH; j++) {
		if (entered[j] != toCheck[j]) {
			return 0;
			} else if (j == PASSWORD_LENGTH-1) {
			return 1;
		}
	}
	return 0;
}

uint8_t getUserIdForEnteredPassword(uint8_t *entered) {
	for (uint8_t i = 0; i < PASSWORD_USERS_COUNT; i++) {
		if (checkEnteredPassword(entered, userPasswords[i])) {
			return i + 1;
		}
	}
	return 0;
}

void initLog(void) {
	logSize = eeprom_read_byte ((const uint8_t*) LOG_SIZE_ADDRESS);
	logIndex = 0;
}

void writeLoginToLog(uint8_t userId) {
	eeprom_update_byte(( uint8_t *) LOG_FIRST_ADDRESS + logSize, userId);
	logSize++;
	eeprom_update_byte (( uint8_t *) LOG_SIZE_ADDRESS, logSize);
}

void clearLog(void) {
	logSize = 0;
	logIndex = 0;
	eeprom_update_byte (( uint8_t *) LOG_SIZE_ADDRESS, 0);
}

uint8_t getUserIdFromLog(uint8_t index) {
	return eeprom_read_byte((const uint8_t*) LOG_FIRST_ADDRESS + index);
}
