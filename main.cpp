



#define F_CPU 8000000UL			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include Delay header file */
//#include <string.h>
#include "LCD16X2_4BIT.h"
#include "keypad.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <avr/interrupt.h>

int rmax,gmax,bmax,rmin,gmin,bmin;
bool calib = false;


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


double getnum()     /* convert the keypad input string to the corresponding 3 digit decimal number*/
{
	long num= 0;
	char key;
	double ikey;
	long mult;
	for (double i=0;i<=2;i++)
	{
		
		key=keyfind();
		ikey = key-'0';     /* get numerical value of the keypad input */
		mult = round(pow(10,(2-i)));
		num+= mult*(ikey);
		
	}
	
	return num;
}


bool validate(int number1,int number2,int number3)
{
	if ((number1>255) || (number2>255) || (number3>255) )   /* Check if the input numbers are less than 255*/
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
		
		LCD_String_xy(1,11, "DONE");
		return true;
	}
}
 
 
 void input()
 {
	
	 double valuer,valueg,valueb;
	 char number[3];
	 bool valid;
	 
	 
	 LCD_String_xy (0, 0, "R=");
	 valuer = getnum();
	 itoa (valuer,number,10);
	 LCD_String_xy (0, 3, number);
	 
	 
	 LCD_String_xy (0, 8, "G=");
	 valueg = getnum();
	 itoa (valueg,number,10);
	 LCD_String_xy (0, 11, number);
 
	 
	 LCD_String_xy (1, 0, "B=");
	 valueb = getnum();
	 itoa (valueb,number,10);
	 LCD_String_xy (1, 3, number);
	 _delay_ms(100);
 
	 
	 valid =validate(valuer,valueg,valueb);
	 if (valid == false)
	 {
		 input();
	 }
	 else if (valid == true)
	 {
		while(1)
		{
			
			TCCR2A = (1<<WGM20) | (1<<WGM21) | (1<<COM2A1) | (1<<COM2B1);   /* Fast PWM mode*/
			TCCR2B = (1<<CS20);
			DDRC =   (1 << PINC4) | (1 <<PINC5);    /* set the PWM pin and selection pins as output*/
			DDRB =  (1 << PINB2) | (1 << PINB3) ;
		
			PORTB = (1<<PORTB2);
			PORTC = (0<<PORTC4) | (0<<PORTC5);
			OCR2A=valuer;
			_delay_ms(5);
			PORTB = (0<<PORTB2);
			PORTC = (1<<PORTC4) | (0<<PORTC5);
			OCR2A=valueg;
			_delay_ms(5);
			PORTB = (0<<PORTB2);
			PORTC = (0<<PORTC4) | (1<<PORTC5);
			OCR2A=valueb;
			_delay_ms(5);
			
		}
	 }
}
 
 
 int values()   /*  Get ADC reading for red, green, blue LED's  into an array */
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


 int minNum(int a, int b)   /* Find the minimum of two numbers */
 {    
	 int min;
	 if(a>b){
		 min=b;
	 }
	 if(a<b){
		 min=a;
	 }
	 return min;
 }


 int maxNum(int a, int b, int c)     /* Find the maximum of three numbers */
 {
	 int max;
	 if(a<b & b<c){
		 max=c;
	 }
	 if(c<a & a<b){
		 max=b;
	 }
	 if(b<c & c<a){
		 max=a;
	 }
	 return max;
 }


void calibrate()
{
	int r[3],g[3],b[3],w[3],bl[3];
	int rR,rG,rB,gR,gG,gB,bR,bG,bB,blR,blG,blB,wR,wG,wB;;
	char key;
	
	/* Calibrating on red surface */
	LCD_String_xy (0, 0, "RED COLOUR");
	_delay_ms(50);
	LCD_Clear();
	LCD_String_xy (0, 0, "Press 0 to start");
	key = keyfind();
	if (key=='0')
	{
		*r = values();
		rR,rG,rB =r[0],r[1],r[2];
	}
	LCD_Clear();
	LCD_String_xy (0, 0, "RED DONE");
	_delay_ms(50);
	LCD_Clear();

	/* Calibrating on green surface */
	LCD_String_xy (0, 0, "GREEN COLOUR");
	_delay_ms(50);
	LCD_Clear();
	LCD_String_xy (0, 0, "Press 0 to start");
	key = keyfind();
	if (key=='0')
	{
		*g = values();
		gR,gG,gB =g[0],g[1],g[2];
	}
	LCD_Clear();
	LCD_String_xy (0, 0, "GREEN DONE");
	_delay_ms(50);
	LCD_Clear();

	/* Calibrating on blue surface */
	LCD_String_xy (0, 0, "BLUE COLOUR");
	_delay_ms(50);
	LCD_Clear();
	LCD_String_xy (0, 0, "Press 0 to start");
	key = keyfind();
	if (key=='0')
	{
		*b = values();
		bR,bG,bB =b[0],b[1],b[2];
	}
	LCD_Clear();
	LCD_String_xy (0, 0, "BLUE DONE");
	_delay_ms(50);
	LCD_Clear();

	/* Calibrating on white surface */
	LCD_String_xy (0, 0, "WHITE COLOUR");
	_delay_ms(50);
	LCD_Clear();
	LCD_String_xy (0, 0, "Press 0 to start");
	key = keyfind();
	if (key=='0')
	{
		*w = values();
		wR,wG,wB =w[0],w[1],w[2];
	}
	LCD_Clear();
	LCD_String_xy (0, 0, "WHITE DONE");
	_delay_ms(50);
	LCD_Clear();

	/* Calibrating on black surface */
	LCD_String_xy (0, 0, "BLACK COLOUR");
	_delay_ms(50);
	LCD_Clear();
	LCD_String_xy (0, 0, "Press 0 to start");
	key = keyfind();
	if (key=='0')
	{
		*bl = values();
		blR,blG,blB =bl[0],bl[1],bl[2];
	}
	LCD_Clear();
	LCD_String_xy (0, 0, "BLACK DONE");
	_delay_ms(50);
	LCD_Clear();
	rmax=minNum(rR,wR);    /* Get the minimum of the maximum readings */
	gmax=minNum(gG,wG);
	bmax=minNum(bB,wB);

	rmin=maxNum(rG,rB,blR);    /* Get the maximum of the minimum readings */
	gmin=maxNum(gR,gB,blG);
	bmin=maxNum(bR,bG,blB);

calib= true;    /* calibrated set to true */
}


void sense()
{
	int delay =100,value,rr,gg,bb;
	char rs[5],gs[5],bs[5],rrf[5],ggf[5],bbf[5];
	int rsi,gsi,bsi;
	
	LCD_Clear();
	LCD_String("SENSING...");

	DDRC = 0b00001110;
	PORTC = 0b00000010;
	_delay_ms(delay);
	rsi=ADC_Read(0);
	itoa(rsi,rs,10);

	PORTC = 0b00000100;
	_delay_ms(delay);
	gsi=ADC_Read(0);
	itoa(gsi,gs,10);


	PORTC = 0b00001000;
	_delay_ms(delay);
	bsi=ADC_Read(0);
	itoa(bsi,bs,10);
	
	PORTC = 0b00000000;
	
    /* mapping the sensor readings */
	rr= (rsi-rmin)*255/(rmax-rmin);
	if (rr>255)
	{
		rr = 255;
	}
	else if (rr<0)
	{
		rr = 0;
	}
	itoa(rr,rrf,10);
	gg= (gsi-gmin)*255/(gmax-gmin);
	if (gg>255)
	{
		gg = 255;
	}
	else if (gg<0)
	{
		gg = 0;
	}
	itoa(gg,ggf,10);
	bb= (bsi-bmin)*255/(bmax-bmin);
	if (bb>255)
	{
		bb = 255;
	}
	else if (bb<0)
	{
		bb = 0;
	}
	itoa(bb,bbf,10);

	/* showing the outputs in the LCD Display */
	LCD_Clear();
	LCD_String("R- ");
	LCD_String(rrf);
	LCD_String("    ");
	LCD_String("G- ");
	LCD_String(ggf);
	LCD_Command(0xC0);
	LCD_String("B- ");
	LCD_String(bbf);
	LCD_String("  ");
	
	while(1)
	{
		
		TCCR2A = (1<<WGM20) | (1<<WGM21) | (1<<COM2A1) | (1<<COM2B1);   /* Fast PWM mode*/
		TCCR2B = (1<<CS20);
		DDRC =   (1 << PINC4) | (1 <<PINC5);    /* set the PWM pin and selection pins as output*/
		DDRB =  (1 << PINB2) | (1 << PINB3) ;
		
		PORTB = (1<<PORTB2);
		PORTC = (0<<PORTC4) | (0<<PORTC5);
		OCR2A=rr;
		_delay_ms(5);
		PORTB = (0<<PORTB2);
		PORTC = (1<<PORTC4) | (0<<PORTC5);
		OCR2A=gg;
		_delay_ms(5);
		PORTB = (0<<PORTB2);
		PORTC = (0<<PORTC4) | (1<<PORTC5);
		OCR2A=bb;
		_delay_ms(5);
		
	}
	
	
}

	
int main(void)
{
	    EICRA |= 0b00001100;   /* set interrupt  */
		EIMSK |= 0b00000010;
		sei();	
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
			if (calib==true)
				{
				sense();
				_delay_ms(1000);
				}
			else
				{
				LCD_Clear();
				LCD_String_xy (0, 0,"NOT CALIBRATED");	
				_delay_ms(300);		
				}
				
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


ISR (INT1_vect)     /* Interrupt Service Routine */
{
	/* turn off the LEDs and return to main*/
	PORTC &= 0b11000001;
	PORTB &= ~(1<<PORTB2); 
	PORTB &= ~(1<<PORTB3); 
	main();	
}

