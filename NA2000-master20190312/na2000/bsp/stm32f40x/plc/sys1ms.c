#include "sysapp.h"
#include  "sys1ms.h"
#include "dbext.h"

#include "stm32f4xx.h"
#include "manage.h"



/********************1ms¡¢10ms Timer*************************************/
TSoftTimer SoftTimer[SOFTTIMER_NUM];

static unsigned short mscnt = 0;
static int tmrcnt[MAX_INT_PRG_NUM];

extern struct rt_semaphore sem_tmr[];
extern rt_mutex_t mutex_scan_time;

void process_1ms(void *);
extern void di_filt(void);

void process_10ms(void *parameter)
{
	unsigned char i = 0;

	for (i=0;i<SOFTTIMER_NUM;i++)
	{
		if(SoftTimer[i].flag==TIMER_FLAG_ENABLE)
		{
			SoftTimer[i].count ++;
			if(SoftTimer[i].count>=SoftTimer[i].max)
			{
				LockSoftTimer(i);
				SoftTimer[i].flag=TIMER_FLAG_TRIGGER;
				UnLockSoftTimer(i);
			}
		}
	}
}

void process_1ms(void* parameter)
{
	int i = 0;
	
	mscnt++;
	if (mscnt>=1000)
	{
		mscnt=0;
		if (pCCUClock->Second<59)
			pCCUClock->Second=pCCUClock->Second+1;
		else
		{
			pCCUClock->Second=0;
			CalenderCheck();
		}
		Light(SYS_RUN_LED,1);
		Output_S(MST_SEC1,1);
		Output_S(T_SEC,1);
	}
	else if (mscnt==500)
	{
		Light(SYS_RUN_LED,0);
		Output_S(MST_SEC1,0);
		Output_S(T_SEC,0);
	}
	pCCUClock->Ms = mscnt;
	pStaticData->Pad3+=1;
	if (pStaticData->Pad3==1000)
		pStaticData->Pad3=0;
	
	if (pStaticData->int_enable_flag)
	{
		for(i=0;i<MAX_INT_PRG_NUM;i++)
		{
			if( pStaticData->IntPrgTime[i]!=0)
			{
				tmrcnt[i]+=1;
				if (tmrcnt[i]>=pStaticData->IntPrgTime[i])
				{
					tmrcnt[i]=0;
					rt_sem_release(&sem_tmr[i]);
				}
			}
		}
	}
	else
	{
		for(i=0;i<MAX_INT_PRG_NUM;i++)
		{
			tmrcnt[i]=0;
		}
	}
}

void set_mscnt(int ms)
{
	
}

void Na2000_Timer_init()
{
	rt_timer_t timer1ms;
	rt_timer_t timer10ms;
	
	timer1ms = rt_timer_create("1ms",process_1ms, RT_NULL,1,RT_TIMER_FLAG_PERIODIC);
	if (timer1ms != RT_NULL) 
		rt_timer_start(timer1ms);
	
	timer10ms = rt_timer_create("10ms",process_10ms, RT_NULL,10,RT_TIMER_FLAG_PERIODIC);
	if (timer10ms != RT_NULL) 
		rt_timer_start(timer10ms);
}
