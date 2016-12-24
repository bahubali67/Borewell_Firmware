#include "main.h"

char rtc_reg[7];
char time[15],date[15];	
/*---Fuction to read RTC---*/
void rtc_read(){									//Sending Address and data from controller to access RTC 

	unsigned char i2c_error, data, i;

	i2c_error = i2c_start();
	if(i2c_error == 1){
		i2c_stop();
		return;
	} 

	i2c_error = i2c_sendAddress(DS1307_W);			//RTC in write mode 0b11010000
	if(i2c_error == 1) {
		i2c_stop();
		return;
	} 

	i2c_error = i2c_sendData(0x00);
	if(i2c_error == 1){
		i2c_stop();
		return;
	} 

	i2c_error = i2c_repeatStart();
	if(i2c_error == 1){
		i2c_stop();
		return;
	} 

	i2c_error = i2c_sendAddress(DS1307_R);			//RTC in write mode 0b11010001
	if(i2c_error == 1){
		i2c_stop();
		return;
	} 


	for(i=0;i<=7;i++) {
		if(i == 7) {  	                            //No acknowledge
			data = i2c_receiveData_NACK();
		}
		else {
			data = i2c_receiveData_ACK();			//acknowledge
			if(data == ERROR_CODE) {
				i2c_stop();
				return;
			}
			rtc_reg[i] = data;
		}
	}
	i2c_stop();
}	  

/*---Fuction to get time from RTC---*/ 
void rtc_get_time(char *time) 
{

	rtc_set_registers();
	rtc_read();

	time[8] = 0x00;	                                                  
	time[7] = (SECONDS & 0x0f) | 0x30;		       //Converting all values to decimal from BCD.         
	time[6] = ((SECONDS & 0xf0) >> 4) | 0x30;	                      	
	time[5] = ':';

	time[4] = (MINUTES & 0x0f) | 0x30;
	time[3] = ((MINUTES & 0x70) >> 4) | 0x30;
	time[2] = ':'; 

	time[1] = (HOURS & 0x0f) | 0x30;	
	time[0] = ((HOURS & 0x30) >> 4) | 0x30;
}

/*---Fuction to get date from RTC---*/ 
void rtc_get_date(char *date) 
{
	rtc_set_registers();							
	rtc_read();

	date[10] = 0x00;
	date[9] = (YEAR & 0x0f) | 0x30;					//Converting all values to decimal from BCD.
	date[8] = ((YEAR & 0xf0) >> 4) | 0x30;
	date[7] = '0';
	date[6] = '2';
	date[5] = '/';
	date[4] = (MONTH & 0x0f) | 0x30;
	date[3] = ((MONTH & 0x10) >> 4) | 0x30;
	date[2] = '/';
	date[1] = (DATE & 0x0f) | 0x30;
	date[0] = ((DATE & 0x30) >> 4) | 0x30;
}  

/*---Fuction to set registers of RTC---*/
void rtc_set_registers() {									//Converting all values to BCD from Decimal.
	SECONDS = ((time[6] & 0x07) << 4) | (time[7] & 0x0f);
	MINUTES = ((time[3] & 0x07) << 4) | (time[4] & 0x0f);
	HOURS = ((time[0] & 0x03) << 4) | (time[1] & 0x0f);  
	//HOURS = (((time[0] & 0x03) << 4) | 0x40 ) | (time[1] & 0x0f); //for 12hr format

	DATE = ((date[0] & 0x03) << 4) | (date[1] & 0x0f);
	MONTH = ((date[3] & 0x01) << 4) | (date[4] & 0x0f);
	YEAR = ((date[8] & 0x0f) << 4) | (date[9] & 0x0f);
}

/*---Fuction to write to RTC---*/   
unsigned char rtc_write_time() {
	unsigned char i2c_error, i;

	i2c_error = i2c_start();
	if(i2c_error == 1) {
		i2c_stop();
		return(1);
	} 

	i2c_error = i2c_sendAddress(DS1307_W);
	if(i2c_error == 1) {
		i2c_stop();
		return(1);
	} 

	i2c_error = i2c_sendData(0x00);			//Time registers address starts from 00H
	if(i2c_error == 1) {
		i2c_stop();
		return(1);
	} 

	for(i=0;i<3;i++) {
		i2c_error = i2c_sendData(rtc_reg[i]);  
		if(i2c_error == 1) {
			i2c_stop();
			return(1);
		}
	}
	i2c_stop();
	return(0);
}

/*Function to write new date in the RTC*/   
unsigned char rtc_write_date() {
	unsigned char i2c_error, i;

	i2c_error = i2c_start();
	if(i2c_error == 1) {
		i2c_stop();
		return(1);
	} 

	i2c_error = i2c_sendAddress(DS1307_W);
	if(i2c_error == 1) {
		i2c_stop();
		return(1);
	} 

	i2c_error = i2c_sendData(0x04);		//date register address.
	if(i2c_error == 1) {
		i2c_stop();
		return(1);
	}
	for(i=4;i<7;i++) {				//Date registers start from 03H.
		i2c_error = i2c_sendData(rtc_reg[i]);  
		if(i2c_error == 1) {
			i2c_stop();
			return(1);
		}
	}
	i2c_stop();
	return(0);
}

/*---Fuction to set time---*/  	//Hardcoded time
void rtc_set_time(){
	unsigned char data;

	time[0]= '2';
	time[1]= '3';				//Hour.
	time[2]= ':';
	time[3]= '5';
	time[4]= '9';				//Minute.
	time[5]= ':';
	time[6]= '0';
	time[7]= '0';				//second.

	rtc_set_registers();  		//store above values in register array.
	data = rtc_write_time();

	if(!(data == 0)) {
		lcd_clr();
		lcd_cmd(0xC0);
		lcd_write_str("Time Updated");
		_delay_ms(20);
	}	  
	return;
}  

/*---Fuction to set date---*/   //Hardcoded Date
void rtc_set_date() {
	unsigned char data;

	date[0] = '2';				//Date
	date[1] = '1';
	date[2] = '/';
	date[3] = '0';				//month
	date[4] = '2';
	date[5] = '/';
	date[6] = '2'; 	   	   	  	//Year	                                
	date[7] = '0';
	date[8] = '1';
	date[9] = '6';

	rtc_set_registers(); 			//store above values in register array.
	data = rtc_write_date();		//write values to RTC

	if(!(data == 0)){
		lcd_clr();
		lcd_cmd(0x80);
		lcd_write_str("Date Updated");
		_delay_ms(30);    
	}	  
	return;
}


int rtc_init() {
	//I2C initiliasation.
	//disable twi
	TWCR = 0X00;
	//set bit rate 18 (freq: 100Khz) 		
	TWBR = 0x48;
	//set prescale 		
	TWSR = 0x00;
	//enable TWI		
	TWCR = 0x44; 		

	return 0;
}
