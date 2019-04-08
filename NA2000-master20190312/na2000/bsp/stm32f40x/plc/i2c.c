/*************************************************************************
	> File Name: src/rtc.c
# Author: jerry(yang wanying)
# mail: yangwy@nandaauto.com
	> Created Time: 2015年05月21日 星期四 08时54分31秒
 ************************************************************************/
#include <sysapp.h>

extern void set_date(rt_uint32_t year, rt_uint32_t month, rt_uint32_t day);
extern void set_time(rt_uint32_t hour, rt_uint32_t minute, rt_uint32_t second);

int RTCRead(CLOCK *p)
{
	time_t now;
	struct tm* ti;
	time(&now);
	ti = localtime(&now);
	
	if (ti != RT_NULL)
	{
		p->Second = ti->tm_sec;
		p->Minute = ti->tm_min;
		p->Hour = ti->tm_hour;
		p->Day = ti->tm_mday;
		p->Week = (ti->tm_wday == 0)? 7:ti->tm_wday;
		p->Month = ti->tm_mon + 1;
		p->Year = ti->tm_year + 1900;	
	}
	return 0;
}

int RTCWrite(CLOCK *p) 
{
	rt_uint32_t year, month, day;
	rt_uint32_t hour,minute,second;
	
	second =  p->Second;
	minute = p->Minute;
	hour =  p->Hour;
	day = p->Day;
	month =  p->Month;
	year = p->Year;
	
	set_date(year,month,day);
	set_time(hour,minute,second);

	return 0;
}

