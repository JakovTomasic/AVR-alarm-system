
# define F_CPU 7372800UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lcd.h"
#include "utils.h"
#include "keypad.h"
#include "door.h"


/*
 * b7 -> 1 if alarm turned on, 0 if in deactivated state
 * b6 -> 1 if movement detected, 0 otherwise
 * b5 -> special action input mode. 1 if active, 0 otherwise
 * b4 -> 1 if intruder detected, 0 otherwise
 * other bits -> reserved
 */
uint8_t state;

#define DEFAULT_STATE 0

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

uint16_t tickCounter = 0;

// TODO: set to 6000
#define MOTION_DETECTED_COUNTDOWN 600


void initLcd() {
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
			tickCounter = MOTION_DETECTED_COUNTDOWN + 1;
		} else {
			resetEnteredDigits;
		}
	} else {
		resetEnteredDigits;
	}
	
	if (alarmOn && intruderDetected) {
		closeDoor();
		tickCounter = 0;
		startPolice();
	} else {
		openDoor();
		stopPolice();
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
				tripleBuzz();
				_delay_ms(500);
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

void tick() {
	if (alarmOn && motionDetected && !intruderDetected) {
		if (--tickCounter % 100 == 0) {
			lcd_gotoxy(14, 1);
			writeLCD_alignRight(tickCounter / 100, 2);
			buzz();
		}
		
		if (tickCounter == 0) {
			state |= 0x10;
			refreshState();
		}
	} else if (intruderDetected) {
		if (++tickCounter == 100) {
			togglePolice();
			tickCounter = 0;
		}
	}
}

void init() {
	initUtils();
	initLcd();
	initDoor();
	
	state = DEFAULT_STATE;
	refreshState();
}

int main(void) {
	
	init();

	buzz();

	uint8_t key, lastKey = KEY_NONE;
	
	
	/*
	//configure timer 1 - ctc mode, oc1a/b disconnected, prescaler 8
	//ocr = 9216 -> output compare match f=0.01s
	TCCR1A = 0;
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1A = 9216;
	//enable OC interrupt
	TIMSK |= _BV(OCIE1A);

	sei();
	*/
	
	
	while (1) {
		_delay_ms(10);

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
		
		tick();
	}
}

