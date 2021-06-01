#define F_CPU 8000000UL			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include Delay header file */
#include <stdlib.h>

#define LCD_Dir  DDRB			/* Define LCD data port direction */
#define LCD_Port PORTB			/* Define LCD data port */
#define RS PB0				/* Define Register Select pin */
#define EN PB1 				/* Define Enable signal pin */
#define r PINC0
#define g PINC1
#define b PINC2

#include "LCD16x2_4bit.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define KEY_PRT PORTD
#define KEY_DDR	DDRD
#define KEY_PIN	PIND

int a = 0x80;
char rl[5];
char gl[5];
char bl[5];
char rh[5];
char gh[5];
char bh[5];
int rli;
int gli;
int bli;
int rhi;
int ghi;
int bhi;
int val[3][3];
int rm;
int gm;
int bm;

char key;
unsigned char keypad[4][3] = {	{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'*','0','#'}};
unsigned char colloc, rowloc;


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

void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);	/* Command of first row and required position<16 */
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
	a+=1;
	while(1)
	{
		KEY_DDR = 0xF0;           /* set port direction as input-output */
		KEY_PRT = 0xFF;

		do
		{
			KEY_PRT &= 0x0F;      /* mask PORT for column read only */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F); /* read status of column */
		}while(colloc != 0x0F);
		
		do
		{
			do
			{
				_delay_ms(20);             /* 20ms key debounce time */
				colloc = (KEY_PIN & 0x0F); /* read status of column */
				}while(colloc == 0x0F);        /* check for any key press */
				
				_delay_ms (40);	            /* 20 ms key debounce time */
				colloc = (KEY_PIN & 0x0F);
			}while(colloc == 0x0F);

			/* now check for rows */
			KEY_PRT = 0xEF;            /* check for pressed key in 1st row */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 0;
				break;
			}

			KEY_PRT = 0xDF;		/* check for pressed key in 2nd row */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 1;
				break;
			}
			
			KEY_PRT = 0xBF;		/* check for pressed key in 3rd row */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 2;
				break;
			}

			KEY_PRT = 0x7F;		/* check for pressed key in 4th row */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 3;
				break;
			}
		}

		if(colloc == 0x0E)
		return(keypad[rowloc][0]);
		else if(colloc == 0x0D)
		return(keypad[rowloc][1]);
		else if(colloc == 0x0B)
		return(keypad[rowloc][2]);
		else
		return(keypad[rowloc][3]);
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

void sense(){
	int delay =100,value,rr,gg,bb;
	char rs[5],gs[5],bs[5],rrf[5],ggf[5],bbf[5];
	int rsi,gsi,bsi;
	
	LCD_Clear();
	LCD_String("Sensing...");
	
	PORTC = (PORTC | 16);
	_delay_ms(delay);
	rsi=ADC_Read(0);
	itoa(rsi,rs,10);
	
	PORTC = (PORTC | 8);
	PORTC = (PORTC & 0b11101111);
	_delay_ms(delay);
	gsi=ADC_Read(0);
	itoa(gsi,gs,10);


	PORTC = (PORTC | 4);
	PORTC = (PORTC & 0b11110111);
	_delay_ms(delay);
	bsi=ADC_Read(0);
	itoa(bsi,bs,10);
	PORTC = (PORTC & 0b11111011);
	
	rr= (rsi-rm)*255/(val[0][0]-rm);
	itoa(rr,rrf,10);
	gg= (gsi-gm)*255/(val[1][1]-gm);
	itoa(gg,ggf,10);
	bb= (bsi-bm)*255/(val[2][2]-bm);
	itoa(bb,bbf,10);
	
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
}

void rgbl(int valuer,int valueg,int valueb){
	DDRB|=((1<<PORTB3)|(1<<PORTB2));
	DDRD|=(1<<PORTD3); //set OC0 pin as output/
	OCR1B = valuer;
	OCR2A =valueg;
	OCR2B = valueb;
	TCCR1A = (1<<WGM10) | (1<<COM1B1) ;
	TCCR1B = (1<<CS10) | (1<<WGM12);
	TCCR2A = (1<<WGM20) | (1<<WGM21) | (1<<COM2A1)| (1<<COM2B1) ;
	TCCR2B = (1<<CS20);
}

void test(){
	char in[3]={'.','.','.'};
	char ou[2];
	int rgb[2];

	LCD_Clear();
	LCD_String("Sensing...");
	_delay_ms(100);
	LCD_Clear();
	for(int j=0; j<3 ;j++){
		LCD_Clear();
		if (j==0){
			LCD_String("Enter RED");
		}else if(j==1){
			LCD_String("Enter GREEN");
		}else if(j==2){
			LCD_String("Enter BLUE");
		}
		LCD_Command(0xC0);
		LCD_String(in);
		for(int i=0; i<3 ;i++){
			key=keyfind();
			in[i]=key;
			LCD_Command(0xC0);
			LCD_String(in);
			rgb[j]=atoi(in);
			if (rgb[j]<0 || rgb[j]>255){
				LCD_Clear();
				LCD_String("Invalid value");
				_delay_ms(50);
				LCD_Clear();
				j-=1;
			}
		}
		in[0]='.';
		in[1]='.';
		in[2]='.';
		in[3]=' ';
	}
	rgbl(rgb[0],rgb[1],rgb[2]);
}
	
	
void ADC_Init()
{
	ADCSRA = 0b10000111;      // C1:: Enable ADC, pre-scaler = 128
	ADMUX  = 0b01000000;	
}

 void cali(){
	char col[3]={'r','g','b'};
	int led1[3]={16,8,4};
	int led2[4]={255,239,247,251};
	int delay=100;
	char re[3];
	for(int j=0; j<3 ;j++){
		LCD_Clear();
		LCD_String("Enter ");
		if (col[j]=='r'){
			LCD_String("RED");
		}else if (col[j]=='g'){
			LCD_String("GREEN");
		}else {
			LCD_String("BLUE");
		}
		keyfind();
		for(int i=0; i<3 ;i++){
			PORTC = (PORTC | led1[i]);
			PORTC = (PORTC & led2[i]);
			_delay_ms(delay);
			val[j][i]=ADC_Read(0);
		}
		PORTC = (PORTC & led2[3]);
		LCD_Clear();
		itoa(val[0][0],re,10);
		LCD_String(re);
	}
	if (val[1][0]<val[2][0]){
		rm=val[2][0];
	}else{
		rm=val[1][0];
	}
	if (val[0][1]<val[2][1]){
		gm=val[2][1];
	}else{
		gm=val[0][1];
	}
	if (val[0][2]<val[1][2]){
		bm=val[1][2];
		}else{
		bm=val[0][2];
	}
}

 
int main()
{
	ADC_Init();
	LCD_Init();
	LCD_String("   RGB Sensor");	/* Write string on 1st line of LCD*/
	LCD_Command(0xC0);		/* Go to 2nd line*/
	LCD_String("    Group 29");	/* Write string on 2nd line*/
	_delay_ms(50);
	LCD_Clear();
	LCD_String("1-Cali 2-Sensing");	/* Write string on 1st line of LCD*/
	LCD_Command(0xC0);		/* Go to 2nd line*/
	LCD_String("3-Testing");	/* Write string on 2nd line*/
	while(1)
	{
		LCD_Command(a);
		key=keyfind();
		if (key=='1'){
			cali();
		}else if (key=='2'){
			sense();
		}else if (key=='3'){
			test();
	}
	}
}
 
 
 
