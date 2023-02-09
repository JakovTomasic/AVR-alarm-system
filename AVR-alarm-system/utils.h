#ifndef UTILS_H_
#define UTILS_H_

#define MOTION_PIN 			PINA
#define MOTION_PORT			PORTA
#define MOTION_PIN_NUMBER	0

// TODO: make faster using define (and activate pull-up resistor only on init)
extern uint8_t readMotion(void);


#endif /* UTILS_H_ */