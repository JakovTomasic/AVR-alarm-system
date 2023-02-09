
#include <avr/io.h>

#include "door.h"

void initDoor(void) {
	
	DDRD |= (1<<PD5);	/* Make OC1A pin as output */
	TCNT1 = 0;			/* Set timer1 count zero */
	ICR1 = 2499;		/* Set TOP count for timer1 in ICR1 register */
	OCR1A = SERVO_OPEN_OCR;

	/* Set Fast PWM, TOP in ICR1, Clear OC1A on compare match, clk/64 */
	TCCR1A = (1<<WGM11)|(1<<COM1A1);
	TCCR1B = (1<<WGM12)|(1<<WGM13)|(1<<CS10)|(1<<CS11);
}

void openDoor(void) {
	OCR1A = SERVO_OPEN_OCR;
}

void closeDoor(void) {
	OCR1A = SERVO_CLOSED_OCR;
}
