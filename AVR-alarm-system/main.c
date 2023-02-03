
# define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "lcd.h"

#define KEYPAD_PORT PORTA
#define KEYPAD_DDR 	DDRA
#define KEYPAD_PIN 	PINA

#define MOTION_PIN 			PINA
#define MOTION_PORT			PORTA
#define MOTION_PIN_NUMBER	0

#define BUZZER_DDR 			DDRC
#define BUZZER_PORT			PORTC
#define BUZZER_PIN_NUMBER	7

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

void keypad_activateRowPullUps() {
	// Do not set col pins (assigning 0 would set it)
	KEYPAD_PORT |= 0X1E;
}

void keypad_setAllInput() {
	// One pin is not used so ignore it
	KEYPAD_DDR &= 0X01;
}

void keypad_setColAsOutputLow(uint8_t col) {
	// PORTX is always 0 for that pins so the output will be low (sink)
	KEYPAD_DDR |= (0X80 >> col);
}

uint8_t keypad_isRowLow(uint8_t row) {
	return !(KEYPAD_PIN & (0X10>>row));
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
				// Return pressed key identifier.
				// Do not check if other keys are pressed.
				return (r*3+c)+1;
			}
		}
	}

	// Indicate no key pressed
	return 0;
}


int main(void) {
	
	initLcd();

	buzz();

	uint8_t key, lastKey = 0;
	
	while (1) {
		_delay_ms(50);
		
		lcd_clrscr();

		key = getKeyPressed();

		lcd_putc('0' + (key % 10));
		lcd_gotoxy(0, 1);
		lcd_putc('0' + readMotion());
		
		if (key && key != lastKey) {
			buzz();
		}
		
		lastKey = key;
	}
}
