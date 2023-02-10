
#include "cpu_freq.h"

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
 * b3 -> 1 if alarm is being turned on (countdown active), 0 otherwise
 * other bits -> reserved
 */
typedef struct {
	uint8_t bit7:1;
	uint8_t bit6:1;
	uint8_t bit5:1;
	uint8_t bit4:1;
	uint8_t bit3:1;
} stateRegBits;

//use one of otherwise unused SFRs as states register, for instance TWBR
//enables usage of sbi/cbi instructions - compiler produces small and fast code
#define stateReg TWBR

#define alarmOn ((volatile stateRegBits*)_SFR_MEM_ADDR(stateReg))->bit7
#define motionDetected ((volatile stateRegBits*)_SFR_MEM_ADDR(stateReg))->bit6
#define specialInputActive ((volatile stateRegBits*)_SFR_MEM_ADDR(stateReg))->bit5
#define intruderDetected ((volatile stateRegBits*)_SFR_MEM_ADDR(stateReg))->bit4
#define alarmTurningOn ((volatile stateRegBits*)_SFR_MEM_ADDR(stateReg))->bit3

#define DEFAULT_STATE 0


#define PASSWORD_LENGTH 4
const uint16_t password = 1234;

uint8_t enteredDigits[4] = {KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE};

#define resetEnteredDigits enteredDigits[0] = enteredDigits[1] = enteredDigits[2] = enteredDigits[3] = KEY_NONE
#define enteredDigitsValue (enteredDigits[0]*1000 + enteredDigits[1]*100 + enteredDigits[2]*10 + enteredDigits[3])

uint16_t tickCounter = 0;

// TODO: set to 30000
#define MOTION_DETECTED_COUNTDOWN 3000
#define ALARM_TURN_ON_COUNTDOWN 3000
#define TICKS_PER_ONE_SECOND 500


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

void LCDwriteEnteredDigits() {
	for (uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
		if (enteredDigits[i] == KEY_NONE) {
			break;
		} else {
			lcd_putc('0' + enteredDigits[i]);
		}
	}
}

void LcdWriteTickCountdown() {
	lcd_gotoxy(14, 1);
	writeLCD_alignRight(tickCounter / TICKS_PER_ONE_SECOND, 2);
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
			LCDwriteEnteredDigits();
			LcdWriteTickCountdown();
		} else {
			lcd_clrscr();
			lcd_puts("Alarm on");
		}
	} else if (alarmTurningOn) {
		lcd_clrscr();
		lcd_puts("Alarm turning on");
		lcd_gotoxy(0, 1);
		LCDwriteEnteredDigits();
		LcdWriteTickCountdown();
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
	} else if (alarmTurningOn) {
		tickCounter = ALARM_TURN_ON_COUNTDOWN + 1;
		resetEnteredDigits;
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
	if (key == KEY_HASH && ((alarmOn && !intruderDetected) || (!alarmOn && alarmTurningOn) || (alarmOn && intruderDetected))) {
		lcd_gotoxy(0, 1);
		if (alarmOn && intruderDetected) {
			lcd_puts("                ");
			} else {
			lcd_puts("    ");
		}
		resetEnteredDigits;
	} else if (isNumber(key) && ((alarmOn && !intruderDetected) || (!alarmOn && alarmTurningOn) || (alarmOn && intruderDetected))) {
		uint8_t i;
		lcd_gotoxy(0, 1);
		if (alarmOn && intruderDetected) {
			lcd_puts("                ");
		} else {
			lcd_puts("    ");
		}
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
				alarmOn = 0;
				motionDetected = 0;
				intruderDetected = 0;
				alarmTurningOn = 0;
				resetEnteredDigits;
				refreshState();
			} else {
				lcd_clrscr();
				lcd_puts("Wrong password");
				tripleBuzz();
				_delay_ms(1000);
				resetEnteredDigits;
				writeCurrentStateMessage();
			}
		}
	} else if (!alarmOn) {
		if (specialInputActive) {
			if (key == 0) {
				specialInputActive = 0;
				if (motionDetected) {
					lcd_clrscr();
					lcd_puts("Movement error!");
					lcd_gotoxy(0, 1);
					lcd_puts("Stand still!");
					tripleBuzz();
					_delay_ms(1000);
					writeCurrentStateMessage();
				} else {
					alarmTurningOn = 1;
					refreshState();
				}
			} else if (key == KEY_STAR) {
				specialInputActive = 0;
				refreshState();
			}
		} else if (key == KEY_STAR) {
			
			specialInputActive = 1;
			refreshState();
		}
	}
}

void updateMotion() {
	uint8_t motion = readMotion();
	if (!motionDetected && motion) {
		motionDetected = 1;
		if (alarmOn) {
			refreshState();
		}
	} else if (motionDetected && !motion && !alarmOn) {
		// Set motion detected back to false only if alarm is not on
		motionDetected = 0;
	}
}

void tick() {
	if (alarmOn && motionDetected && !intruderDetected) {
		tickCounter--;
		
		if (tickCounter == 0) {
			intruderDetected = 1;
			refreshState();
		} else if (tickCounter % TICKS_PER_ONE_SECOND == 0) {
			LcdWriteTickCountdown();
			buzz();
		}
	} else if (!alarmOn && alarmTurningOn) {
		tickCounter--;
		
		if (tickCounter == 0) {
			tripleBuzz();
			alarmOn = 1;
			refreshState();
		} else if (tickCounter % TICKS_PER_ONE_SECOND == 0) {
			LcdWriteTickCountdown();
			buzz();
		}
	} else if (intruderDetected) {
		if (++tickCounter == TICKS_PER_ONE_SECOND) {
			togglePolice();
			tickCounter = 0;
		}
	}
}

void init() {
	initUtils();
	initLcd();
	initDoor();
	
	stateReg = DEFAULT_STATE;
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
		_delay_ms(2);

		updateMotion();

		key = getKeyPressedDebounce();

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

