#include "main.h"
/*PORT B4 to B7 Data lines to LCD.
 * 
  B0-RS pin
  B1-EN pin*/


/*------clear LCD--------*/
void lcd_clr() {
	lcd_cmd(0X01);
	_delay_ms(5);	
}

/*--------initialization of 16X2 LCD--------*/
void lcd_init(void) {

	lcd_cmd(0x02);						// to initialize LCD in 4-bit mode.
	_delay_ms(5);
	lcd_cmd(0x28);						//to initialize LCD in 2 lines, 5X7 dots and 4bit mode.
	_delay_ms(5);
	lcd_cmd(0x0C);
	_delay_ms(5);
	lcd_cmd(0x06);	
	_delay_ms(5);
	lcd_cmd(0x80);
	_delay_ms(5);
	return;
}

/*-----LCD RS RW and EN Setting/Resetting--------*/
void lcd_cmd(unsigned char cmd) {
	unsigned char cmd1;

	cmd1 = cmd & 0xF0;					//mask lower nibble because PA4-PA7 pins are used. 
	PORTA = cmd1;						// send to LCD

	PORTC &= ~(1<<RS);
	PORTC |= (1<<EN);
	_delay_ms(5);
	PORTC &= ~(1<<EN);

	cmd1 = ((cmd<<4) & 0xF0);			//shift 4-bit and mask
	PORTA = cmd1;						// send to LCD

	PORTC &= ~(1<<RS);
	PORTC |= (1<<EN);
	_delay_ms(5);
	PORTC &= ~(1<<EN);

	return;	
}

/*--------LCD to set for writing data------*/
void lcd_write(unsigned char data) {
	unsigned char data1;

	data1 = data & 0xF0;				//Sending Higher Nibble to LCD.
	PORTA = data1;

	PORTC |= (1<<RS);
	PORTC |= (1<<EN);
	_delay_ms(5);
	PORTC &= ~(1<<EN);

	data1 = ((data<<4) & 0xF0);			//Sending Lower nibble to LCD.
	PORTA = data1;

	PORTC |= (1<<RS);
	PORTC |= (1<<EN);
	_delay_ms(5);
	PORTC &= ~(1<<EN);

	return ;
}

/*------sending data to LCD--------*/
void lcd_write_str(char *data){
	int i;
	for(i=0;i<strlen(data);i++) {
		lcd_write(data[i]);
	}
}

/*------sending int data to LCD--------*/
void lcd_write_int( long int num) {
	char s[6];
	
	ltoa(num, s, 10);
	lcd_write_str(s);
	return ;
}

void LCDGotoXY(unsigned char x, unsigned char y)
{
	volatile unsigned char ddram_addr;
	ddram_addr=0x80;			//initialize data ram address to 0 (default)
	if (y==1) ddram_addr=0xC0;  //start print at 2nd line, DDRAM address 0x40
	lcd_cmd(ddram_addr+ (x&0x7F) );
}