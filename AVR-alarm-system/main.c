
# define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "lcd.h"

#define KEYPAD_PORT PORTA
#define KEYPAD_DDR 	DDRA
#define KEYPAD_PIN 	PINA

#define MOTION_PIN 			PIND
#define MOTION_PIN_NUMBER	0

#define BUZZER_DDR 			DDRA
#define BUZZER_PORT			PORTA
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
	return bit_is_set(MOTION_PIN, MOTION_PIN_NUMBER);
}

void buzz() {
	BUZZER_DDR |= _BV(BUZZER_PIN_NUMBER);
	BUZZER_PORT |= _BV(BUZZER_PIN_NUMBER);
	_delay_ms(200);
	BUZZER_PORT &= ~(_BV(BUZZER_PIN_NUMBER));
}

uint8_t getKeyPressed()
{
	uint8_t r, c, reversedC;

	// Activate pull-up resistor for row pins.
	// Do not set col pins (assigning zeros would set it)
	KEYPAD_PORT |= 0X0F;

	for(reversedC = 0; reversedC < 3 ; reversedC++)
	{
		c = 2 - reversedC;
		
		// Set all pins as input (pin 7 is not used so ignore it)
		KEYPAD_DDR &= 0X80;

		// Set only current col pin as output.
		// PORT is always 0 for that pins so the output will be low (sink)
		KEYPAD_DDR |= (0X40 >> c);
		for(r = 0; r < 4; r++)
		{
			if(!(KEYPAD_PIN & (0X08>>r)))
			{
				return (r*3+c)+1;
			}
		}
	}

	//Indicate No keypressed
	return 0;
}


int main(void) {
	
	initLcd();

	buzz();

	uint8_t lastKey = 0;

	while (1) {
		_delay_ms(50);
		
		lcd_clrscr();

		uint8_t key = getKeyPressed();

		lcd_putc('0' + (key % 10));
		lcd_gotoxy(0, 1);
		lcd_putc('0' + (readMotion() % 10));
		
		if (key && key != lastKey) {
			buzz();
		}
		
		lastKey = key;
	}
}
