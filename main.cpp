



#define F_CPU 8000000UL			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include Delay header file */
//#include <string.h>
#include "LCD16X2_4BIT.h"
#include "keypad.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

void ADC_Init()
{
	DDRC=0x0;			/* Make ADC port as input */
	ADCSRA = 0x87;			/* Enable ADC, fr/128  */
	ADMUX = 0x40;			/* Vref: Avcc, ADC channel: 0 */
	
}

int ADC_Read(char channel)
{
	int Ain,AinLow;
	
	ADMUX=ADMUX|(channel & 0x0f);	/* Set input channel to read */

	ADCSRA |= (1<<ADSC);		/* Start conversion */
	while((ADCSRA&(1<<ADIF))==0);	/* Monitor end of conversion interrupt */
	
	_delay_us(10);
	AinLow = (int)ADCL;		/* Read lower byte*/
	Ain = (int)ADCH*256;		/* Read higher 2 bits and 
					Multiply with weight */
	Ain = Ain + AinLow;				
	return(Ain);			/* Return digital value*/
}

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
bool validate(int number1,int number2,int number3)
{
	if ((number1>255) || (number2>255) || (number3>255) )
	{
		LCD_Clear();
		LCD_String_xy(0,0, "Enter numbers");
		LCD_String_xy(1,0, "between 0-255");
		_delay_ms(300);
		LCD_Clear();
		return false;
	}
	else
	{
		LCD_Clear();
		LCD_String_xy(0,0, "DONE");
		return true;
	}
}
 void input()
 {
	 double valuer,valueg,valueb;
	 char number[3];
	 bool valid;
 
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
 
	 valid =validate(valuer,valueg,valueb);
	 if (valid == false)
	 {
		 input();
	 }
	 else if (valid == true)
	 {
	 
		 DDRB|=((1<<PORTB3)|(1<<PORTB2));
		 DDRD|=(1<<PORTD3); /*set OC0 pin as output*/
		 OCR1B = valuer;
		 OCR2A =valueg;
		 OCR2B = valueb;
		 TCCR1A = (1<<WGM10) | (1<<COM1B1) ;
		 TCCR1B = (1<<CS10) | (1<<WGM12);
		 TCCR2A = (1<<WGM20) | (1<<WGM21) | (1<<COM2A1)| (1<<COM2B1) ;
		 TCCR2B = (1<<CS20);
	 
	 }
 }
 int values()
 {
	 int num[3];
	 ADC_Init();
	 DDRC = 0b00001110;
	 PORTC = 0b00000010;
	 _delay_ms(100);
	 num[0]=ADC_Read(0);
	 PORTC = 0b00000100;
	 _delay_ms(100);
	 num[1]=ADC_Read(0);
	 PORTC = 0b00001000;
	 _delay_ms(100);
	 num[2]=ADC_Read(0);
	 PORTC = 0b00000000;
	 
	 return *num;
	 
	 
 }

void calibrate()
{
	int r[3],g[3],b[3];
	int rmax,gmax,bmax,rmin,gmin,bmin;
	int rr,rg,rb,gr,gg,gb,br,bg,bb;
	char key;
	LCD_String_xy (0, 0, "RED COLOUR");
	_delay_ms(100);
	LCD_Clear();
	LCD_String_xy (0, 0, "Press 0 to start");
	key = keyfind();
	if (key=='0')
	{
		*r = values();
		rr,rg,rb =r[0],r[1],r[2];
	}
	LCD_Clear();
	LCD_String_xy (0, 0, "RED DONE");
	_delay_ms(100);
	LCD_Clear();
	
	LCD_String_xy (0, 0, "GREEN COLOUR");
	_delay_ms(100);
	LCD_Clear();
	LCD_String_xy (0, 0, "Press 0 to start");
	key = keyfind();
	if (key=='0')
	{
		*g = values();
		gr,gg,gb =g[0],g[1],g[2];
	}
	LCD_Clear();
	LCD_String_xy (0, 0, "GREEN DONE");
	_delay_ms(100);
	LCD_Clear();
	
	LCD_String_xy (0, 0, "BLUE COLOUR");
	_delay_ms(100);
	LCD_Clear();
	LCD_String_xy (0, 0, "Press 0 to start");
	key = keyfind();
	if (key=='0')
	{
		*b = values();
		br,bg,bb =b[0],b[1],b[2];
	}
	LCD_Clear();
	LCD_String_xy (0, 0, "BLUE DONE");
	_delay_ms(100);
	LCD_Clear();

	rmax,gmax,bmax=rr,gg,bb;
	if (br<gr){rmin=br;}
	else{rmin=gr;}
		
	if (rg<bg){gmin=rg;}
	else{gmin=bg;}
		
	if (rb<gb){bmin=rb;}
	else{bmin=gb;}


	
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
			LCD_Clear();
			calibrate();
			_delay_ms(300);	
			}
		else 
			{
			LCD_Clear();
			LCD_String_xy (0, 0,"WRONG INPUT");	
			_delay_ms(300);
			}
		
		}

	}



