#ifndef SYS_1MS_H
#define SYS_1MS_H

//#include <pthread.h>

#include <rtthread.h>

#define SOFTTIMERLOCK
#undef SOFTTIMERLOCK

extern int sys1ms_init(void);


typedef struct
{
	unsigned int count;			//时间计数器
	unsigned int max;				//触发点
	unsigned char  flag;		//时钟标志
#ifdef SOFTTIMERLOCK
	struct rt_mutex  mutex;
#endif
}TSoftTimer;

enum
{
	TIMER_FLAG_DISABLE		=0,		//时钟不可用
	TIMER_FLAG_ENABLE			=1,		//时钟有效
	TIMER_FLAG_TRIGGER		=2		//时钟触发
};

enum{

	CAN1_1_TIMER = 0,
	CAN1_2_TIMER = 1,
	CAN1_3_TIMER = 2,

	CAN2_1_TIMER = 3,
	CAN2_2_TIMER = 4,
	CAN2_3_TIMER = 5,

	SOFTTIMER_NUM = 6
};

#ifdef SOFTTIMERLOCK
#define InitSoftTimerMutex(TimerID)		rt_mutex_init(&(SoftTimer[TimerID].mutex),NULL,RT_IPC_FLAG_FIFO)
#define LockSoftTimer(TimerIDs)				rt_mutex_take(&(SoftTimer[TimerID].mutex),RT_WAITING_FOREVER)
#define UnLockSoftTimer(TimerID)			rt_mutex_release(&(SoftTimer[TimerID].mutex))
#else
#define InitSoftTimerMutex(TimerID)		do{}while(0)
#define LockSoftTimer(TimerID)				do{}while(0)
#define UnLockSoftTimer(TimerID)			do{}while(0)
	
#define CloseSoftTimer(TimerID) 			SoftTimer[TimerID].flag=TIMER_FLAG_DISABLE;

#define GetSoftTimerStatus(TimerID)		SoftTimer[TimerID].flag

#define InitSoftTimer(TimerID, nTime) 						\
	do{																							\
		LockSoftTimer(TimerID);												\
		SoftTimer[TimerID].max=nTime;									\
		SoftTimer[TimerID].count=0;										\
		SoftTimer[TimerID].flag=TIMER_FLAG_ENABLE;		\
		UnLockSoftTimer(TimerID);											\
	}while(0)

#define	InitSoftTimerEx(TimerID,nTime,nInitCount)	\
	do{																							\
		InitSoftTimerMutex(TimerID);									\
		LockSoftTimer(TimerID);												\
		SoftTimer[TimerID].max=nTime;									\
		SoftTimer[TimerID].count=nInitCount;					\
		SoftTimer[TimerID].flag=TIMER_FLAG_ENABLE;		\
		UnLockSoftTimer(TimerID);											\
	}while(0)



#endif


extern  TSoftTimer SoftTimer[];

#define CAN1_1_TIMER_VALUE	5		//50ms
#define CAN1_2_TIMER_VALUE	2		//20ms
#define CAN1_3_TIMER_VALUE	6000	//60s

#define CAN2_1_TIMER_VALUE	20		//200ms
#define CAN2_2_TIMER_VALUE	100		//1s
#define CAN2_3_TIMER_VALUE	6000	//60s

void set_mscnt(int ms);
#endif

