#ifndef DOOR_H_
#define DOOR_H_


// TOP = f_CPU / (F * N) - 1
// TOP = 7372800UL / (50 * 64) - 1
// -90 degrees is approximately 0.5ms duty cycle -> OCR = 57
//	90 degrees is approximately 2.4ms duty cycle -> OCR = 277
#define SERVO_PWM_TOP 2303
#define SERVO_OPEN_OCR 277
#define SERVO_CLOSED_OCR 57

extern void initDoor(void);

extern void openDoor(void);

extern void closeDoor(void);

#endif /* DOOR_H_ */