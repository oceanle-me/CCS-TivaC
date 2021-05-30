#include <alarmClock.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
//#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/tm4c123gh6pm.h"
#include "debug.h"
#include "utils/ustdlib.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/hibernate.h"
#include "PCF8574A_LCD_123GH6PM.h"
#include "MyI2C.h"


#define AlarmTimeAction 60//second uint
#define ASCIIOffset 48
#define OFFSETYEAR 2000
#define UARTSYNTIMELENGTH 30
#define UARTALARMTIMELENGTH 30

static bool sw1_OffAlarm, timeOut;
struct_calendar    synTime;
struct_alarm      alarmTime;

char UARTAlarmTime[UARTALARMTIMELENGTH];
char UARTSynTime[UARTSYNTIMELENGTH];
//  s305/11/2021 08:59:26 PMe  a2059e
//  s305/11/2021 08:59:26 AMe  a2059e
//   012345678901234567890123   01234
void AdjustUARTAlarmTime(void)
{
    uint32_t i=1;
    for (i=0;i<=UARTALARMTIMELENGTH; i++)
    {
        UARTAlarmTime[i] -= ASCIIOffset;
    }
}
void AdjustUARTSynTime(void)
{
    uint32_t i=1;
    for (i=0;i<=UARTSYNTIMELENGTH; i++)
    {
        UARTSynTime[i] -= ASCIIOffset;
    }
}
struct_calendar UARTToStructCalendar(void)
{
    struct_calendar structDayTime;
    structDayTime.wday = UARTSynTime[0];
    structDayTime.month = UARTSynTime[1]*10 + UARTSynTime[2];
    structDayTime.day = UARTSynTime[4]*10 + UARTSynTime[5];
    structDayTime.year =UARTSynTime[7]*1000 + UARTSynTime[8]*100
            + UARTSynTime[9]*10 + UARTSynTime[10];
    if      ((UARTSynTime[21]==('P'- ASCIIOffset))&& ((UARTSynTime[12]*10 + UARTSynTime[13])==12))
        structDayTime.hour = 12;
    else if (UARTSynTime[21]==('P'- ASCIIOffset))
        structDayTime.hour = UARTSynTime[12]*10 + UARTSynTime[13] + 12;
    else if ((UARTSynTime[21]==('A'-ASCIIOffset))&& ((UARTSynTime[12]*10 + UARTSynTime[13])==12))
        structDayTime.hour = 0;
    else if (UARTSynTime[21]==('A'-ASCIIOffset))
        structDayTime.hour = UARTSynTime[12]*10 + UARTSynTime[13] ;

    structDayTime.minute = UARTSynTime[15]*10 + UARTSynTime[16];
    structDayTime.second = UARTSynTime[18]*10 + UARTSynTime[19];
    return structDayTime;
}
struct_alarm UARTToStructAlarm(void){
    struct_alarm structTime;
    structTime.hour =   UARTAlarmTime[0]*10 + UARTAlarmTime[1];
    structTime.minute = UARTAlarmTime[2]*10 + UARTAlarmTime[3];
    return structTime;
}
uint32_t StructCalenderToCounter(struct_calendar structDayTime)
{   //RTC at 2000y is 0000
    uint32_t tempDay = 0;
    uint32_t numberOf4Years = (structDayTime.year - OFFSETYEAR) /4;
    uint32_t numberOfYearsOffset  = (structDayTime.year - OFFSETYEAR) % 4;
    if (numberOfYearsOffset==0){
            switch (structDayTime.month)
            {
                case 1:
                    tempDay = 0;
                    break;
                case 2:
                    tempDay = 31;
                    break;
                case 3:
                    tempDay = (31+29);
                    break;
                case 4:
                    tempDay = (31+29+31);
                    break;
                case 5:
                    tempDay = (31+29+31+30);
                    break;
                case 6:
                    tempDay = (31+29+31+30+31);
                    break;
                case 7:
                    tempDay = (31+29+31+30+31+30);
                    break;
                case 8:
                    tempDay = (31+29+31+30+31+30+31);
                    break;
                case 9:
                    tempDay = (31+29+31+30+31+30+31+31);
                    break;
                case 10:
                    tempDay = (31+29+31+30+31+30+31+31+30);
                    break;
                case 11:
                    tempDay = (31+29+31+30+31+30+31+31+30+31);
                    break;
                case 12:
                    tempDay = (31+29+31+30+31+30+31+31+30+31+30);
                    break;
            }
    }
    else{
            switch (structDayTime.month)
            {
                case 1:
                    tempDay = 0;
                    break;
                case 2:
                    tempDay = 31;
                    break;
                case 3:
                    tempDay = (31+28);
                    break;
                case 4:
                    tempDay = (31+28+31);
                    break;
                case 5:
                    tempDay = (31+28+31+30);
                    break;
                case 6:
                    tempDay = (31+28+31+30+31);
                    break;
                case 7:
                    tempDay = (31+28+31+30+31+30);
                    break;
                case 8:
                    tempDay = (31+28+31+30+31+30+31);
                    break;
                case 9:
                    tempDay = (31+28+31+30+31+30+31+31);
                    break;
                case 10:
                    tempDay = (31+28+31+30+31+30+31+31+30);
                    break;
                case 11:
                    tempDay = (31+28+31+30+31+30+31+31+30+31);
                    break;
                case 12:
                    tempDay = (31+28+31+30+31+30+31+31+30+31+30);
                    break;
            }
        tempDay = tempDay + 366  +  (numberOfYearsOffset-1)*365;
    }
    return  ((((structDayTime.day-1) + tempDay + numberOf4Years*1461)*24*3600)  + structDayTime.hour*3600 + structDayTime.minute*60 + structDayTime.second);
}

struct_calendar CounterToStructCalender(uint32_t uint32Counter)
{
    struct_calendar structDayTime;
    structDayTime.second = uint32Counter % 60;
    structDayTime.minute = (uint32Counter/60) % 60;
    structDayTime.hour  = (uint32Counter/3600) % 24;
    uint32_t tempDay= uint32Counter/(3600*24); //number of days from 2000 to today
    //01/01/2000 is Saturday
    switch (tempDay%7){
    case (0):
            structDayTime.wday=saturday;
            break;
    case (1):
            structDayTime.wday=sunday;
            break;
    case (2):
            structDayTime.wday=monday;
            break;
    case (3):
            structDayTime.wday=tuesday;
            break;
    case (4):
            structDayTime.wday=wednesday;
            break;
    case (5):
            structDayTime.wday=thursday;
            break;
    case (6):
            structDayTime.wday=friday;
            break;
    }
    uint32_t numberOf4Years=(tempDay/1461);
    uint32_t tempDayOfSet = tempDay - numberOf4Years*1461;
    if(tempDayOfSet<=366)
    {
        structDayTime.year=numberOf4Years*4 + OFFSETYEAR ;
        if(tempDayOfSet<=31){
            structDayTime.month=  1;
            structDayTime.day= tempDayOfSet +1;
        }
        else if (tempDayOfSet<=(31+29)) {
            structDayTime.month=3;
            structDayTime.day= tempDayOfSet - (31) + 1;
        }
        else if (tempDayOfSet<=(31+29+31)) {
            structDayTime.month=3;
            structDayTime.day= tempDayOfSet - (31+29) +1  ;
        }
        else if (tempDayOfSet<=(31+29+31+30)) {
            structDayTime.month=4;
            structDayTime.day= tempDayOfSet - (31+29+31) +1;
        }
        else if (tempDayOfSet<=(31+29+31+30+31)) {
            structDayTime.month=5;
            structDayTime.day= tempDayOfSet - (31+29+31+30) +1;
        }
        else if (tempDayOfSet<=(31+29+31+30+31+30)) {
            structDayTime.month=6;
            structDayTime.day= tempDayOfSet - (31+29+31+30+31) +1;
        }
        else if (tempDayOfSet<=(31+29+31+30+31+30+31))  {
            structDayTime.month=7;
            structDayTime.day= tempDayOfSet - (31+29+31+30+31+30) +1;
        }
        else if (tempDayOfSet<=(31+29+31+30+31+30+31+31)) {
            structDayTime.month=8;
            structDayTime.day= tempDayOfSet - (31+29+31+30+31+30+31) +1;
        }
        else if (tempDayOfSet<=(31+29+31+30+31+30+31+31+30)){
            structDayTime.month=9;
            structDayTime.day= tempDayOfSet - (31+29+31+30+31+30+31+31) +1;
        }
        else if (tempDayOfSet<=(31+29+31+30+31+30+31+31+30+31)){
            structDayTime.month=10;
            structDayTime.day= tempDayOfSet - (31+29+31+30+31+30+31+31+30) +1;
        }
        else if (tempDayOfSet<=(31+29+31+30+31+30+31+31+30+31+30)) {
            structDayTime.month=11;
            structDayTime.day= tempDayOfSet - (31+29+31+30+31+30+31+31+30+31) +1;
        }
        else if (tempDayOfSet<=(31+29+31+30+31+30+31+31+30+31+30+31)) {
            structDayTime.month=12;
            structDayTime.day= tempDayOfSet - (31+29+31+30+31+30+31+31+30+31+30) +1;
        }
    }
    //the first year is a leap year at cycle 4 years
    else if (tempDayOfSet>366)
    {
        tempDayOfSet = tempDayOfSet - 366; //tru nam nhuan
        structDayTime.year = numberOf4Years*4 + OFFSETYEAR + (tempDayOfSet/365) + 1;
        tempDayOfSet = tempDayOfSet%365;

        if(tempDayOfSet<= +31) {
            structDayTime.month=  1;
            structDayTime.day= tempDayOfSet +1 ;
        }
        else if (tempDayOfSet<=(31+28)){
            structDayTime.month=  2;
            structDayTime.day= tempDayOfSet - 31 +1;
        }
        else if (tempDayOfSet<=(31+28+31)){
            structDayTime.month=3;
            structDayTime.day= tempDayOfSet - (31+28) +1;
        }
        else if (tempDayOfSet<=(31+28+31+30)) {
            structDayTime.month=4;
            structDayTime.day= tempDayOfSet - (31+28+31) +1;
        }
        else if (tempDayOfSet<=(31+28+31+30+31)){
            structDayTime.month=5;
            structDayTime.day= tempDayOfSet - (31+28+31+30) +1;
        }
        else if (tempDayOfSet<=(31+28+31+30+31+30)){
            structDayTime.month=6;
            structDayTime.day= tempDayOfSet - (31+28+31+30+31)  +1;
        }
        else if (tempDayOfSet<=(31+28+31+30+31+30+31)){
            structDayTime.month=7;
            structDayTime.day= tempDayOfSet - (31+28+31+30+31+30)  +1;
        }
        else if (tempDayOfSet<=(31+28+31+30+31+30+31+31)){
            structDayTime.month=8;
            structDayTime.day= tempDayOfSet - (31+28+31+30+31+30+31) +1;
        }
        else if (tempDayOfSet<=(31+28+31+30+31+30+31+31+30)){
            structDayTime.month=9;
            structDayTime.day= tempDayOfSet - (31+28+31+30+31+30+31+31) +1;
        }
        else if (tempDayOfSet<=(31+28+31+30+31+30+31+31+30+31)){
            structDayTime.month=10;
            structDayTime.day= tempDayOfSet - (31+28+31+30+31+30+31+31+30) +1;
        }
        else if (tempDayOfSet<=(31+28+31+30+31+30+31+31+30+31+30)) {
            structDayTime.month=11;
            structDayTime.day= tempDayOfSet - (31+28+31+30+31+30+31+31+30+31) +1;
        }
        else if (tempDayOfSet<=(31+28+31+30+31+30+31+31+30+31+30+31)) {
            structDayTime.month=12;
            structDayTime.day= tempDayOfSet - (31+28+31+30+31+30+31+31+30+31+30) +1;
        }
    }
    return structDayTime;
}
void SetDayAndTime( uint32_t     day,  uint32_t   month,uint32_t    year, uint32_t     hour, uint32_t     minute,  uint32_t     second){
    synTime.day = day;
    synTime.month= month;
    synTime.year= year;
    synTime.hour=hour;
    synTime.minute=minute;
    synTime.second=second;
}
bool CheckAlarm(struct_alarm structTime, struct_calendar structDayTime){
    if ((structTime.hour==structDayTime.hour)&&(structTime.minute==structDayTime.minute)){
        return true;
    }
    else {
        return false;
    }
}
void AlarmAction(void){
    AlarmSetUp();
    PlayAudio();
    while (!timeOut){ //if not timeOut, then no sleep and still update LCD
        synTime = CounterToStructCalender(HibernateRTCGet());
        DBG("%u %u/%u/%u %u:%u:%u\n",synTime.wday, synTime.day,synTime.month, synTime.year,
                   synTime.hour, synTime.minute, synTime.second);
        SysCtlDelay((SysCtlClockGet()/3)*15);//wait 15 seconds
    }
}
void AlarmSetUp(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);//count down
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT); // in full width, only use timerA for two timers
    TimerLoadSet(TIMER0_BASE, TIMER_A, (SysCtlClockGet()-1)*AlarmTimeAction);  // set value overflow AlarmTimeAction seconds
    TimerIntRegister(TIMER0_BASE, TIMER_A, &Timer0A_Int);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntRegister(GPIO_PORTF_BASE, &Switch1_Int);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
    IntEnable(INT_GPIOF);
    IntMasterEnable();
    TimerEnable(TIMER0_BASE, TIMER_A);
    IntPrioritySet(INT_GPIOF, 0b0);
    IntPrioritySet(INT_TIMER0A,0b0);
    IntPrioritySet(INT_HIBERNATE, 5<<5);//only consider top 3 bits in 8 bits
    timeOut=false;
    sw1_OffAlarm=false;
}
void Timer0A_Int(void){
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);    // Clear the timer interrupt
    DBG("int timer0");
    timeOut=true;
}
void Switch1_Int(void){
    GPIOIntClear(GPIO_PORTF_BASE, GPIOIntStatus(GPIO_PORTF_BASE, true));
    DBG("int sw1\n");
    sw1_OffAlarm=true;

}
void PlayAudio(void){
    while ( (!timeOut) && (!sw1_OffAlarm)){
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
    GPIO_PORTB_DATA_R ^= GPIO_PIN_4 ;
    SysCtlDelay((SysCtlClockGet()/3/10000/2)); //blink PB4 at 10KHz
    }
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0);
}
void DisplayLCD_DayTime(struct_calendar structDayTime){
    Lcd_Goto(0,0);
    switch (structDayTime.wday){
    case monday:
        LCD_Puts("Mon");
        break;
    case tuesday:
        LCD_Puts("Tue");
        break;
    case wednesday:
        LCD_Puts("Wed");
        break;
    case thursday:
        LCD_Puts("Thu");
        break;
    case friday:
        LCD_Puts("Fri");
        break;
    case saturday:
        LCD_Puts("Sat");
        break;
    case sunday:
        LCD_Puts("Sun");
        break;
    }
    LCD_Puts(" ");
    Lcd_write_4(2,structDayTime.day,1);
    LCD_Puts("/");
    Lcd_write_4(2,structDayTime.month,1);
    LCD_Puts("/");
    Lcd_write_4(4,structDayTime.year,1);

    Lcd_Goto(5,1);
    Lcd_write_4(2,structDayTime.hour,1);
    LCD_Puts(":");
    Lcd_write_4(2, structDayTime.minute,1);
}
void DisplayLCD_OffAlarm(void){
    Lcd_write(0,0,0x01   ,1); // xoa man hinh
    Delayms(5);
    Lcd_write(0,0,0x02   ,1); //return home
    Delayms(5);
    Lcd_Goto(0,0);
    LCD_Puts("Off Alarm");
}
void DisplayLCD_OnAlarm(struct_alarm Time){
    Lcd_write(0,0,0x01   ,1); // xoa man hinh
    Delayms(5);
    Lcd_write(0,0,0x02   ,1); //return home
    Delayms(5);
    Lcd_Goto(0,0);
    LCD_Puts(" Set Alarm for ");
    Lcd_Goto(5,1);
    Lcd_write_4(2,Time.hour,1);
    LCD_Puts(":");
    Lcd_write_4(2,Time.minute,1);
}
void DisplayLCD_SynTime(struct_calendar dayTime){
    Lcd_write(0,0,0x01   ,1); // xoa man hinh
    Delayms(5);
    Lcd_write(0,0,0x02   ,1); //return home
    Delayms(5);
    DisplayLCD_DayTime(dayTime);
    Lcd_Goto(0,1);
    LCD_Puts("Syn@");
}
