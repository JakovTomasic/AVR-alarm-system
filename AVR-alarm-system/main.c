
# define F_CPU 7372800UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h> // TODO: treba li mi ovo?

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
 * b5 -> special action input mode. 1 if active, 0 otherwise
 * b4 -> 1 if intruder detected, 0 otherwise
 * other bits -> reserved
 */
uint8_t state;

#define DEFAULT_STATE 0x80

// Warning: do not use these for setting state (fix that)
#define alarmOn (state & 0x80)
#define motionDetected (state & 0x40)
#define specialInputActive (state & 0x20)
#define intruderDetected (state & 0x10)


#define PASSWORD_LENGTH 4
const uint16_t password = 1234;

uint8_t enteredDigits[4] = {KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE};

#define resetEnteredDigits enteredDigits[0] = enteredDigits[1] = enteredDigits[2] = enteredDigits[3] = KEY_NONE
#define enteredDigitsValue (enteredDigits[0]*1000 + enteredDigits[1]*100 + enteredDigits[2]*10 + enteredDigits[3])

uint16_t motionDetectionCountdown = 0;

// Utils:

void writeLCD_alignRight(uint16_t val, uint8_t width) {
	uint8_t num;
	char valStr[width+1];
	for (uint8_t i = 1; i <= width; i++) {
		if (val) {
			valStr[width - i] = '0' + (val % 10);
			val /= 10;
		} else {
			valStr[width - i] = ' ';
		}
	}
	valStr[width] = '\0';
	lcd_puts(valStr);
}

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
	
	DDRB |= _BV(3);

	TCCR0 |= _BV(COM01) | _BV(WGM01) | _BV(WGM00) | _BV(CS01);
	OCR0 = 128;
	
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

void writeCurrentStateMessage() {
	if (alarmOn) {
		if (intruderDetected) {
			lcd_clrscr();
			lcd_puts("Intruder!!!");
			lcd_gotoxy(0, 1);
			lcd_puts("Calling police!");
		} else if (motionDetected) {
			lcd_clrscr();
			lcd_puts("I see you moving");
			lcd_gotoxy(0, 1);
			for (uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
				if (enteredDigits[i] == KEY_NONE) {
					break;
				} else {
					lcd_putc('0' + enteredDigits[i]);
				}
			}
		} else {
			lcd_clrscr();
			lcd_puts("Alarm on");
		}
	} else {
		if (specialInputActive) {
			lcd_clrscr();
			lcd_puts("Enter an action");
		} else {
			lcd_clrscr();
			lcd_puts("Alarm off");
		}
	}
}

void refreshState() {
	writeCurrentStateMessage();
	if (alarmOn) {
		if (motionDetected && !intruderDetected) {
			motionDetectionCountdown = 601; // TODO: set to 6001
		} else {
			resetEnteredDigits;
		}
	} else {
		resetEnteredDigits;
	}
}

void handleKeypress(uint8_t key) {
	if (alarmOn && !intruderDetected && key == KEY_HASH) {
		lcd_gotoxy(0, 1);
		lcd_puts("    ");
		resetEnteredDigits;
	} else if (alarmOn && !intruderDetected && isNumber(key)) {
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
				writeCurrentStateMessage();
			}
		}
	} else if (!alarmOn) {
		if (specialInputActive) {
			if (key == 0) {
				state |= 0x80;
				state &= 0x9F;
				refreshState();
			} else if (key == KEY_STAR) {
				state &= ~0x20;
				refreshState();
			}
		} else if (key == KEY_STAR) {
			state |= 0x20;
			refreshState();
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
	
	//configure timer 1 - ctc mode, oc1a/b disconnected, prescaler 8
	//ocr = 9216 -> output compare match f=0.01s
	TCCR1A = 0;
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1A = 9216;
	//enable OC interrupt
	TIMSK |= _BV(OCIE1A);

	sei();
	
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

ISR(TIMER1_COMPA_vect)
{
	if (alarmOn && motionDetected && !intruderDetected) {
		if (--motionDetectionCountdown % 100 == 0) {
			lcd_gotoxy(14, 1);
			writeLCD_alignRight(motionDetectionCountdown / 100, 2);
		}
		
		if (motionDetectionCountdown == 0) {
			state |= 0x10;
			refreshState();
		}
	}
}
