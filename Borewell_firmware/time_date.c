/*
 * main.c
 *
 * Created: 2/19/2016 9:35:19 PM
 *  Author: shri
 */ 
#include "main.h"
//#define F_CPU 1000000UL

void LCDGotoXY(unsigned char x, unsigned char y)
{
	volatile unsigned char ddram_addr;
	ddram_addr=0x80;			//initialize data ram address to 0 (default)
	if (y==1) ddram_addr=0xC0;  //start print at 2nd line, DDRAM address 0x40
	lcd_cmd(ddram_addr+ (x&0x7F) );
}

void main()
{
	DDRC = 0XFF;           //port a as potput to send data and command to lcd
	DDRA = 0XFF;
	char time[15],date[15];
	lcd_init(); 			//LCD Port Initiliasation.
	i2c_init();
	rtc_init();
	lcd_clr();
	/*******use this only to set time and date**********/
	//rtc_set_time();
	//rtc_set_date();
	/***************************************************/
	while(1)
	{
		//lcd_write_str("***SMART HOME***");
		//_delay_ms(1000);
		//lcd_clr();
		rtc_get_time(&time);
		LCDGotoXY(4,0);
		lcd_write_str(time);
		LCDGotoXY(3,1);
		
		rtc_get_date(&date);
		lcd_write_str(date);
		_delay_ms(1000);
		lcd_clr();

	}	

}