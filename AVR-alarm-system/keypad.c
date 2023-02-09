
#include <avr/io.h>

#include "keypad.h"


// TODO: make faster using define
void keypad_activateRowPullUps() {
	// Do not set col pins (assigning 0 would set it)
	KEYPAD_PORT |= 0X1E;
}

// TODO: make faster using define
void keypad_setAllInput() {
	// One pin is not used so ignore it
	KEYPAD_DDR &= 0X01;
}

// TODO: make faster using define
void keypad_setColAsOutputLow(uint8_t col) {
	// PORTX is always 0 for that pins so the output will be low (sink)
	KEYPAD_DDR |= (0X80 >> col);
}

// TODO: make faster using define
uint8_t keypad_isRowLow(uint8_t row) {
	return bit_is_clear(KEYPAD_PIN, 4 - row);
}

uint8_t getKeyPressed(void)
{
	uint8_t r, c, reversedC;

	keypad_activateRowPullUps();

	for(reversedC = 0; reversedC < 3 ; reversedC++)
	{
		c = 2 - reversedC;
		
		keypad_setAllInput();

		// Set only current col pin as output.
		keypad_setColAsOutputLow(c);
		for(r = 0; r < 4; r++)
		{
			if(keypad_isRowLow(r))
			{
				// Return pressed key.
				// Do not check if other keys are pressed.
				if (r < 3) {
					return (r*3+c)+1;
					} else if (c == 0) {
					return KEY_STAR;
					} else if (c == 1) {
					return 0;
					} else {
					return KEY_HASH;
				}
			}
		}
	}

	// Indicate no key pressed
	return KEY_NONE;
}


