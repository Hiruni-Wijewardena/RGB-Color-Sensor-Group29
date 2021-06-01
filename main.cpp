



#define F_CPU 8000000UL			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include Delay header file */
//#include <string.h>
#include "LCD16X2_4BIT.h"
#include "keypad.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


double getnum()
{
	double num= 0;
	char key;
	double ikey;
	double mult;
	
	for (double i=0;i<=2;i++)
	{
		
		key=keyfind();
		ikey = key-'0';
		mult = pow(10,(2-i));
		num+= mult*(ikey);
		
	}
	
	return num;
}
 void input()
 {
 double valuer,valueg,valueb;
 char number[3];
 
 
 valuer = getnum();
 
 itoa (valuer,number,10);
 LCD_String_xy (0, 0, "r=");
 LCD_String_xy (0, 3, number);
 
 valueg = getnum();
 
 itoa (valueg,number,10);
 LCD_String_xy (0, 8, "g=");
 LCD_String_xy (0, 11, number);
 
 valueb = getnum();
 itoa (valueb,number,10);
 LCD_String_xy (1, 0, "b=");
 LCD_String_xy (1, 3, number);
 _delay_ms(100);
 }

void calibrate()
{
	
}

void sense()
{
	
	
}

	
int main(void)
{
		LCD_Init();
		LCD_String_xy (0, 5,"WELCOME!");
		LCD_String_xy (1, 0,"Select a mode");
		
		_delay_ms(100);
		LCD_Clear();
		char mode;
		
		while(1)
		{
		LCD_String_xy (0, 0,"1-INPUT  2-SENSE");
		LCD_String_xy (1, 0,"3-CALIBRATE");
			
		mode =	keyfind();
		if (mode == '1')
			{
			LCD_Clear();
			input();
			_delay_ms(300);
			}
		else if (mode =='2')
			{
			sense();
			}	
			
		else if (mode == '3')
			{
			calibrate();	
			}
		else 
			{
			LCD_Clear();
			LCD_String_xy (0, 0,"WRONG INPUT");	
			_delay_ms(300);
			}
		
		}

	}



