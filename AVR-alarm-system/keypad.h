#ifndef KEYPAD_H_
#define KEYPAD_H_


#define KEYPAD_PORT			PORTA
#define KEYPAD_DDR 			DDRA
#define KEYPAD_PIN 			PINA

#define KEY_STAR			10
#define KEY_HASH			11
#define KEY_NONE			12

extern uint8_t getKeyPressed(void);

// TODO: make faster using define?
extern uint8_t isNumber(uint8_t key);


#endif /* KEYPAD_H_ */