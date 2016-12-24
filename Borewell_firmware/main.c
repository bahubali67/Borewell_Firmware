/*
 * main.c
 *
 * Created: 2/19/2016 9:35:19 PM
 *  Author: Bahubali
 * added EEPROM erase(erases on time)(make PD0 = 0 to erase on time)
 * added buzzer sound before motor turns on(before 1min)
 */ 
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/wdt.h> 
#include <avr/eeprom.h>

//void check_wdt(void);
long int calculate_next_ON_OFF_time(long int);
void int_to_time_format(long int );

 void clear_ON_TIME(void)
 {
	 eeprom_write_dword((uint32_t *)0,0);	
 }

void temp_func(void)
{
	eeprom_write_byte((uint8_t*)10,0);//change motor count value
}
void main()
{
	wdt_disable();
	DDRC = 0XC0;   //RS and EN
	DDRA = 0XFF;   //Whole port as LCD data lines
	DDRB = 0x07; // PB0 and PB1 as LED and Relay pins,PB2 as buzzer pin
	DDRD = 0x00;	//input for selecting ON time delay
	PORTD = 0xFF; //pull up the portd
	
	char time[15];
	//char date[2];
	char Intial_Time_Buff[8];
	char Temp_Buff[15];
	long int Intial_Time;
	long int current_time;
	long int ON_Time;
	long int Buzzer_ON_Time = 0;
	long int OFF_Time = 10;
	long int mod_val;
	//char buff[16];
	char on_flag = 0;
	char off_flag = 0;
	char motor_on_counter = 0;
	long int eeprom_on_time = 0;
	int ON_TIME_DELAY = 40;
	const int OFF_TIME_DELAY = 20000;//for 1hr=10000, 1min=100, 1sec=1

	lcd_init(); 			//LCD Port Initiliasation.
	i2c_init();
	rtc_init();
	lcd_clr();

	/*******use this only to set time and date**********/
	//rtc_set_time();
	//rtc_set_date();
	/***************************************************/
	//check_wdt();
	/*******testing purpose,comment it**********/
	//temp_func();
	//clear_ON_TIME();
	/***************************************************/
	switch(PIND & 0xFF)
	{
		case 0xFE://ON_TIME_DELAY = 20;
				  clear_ON_TIME();
				  LCDGotoXY(0,0);
				  lcd_write_str("EEPROM CLEARED");
				  _delay_ms(600);
				  lcd_clr();
		break;
		case 0xFD:ON_TIME_DELAY = 60;
		break;
		case 0xFB:ON_TIME_DELAY = 40;
		break;
		case 0xF7:ON_TIME_DELAY = 200;
		break;
		case 0xEF:ON_TIME_DELAY = 300;
		break;
		case 0xDF:ON_TIME_DELAY = 400;
		break;
		case 0xBF:ON_TIME_DELAY = 500;
		break;
		case 0x7F:ON_TIME_DELAY = 700;
		break;
		default: ON_TIME_DELAY = 50;
		break;
	}
	LCDGotoXY(0,0);
	lcd_write_str("ON DELAY:");
	lcd_write_int(ON_TIME_DELAY);
	_delay_ms(300);
	rtc_get_time(&time);
	strcpy(Temp_Buff,time);
	sprintf(Intial_Time_Buff,"%c%c%c%c%c%c",Temp_Buff[0],Temp_Buff[1],Temp_Buff[3],Temp_Buff[4],Temp_Buff[6],Temp_Buff[7]);// HH:MM:SS
	Intial_Time = (atol(Intial_Time_Buff));
	eeprom_on_time = eeprom_read_dword((uint32_t *)0); //in address 0 ,the next ON time will be saved
	if( Intial_Time <=  eeprom_on_time && ((eeprom_on_time - Intial_Time) <= 20000) )  //difference between off time should not cross 2hrs(02:00:00)
	{
		ON_Time = eeprom_on_time;
	}
	else
	{
		ON_Time = calculate_next_ON_OFF_time(Intial_Time + 30); //after  boot delay(30seconsds
	}
	PORTB =0x00;
	motor_on_counter = eeprom_read_byte((uint8_t*)10); //address 10 is to store motor_on_counter value
	LCDGotoXY(0,1);	

	lcd_write_str("NEXT ON:");
	int_to_time_format(ON_Time);

	wdt_enable(WDTO_2S);
	while(1)
	{
		rtc_get_time(&time);
		strcpy(Temp_Buff,time);
		sprintf(Intial_Time_Buff,"%c%c%c%c%c%c",Temp_Buff[0],Temp_Buff[1],Temp_Buff[3],Temp_Buff[4],Temp_Buff[6],Temp_Buff[7]);
		current_time = (atol(Intial_Time_Buff));
		LCDGotoXY(0,0);
		//lcd_clr();
		if(current_time >= 90000 && current_time <= 90005 )//counter = 0 at 9am
		{
			motor_on_counter = 0;
			eeprom_write_byte((uint8_t*)10,motor_on_counter);
		}
		if (((Buzzer_ON_Time = (ON_Time - 100)) % 10000) >= 6000)
		{
			Buzzer_ON_Time = Buzzer_ON_Time - 4000;
		}	
		//lcd_write_int(Buzzer_ON_Time);
		
		if (((current_time >= Buzzer_ON_Time ) && (current_time <= ON_Time)) && ((current_time >= 70000) && (current_time <= 210000)))//works only at 6am to 10pm(buzzer)
		{
			PORTB ^= 0x04; //for buzzer
		}
		lcd_write_str("C:");
		lcd_write_int(motor_on_counter);
		lcd_write_str("   ");
		LCDGotoXY(6,0);
		lcd_write_str("T:");
		lcd_write_str(time);			
		if(current_time == ON_Time)
		{
			PORTB = 0x03;
			lcd_clr();
			LCDGotoXY(0,0);
			lcd_write_str("****MOTOR ON****");
			OFF_Time = current_time + ON_TIME_DELAY; //ON delay --> 1.3min
			OFF_Time = calculate_next_ON_OFF_time(OFF_Time);
			LCDGotoXY(0,1);	
			lcd_write_str("NXT OFF:");
			int_to_time_format(OFF_Time);
			on_flag = 1;
			off_flag = 1;
			motor_on_counter++;
			eeprom_write_byte((uint8_t*)10,motor_on_counter);		
		}
		if(current_time == OFF_Time)
		{
			lcd_clr();
			PORTB =0x00;
			LCDGotoXY(0,0);
			lcd_write_str("***MOTOR OFF****");
			ON_Time = current_time + OFF_TIME_DELAY; //OFF delay --> 1hours
			ON_Time = calculate_next_ON_OFF_time(ON_Time);
			LCDGotoXY(0,1);
			lcd_write_str("NEXT ON:");
			//lcd_write_int(ON_Time);
			int_to_time_format(ON_Time);
			on_flag = 0;
			off_flag = 0;
			eeprom_write_dword((uint32_t *)0,ON_Time);//write on time in eeprom
		}
		if((current_time > ON_Time) && (on_flag == 0) && (ON_Time >= 20000))
		{
			lcd_clr();
			LCDGotoXY(0,0);
			lcd_write_str("inside ON TIME");
			_delay_ms(500);
			on_flag = 1;
			ON_Time = current_time + 5; //offset delay
			ON_Time = calculate_next_ON_OFF_time(ON_Time);
		}
		if((current_time > OFF_Time) && (off_flag == 1) /*&& (OFF_Time >= 5950)*/)
		{
			lcd_clr();
			LCDGotoXY(0,0);
			lcd_write_str("inside OFF TIME");
			_delay_ms(500);
			PORTB =0x00;
			off_flag = 0;
			OFF_Time = current_time + 5; //offset delay
			OFF_Time = calculate_next_ON_OFF_time(OFF_Time);
		}
		wdt_reset();
		_delay_ms(700);
		//PORTB &= ~(0x04); //for buzzer
	}	
}
#if 0
void check_wdt(void){
	char cnt;
	/***************Enabke this************************/
	//cnt = eeprom_read_byte((uint8_t*)20);
	/**************************************************/
    if(MCUSR & _BV(WDRF)){	                  // If a reset was caused by the Watchdog Timer...
        MCUSR &= ~_BV(WDRF);                // Clear the WDT reset flag
        WDTCSR |= (_BV(WDCE) | _BV(WDE));   // Enable the WD Change Bit
        WDTCSR = 0x00;                      // Disable the WDT
	  	cnt++;
		eeprom_write_byte((uint8_t*)20,cnt); // in 20th address will save the wdt counter value,,
		if(cnt >= 5)
		{
			cnt = 0;
			eeprom_write_byte((uint8_t*)20,cnt); 
			lcd_clr();
			LCDGotoXY(0,0);
			lcd_write_str("****ERROR****");
			LCDGotoXY(0,1);
			lcd_write_str("TURN OFF SYSTEM");
			while(1);
		}
    }
}
#endif
long int calculate_next_ON_OFF_time(long int ON_Time)
{
	wdt_reset();
	long int mod_val;
	long int inc_hr_min;
	long int offset;
	mod_val = ON_Time % 100;
	if(mod_val >= 60)
	{
		inc_hr_min = mod_val/60;
		//offset = mod_val - 60;
		ON_Time = (ON_Time - 60) + (inc_hr_min * 100);
	}	
	mod_val = ON_Time % 10000;
	if(mod_val >= 6000)
	{
		inc_hr_min = mod_val/6000;
		//offset = mod_val - 60;
		ON_Time = (ON_Time - 6000) + (inc_hr_min * 10000);
	}
	if(ON_Time >= 240000)
	{
	ON_Time = ON_Time - 240000;		
	}
	return ON_Time;
}

void int_to_time_format(long int int_val)
{
	wdt_reset();
	char s[10];
	long int n;
	int count = 0;
	n = int_val;
	while(n!=0)
	{
		n/=10;             /* n=n/10 */
		++count;
	}
	memset(s,0,10);
	ltoa(int_val, s, 10);
	if( count == 5)
	{
		lcd_write('0');
		lcd_write(s[0]);
		lcd_write(':');
		lcd_write(s[1]);
		lcd_write(s[2]);
		lcd_write(':');
		lcd_write(s[3]);
		lcd_write(s[4]);		
	}		
	if (count == 6)
	{
		lcd_write(s[0]);
		lcd_write(s[1]);
		lcd_write(':');
		lcd_write(s[2]);
		lcd_write(s[3]);
		lcd_write(':');
		lcd_write(s[4]);
		lcd_write(s[5]);
	}	
	if(count < 5)
		lcd_write_int(int_val);
}