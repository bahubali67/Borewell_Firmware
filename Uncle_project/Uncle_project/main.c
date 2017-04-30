/*
 * Uncle_project.c
 *
 * Created: 4/5/2017 7:54:30 PM
 *  Author: Scorpio
 */ 


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <stdbool.h>
#include "main.h"
#define TURN_OFF 0x00
#define TURN_ON 0x01
#define MOTOR PORTB
#define OFF_TIME       "202500"
#define MNG_WISH_START "060000"
#define MNG_WISH_STOP  "090000"
#define EVE_WISH_START "170000"
#define EVE_WISH_STOP  "190000"
	long int kk = 4000;
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

char * derive_off_time(char *data){
		long int a,b,temp;
		char s[7];
		a = atol(data);
		temp = a + (long int)4000;
		b = calculate_next_ON_OFF_time(temp); // 004000 ==> 40 minutes HH:MM:SS
	//	lcd_clr();
	//	LCDGotoXY(4,0);
	//	lcd_write_int(b);
	//	_delay_ms(1000);
		ltoa(b, s, 10);

		return &s[0];
		
}	
	
int main(void)
{
	DDRD = 0x00;	//input 
	DDRB = 0x07; // PB0 and PB1 as LED and Relay pins,PB2 as buzzer pin
	PORTD = 0xFF; //pull up the portd
	DDRA = 0XFF;   //Whole port as LCD data lines
	DDRC = 0XC0;   //RS and EN
	
	
	char time[15],date[15],temp_buf[15];
	char ON_time[8];
	long Int_Time,Turn_off_time;
	bool over_head_tank_high = 0;
	bool over_head_tank_low = 0;
	bool under_ground_tank_high = 0;
	bool over_head_under_ground_tank_low = 0;
	bool over_time_on = 0;
	
	lcd_init(); 			//LCD Port Initialiasation.
	i2c_init();
	rtc_init();
	lcd_clr();
	
	
	/*******use this only to set time and date**********/
	//rtc_set_time();
	//rtc_set_date();
	/***************************************************/
	char Motor_Off[7];
	strcpy(Motor_Off,"9999999");
	MOTOR = TURN_OFF;
	LCDGotoXY(0,0);
	//lcd_write_str(" ***WELCOME***");
	//_delay_ms(1000);
	lcd_clr();
    while(1)
    {
		rtc_get_time(&time);
		rtc_get_date(&date);
		sprintf(temp_buf,"%c%c%c%c%c%c",time[0],time[1],time[3],time[4],time[6],time[7]);
		if(strcmp(temp_buf,Motor_Off) > 0 ){
			if(over_time_on == 1){		
				over_time_on = 0;
				over_head_tank_low = 0;
				MOTOR = TURN_OFF;
				lcd_clr();
				LCDGotoXY(0,0);
				lcd_write_str("CROSSED ON LIMIT");
				LCDGotoXY(0,1);
				lcd_write_str("   MOTOR OFF    ");
				_delay_ms(800);
				lcd_clr();
			}			
		}	
		if(strcmp(temp_buf,MNG_WISH_START) > 0 && strcmp(temp_buf,MNG_WISH_STOP) < 0){
			if(over_head_tank_low == 0){
				lcd_clr();
				LCDGotoXY(0,0);
				lcd_write_str("  GOOD MORNING  ");
				LCDGotoXY(0,1);
				lcd_write_str("  UNCLE AUNTIE ");
				_delay_ms(800);
			}			
		}
		if(strcmp(temp_buf,EVE_WISH_START) > 0 && strcmp(temp_buf,EVE_WISH_STOP) < 0){
			if(over_head_tank_low == 0){
				lcd_clr();
				LCDGotoXY(0,0);
				lcd_write_str("  GOOD EVENING ");
				LCDGotoXY(0,1);
				lcd_write_str("  UNCLE AUNTIE ");
				_delay_ms(800);
			}				
		}
		if(over_head_tank_low != 1){
			lcd_clr();
			LCDGotoXY(4,0);
			lcd_write_str(time);	
			LCDGotoXY(3,1);
			lcd_write_str(date);		
		}			

        switch(PIND & 0xFF)
        {
	        case 0x7F://over head tank high
			if(over_head_tank_high == 0){
				over_head_tank_high = 1;
				over_head_tank_low = 0;
				MOTOR = TURN_OFF;
				lcd_clr();
				LCDGotoXY(0,0);
				lcd_write_str("OVER HEAD FULL");
				LCDGotoXY(0,1);
				lcd_write_str("MOTOR TURNED OFF");
			}					
	        break;
	        case 0xBF://over head tank low
			if(over_head_tank_low == 0){
				over_head_tank_low = 1;
				over_head_tank_high = 0;
				over_head_under_ground_tank_low = 0;
				under_ground_tank_high = 0;
				over_time_on = 1;
	        	MOTOR = TURN_ON;
				strcpy(Motor_Off,derive_off_time(temp_buf));
			//	strcpy(Motor_Off,"212300");
			//	lcd_clr();
			//	LCDGotoXY(0,0);
			//	lcd_write_str(Motor_Off);
			//	_delay_ms(1000);
				lcd_clr();
	        	lcd_write_str("MOTOR TURNED ON AT");
				LCDGotoXY(0,1);
				lcd_write_str("AT ");
				LCDGotoXY(4,1);
				lcd_write_str(time);	
			}				
			break;
	        case 0xDF://under ground tank high
			if(under_ground_tank_high == 0){
				under_ground_tank_high = 1;
				over_head_tank_low = 0;
	        	MOTOR = TURN_OFF;
	        	lcd_clr();
	        	LCDGotoXY(0,0);
	        	lcd_write_str("UNDERGROUND EMPTY");
	        	LCDGotoXY(0,1);
	        	lcd_write_str("MOTOR TURNED OFF");
			}					
	        break;
			case 0x9F:
			if(over_head_under_ground_tank_low == 0){
				over_head_under_ground_tank_low = 1;
				over_head_tank_low = 0;
				MOTOR = TURN_OFF;
				lcd_clr();
				LCDGotoXY(0,0);
				lcd_write_str("    MOTOR    ");
				LCDGotoXY(0,1);
				lcd_write_str("  TURNED OFF  ");		
			}					
			break;
			case 0x3F:
			MOTOR = TURN_OFF;
			lcd_clr();
			LCDGotoXY(0,0);
			lcd_write_str("    MOTOR    ");
			LCDGotoXY(0,1);
			lcd_write_str("  TURNED OFF  ");
			break;
/*			case 0xFF:
			MOTOR = TURN_OFF;
			lcd_clr();
			LCDGotoXY(0,0);
			lcd_write_str("    MOTOR    ");
			LCDGotoXY(0,1);
			lcd_write_str("  TURNED OFF  ");
			break;*/
				
        }
		_delay_ms(800);
	}
}
