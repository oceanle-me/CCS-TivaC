


#include "PCF8574A_LCD_123GH6PM.h"
#include "MyI2C.h"
#define add_PCF8574	 0x27 // I2C address of PCF8574A : 0x3F or 0x27
/*
Pins map PCF8574A connect to LCD2004:
(chan 4)  P0 : RS
(chan 5)  P1 : RW
(chan 6)  P2 : EN
(chan 7)  P3 : led ( 1 led on, 0 led off)
(chan 9)  P4 : D4
(chan 10) P5 : D5
(chan 11) P6 : D6
(chan 12) P7 : D7
*/

//==============================================================================

void Delayms(uint32_t a)
{
    SysCtlDelay(a*(SysCtlClockGet()/3/1000));
}
void Delayus(uint32_t a)
{
    SysCtlDelay(a*(SysCtlClockGet()/3/1000/1000));
}
//static uint8_t Lcd_Read(uint8_t led) ;
//==============================================================================
static void Lcd_write_4bit(uint8_t RS,uint8_t RW,uint8_t data,uint8_t led) // ghi 4 bit thap
// RS = 0 lenh,= 1 du lieu, RW = 0 ghi, = 1 doc, led = 1 on, = 0 off
{
  uint8_t a = 0;

    a = (data & 0x0F)<<4; // 4 low bits of data will be sent first.

    if(RS==1) a += 0x1;   // bit 0
    if(RW==1) a += 0x2;   // bit 1
    a += 0x4 ;  // bit 2 , EN = 1
    if(led>0) a += 0x8;   // bit 3

	I2CSend(add_PCF8574, 1, a);     //send 4bit lower
	Delayus(80);
	a -= 0x4 ; // bit 2 , EN = 0
	I2CSend(add_PCF8574, 1, a);//disable to write/read
	Delayus(80);
}




/*static uint8_t Lcd_Read_4bit(uint8_t led) // doc 4 bit cao
// RS = 0 lenh,= 1 du lieu, RW = 0 ghi, = 1 doc, led = 1 on, = 0 off
{
  uint8_t a = 0;
  a = 0xF0; // 4 bit low
  //if(RS==1) a += 0x1;   // bit 0
  a += 0x2;   // bit 1 / RW = 1
  a           += 0x4 ;  // bit 2 , EN = 1
  if(led>0) a += 0x8;   // bit 3
  //I2CSend(add_PCF8574, 1,a); // EN = 1
	//b[0] = a;
	I2CSend(add_PCF8574, 1, a);//HAL_I2C_Master_Transmit(&hi2c1,add_PCF8574<<1,b,1,10);
  a -= 0x4 ; // bit 2 , EN = 0
	//b[0] = a;
	//HAL_Delay(1);
	//HAL_I2C_Master_Transmit(&hi2c1,add_PCF8574<<1,b,1,10);
	//HAL_Delay(1);
	I2CSend(add_PCF8574, 1, a);//a = HAL_I2C_Master_Receive(&hi2c1,add_PCF8574<<1,b,1,10);
	//HAL_Delay(3);
	a = a & 0xF0;
	return a;
}*/
/*static uint8_t Lcd_Read(uint8_t led) // doc 4 bit cao
// RS = 0 lenh,= 1 du lieu, RW = 0 ghi, = 1 doc, led = 1 on, = 0 off
{
  uint8_t a = 0;
	a = Lcd_Read_4bit(led);
	//HAL_Delay(1);
	a = a + (Lcd_Read_4bit(led)>>4);
	while(a&0x80);
}
*/

//==============================================================================
void Lcd_write(uint8_t RS,uint8_t RW,uint8_t data,uint8_t led)
{// RS = 0 lenh,= 1 du lieu, RW = 0 ghi, = 1 doc, led = 1 on, = 0 off
  Lcd_write_4bit(RS,RW,data>>4   ,led); // 4 high bits will be sent first
  Lcd_write_4bit(RS,RW,data      ,led); // 4 bit low

}
//==============================================================================
void Lcd_init(void)
{

    Delayms(50);// wait for >40ms
    Lcd_write(0,0,0x30   ,0);
    Delayms(5);// wait for >4.1ms
    Lcd_write(0,0,0x30   ,0);
    Delayms(1);// wait for >100us
    Lcd_write(0,0,0x30   ,0);

    Delayms(1);// wait for >100us

    Lcd_write(0,0,0x20   ,1); //set 4bit mode in 8bit interface
    // dislay initialisation
    Lcd_write(0,0,0x28   ,1);  // 4bit interface, hien thi 2 hang, ki tu 5x8
    Lcd_write(0,0,0x0C   ,1); // bat hien thi, tat con tro
    Lcd_write(0,0,0x06   ,1); // tang con tro, khong dich hinh
    Lcd_write(0,0,0x01   ,1); // xoa man hinh
    Delayms(5);
    Lcd_write(0,0,0x02   ,1); //return home
    Delayms(5);
}
//==============================================================================
void Lcd_write_char(uint8_t data,uint8_t led)
{// data ky tu can ghi, led = 1 on, = 0 off
  Lcd_write(1,0,data,led);
}
//==============================================================================
void LCD_Puts (char *s)
{//Ham ghi 1 chuoi ky tu
        while (*s){
                Lcd_write_char(*s,1);
                s++;
        }
}
//==============================================================================
void Lcd_write_1(uint8_t num,uint8_t led)
{// ghi 1 chu so len lcd, led = 1 on, = 0 off
  if((num<=9)) Lcd_write(1,0,num+0x30,led);
}
//==============================================================================
uint8_t Lcd_write_digit(uint8_t n,uint16_t num,uint8_t space,uint8_t led)
{// ghi chu so thu n (tinh tu so hang nghin)cua so num len lcd, led = 1 on, = 0 off
  // neu chu so duoc ghi = 0 va space = 1 thi ghi ra khoang trang thay vi so 0
  uint8_t temp;
  if(n==1) // chu so hang nghin
  {
    temp = num/1000; // lay so hang nghin
    if((space>0)&(temp==0)) Lcd_write_char(0x20,led); else
    Lcd_write_1(temp,led); // ghi len lcd 
  }
  else if(n==2) // chu so hang tram
  {
    num = num/100; // lay so hang nghin
    temp = num %10; // lay phan du
    if((space>0)&(temp==0)) Lcd_write_char(0x20,led); else
    Lcd_write_1(temp,led); // ghi len lcd
  }
  else if(n==3) // chu so hang chuc
  {
    num = num/10; // lay so hang chuc
    temp = num %10; // lay phan du
    if((space>0)&(temp==0)) Lcd_write_char(0x20,led); else
    Lcd_write_1(temp,led); // ghi len lcd
  }
  else if(n==4) // chu so hang dvi
  {
    temp = num%10; // lay so hang dvi
    Lcd_write_1(temp,led); // ghi len lcd
  }
  if((space>0)&(temp==0)) return 1 ; else return 0; // kiem tra xem chu so nay co duoc hien thi hay khong = 1 ko, = 0 co
}
//==============================================================================
void Lcd_write_4(uint8_t n,uint16_t num,uint8_t led)
{// ghi n chu so cua so num len lcd, led = 1 on, = 0 off
  // chu so vo nghia duoc thay bang dau cach
  uint8_t space;
  if     (n==1) // 1 chu so
  {
    Lcd_write_digit(4,num,0,1); // so hang dvi
  }
  else if(n==2) // 2 chu so
  {
    Lcd_write_digit(3,num,0,1); // so hang chuc//edit 28/5, hien thi ca so 0, vi la ngay gio
    Lcd_write_digit(4,num,0,1); // so hang dvi
  }
  else if(n==3)  // 3 chu so
  {
    space = Lcd_write_digit(2,num,1,1); // so hang tram
    Lcd_write_digit(3,num,space,1); // so hang chuc
    Lcd_write_digit(4,num,0,1); // so hang dvi
  }
  else if(n==4) // 4 chu so
  { 
    space = Lcd_write_digit(1,num,1,1); // so hang nghin
    space = Lcd_write_digit(2,num,space,1); // so hang tram
    Lcd_write_digit(3,num,space,1); // so hang chuc
    Lcd_write_digit(4,num,0,1); // so hang dvi
  }
}
//==============================================================================
void Lcd_Goto(unsigned char x, unsigned char y)
{ // di chuyen con tro den cot x hang y
  // x = 0 -> 19
  // y = 0 -> 3
        unsigned char address;
        if     (y==0) address=(0x80+x);
        else if(y==1) address=(0xC0+x);
        else if(y==2) address=(0x94+x);
        else if(y==3) address=(0xD4+x);
        Lcd_write(0,0,address  ,1); // ghi lenh
}
//==============================================================================
void BCD_write(uint8_t bcd_num,uint8_t led)
{
	uint8_t a;
	a = (bcd_num>>4)&0xf;
				Lcd_write_1(a,led);
	a = bcd_num&0xf;
				Lcd_write_1(a,led);
}



