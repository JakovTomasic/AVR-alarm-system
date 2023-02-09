#ifndef UTILS_H_
#define UTILS_H_

#define MOTION_PIN 			PINA
#define MOTION_PORT			PORTA
#define MOTION_PIN_NUMBER	0

#define BUZZER_DDR 			DDRC
#define BUZZER_PORT			PORTC
#define BUZZER_PIN_NUMBER	7
#define BUZZER_DURATION_MS	200
#define BUZZER_TRIPLE_STEP_DURATION_MS	65


// TODO: make faster using define (and activate pull-up resistor only on init)
extern uint8_t readMotion(void);

extern void buzz(void);

extern void tripleBuzz(void);

#endif /* UTILS_H_ */