#include "sysapp.h"
extern rt_err_t rt_thread_sleep(rt_tick_t tick);

uint8_t gbiax=0;

extern struct rt_semaphore sem_tmr[];

void intprgthread(void *lp)
{
	int rc;
	unsigned char id = *(unsigned char *)(lp);
	
	while (1)
	{
		rt_sem_take(&sem_tmr[id],RT_WAITING_FOREVER);
		if (Get_S(MST_M1) && !Get_S(MST_STOP1))
		{
			if( pStaticData->IntPrgNo[id] <= pPrgData->PrgNum && pStaticData->IntPrgNo[id]>0)
			{
				rt_sem_take(sem_prg,RT_WAITING_FOREVER);
				rt_sem_release(sem_prg);
				rc=PrgEntry[pStaticData->IntPrgNo[id]-1]();
				pPrgData->CurrentStep[pStaticData->IntPrgNo[id]-1]=0;
				if (rc)
				{
					pDynamicData->pSW.Value[EXECERR_INFO-1] = pStaticData->IntPrgNo[id];
					pDynamicData->pSW.Value[EXECERR_INFO] = rc;
					pDynamicData->pSW.Value[EXECERR_INFO+1] = 0;
					Output_S(LD_EXECERR, 1);
				}
			}
		}
	}
}

void taskprgthread(void *lp)
{
	int rc;
	unsigned char id = *((unsigned char *)lp);
	
	while (1)
	{
		if (Get_S(MST_M1) && !Get_S(MST_STOP1) && !Get_S(FST_SCN))
		{
			if( pStaticData->TaskPrgNo[id] <= pPrgData->PrgNum )
			{
				rt_sem_take(sem_prg,RT_WAITING_FOREVER);
				rt_sem_release(sem_prg);
				rc=PrgEntry[pStaticData->TaskPrgNo[id]-1]();
				pPrgData->CurrentStep[pStaticData->TaskPrgNo[id]-1]=0;
				if (rc)
				{
					pDynamicData->pSW.Value[EXECERR_INFO-1] = pStaticData->TaskPrgNo[id];
					pDynamicData->pSW.Value[EXECERR_INFO] = rc;
					pDynamicData->pSW.Value[EXECERR_INFO+1] = 0;
					Output_S(LD_EXECERR, 1);
				}
			}
			rt_thread_delay(pStaticData->TaskPrgTime[id] * RT_TICK_PER_SECOND/1000);
		}
	}
}


void lineprgthread(void *lp)
{
	uint8_t State=0;
	int32_t Pon0=0;
	int32_t Pon1=0;
	int32_t Pos0=0;	
	int32_t Pos1=0;
	uint32_t time_8ms=0;
	uint8_t a=0,b=0;
	uint8_t multiple=0;
	//uint8_t line_time=0;
	rt_kprintf("\n****enter lineprgthread\n");
	extern unsigned int DI_16_Read(void);
	while (1)
	{
		if(DI_16_Read()&(1<<3))//I4=1
			multiple=2;
		else multiple=1;
		
		if (Get_S(MST_M1) && !Get_S(MST_STOP1) )
		{
			if(DI_16_Read()&(1<<2))//I3=1
			{
				for(uint32_t i=0;i<400&&(a==0);i++)
				{
					rt_sem_take(sem_lineprg,RT_WAITING_FOREVER);
					//if(line_time==0)
					mypline(10,PQ1,PQ2,ON,INC,80*multiple, 80*multiple,&State, &Pon0, &Pon1, &Pos0, &Pos1);
					if(State==1)
					{
						mypline(10,PQ1,PQ2,OFF,INC,80*multiple, 80*multiple,&State, &Pon0, &Pon1, &Pos0, &Pos1);
					  rt_sem_release(sem_lineprg);
					}
				}
				a=1;
				b=0;
			}
			else//I3=0
			{
				for(uint32_t i=0;i<400&&(b==0);i++)
				{
					rt_sem_take(sem_lineprg,RT_WAITING_FOREVER);
					//if(line_time==0)
					mypline(10,PQ1,PQ2,ON,INC,-80*multiple, -80*multiple,&State, &Pon0, &Pon1, &Pos0, &Pos1);
					if(State==1)
					{
						mypline(10,PQ1,PQ2,OFF,INC,-80*multiple, -80*multiple,&State, &Pon0, &Pon1, &Pos0, &Pos1);
					  rt_sem_release(sem_lineprg);
					}
				}
				a=0;
				b=1;
			}
			if(time_8ms<1)
				printf("Pos0=%d,Pos1=%d,Pon0=%d,Pon1=%d\n",Pos0,Pos1,Pon0,Pon1);
			//line_time=1;
			rt_thread_sleep(2);
			if(++time_8ms>=1000)
			{
		//		printf("Pos0=%d,Pos1=%d,Pon0=%d,Pon1=%d\n",Pos0,Pos1,Pon0,Pon1);
				time_8ms=0;
			}
		}	
	}
}


