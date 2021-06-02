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


void LCD_Command( unsigned char cmnd )
{
	LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0); /* sending upper nibble */
	LCD_Port &= ~ (1<<RS);		/* RS=0, command reg. */
	LCD_Port |= (1<<EN);		/* Enable pulse */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4);  /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}


void LCD_Char( unsigned char data )
{
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0); /* sending upper nibble */
	LCD_Port |= (1<<RS);		/* RS=1, data reg. */
	LCD_Port|= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (data << 4); /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}

void LCD_Init (void)			/* LCD Initialize function */
{
	LCD_Dir = 0xFF;			/* Make LCD port direction as o/p */
	_delay_ms(20);			/* LCD Power ON delay always >15ms */
	
	LCD_Command(0x02);		/* send for 4 bit initialization of LCD  */
	LCD_Command(0x28);              /* 2 line, 5*7 matrix in 4-bit mode */
	LCD_Command(0x0c);              /* Display on cursor off*/
	LCD_Command(0x06);              /* Increment cursor (shift cursor to right)*/
	LCD_Command(0x01);              /* Clear display screen*/
	_delay_ms(2);
}


void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char col, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && col<16)
	LCD_Command((col & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && col<16)
	LCD_Command((col & 0x0F)|0xC0);	/* Command of first row and required position<16 */
	LCD_String(str);		/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);		/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);		/* Cursor at home position */
}


char keyfind()
{
	while(1)
	{
		KEY_DDR = 0xF0;           /* set port direction as input-output */
		KEY_PRT = 0xF7;

		do
		{
			KEY_PRT &= 0x07;      /* mask PORT for column read only */
			asm("NOP");
			colloc = (KEY_PIN & 0x07); /* read status of column */
		}while(colloc != 0x07);
		
		do
		{
			do
			{
				_delay_ms(20);             /* 20ms key debounce time */
				colloc = (KEY_PIN & 0x07); /* read status of column */
				}while(colloc == 0x07);        /* check for any key press */
				
				_delay_ms (40);	            /* 20 ms key debounce time */
				colloc = (KEY_PIN & 0x07);
			}while(colloc == 0x07);

			/* now check for rows */
			KEY_PRT = 0xE7;            /* check for pressed key in 1st row */
			asm("NOP");
			colloc = (KEY_PIN & 0x07);
			if(colloc != 0x07)
			{
				rowloc = 0;
				break;
			}

			KEY_PRT = 0xD7;		/* check for pressed key in 2nd row */
			asm("NOP");
			colloc = (KEY_PIN & 0x07);
			if(colloc != 0x07)
			{
				rowloc = 1;
				break;
			}
			
			KEY_PRT = 0xB7;		/* check for pressed key in 3rd row */
			asm("NOP");
			colloc = (KEY_PIN & 0x07);
			if(colloc != 0x07)
			{
				rowloc = 2;
				break;
			}

			KEY_PRT = 0x77;		/* check for pressed key in 4th row */
			asm("NOP");
			colloc = (KEY_PIN & 0x07);
			if(colloc != 0x07)
			{
				rowloc = 3;
				break;
			}
		}

		if(colloc == 0x06)
		return(keypad[rowloc][0]);
		else if(colloc == 0x05)
		return(keypad[rowloc][1]);
		else if(colloc == 0x03)
		return(keypad[rowloc][2]);
		else
		return(keypad[rowloc][3]);
	} 


 int minNum(int a, int b){
	 int min;
	 if(a>b){
		 min=b;
	 }
	 if(a<b){
		 min=a;
	 }
	 return min;
 }


 int maxNum(int a, int b, int c){
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


 void Calibrate()
 {
	 DDRC=0b00001110;
	 ADC_Init();
	 char num;
	 int rR,rG,rB,gR,gG,gb,bR,bG,bB,blR,blG,blB,wR,wG,wB;
	 
	 LCD_String("RED");
	 _delay_ms(100);
	 LCD_Clear();
	 LCD_String("Press 0 to start");
	 _delay_ms(100);
	 LCD_Clear();
	 num= keyfind();
	 if(num=='0'){
		 PORTC=0b00000010;
		 _delay_ms(200);
		 rR=ADC_Read(0);
		 PORTC=0b00000100;
		 _delay_ms(200);
		 rG=ADC_Read(0);
		 PORTC=0b00001000;
		 _delay_ms(200);
		 rB=ADC_Read(0);
		 PORTC=0b00000000;
	 }
	 LCD_String("RED Done");
	 _delay_ms(100);
	 LCD_Clear();

	 
	 LCD_String("Green");
	 _delay_ms(100);
	 LCD_Clear();
	 LCD_String("Press 0 to start");
	 _delay_ms(100);
	 LCD_Clear();
	 num= keyfind();
	 if(num=='0'){
		 PORTC=0b00000010;
		 _delay_ms(200);
		 gR=ADC_Read(0);
		 PORTC=0b00000100;
		 _delay_ms(200);
		 gG=ADC_Read(0);
		 PORTC=0b00001000;
		 _delay_ms(200);
		 gB=ADC_Read(0);
		 PORTC=0b00000000;
	 }
	 LCD_String("GREEN Done");
	 _delay_ms(100);
	 LCD_Clear();
	 
	 
	 LCD_String("BLUE");
	 _delay_ms(100);
	 LCD_Clear();
	 LCD_String("Press 0 to start");
	 _delay_ms(100);
	 LCD_Clear();
	 num= keyfind();
	 if(num=='0'){
		 PORTC=0b00000010;
		 _delay_ms(200);
		 bR=ADC_Read(0);
		 PORTC=0b00000100;
		 _delay_ms(200);
		 bG=ADC_Read(0);
		 PORTC=0b00001000;
		 _delay_ms(200);
		 bB=ADC_Read(0);
		 PORTC=0b00000000;
	 }
	 LCD_String("BLUE Done");
	 _delay_ms(100);
	 LCD_Clear();
	 
	 
	 LCD_String("BLACK");
	 _delay_ms(100);
	 LCD_Clear();
	 LCD_String("Press 0 to start");
	 _delay_ms(100);
	 LCD_Clear();
	 num= keyfind();
	 if(num=='0'){
		 PORTC=0b00000010;
		 _delay_ms(200);
		 blR=ADC_Read(0);
		 PORTC=0b00000100;
		 _delay_ms(200);
		 blG=ADC_Read(0);
		 PORTC=0b00001000;
		 _delay_ms(200);
		 blB=ADC_Read(0);
		 PORTC=0b00000000;
	 }
	 LCD_String("BLACK Done");
	 _delay_ms(100);
	 LCD_Clear();
	 
	 
	 LCD_String("WHITE");
	 _delay_ms(100);
	 LCD_Clear();
	 LCD_String("Press 0 to start");
	 _delay_ms(100);
	 LCD_Clear();
	 num= keyfind();
	 if(num=='0'){
		 PORTC=0b00000010;
		 _delay_ms(200);
		 wR=ADC_Read(0);
		 PORTC=0b00000100;
		 _delay_ms(200);
		 wG=ADC_Read(0);
		 PORTC=0b00001000;
		 _delay_ms(200);
		 wB=ADC_Read(0);
		 PORTC=0b00000000;
	 }
	 LCD_String("WHITE Done");
	 _delay_ms(100);
	 LCD_Clear();
	 
	 maxR=minNum(rR,wR);
	 maxG=minNum(gG,wG);
	 maxB=minNum(bB,wB);
	 
	 minR=maxNum(rG,rB,blR);
	 minG=maxNum(gR,gB,blG);
	 minB=maxNum(bR,bG,blB);
	 
}


void Sense(){
	int rVal, gVal, bVal,R,G,B;
	DDRC=0b00001110;
	ADC_Init();
	PORTC=0b00000010;
	_delay_ms(200);
	rVal=ADC_Read(0);
	PORTC=0b00000100;
	_delay_ms(200);
	gVal=ADC_Read(0);
	PORTC=0b00001000;
	_delay_ms(200);
	bVal=ADC_Read(0);
	PORTC=0b00000000;
	R=(rVal*255)/(maxR-minR);
	G=(gVal*255)/(maxG-minG);
	B=(bVal*255)/(maxB-minB);

	if(R>255){
		R=255;
	}
	
	if(G>255){
		g=255;
	}
	
	if(B>255){
		B=255;
	}
	
	if(R<255){
		R=0;
	}
	
	if(G<255){
		G=0;
	}
	
	if(B<255){
		B=0;
	}
	
	DDRB|=((1<<PORTB3)|(1<<PORTB2));
	DDRD|=(1<<PORTD3);
	OCR1B=R;
	OCR2A=G;
	OCR2B=B;
	TCCR1A=(1<<WGM10)|(1<<COM1B1);
	TCCR1B=(1<<CS10)|(1<<WGM12);
	TCCR2A=(1<<WGM20)|(1<<WGM21)|(1<<COM2A1)|(1<<COM2B1);
	TCCR2B=(1<<CS20);
	
}


int main(void)
{
	LCD_Init();
	LCD_String("Welcome!");
	_delay_ms(100);
	LCD_Clear();
	char Num;
	
	while(1)
	{
		LCD_String("Select a mode");
		_delay_ms(100);
		LCD_Clear();
		LCD_String_xy(0,0,"1-Calibrate");
		LCD_String_xy(1,0,"2-Sense 3-Mode 3");
		_delay_ms(200);
		LCD_Clear();
		
		Num=keyfind();
		
		if (Num=='1'){
			LCD_String("Calibrate");
			_delay_ms(100);
			LCD_Clear();
			Calibrate();
		}
		
		if(Num=='2'){
			LCD_String("Sense");
			_delay_ms(100);
			LCD_Clear();
			Sense();
		}
		
		if(Num=='3'){
			LCD_String('Input');
			_delay_ms(100);
			LCD_Clear();
			
		}
	}
}
