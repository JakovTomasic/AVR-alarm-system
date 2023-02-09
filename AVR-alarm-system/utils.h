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

#define POLICE_1_DDR 			DDRC
#define POLICE_1_PORT			PORTC
#define POLICE_1_PIN_NUMBER		1

#define POLICE_2_DDR 			DDRC
#define POLICE_2_PORT			PORTC
#define POLICE_2_PIN_NUMBER		6

extern void initUtils(void);

extern uint8_t readMotion(void);

extern void buzz(void);

extern void tripleBuzz(void);

extern void startPolice(void);
extern void stopPolice(void);
extern void togglePolice(void);

#endif /* UTILS_H_ */