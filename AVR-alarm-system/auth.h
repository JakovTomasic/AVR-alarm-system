#ifndef AUTH_H_
#define AUTH_H_

#define LOG_SIZE_ADDRESS 1
#define LOG_FIRST_ADDRESS 2

#define PASSWORD_LENGTH 4
#define PASSWORD_USERS_COUNT 4

extern uint8_t adminPassword[4];
extern uint8_t userPasswords[4][4];

uint8_t logSize;
uint8_t logIndex;


extern uint8_t checkEnteredPassword(uint8_t *entered, uint8_t *toCheck);

extern uint8_t getUserIdForEnteredPassword(uint8_t *entered);

extern void initLog(void);

extern void writeLoginToLog(uint8_t userId);

extern void clearLog(void);

extern uint8_t getUserIdFromLog(uint8_t index);



#endif /* AUTH_H_ */