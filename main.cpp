#define F_CPU 8000000UL			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include Delay header file */
#include <stdlib.h>

#define LCD_Dir  DDRB			/* Define LCD data port direction */
#define LCD_Port PORTB			/* Define LCD data port */
#define RS PB0				/* Define Register Select pin */
#define EN PB1 				/* Define Enable signal pin */

#define KEY_PRT 	PORTD
#define KEY_DDR		DDRD
#define KEY_PIN		PIND

unsigned char keypad[4][3] = {	{'1','2','3'},
								{'4','5','6'},
								{'7','8','9'},
								{'*','0','#'}};

unsigned char colloc, rowloc;

int maxR,maxG,maxB,minR,minG,minB;
