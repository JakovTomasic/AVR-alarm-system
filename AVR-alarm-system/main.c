
# define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

#define KEYPAD_PORT PORTA
#define KEYPAD_DDR 	DDRA
#define KEYPAD_PIN 	PINA

void initLcd() {
	
	DDRD |= _BV(4);

	TCCR1A |= _BV(COM1B1) | _BV(WGM10);
	TCCR1B |= _BV(WGM12) | _BV(CS11);
	OCR1B = 128;

	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_puts("Hello World");
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
				return (r*3+c);
			}
		}
	}

	//Indicate No keypressed
	return 0XFF;
}


int main(void) {
	
	initLcd();

	while (1) {
		_delay_ms(500);
		
		lcd_clrscr();
		lcd_putc('0' + (getKeyPressed() % 10));
	}
}
