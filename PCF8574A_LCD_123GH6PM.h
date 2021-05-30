#ifndef __PCF8574A_LCD2004_123GH6PM_H__
#define __PCF8574A_LCD2004_123GH6PM_H__
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"


extern void Delayms(uint32_t a);
extern void Delayus(uint32_t a);
extern void Lcd_init(void);
extern void Lcd_write(uint8_t RS,uint8_t RW,uint8_t data,uint8_t led);
extern void Lcd_write_char(uint8_t data,uint8_t led);
extern void LCD_Puts (char *s);
extern void Lcd_write_1(uint8_t num,uint8_t led);
extern uint8_t Lcd_write_digit(uint8_t n,uint16_t num,uint8_t space,uint8_t led);
extern void Lcd_write_4(uint8_t n,uint16_t num,uint8_t led);
extern void Lcd_Goto(unsigned char x, unsigned char y);
extern void BCD_write(uint8_t bcd_num,uint8_t led);

#endif
