
# define F_CPU 7372800UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h> // TODO: treba li mi ovo?

#include "lcd.h"
//#include "fat.h"

#define KEYPAD_PORT			PORTA
#define KEYPAD_DDR 			DDRA
#define KEYPAD_PIN 			PINA

#define KEY_STAR			10
#define KEY_HASH			11
#define KEY_NONE			12

#define MOTION_PIN 			PINA
#define MOTION_PORT			PORTA
#define MOTION_PIN_NUMBER	0

#define BUZZER_DDR 			DDRC
#define BUZZER_PORT			PORTC
#define BUZZER_PIN_NUMBER	7
/*
 * b7 -> 1 if alarm turned on, 0 if in deactivated state
 * b6 -> 1 if movement detected, 0 otherwise
 * b5 -> special action input mode. 1 if active, 0 otherwise
 * b4 -> 1 if intruder detected, 0 otherwise
 * other bits -> reserved
 */
uint8_t state;

#define DEFAULT_STATE 0x80

// Warning: do not use these for setting state (fix that)
#define alarmOn (state & 0x80)
#define motionDetected (state & 0x40)
#define specialInputActive (state & 0x20)
#define intruderDetected (state & 0x10)


#define PASSWORD_LENGTH 4
const uint16_t password = 1234;

uint8_t enteredDigits[4] = {KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE};

#define resetEnteredDigits enteredDigits[0] = enteredDigits[1] = enteredDigits[2] = enteredDigits[3] = KEY_NONE
#define enteredDigitsValue (enteredDigits[0]*1000 + enteredDigits[1]*100 + enteredDigits[2]*10 + enteredDigits[3])

uint16_t motionDetectionCountdown = 0;

// Utils:

void writeLCD_alignRight(uint16_t val, uint8_t width) {
	uint8_t num;
	char valStr[width+1];
	for (uint8_t i = 1; i <= width; i++) {
		if (val) {
			valStr[width - i] = '0' + (val % 10);
			val /= 10;
		} else {
			valStr[width - i] = ' ';
		}
	}
	valStr[width] = '\0';
	lcd_puts(valStr);
}

// TODO: make faster using define (and activate pull-up resistor only on init)
uint8_t readMotion() {
	// Activate pull-up resistor
	MOTION_PORT |= _BV(MOTION_PIN_NUMBER);
	return bit_is_set(MOTION_PIN, MOTION_PIN_NUMBER);
}

void buzz() {
	BUZZER_DDR |= _BV(BUZZER_PIN_NUMBER);
	BUZZER_PORT |= _BV(BUZZER_PIN_NUMBER);
	_delay_ms(200);
	BUZZER_PORT &= ~(_BV(BUZZER_PIN_NUMBER));
}

// TODO: make faster using define
void keypad_activateRowPullUps() {
	// Do not set col pins (assigning 0 would set it)
	KEYPAD_PORT |= 0X1E;
}

// TODO: make faster using define
void keypad_setAllInput() {
	// One pin is not used so ignore it
	KEYPAD_DDR &= 0X01;
}

// TODO: make faster using define
void keypad_setColAsOutputLow(uint8_t col) {
	// PORTX is always 0 for that pins so the output will be low (sink)
	KEYPAD_DDR |= (0X80 >> col);
}

// TODO: make faster using define
uint8_t keypad_isRowLow(uint8_t row) {
	return bit_is_clear(KEYPAD_PIN, 4 - row);
}

uint8_t getKeyPressed()
{
	uint8_t r, c, reversedC;

	keypad_activateRowPullUps();

	for(reversedC = 0; reversedC < 3 ; reversedC++)
	{
		c = 2 - reversedC;
		
		keypad_setAllInput();

		// Set only current col pin as output.
		keypad_setColAsOutputLow(c);
		for(r = 0; r < 4; r++)
		{
			if(keypad_isRowLow(r))
			{
				// Return pressed key.
				// Do not check if other keys are pressed.
				if (r < 3) {
					return (r*3+c)+1;
				} else if (c == 0) {
					return KEY_STAR;
				} else if (c == 1) {
					return 0;
				} else {
					return KEY_HASH;
				}
			}
		}
	}

	// Indicate no key pressed
	return KEY_NONE;
}

// TODO: make faster using define?
uint8_t isNumber(uint8_t key) {
	return key < 10;
}



// App logic:

void initLcd() {
	
	DDRB |= _BV(3);

	TCCR0 |= _BV(COM01) | _BV(WGM01) | _BV(WGM00) | _BV(CS01);
	OCR0 = 128;
	
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_puts("Starting");
	_delay_ms(200);
	lcd_putc('.');
	_delay_ms(200);
	lcd_putc('.');
	_delay_ms(200);
	lcd_putc('.');
	_delay_ms(200);
}

void writeCurrentStateMessage() {
	if (alarmOn) {
		if (intruderDetected) {
			lcd_clrscr();
			lcd_puts("Intruder!!!");
			lcd_gotoxy(0, 1);
			lcd_puts("Calling police!");
		} else if (motionDetected) {
			lcd_clrscr();
			lcd_puts("I see you moving");
			lcd_gotoxy(0, 1);
			for (uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
				if (enteredDigits[i] == KEY_NONE) {
					break;
				} else {
					lcd_putc('0' + enteredDigits[i]);
				}
			}
		} else {
			lcd_clrscr();
			lcd_puts("Alarm on");
		}
	} else {
		if (specialInputActive) {
			lcd_clrscr();
			lcd_puts("Enter an action");
		} else {
			lcd_clrscr();
			lcd_puts("Alarm off");
		}
	}
}

void refreshState() {
	writeCurrentStateMessage();
	if (alarmOn) {
		if (motionDetected && !intruderDetected) {
			motionDetectionCountdown = 601; // TODO: set to 6001
		} else {
			resetEnteredDigits;
		}
	} else {
		resetEnteredDigits;
	}
}

void handleKeypress(uint8_t key) {
	if (alarmOn && !intruderDetected && key == KEY_HASH) {
		lcd_gotoxy(0, 1);
		lcd_puts("    ");
		resetEnteredDigits;
	} else if (alarmOn && !intruderDetected && isNumber(key)) {
		uint8_t i;
		lcd_gotoxy(0, 1);
		lcd_puts("    ");
		lcd_gotoxy(0, 1);
		for (i = 0; i < PASSWORD_LENGTH; i++) {
			if (enteredDigits[i] == KEY_NONE) {
				enteredDigits[i] = key;
				lcd_putc('0' + enteredDigits[i]);
				break;
			} else {
				lcd_putc('0' + enteredDigits[i]);
			}
		}
		
		if (i == PASSWORD_LENGTH - 1) {
			if (enteredDigitsValue == password) {
				state &= 0x7F;
				state &= 0xBF;
				resetEnteredDigits;
				refreshState();
				} else {
				lcd_clrscr();
				lcd_puts("Wrong password");
				_delay_ms(1000);
				resetEnteredDigits;
				writeCurrentStateMessage();
			}
		}
	} else if (!alarmOn) {
		if (specialInputActive) {
			if (key == 0) {
				state |= 0x80;
				state &= 0x9F;
				refreshState();
			} else if (key == KEY_STAR) {
				state &= ~0x20;
				refreshState();
			}
		} else if (key == KEY_STAR) {
			state |= 0x20;
			refreshState();
		}
	}
}

void checkMotion() {
	if (alarmOn && !motionDetected && readMotion()) {
		state |= 0x40;
		refreshState();
	}
}


/*
int main(void) {
	
	
	initLcd();
	lcd_clrscr();
	
	DIR dir;
	FILE file;
	uint8_t return_code = 0;
	uint8_t dirItems = 0;
	uint8_t* read_buf = 0;
	
	lcd_puts("my start");
	
	// Mount the memory card
	return_code = FAT_mountVolume();
	
	lcd_putc('0' + (return_code % 10));
	
	// If no error
	if(return_code == MR_OK){
		lcd_puts("ok");
		// Open folder
		return_code = FAT_openDir(&dir, "/Logging");
		if(return_code == FR_OK){
			// ... optionally print folder name using an LCD library: print(FAT_getFilename());
			
			// Get number of folders and files inside the directory
			dirItems = FAT_dirCountItems(&dir);
			
			// Open a file for reading or writing
			return_code = FAT_fopen(&dir, &file, "log.txt");
			
			// Read block of 512 bytes until the end-of-file flag is set
			while(FAT_feof(&file) == 0 && FAT_ferror(&file) == 0){
				read_buf = FAT_fread(&file);
				
				// Function included in DIS_ST7735.h LCD library
				DIS_ST7735_drawString((char*)read_buf);
			}
			
			// Move file read pointer back to the beginning (optional)
			FAT_fseek(&file, 0);
		}
		
	}
	
	
	while(1);
	*/
	
	/*
	
	state = DEFAULT_STATE;
	refreshState();

	buzz();

	uint8_t key, lastKey = KEY_NONE;
	
	//configure timer 1 - ctc mode, oc1a/b disconnected, prescaler 8
	//ocr = 9216 -> output compare match f=0.01s
	TCCR1A = 0;
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1A = 9216;
	//enable OC interrupt
	TIMSK |= _BV(OCIE1A);

	sei();
	
	while (1) {
		_delay_ms(50);

		checkMotion();

		key = getKeyPressed();
*/
		/*
		lcd_clrscr();
		lcd_putc('0' + (key % 10));
		lcd_gotoxy(0, 1);
		lcd_putc('0' + readMotion());
		*/
		/*
		if (key != KEY_NONE && key != lastKey) {
			handleKeypress(key);
			buzz();
		}
		
		lastKey = key;
	}
	*/
//}

/*
// TODO: do i need this timer? Maybe start it only on motion detect and stop it if correct password entered
ISR(TIMER1_COMPA_vect)
{
	if (alarmOn && motionDetected && !intruderDetected) {
		if (--motionDetectionCountdown % 100 == 0) {
			lcd_gotoxy(14, 1);
			writeLCD_alignRight(motionDetectionCountdown / 100, 2);
		}
		
		if (motionDetectionCountdown == 0) {
			state |= 0x10;
			refreshState();
		}
	}
}
*/



//Adding all the codes:
#include <avr/iom16.h>
#define FOSC 7372800
char chars[512];


#define DI 4                         // Port B bit 6 (pin7): data in (data from MMC)
#define DT 5                        // Port B bit 5 (pin6): data out (data to MMC)
#define CLK 1                     // Port B bit 7 (pin8): clock
#define CS 6                        // Port B bit 4 (pin5): chip select for MMC
//SPI Initialization:
void ini_SPI() {
	DDRB &= ~(_BV(DI));                     //input
	DDRB |= _BV(CLK);                     //outputs
	DDRB |= _BV(DT);                     //outputs
	DDRB |= _BV(CS);                     //outputs
	SPCR |= _BV(SPE);                     //SPI enable
	SPCR |= _BV(MSTR);                     //Master SPI mode
	SPCR &= ~(_BV(SPR1));                    //fosc/16
	SPCR |= _BV(SPR0);                    //fosc/16
	SPSR &= ~(_BV(SPI2X));                    //speed is not doubled
	PORTB &= ~(_BV(CS));                     //Enable CS pin for the SD card
}
//Functions for sending and receiving one byte through SPI:
char SPI_sendchar(char chr) {
	char receivedchar = 0;
	SPDR = chr;
	while(!(SPSR & (1<<SPIF)));
	receivedchar = SPDR;
	return (receivedchar);
}
//Function to send a command frame Command:
char Command(char cmd, uint16_t ArgH, uint16_t ArgL, char crc ) {
	SPI_sendchar(0xFF);
	SPI_sendchar(cmd);
	SPI_sendchar((uint8_t)(ArgH >> 8));
	SPI_sendchar((uint8_t)ArgH);
	SPI_sendchar((uint8_t)(ArgL >> 8));
	SPI_sendchar((uint8_t)ArgL);
	SPI_sendchar(crc);
	SPI_sendchar(0xFF);
	return SPI_sendchar(0xFF);                // Returns the last byte received
}
uint8_t toBin(uint8_t aa) {
	if (aa) {
		return 1;
	} else {
		return 0;
	}
}
//Initialization card:
void ini_SD(void) {
	char i;
	PORTB |= _BV(CS);                    //disable CS
	for(i=0; i < 10; i++)
		SPI_sendchar(0xFF);                // Send 10 * 8 = 80 clock pulses 400 kHz
	PORTB &= ~(_BV(CS));                 //enable CS
	for(i=0; i < 2; i++)
		SPI_sendchar(0xFF);                // Send 2 * 8 = 16 clock pulses 400 kHz
	Command(0x40,0,0,0x95);              // reset
	lcd_puts("g");
	idle_no:
	lcd_clrscr();
	lcd_puts("g");
	char temp = Command(0x41,0,0,0xFF);
	lcd_puts("T");
	lcd_putc('0' + toBin(temp & _BV(7)));
	lcd_putc('0' + toBin(temp & _BV(6)));
	lcd_putc('0' + toBin(temp & _BV(5)));
	lcd_putc('0' + toBin(temp & _BV(4)));
	lcd_putc('0' + toBin(temp & _BV(3)));
	lcd_putc('0' + toBin(temp & _BV(2)));
	lcd_putc('0' + toBin(temp & _BV(1)));
	lcd_putc('0' + toBin(temp & _BV(0)));
	if (temp !=0)
	//goto idle_no;                        //idle = L?
	lcd_puts("T");
	SPCR &= ~(_BV(SPR0));                //fosc/4
}
//Writing to the card:
//The function returns 1 if an error occurs  else returns 0 if successful
int write(void) {
	int i;
	uint8_t wbr;
	//Set write mode 512 bytes
	if (Command(0x58,0,512,0xFF) !=0) {
		//Determine value of the response byte 0 = no errors
		return 1;
		//return value 1 = error
	}
	SPI_sendchar(0xFF);
	SPI_sendchar(0xFF);
	SPI_sendchar(0xFE);
	//recommended by posting a terminator sequence [2]
	//write data from chars [512] tab
	uint16_t ix;
	char r1 =  Command(0x58,0,512,0xFF);
	for (ix = 0; ix < 50000; ix++) {
		if (r1 == (char)0x00) break;
		r1 = SPI_sendchar(0xFF);
	}
	if (r1 != (char)0x00) {
		return 1;
		//return value 1 = error
	}
	//recommended by the control loop [2]
	SPI_sendchar(0xFF);
	SPI_sendchar(0xFF);
	wbr = SPI_sendchar(0xFF);
	//write block response and testing error
	wbr &= 0x1F;
	//zeroing top three indeterminate bits 0b.0001.1111
	if (wbr != 0x05) { // 0x05 = 0b.0000.0101
		//write error or CRC error
		return 1;
	}
	while(SPI_sendchar(0xFF) != (char)0xFF);
	//wait for the completion of a write operation to the card
	return 0;
}
//Reading from the card:
//The function returns 1 if an error occurs  else returns 0 if successful
int read(void) {
	int i;
	uint16_t ix;
	char r1 =  Command(0x51,0,512,0xFF);
	for (ix = 0; ix < 50000; ix++) {
		if (r1 == (char)0x00) break;
		r1 = SPI_sendchar(0xFF);
	}
	if (r1 != (char)0x00) {
		return 1;
	}
	//read from the card will start after the framework
	while(SPI_sendchar(0xFF) != (char)0xFE);
	for(i=0; i < 512; i++) {
		while(!(SPSR & (1<<SPIF)));
		chars[i] = SPDR;
		SPDR = SPI_sendchar(0xFF);
	}
	SPI_sendchar(0xFF);
	SPI_sendchar(0xFF);
	return 0;
}
int main(void) {
	initLcd();
	lcd_puts("1");
	ini_SPI();
	lcd_puts("2");
	ini_SD();
	lcd_puts("3");
	sei();
	lcd_puts("4");
	write();
	lcd_puts("5");
	read();
	lcd_puts("6");
	
	lcd_puts("jeej");
	
	while(1);
	
	return 0;
}