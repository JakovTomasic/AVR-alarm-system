
# define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "lcd.h"

#define KEYPAD_PORT			PORTA
#define KEYPAD_DDR 			DDRA
#define KEYPAD_PIN 			PINA

#define KEY_STAR			10
#define KEY_HASH			11
#define KEY_NONE			12

#define MOTION_PIN 			PINA
#define MOTION_PORT			PORTA
#define MOTION_PIN_NUMBER	0

#define BUZZER_DDR 			DDRC
#define BUZZER_PORT			PORTC
#define BUZZER_PIN_NUMBER	7
/*
 * b7 -> 1 if alarm turned on, 0 if in deactivated state
 * b6 -> 1 if movement detected, 0 otherwise
 * other bits -> reserved
 */
uint8_t state;

#define DEFAULT_STATE 0x80

// Warning: do not use these for setting state (fix that)
#define alarmOn (state & 0x80)
#define motionDetected (state & 0x40)


const uint16_t password = 1234;
#define PASSWORD_LENGTH 4
uint8_t enteredDigits[4] = {KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE};

#define resetEnteredDigits enteredDigits[0] = enteredDigits[1] = enteredDigits[2] = enteredDigits[3] = KEY_NONE
#define enteredDigitsValue (enteredDigits[0]*1000 + enteredDigits[1]*100 + enteredDigits[2]*10 + enteredDigits[3])


// Utils:

// TODO: make faster using define (and activate pull-up resistor only on init)
uint8_t readMotion() {
	// Activate pull-up resistor
	MOTION_PORT |= _BV(MOTION_PIN_NUMBER);
	return bit_is_set(MOTION_PIN, MOTION_PIN_NUMBER);
}

void buzz() {
	BUZZER_DDR |= _BV(BUZZER_PIN_NUMBER);
	BUZZER_PORT |= _BV(BUZZER_PIN_NUMBER);
	_delay_ms(200);
	BUZZER_PORT &= ~(_BV(BUZZER_PIN_NUMBER));
}

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

uint8_t getKeyPressed()
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

// TODO: make faster using define?
uint8_t isNumber(uint8_t key) {
	return key < 10;
}



// App logic:

void initLcd() {
	
	DDRD |= _BV(4);

	TCCR1A |= _BV(COM1B1) | _BV(WGM10);
	TCCR1B |= _BV(WGM12) | _BV(CS11);
	OCR1B = 128;

	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_puts("Starting");
	_delay_ms(200);
	lcd_putc('.');
	_delay_ms(200);
	lcd_putc('.');
	_delay_ms(200);
	lcd_putc('.');
	_delay_ms(200);
}

void refreshState() {
	if (alarmOn) {
		if (motionDetected) {
			lcd_clrscr();
			lcd_puts("I see you moving");
		} else {
			lcd_clrscr();
			lcd_puts("Alarm on");
			resetEnteredDigits;
		}
	} else {
		lcd_clrscr();
		lcd_puts("Alarm off");
		resetEnteredDigits;
	}
}

void handleKeypress(uint8_t key) {
	if (!alarmOn && key == KEY_STAR) {
		state |= 0x80;
		state &= 0xBF;
		refreshState();
	} else if (alarmOn && key == KEY_HASH) {
		lcd_gotoxy(0, 1);
		lcd_puts("    ");
		resetEnteredDigits;
	} else if (alarmOn && isNumber(key)) {
		uint8_t i;
		lcd_gotoxy(0, 1);
		lcd_puts("    ");
		lcd_gotoxy(0, 1);
		for (i = 0; i < PASSWORD_LENGTH; i++) {
			if (enteredDigits[i] == KEY_NONE) {
				enteredDigits[i] = key;
				lcd_putc('0' + enteredDigits[i]);
				break;
			} else {
				lcd_putc('0' + enteredDigits[i]);
			}
		}
		
		if (i == PASSWORD_LENGTH - 1) {
			if (alarmOn) {
				if (enteredDigitsValue == password) {
					state &= 0x7F;
					state &= 0xBF;
					resetEnteredDigits;
					refreshState();
				} else {
					lcd_clrscr();
					lcd_puts("Wrong password");
					_delay_ms(1000);
					resetEnteredDigits;
					refreshState();
				}
			}
		}
	}
}

void checkMotion() {
	if (alarmOn && !motionDetected && readMotion()) {
		state |= 0x40;
		refreshState();
	}
}



int main(void) {
	
	initLcd();
	
	state = DEFAULT_STATE;
	refreshState();

	buzz();

	uint8_t key, lastKey = KEY_NONE;
	
	while (1) {
		_delay_ms(50);

		checkMotion();

		key = getKeyPressed();

		/*
		lcd_clrscr();
		lcd_putc('0' + (key % 10));
		lcd_gotoxy(0, 1);
		lcd_putc('0' + readMotion());
		*/
		
		if (key != KEY_NONE && key != lastKey) {
			handleKeypress(key);
			buzz();
		}
		
		lastKey = key;
	}
}
