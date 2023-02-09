#ifndef DOOR_H_
#define DOOR_H_


// TODO: izracunaj prave vrijednosti i postavi
#define SERVO_OPEN_OCR 175
#define SERVO_CLOSED_OCR 300

extern void initDoor(void);

extern void openDoor(void);

extern void closeDoor(void);

#endif /* DOOR_H_ */