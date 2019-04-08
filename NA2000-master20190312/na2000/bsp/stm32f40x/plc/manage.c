#include <stdint.h>
#include <rtthread.h>
#include "stm32f4xx.h"
#include <stdint.h>
#include "sysapp.h"
#include "threaddef.h"
#include <time.h>

#include "flash_if.h"

#include "manage.h"
#include "lwip/netifapi.h"
#include "CPLD.h"

#include "gprs.h"
#include "commgr.h"

#include "history.h"

#include <drivers/board.h>

#define LINE_PRG_PRIORITY        (14)
#define INT_PRG_PRIORITY                       				17
#define MANAGE_PRIORITY                               18
#define CAN1_PRIORITY                                 19
#define NORMAL_PRIORITY                               20

#define MIN_SLEEP_TIME																3


#define NA_DEBUG
#undef NA_DEBUG

#ifdef NA_DEBUG
#define dbg_printf(fmt,arg...) rt_kprintf(fmt,##arg)
#else 
#define dbg_printf(fmt,arg...) do{}while(0)
#endif

struct rt_semaphore sem_tmr[MAX_INT_PRG_NUM] = {{0,},};
const unsigned char id_table[MAX_TASK_PRG_NUM]= {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,};
struct rt_completion can1;
struct rt_completion can1ok;

unsigned char cputype;

extern char get_submodule_addr_flag(void);
extern void Init_CPLD(void);
extern void init_bd_board(int type);
extern void wdgenable(void);
extern void wdgreset(void);

void sem_init()
{
	int i=0;
	char sem_name[10] = "";
	for(i=0;i<MAX_INT_PRG_NUM;i++)
	{
		rt_sprintf(sem_name,"semtmr%d",i+1);
		rt_sem_init(&sem_tmr[i],sem_name,0,RT_IPC_FLAG_FIFO);
	}

	rt_completion_init(&can1);
	rt_completion_init(&can1ok);
}

static int get_cpu_type()
{
	int fd = -1;
	char tmp[10] = "0";
	int type = 45;
	int len = 0;
	if ((fd = open("/na/cputype.txt", O_RDONLY, 0)) < 0)
	{
		rt_kprintf("open /na/cputype.txt file failed!!\n");
		return type;
	}
	
	len = lseek(fd,0L,SEEK_END);
	if(len > 0)
	{
		lseek(fd,0,SEEK_SET);
		read(fd,tmp,len>9?9:len);
	}
	close(fd);
	
	sscanf(tmp,"%d",&type);
	return type;
}

int set_cpu_type(int type)
{
	int fd = -1;
	char tmp[10] = "0";
	int old = get_cpu_type();

	if ((fd = open("/na/cputype.txt", O_RDWR | O_CREAT | O_TRUNC, 0)) < 0)
	{
		rt_kprintf("open /na/cputype.txt file failed!!\n");
		return -1;
	}

	lseek(fd,0,SEEK_SET);
	sprintf(tmp,"%d",type);
	write(fd,tmp,strlen(tmp));

	close(fd);

	rt_kprintf("The old cpu type is:%d, the new type:%d\n",old,type);
	return old;
}

#ifdef RT_USING_FINSH
#include "finsh.h"
FINSH_FUNCTION_EXPORT_ALIAS(set_cpu_type,cputype,set cpu type. e.g:cputype(41):2401->41 2402->42 ...);
#endif

int OnLineMod(void)
{
	int fd = -1,count,ret = -1,len=0,i=0;
	char* fullpath  = "/na/na2000.bin";
	uint32_t bin_flashaddr ;//bin文件，flash的起始地址
	unsigned short PrgNum;
	unsigned short FbNum;
	uint8_t* buff = RT_NULL;
	
	if (rt_sem_take(sem_prg,100)!=0)
		return -1;

	//if (rt_sem_take(sem_lineprg,100)!=0)
		
		//return -1;
	buff = rt_malloc(1024);
	if(buff == RT_NULL)
		goto out;
	
	if ((fd = open("/na/na2000.ent", O_RDONLY, 0)) < 0)
	{
		rt_kprintf("open /na/na2000.ent file faied!!\n");
		goto out;
	}
	
	read(fd,&PrgNum,2);
	read(fd,&FbNum,2);
	
	read(fd,PrgEntry,PrgNum*4);
	read(fd,FbEntry,FbNum*4);
	
	close(fd);

	//FLASH
	fd = open(fullpath,O_RDONLY,0);
	if(fd < 0)
	{
		rt_kprintf("open /na/na2000.bin file faied!!\n");
		goto out;
	}
	
	len = lseek(fd,0L,SEEK_END);
	
	count = (len + 128*1024 - 1)/(128*1024);
	FLASH_If_Init();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |FLASH_FLAG_PGSERR);  
	for(i=0;i<count;i++)
	{
		FLASH_EraseSector(FLASH_Sector_8 + i*8, VoltageRange_3);	//128K
		wdgreset();
	}
	
	bin_flashaddr = USER_BIN_ADDRESS;
	lseek(fd,0L,SEEK_SET);
	
	while((count = read(fd,buff,1024)) > 0)
	{
		ret= FLASH_If_Write(bin_flashaddr, (uint32_t*)(buff) ,(count+3)/4);
		if( ret!= 0)
			rt_kprintf("FLASH Write error,ret:%d\n",ret);
		bin_flashaddr += count;
	}
	
	dbg_printf("download /na/na2000.bin file OK!!\n");
	dbg_printf("pPrgData->PrgNum=%d,pFBData->FbNum=%d\n",pPrgData->PrgNum,pFBData->FbNum);
	dbg_printf("ld OK!\n");
	ret = 0;
	
out:
	if(fd >= 0)
		close(fd);
	
	if(buff)
		rt_free(buff);
	
	rt_sem_release(sem_prg);
	//rt_sem_release(sem_lineprg);
	
	return ret;
}

int jump_bin_app()
{
	typedef  int (*pFunction)(void);
	return ((pFunction)(USER_BIN_ADDRESS + 1))();
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(jump_bin_app,jump_bin_app);
#endif

void store_onlinemod_flag(char flag)
{
	int fd = -1;
	if ((fd = open("/na/mod.flg", O_RDWR | O_CREAT | O_TRUNC, 0)) < 0)
	{
		rt_kprintf("open /na/mod.flg file failed!!\n");
		return;
	}
	if(flag < '0')
		flag += '0';
	
	lseek(fd,0L,SEEK_SET);
	write(fd,&flag,1);
	close(fd);
}

char load_onlinemod_flag()
{
	int fd = -1;
	char flag = '0';
	int len = 0;
	if ((fd = open("/na/mod.flg", O_RDONLY, 0)) < 0)
	{
		rt_kprintf("open /na/mod.flg file failed!!\n");
		return flag - '0';
	}
	
	len = lseek(fd,0L,SEEK_END);
	if(len > 0)
	{
		lseek(fd,0,SEEK_SET);
		read(fd,&flag,1);
	}
	close(fd);
	
	return flag - '0';
}

void msgprc(rt_uint32_t tick)
{
	unsigned char buff[256];

	while (rt_mq_recv(MsgQid, buff, sizeof(buff), tick)==0)
	{
		switch (buff[1])
		{
		case 0x81:
			ResetCPU();
			break;

		case 0x83:
			rt_memcpy(pCCUClock,&buff[2],8);
			pCCUClock->Week = CalculateWeekDay(pCCUClock->Year,pCCUClock->Month,pCCUClock->Day);
			RTCWrite(pCCUClock);
			break;

		case 0x92:
			if(buff[2] == 0)
			{
				OnLineMod();
			}
			else if(buff[2] == 1)
			{
				dbg_printf("store flash flag into /na/mod.flg\n");
				store_onlinemod_flag(buff[2]);
			}
			break;

		default:
			MGR_ProcessWT(buff);
			break;
		}
	}
}

void di_process()
{
	unsigned short divalue;
	divalue = DI_16_Read();

	Move_Fuc( &pDynamicData->pI.RawData[0], (unsigned int)pStaticData->pMDU[mon_module-1].IRefAddr, 
		&divalue, 1, (unsigned int)pStaticData->pMDU[mon_module-1].INum	);
	Drv_Multi_I(pStaticData->pMDU[mon_module-1].IRefAddr, pStaticData->pMDU[mon_module-1].INum);
}

void do_process()
{
	unsigned short i,no,bias;
	unsigned char dovalue;

	for (i=pStaticData->pMDU[mon_module-1].QRefAddr-1;i<pStaticData->pMDU[mon_module-1].QRefAddr+pStaticData->pMDU[mon_module-1].QNum-1;i++) 
	{
		if ((pDynamicData->pQ.PulseTime[i] != HOLD_TYPE) && (pDynamicData->pQ.PulseTime[i] != HOLD_TYPE1))/*处理脉冲开出*/
		{
			if (pDynamicData->pQ.PulseTime[i]>(pStaticData->Myticks))
				pDynamicData->pQ.PulseTime[i]-=(pStaticData->Myticks);
			else
				pDynamicData->pQ.PulseTime[i]=0;
			if (pDynamicData->pQ.PulseTime[i]==0)
			{
				pDynamicData->pQ.State[i/8]= ( pDynamicData->pQ.State[i/8] & CLRBIT(i%8) ); 
				pDynamicData->pQ.Output[i/8]= ( pDynamicData->pQ.Output[i/8] & CLRBIT(i%8) );
			}
		}
	}

	no=(pStaticData->pMDU[mon_module-1].QRefAddr-1)/8;
	bias=(pStaticData->pMDU[mon_module-1].QRefAddr-1)%8;

	if (bias==0)
		dovalue=pDynamicData->pQ.Output[no];
	else
		dovalue=(pDynamicData->pQ.Output[no]>>bias)|(pDynamicData->pQ.Output[no+1]<<(8-bias));
	DO_8_Write(dovalue);
}

//success:return value is 0
//failed:return value is -1
int na_create_thread(const char* name,
														void (*entry)(void *parameter),
                            void       *parameter,
                            rt_uint32_t stack_size,
                            rt_uint8_t  priority,
                            rt_uint32_t tick)
{
	rt_thread_t tid;
	tid = rt_thread_create(name,entry,parameter,stack_size,priority,tick);
	if(tid != RT_NULL)
	{
		dbg_printf("create %s thread successed\n",name);
		rt_thread_startup(tid);
	}
	else
	{
		rt_kprintf ("create %s thread error\n",name);
		return -1;
	}
	return 0;
}

static int create_int_prgs()
{
	unsigned char i=0;
	int ret = 0;
	for(i = 0;i < MAX_INT_PRG_NUM;i++)
	{
		char name[10] = "";
		if(pStaticData->IntPrgTime[i]==0) continue;
		rt_sprintf(name,"intprg%d",i);
		ret += na_create_thread(name,intprgthread,(void*)&id_table[i],1024,INT_PRG_PRIORITY,20);
	}
	return ret;
}

static int create_line_prgs()
{
	
	int ret = 0;
	
	ret = na_create_thread("line",lineprgthread,RT_NULL,1024,LINE_PRG_PRIORITY,2);
	
	return ret;
}

static int create_task_prgs()
{
	unsigned char i=0;
	int ret = 0;
	for(i = 0;i < MAX_TASK_PRG_NUM;i++)
	{
		char name[10] = "";
		if (pStaticData->TaskPrgNo[i]==0) continue;
		rt_sprintf(name,"tprg%d",i);
		ret += na_create_thread(name,taskprgthread,(void*)&id_table[i],1024,NORMAL_PRIORITY,20);
	}
	return ret;
}

static void def_led_indicate(void *para)
{
	unsigned char defflag = *(unsigned char*)para;
	static int i = 0;
	i = !i;
	if (defflag)
	{
		Light(SYS_RUN_LED,i);
		Light(SYS_FAULT_LED,i);
	}
	else
		Light(SYS_FAULT_LED,i);
}

static void clear_backup_sram_when_first_startup()
{
	char flag = '0';
	int i = 0;
	int fd = open("/na/firsts.flg", O_RDWR, 0);
	
	if(fd >= 0) 
		goto out;
	
	if ((fd = open("/na/firsts.flg", O_RDWR | O_CREAT | O_TRUNC, 0)) < 0) 
		goto out;
	
	for(i = 0; i < 4096; i++)
	{
		*(char *) (BKPSRAM_BASE+i) = 0;
	}
	
	lseek(fd,0,SEEK_SET);
	write(fd,&flag,1);
	
out:
	if(fd >= 0)
		close(fd);
}

void manage_main(void* param)
{
	int rc=0,i=0;
	unsigned short msold,msnow;
	unsigned char lastState=0;
	unsigned char defflag=0;
	char onlinemod_flag = 0;
 
	//make sure rt-thread core has init finished!!!so we first sleep 400 ms
	rt_thread_delay((400*RT_TICK_PER_SECOND)/1000);
	
	rt_kprintf("NA2000 PLC (stm32f407) is starting ...\n");

	SW_GPIO_Init();
	serial_fd_init();
	clear_backup_sram_when_first_startup();
	
	plc_data_init();
	
	RTCRead(pCCUClock);

	debug_flag=(GetHardSW()==1);
	if (debug_flag) defflag=1;

	cputype = get_cpu_type();
	
	if (!defflag)
	{
		rc=SysInit(cputype);
		if (rc) 
		{
			rt_kprintf("system init [%d] error = %d\n", cputype,rc);
		}
		else
		{
			rt_kprintf("system init OK\n");
		}
	}
	else
	{
		rc=DefaultSysInit(cputype);
	}
	
	sem_prg = rt_sem_create("prg",1,0);
	
	if (RT_NULL == sem_prg) 
	{
		rt_kprintf("create sem_prg failed\n");
		return;
	}
	sem_lineprg = rt_sem_create("lineprg",1,0);
	
	if (RT_NULL == sem_lineprg) 
	{
		rt_kprintf("create sem_lineprg failed\n");
		return;
	}
	
	MsgQid = rt_mq_create("msg",256,16,0);
	if (RT_NULL == MsgQid)
	{
		rt_kprintf("create msg queue failed.\n");
		return;
	}
	
	if(na_create_thread("ethdbg",ethdbg,RT_NULL,1024,NORMAL_PRIORITY,20)<0)
		return;

	#ifdef RT_USING_NETUTILS
		{
			extern void telnet_srv(void);
			telnet_srv();
			rt_thread_delay(100);
		}
	#endif
		
	if(pConfigData->Com[0].Protocol == 0)
	{
		#ifdef RT_USING_FINSH
    /* init finsh */
		#ifdef RT_USING_CONSOLE
			rt_console_set_device(CONSOLE_DEVICE);
		#endif
			finsh_set_device( FINSH_DEVICE_NAME );
		#endif
	}
	
	if (defflag || rc)
	{
		rt_timer_t def_led_t;
		Output_S(MST_DEBUG1,1);
		
		//create a 500 ms timer for led show
		def_led_t = rt_timer_create("def_led",def_led_indicate,&defflag,
																500*RT_TICK_PER_SECOND/1000,RT_TIMER_FLAG_PERIODIC);
		if(def_led_t)
			rt_timer_start(def_led_t);
		
		while(1)
		{
			rt_thread_delay(2);
			msgprc(20);
		}
	}
	
	onlinemod_flag = load_onlinemod_flag();
	dbg_printf("OnLineMod_flag = %d\n",onlinemod_flag);
	if(onlinemod_flag)
	{
		OnLineMod();
		store_onlinemod_flag(0);
	}

	wdgenable();
	
	/*设置以太网*/
	SetIP();

	debug_flag=(GetHardSW()==1);
	Output_S(MST_DEBUG1,debug_flag);

	Output_S(MST_STOP1,(GetHardSW()==2));
	Light(SYS_ACTIVE_LED,!Get_S(MST_STOP1));

	/*rt_sem_init*/
	sem_init();
	
	for(i=0;i<MAX_COM_NUM-2;i++)
	{
		char name[20] = "";
		/*打开串口线程*/
		switch(pConfigData->Com[i].Protocol)
		{
			case 1:
			{
				/*自由口*/
				/* 初始化消息队列 */
				rt_sprintf(name,"com%dqueue",i+1);
				ComQid[i] = rt_mq_create(name,256+1,8,RT_IPC_FLAG_FIFO);
				if(ComQid[i] == (rt_mq_t)RT_ERROR)
				{
					rt_kprintf("create com1_free_queue failed.\n");
					return;
				}
				
				rt_sprintf(name,"com%dfree",i+1);
				if(na_create_thread(name,comfree,(void*)&id_table[i],512*4, MANAGE_PRIORITY, 20)<0)
					return;
			}
			break;
			case 2:
			{
				/*ModBus RTU*/
				rt_sprintf(name,"com%drtu",i+1);
				if(na_create_thread(name,commodrtu,(void*)&id_table[i],512*3, NORMAL_PRIORITY, 20)<0)
					return;
			}
			break;
			case 3:
			if (pConfigData->Com[i].ModNum>0)
			{//ModBus Master
				rt_sprintf(name,"com%dmst",i+1);
				if(na_create_thread(name,commodmst,(void*)&id_table[i],512*4, NORMAL_PRIORITY, 20)<0)
					return;
			}
			break;
			default:
			break;
		}
	}

	//todo:support lcd
//	if(na_create_thread("lcd_mst",commodmst_for_lcd,(void*)&id_table[2],512*2, NORMAL_PRIORITY, 20)<0)
//		return;
	
	if(na_create_thread("gprs",gprs_thread,(void*)&id_table[2],512*2, NORMAL_PRIORITY, 20)<0)
			return;

	if (pConfigData->CAN1MduSum)
	{
		if(na_create_thread("can1mgr",can1mgr, RT_NULL,512*3, CAN1_PRIORITY, 20)<0)
			return;
	}

	if(create_int_prgs() < 0)
		return;
	
	
	if(create_task_prgs() < 0)
		return;

	if(na_create_thread("ethrun",ethrun,RT_NULL,1024,NORMAL_PRIORITY,20)<0)
		return;
	
	if (pStaticData->ModtcpNum > 0) 
	{
		if(na_create_thread("ethmtm",ethmtm,RT_NULL,512,NORMAL_PRIORITY,20)<0)
			return;
	}
	
	Na2000_Timer_init();
	
	Init_CPLD();
	
	if(create_line_prgs() < 0)
		return;
	
	if(pStaticData->pMDU[0].ComDef1[0].End !=0 )
		init_bd_board(pStaticData->pMDU[0].ComDef1[0].End);

//	start_history_dem(1,0,10000,0,200,0,1);
	
	//make sure all sub-thread startup success!!so we first sleep 100 ms
	rt_thread_delay(100*RT_TICK_PER_SECOND/1000);
	
	msold=msnow=pCCUClock->Ms;
	
	while(1)
	{ 
		rt_thread_delay(MIN_SLEEP_TIME);
		
		debug_flag=(GetHardSW()==1);
		
		Output_S(MST_DEBUG1,debug_flag);
		Output_S(MST_STOP1,(GetHardSW()==2));
		
		Output_S(MST_SELF_ON1,!Get_S(MST_STOP1));
		Light(SYS_ACTIVE_LED,Get_S(MST_SELF_ON1));

		lastState= Get_S(MST_M1);

		SysTimerRead(pCCUClock);

		msnow=pCCUClock->Ms;
		pStaticData->Myticks=msold>msnow ? 1000+msnow-msold : msnow-msold;
		msold=msnow;
		
		if(lastState)
		{
			Output_SW(TIME_INFO, pCCUClock->Year);
			Output_SW(TIME_INFO+1, pCCUClock->Month);
			Output_SW(TIME_INFO+2, pCCUClock->Day);
			Output_SW(TIME_INFO+3, pCCUClock->Hour);
			Output_SW(TIME_INFO+4, pCCUClock->Minute);
			Output_SW(TIME_INFO+5, pCCUClock->Second);
			Output_SW(TIME_INFO+6, pCCUClock->Ms);
			Output_SW(TIME_INFO+7, pCCUClock->Week);
			memcpy(&pDynamicData->pSW.Value[SOE_PTR-1], &pDynamicData->pEVENT.pDef[0].CurPtr, 2);
			memcpy(&pDynamicData->pSW.Value[ALARM_PTR-1], &pDynamicData->pEVENT.pDef[1].CurPtr, 2);
			
			pDynamicData->pSW.Value[SCAN_TIME-1]= pStaticData->Myticks;
			
			if (pDynamicData->pSys.LadExecFirstTime == 1)
			{
				if (pConfigData->CAN1MduSum )
				{
					rt_completion_done(&can1);
					if(has_sub_modules() && get_submodule_addr_flag() )
						rt_completion_wait(&can1ok,RT_WAITING_FOREVER);
				}
				pDynamicData->pSys.LadExecFirstTime = 0;
				pDynamicData->pSW.Value[SCAN_TIME-1]= MIN_SLEEP_TIME;
			}

			Check_T_PerTick(pStaticData->Myticks);
			Check_PID_PerTick(pStaticData->Myticks);

			pStaticData->FbInstState=0;
			memset(pStaticData->FbInstBool,0,sizeof(pStaticData->FbInstBool));

			di_process();
			get_hsc_status();
			set_pto_status();
			if(!Get_S(LD_OVERRUN) && !Get_S(MST_STOP1))
			{
				rc = jump_bin_app();//rc=PrgEntry[0]();
				if (rc)
				{
					pDynamicData->pSW.Value[EXECERR_INFO-1] = 1;
					pDynamicData->pSW.Value[EXECERR_INFO] = rc;
					pDynamicData->pSW.Value[EXECERR_INFO+1] = 0;
					Output_S(LD_EXECERR, 1);
				}
				Output_S( FST_SCN, 0 );
			}
			
			SeqTsk();
			do_process();
			set_hsc_param();
			msgprc(0);
		}

		if (Get_S(MST_CAN11FLT) || Get_S(MST_CAN12FLT) || Get_S(LD_OVERRUN) || defflag)  //???
		{
			Light(SYS_FAULT_LED,1);
			Output_S(MST_FLT1,1);
		}
		else
		{
			Light(SYS_FAULT_LED,0);
			Output_S(MST_FLT1,0);
		}

		if (pConfigData->CAN1MduSum )
		{
			rt_completion_done(&can1);
			if(has_sub_modules() && get_submodule_addr_flag())
				rt_completion_wait(&can1ok,RT_WAITING_FOREVER);
		}
	}
}

void SetIP()
{
	unsigned long	EthAddr;
	unsigned long	GwAddr;
	unsigned long	MaskAddr;

	EthAddr  = htonl(pStaticData->pMDU[0].EthAddr);
	MaskAddr = htonl(pStaticData->pMDU[0].ComDef[0].Baud);
	GwAddr = htonl(pStaticData->pMDU[0].ComDef[1].Baud);
	
	if (netif_default != RT_NULL)
	{
		netifapi_netif_set_addr(netif_default, (ip_addr_t *)&EthAddr, (ip_addr_t *)&MaskAddr, (ip_addr_t *)&GwAddr);
	}
	else
	{
		rt_kprintf("%s set IP error!!",__func__);
	}
}

#ifdef RT_USING_FINSH
#include <finsh.h>
long reset(void)
{
	char buff[2] = {0,0x81};
	rt_mq_send(MsgQid, buff, sizeof(buff));
	return 0;
}
FINSH_FUNCTION_EXPORT(reset, reset system);

long kill(void)
{
	typedef  int (*pFunction)(void);
	((pFunction)(0))();
	return 0;
}
FINSH_FUNCTION_EXPORT(kill, kill system and reset if watch dog enabled);
#endif
