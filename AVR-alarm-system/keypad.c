
#include <avr/io.h>

#include "keypad.h"


uint8_t getKeyPressed(void)
{
	uint8_t r, c, reversedC;

	// Activate pull-up resistors for row pins.
	// Do not set col pins (assigning 0 would set it).
	KEYPAD_PORT |= 0X1E;

	for(reversedC = 0; reversedC < 3 ; reversedC++)
	{
		c = 2 - reversedC;
		
		// Set all pins as input.
		// One pin is not used so ignore it
		KEYPAD_DDR &= 0X01;

		// Set only current col pin as output. Write logical 0 to it.
		// PORTX is always 0 for that pins so the output will be low (sink)
		KEYPAD_DDR |= (0X80 >> c);
		
		for(r = 0; r < 4; r++)
		{
			if(bit_is_clear(KEYPAD_PIN, 4 - r))
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
