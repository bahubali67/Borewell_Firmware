#ifndef __LCD_H__
#define __LCD_H__


#define EN PC7	// enable signal
#define RS PC6	// register select signal

void lcd_clr();
void lcd_cmd(unsigned char cmd);
void lcd_init(void);
void lcd_write(unsigned char data);
void lcd_write_str(char *data);
void lcd_write_int(  long int num);
void LCDGotoXY(unsigned char , unsigned char );

#endif
