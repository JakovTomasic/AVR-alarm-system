#ifndef KEYPAD_H_
#define KEYPAD_H_


#define KEYPAD_PORT			PORTA
#define KEYPAD_DDR 			DDRA
#define KEYPAD_PIN 			PINA

#define KEY_STAR			10
#define KEY_HASH			11
#define KEY_NONE			12

#define isNumber(key) (key < 10)

extern uint8_t getKeyPressed(void);

// Call this function every few ms
extern uint8_t getKeyPressedDebounce(void);


#endif /* KEYPAD_H_ */