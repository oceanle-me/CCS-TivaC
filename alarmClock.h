#ifndef ALARMCLOCK_H_
#define ALARMCLOCK_H_
#include <stdint.h>
#include <stdbool.h>
typedef enum {synSignal, alarmSignal} typeUARTSignal;
typedef enum {monday=2, tuesday, wednesday, thursday, friday, saturday, sunday} weekdayname;
//typedef uint32_t weekdayname;
typedef struct {
    weekdayname wday;
    uint32_t     day;
    uint32_t     month;
    uint32_t    year;
    uint32_t     hour;
    uint32_t     minute;
    uint32_t     second;
} struct_calendar;
typedef struct {
    uint32_t     hour;
    uint32_t     minute;
} struct_alarm;

extern struct_calendar   synTime;
extern struct_alarm     alarmTime;
extern char UARTAlarmTime[];
extern char UARTSynTime[];

extern void AdjustUARTAlarmTime(void);
extern void AdjustUARTSynTime(void);
extern struct_calendar UARTToStructCalendar(void);
extern struct_alarm UARTToStructAlarm(void);
extern uint32_t StructCalenderToCounter(struct_calendar structDayTime);
extern struct_calendar CounterToStructCalender(uint32_t uint32Counter);
extern bool CheckAlarm(struct_alarm structTime, struct_calendar structDayTime);
extern void AlarmAction(void);
extern void Timer0A_Int(void);
extern void AlarmSetUp(void);
extern void Switch1_Int(void);
extern void PlayAudio(void);

extern void SetDayAndTime( uint32_t day,  uint32_t month, uint32_t year, uint32_t hour, uint32_t minute,  uint32_t second);

void DisplayLCD_DayTime(struct_calendar structDayTime);
void DisplayLCD_OffAlarm(void);
void DisplayLCD_OnAlarm(struct_alarm Time);
void DisplayLCD_SynTime(struct_calendar dayTime);

#endif /* ALARMCLOCK_H_ */
