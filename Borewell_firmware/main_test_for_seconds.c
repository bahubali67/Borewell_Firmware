/*
 * main_test.c
 *
 * Created: 2/24/2016 9:07:37 PM
 *  Author: Bahubali
 */ 
/*
 * main.c
 *
 * Created: 2/19/2016 9:35:19 PM
 *  Author: Bahubali
 */ 
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/wdt.h> 
#include <avr/eeprom.h>



int calculate_next_ON_OFF_time(int *,int );

void main()
{
	wdt_disable();
	DDRC = 0XC0;   //RS and EN
	DDRA = 0XFF;   //Whole port as LCD data lines
	DDRB = 0x02; // PB0 and PB1 as LED and Relay pins
	DDRD = 0x00;	//input for selecting ON time delay
	PORTD = 0xFF; //pull up the portd
	char time[15];
	char date[15];
	char buffer[16];
	char Intial_Time_Buff[30];
	char current_time_buff[30];
	char Temp_Buff[15];
	int Intial_Time;
	int current_time;
	int ON_Time;
	int OFF_Time;
	int mod_val;
	char buff[16];
	char on_flag = 0;
	char off_flag = 0;
	char motor_on_counter = 0;
	int eeprom_on_time = 0;
	int ON_TIME_DELAY = 5;
	const int OFF_TIME_DELAY = 10;

	lcd_init(); 			//LCD Port Initiliasation.
	i2c_init();
	rtc_init();
	lcd_clr();

	/*******use this only to set time and date**********/
	//rtc_set_time();
	//rtc_set_date();
	/***************************************************/
	rtc_get_time(&time);
	strcpy(Temp_Buff,time);
	sprintf(Intial_Time_Buff,"%c%c%c%c",Temp_Buff[3],Temp_Buff[4],Temp_Buff[6],Temp_Buff[7]);// MM:SS
	Intial_Time = (atoi(Intial_Time_Buff));
	eeprom_on_time = eeprom_read_word((uint16_t *)0); //in address 0 ,the next ON time will be saved
	if( Intial_Time <=  eeprom_on_time ) 
	{
		ON_Time = eeprom_on_time;
	}
	else
	{
		ON_Time = (Intial_Time + 10);
		mod_val = (ON_Time % 100);
		if( mod_val >= 60)
		{
			ON_Time = calculate_next_ON_OFF_time(&ON_Time,mod_val);
		}
	}
	OFF_Time = 0;
	PORTD =0x00;
	motor_on_counter = eeprom_read_byte((uint8_t*)10); //address 10 is to store motor_on_counter value
	LCDGotoXY(0,1);	
	sprintf(buff,"TURNS ON AT:%d",ON_Time);	
	lcd_write_str(buff);
	wdt_enable(WDTO_2S);
	switch(PIND & 0xFF)
	{
		case 0xFE:ON_TIME_DELAY = 2;
		break;
		case 0xFD:ON_TIME_DELAY = 3;
		break;
		case 0xFB:ON_TIME_DELAY = 4;
		break;
		case 0xF7:ON_TIME_DELAY = 5;
		break;
		case 0xEF:ON_TIME_DELAY = 6;
		break;
		case 0xDF:ON_TIME_DELAY = 7;
		break;
		case 0xBF:ON_TIME_DELAY = 8;
		break;
		case 0x7F:ON_TIME_DELAY = 9;
		break;
		default: ON_TIME_DELAY = 5;
		break;
	}
	while(1)
	{
		rtc_get_time(&time);
		strcpy(Temp_Buff,time);
		sprintf(current_time_buff,"%c%c%c%c",Temp_Buff[3],Temp_Buff[4],Temp_Buff[6],Temp_Buff[7]);
		current_time = (atoi(current_time_buff));
		LCDGotoXY(0,0);
		if(current_time >= 5959)/////////make it 2359 for hour based
		{
			motor_on_counter = 0;
		}
		/*if(on_flag == 1)
			lcd_write_str("**MOTOR ON**");
		else*/
		{
			lcd_write_str("   ");
			lcd_write_str(time);
			lcd_write_str("   ");
			lcd_write_int(motor_on_counter);
		}
		if(current_time == ON_Time)
		{
			lcd_clr();
			PORTD =0x01;
			lcd_clr();
			LCDGotoXY(0,0);
			lcd_write_str("**MOTOR ON**");
			OFF_Time = current_time + ON_TIME_DELAY; //ON delay --> 3min
			mod_val = (OFF_Time % 100);
			if( mod_val >= 60)
			{
				OFF_Time = calculate_next_ON_OFF_time(&OFF_Time,mod_val);
			}
			LCDGotoXY(0,1);	
			sprintf(buff,"TURNS OFF :%d",OFF_Time);
			lcd_write_str(buff);
			on_flag = 1;
			off_flag = 1;
			motor_on_counter++;
			eeprom_write_byte((uint8_t*)10,motor_on_counter);		
		}
		if(current_time == OFF_Time)
		{
			lcd_clr();
			PORTD =0x00;
			LCDGotoXY(0,0);
			lcd_write_str("**MOTOR OFF**");
			ON_Time = current_time + OFF_TIME_DELAY; //OFF delay --> 2hours
			mod_val = (ON_Time % 100);
			if( mod_val >= 60)
			{
				ON_Time = calculate_next_ON_OFF_time(&ON_Time,mod_val);
			}
			LCDGotoXY(0,1);
			sprintf(buff,"TURNS ON AT:%d",ON_Time);	
			lcd_write_str(buff);
			on_flag = 0;
			off_flag = 0;
			eeprom_write_word((uint16_t *)0,ON_Time);
		}
		if((current_time > ON_Time) && (on_flag == 0))
		{
			/*lcd_clr();
			LCDGotoXY(0,0);
			lcd_write_str("inside2");
			_delay_ms(500);*/
			on_flag = 1;
			ON_Time = current_time + 5; //offset delay
			mod_val = (ON_Time % 100);
			if( mod_val >= 60)
			{
				ON_Time = calculate_next_ON_OFF_time(&ON_Time,mod_val);
			}
		}
		if((current_time > OFF_Time) && (off_flag == 1))
		{
			/*lcd_clr();
			LCDGotoXY(0,0);
			lcd_write_str("inside1");
			_delay_ms(500);*/
			PORTD =0x00;
			off_flag = 0;
			OFF_Time = current_time + 5; //offset delay
			mod_val = (OFF_Time % 100);
			if( mod_val >= 60)
			{
				OFF_Time = calculate_next_ON_OFF_time(&OFF_Time,mod_val);
			}
		}
		wdt_reset();
		_delay_ms(800);
	}	

}

int calculate_next_ON_OFF_time(int *ON_Time,int mod_val)
{
	(*ON_Time) = ((*ON_Time) - mod_val + 100 + ( mod_val - 60 ));
	if(((*ON_Time)/100) >= 60) //make it 24 for hour and 60 for min & sec
	{
		(*ON_Time) = (*ON_Time) % 100;
	} 
	return (*ON_Time);
}