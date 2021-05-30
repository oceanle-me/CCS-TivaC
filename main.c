
#include <alarmClock.h>
#include <MyI2C.h>
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
#include "PCF8574A_LCD2004_123GH6PM.h"
#include "utils/ustdlib.h"
#include "driverlib/hibernate.h"

void UART0_Int(void);
void UART1_Int(void);
void Hibernate_Int(void);
void Initialzation(void);


int main(void)
{

    Initialzation();
 //   AlarmAction();
    int cnt=0;
while(1)
{ cnt++;}
}

void Hibernate_Int(void){
    HibernateIntClear(HIBERNATE_INT_RTC_MATCH_0);
    synTime = CounterToStructCalender(HibernateRTCGet());
    DBG("%u %u/%u/%u %u:%u:%u\n",synTime.wday, synTime.day,synTime.month, synTime.year,
               synTime.hour, synTime.minute, synTime.second);
    HibernateDataGet((uint32_t *)(&alarmTime), 2);
    /*********************************************/
    if  (CheckAlarm(alarmTime, synTime)) {
        AlarmAction();
    }
    uint32_t a = GPIO_PORTF_DATA_R;// tai vi day xoa port F nen green cung bi xoa

    DisplayLCD_DayTime(synTime);

    SysCtlDelay(SysCtlClockGet()/3);
    GPIO_PORTF_DATA_R=0b10;
    do{
       HibernateRTCMatchSet(0,HIB_RTCC_R+20);
  }
   while (HIB_RTCC_R > HIB_RTCM0_R+5);

HibernateIntDisable(HIBERNATE_INT_RTC_MATCH_0);
IntMasterDisable();
HibernateRequest();
}



void UART1_Int(void)
{
    static int countChar=0;
    char charGet;
    static typeUARTSignal UARTSignal;
    uint32_t ui32Status;
    ui32Status = UARTIntStatus(UART1_BASE, true); //get interrupt status
    UARTIntClear(UART1_BASE, ui32Status);

    charGet = UARTCharGet(UART1_BASE);
    if (charGet=='s'){
        UARTSignal= synSignal;
    }
    else if (charGet=='a'){
        UARTSignal= alarmSignal;
    }
    else  if (charGet=='x') {
        alarmTime.hour=100;     //this is not a valid hour value
        alarmTime.minute=100;   //this is not a valid minute value
        HibernateDataSet((uint32_t *)(&alarmTime), 2);
        DBG("Off Alarm\n");
        DisplayLCD_OffAlarm();
    }
    else if ((charGet=='e')&&(UARTSignal== synSignal)) {
        countChar=0;
        AdjustUARTSynTime();
        synTime = UARTToStructCalendar();
        HibernateRTCSet(StructCalenderToCounter(synTime)); //save synTime to RTC
        DBG("Syn Day and Time %u %u/%u/%u %u:%u:%u\n",synTime.wday, synTime.day,synTime.month, synTime.year,
            synTime.hour, synTime.minute, synTime.second);
        DisplayLCD_SynTime(synTime) ;
    }
    else if ((charGet=='e')&&(UARTSignal== alarmSignal)) {
        countChar=0;
        AdjustUARTAlarmTime();
        alarmTime = UARTToStructAlarm();
        HibernateDataSet((uint32_t *)(&alarmTime), 2);//save alarmTime to hibernation memory
        DBG("Set Alarm %u:%u\n",alarmTime.hour,alarmTime.minute);
        DisplayLCD_OnAlarm(alarmTime);
    }
    else if(UARTSignal== synSignal){
        UARTSynTime[countChar]= charGet;
        countChar++;
    }
    else if (UARTSignal== alarmSignal){
        UARTAlarmTime[countChar]= charGet;
        countChar++;
    }
}
void Initialzation(void){

    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
         (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    IntEnable(INT_UART0); //enable the UART interrupt
    UARTIntEnable(UART0_BASE, UART_INT_RX|UART_INT_RT);
    UARTEnable(UART0_BASE);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
         (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    IntEnable(INT_UART1); //enable the UART interrupt
    UARTIntEnable(UART1_BASE, UART_INT_RX);
    UARTEnable(UART1_BASE);
    UARTStdioConfig(0, 115200, SysCtlClockGet());
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_4);

    InitI2C0();
    //IntPriorityMaskSet(ui32PriorityMask)

    SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
    HibernateEnableExpClk(0);  //Enables the Hibernation module, the argument is not helpful
    HibernateRTCDisable();
    HibernateGPIORetentionEnable();  // using VDD3ON mode
    HibernateRTCEnable();
    HibernateWakeSet(HIBERNATE_WAKE_PIN | HIBERNATE_WAKE_RTC);
    HibernateIntRegister(Hibernate_Int);
    HibernateIntEnable(HIBERNATE_INT_RTC_MATCH_0);// set bit in IM enable HIB to send a INT signal
    IntMasterEnable();  // this is really not helpful
    HibernateRTCMatchSet(0,HibernateRTCGet()+20);
    Lcd_init();
    synTime = CounterToStructCalender(HibernateRTCGet());
    DisplayLCD_DayTime(synTime);

}


