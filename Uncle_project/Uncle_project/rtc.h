#ifndef __RTC_H__
#define __RTC_H__



#define		SECONDS		rtc_reg[0]
#define		MINUTES		rtc_reg[1]
#define		HOURS		rtc_reg[2]
#define		DAY    		rtc_reg[3]
#define		DATE   		rtc_reg[4]
#define		MONTH  		rtc_reg[5]
#define		YEAR   		rtc_reg[6]



void rtc_read();
void rtc_write();
void rtc_get_time(char *time);
void rtc_get_date(char *date);
void rtc_set_registers();
unsigned char rtc_write_time();
unsigned char rtc_write_date();
void rtc_set_time();
void rtc_set_date();
int rtc_init();

#endif
