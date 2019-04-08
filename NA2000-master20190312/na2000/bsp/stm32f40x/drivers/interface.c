#include <rtthread.h>
#include "stm32f4xx_spi.h"
#include "interface.h"
#include "plc/dbext.h"
#include "plc/sysapp.h"


/*****************double entry***************************/
typedef double (*doublef)(DOUBLE_APP_FUN_CODE code,int *errcode,va_list argp);

static doublef DOUBLE_FUN_LIST[AFC_UNKNOWN_DOUBLE] = {
0,
};

//important!!!!!!!
#pragma arm section code=".ARM.__at_0x0801C000"
double entry_double_f(DOUBLE_APP_FUN_CODE code,...)
{
	doublef func = 0;
	va_list argp;
	int *errcode = 0;
	double ret = 0;

	func = (doublef)DOUBLE_FUN_LIST[code];
	if(func == 0)
		return INTERFACR_ERROR_OUTOF_RANGE;

	va_start (argp, code);
	errcode = va_arg(argp, int *);

	ret = func(code,errcode,argp);

	va_end (argp);

	return ret;
}
#pragma arm section


INTERFACR_ERROR_CODE register_double_func(DOUBLE_APP_FUN_CODE code,doublef func)
{
	if(AFC_UNKNOWN_DOUBLE < code)
		return INTERFACR_ERROR_OUTOF_RANGE;

	if(DOUBLE_FUN_LIST[code] != 0)
		return INTERFACR_ERROR_REG_FUNC;

	DOUBLE_FUN_LIST[code] = func;

	return INTERFACR_NO_ERROR;
}
/*****************double entry***************************/

// define interface raw model
typedef int (*intf)(APP_FUN_CODE code,int *errcode,va_list argp);

static intf FUN_LIST[AFC_UNKNOWN] = {
0,
};

//important!!!!!!!
#pragma arm section code=".ARM.__at_0x0801C400"
int entry_f(APP_FUN_CODE code,...)
{
	intf func = 0;
	va_list argp;
	int *errcode = 0;
	int ret = 0;
	
	func = (intf)FUN_LIST[code];
	if(func == 0)
		return INTERFACR_ERROR_OUTOF_RANGE;
	
	va_start (argp, code);
	errcode = va_arg(argp, int *);
	
	ret = func(code,errcode,argp);
	
	va_end (argp);

	return ret;
}
#pragma arm section

//this for register a command to FUN_LIST
//if the index has a function,got a INTERFACR_ERROR_REG_FUNC error code
INTERFACR_ERROR_CODE register_func(APP_FUN_CODE code,intf func)
{
	if(AFC_UNKNOWN < code)
		return INTERFACR_ERROR_OUTOF_RANGE;
	
	if(FUN_LIST[code] != 0)
		return INTERFACR_ERROR_REG_FUNC;
	
	FUN_LIST[code] = func;
	
	return INTERFACR_NO_ERROR;
}

//for test
//if user application call Fun(APP_FUN_CODE_DEBUG,...),will show "this is a test"
//static int mymalloc(APP_FUN_CODE code,int* errcode,va_list argp)
/*static int debug_fun2(APP_FUN_CODE code,int* errcode,va_list argp)
{
	int line = 0;
	char *fun = 0;
	char *msg = 0;
	//first we get format
	line = va_arg(argp, int);
	fun = va_arg(argp, char*);
	msg = va_arg(argp, char*);
	rt_kprintf("[%s:%d]:%s\n",fun,line,msg);
	
	*errcode = 0;
	
	return 0;
}

//get static  data memory address
static int get_static_memory(APP_FUN_CODE code,va_list argp)
{
	long *pAddr = 0;
	pAddr = va_arg(argp, long*);
	*pAddr = (long)pStaticData;
	
	return INTERFACR_NO_ERROR;
}

//get dynamic data memory address
static int get_dynamic_memory(APP_FUN_CODE code,va_list argp)
{
	long *pAddr = 0;
	pAddr = va_arg(argp, long*);
	*pAddr = (long)pDynamicData;
	
	return INTERFACR_NO_ERROR;
}

//format a string
static int format_string(APP_FUN_CODE code,va_list argp)
{
	long *addr = 0;
	addr = va_arg(argp, long*);
	*(addr) = (long)rt_sprintf;
	
	return INTERFACR_NO_ERROR;
}

//for test
//if user application call Fun(APP_FUN_CODE_DEBUG,...),will show "this is a test"
static int debug_net_fun(APP_FUN_CODE code,va_list argp)
{
	// int line = 0;
	// char *fun = 0;
	// char *msg = 0;
	// //first we get format
	// line = va_arg(argp, int);
	// fun = va_arg(argp, char*);
	// msg = va_arg(argp, char*);

	// {
	// 	extern void log_net_print(char* fmt,...);
	// 	log_net_print("[%s:%d]:%s\n",fun,line,msg);
	// }

	return 0;
}

static int create_thread(APP_FUN_CODE code,va_list argp)
{
	char *name = 0;
	void *parameter = 0;
	rt_uint32_t stack_size;
	rt_uint8_t  priority;
	rt_uint32_t tick;
	rt_thread_t thread_id;
	typedef void (*entry)(void *parameter);
	
	name = va_arg(argp, char*);
	entry en = va_arg(argp, entry);
	parameter = va_arg(argp, void*);
	stack_size = va_arg(argp, rt_uint32_t);
	priority = 	va_arg(argp, rt_uint8_t);
	tick = va_arg(argp, rt_uint32_t);

	if(stack_size == 0)
		stack_size = 1024;
	if(priority == 0)
		priority = 20;
	if(tick == 0)
		tick = 10;
	
	thread_id = rt_thread_create(name,en,parameter,stack_size,priority,tick);
	
	if(thread_id != RT_NULL) {
		rt_thread_startup(thread_id);
	}
	return 0;
}

static int delay(APP_FUN_CODE code,va_list argp)
{
	rt_uint32_t tick;
	tick = va_arg(argp, rt_uint32_t);
	rt_thread_delay(tick);
	
	return 0;
}

static int create_sem(APP_FUN_CODE code,va_list argp)
{
	long* p_sem = RT_NULL;
	char *name = RT_NULL;
	unsigned char value;
	
	p_sem = va_arg(argp, long*);
	name = va_arg(argp, char*);
	value = va_arg(argp, unsigned char);
	
	*p_sem = (long)rt_sem_create("sem",value,RT_IPC_FLAG_FIFO);
	return 0;
}

static int take_sem(APP_FUN_CODE code,va_list argp)
{
	long* p_sem = RT_NULL;
	rt_int32_t tick;
	rt_err_t *res;
	p_sem = va_arg(argp, long*);
	tick = va_arg(argp, rt_uint32_t);
	res = va_arg(argp, rt_err_t*);
	
	*res = rt_sem_take((rt_sem_t)(*p_sem),tick);
	
	return 0;
}

static int release_sem(APP_FUN_CODE code,va_list argp)
{
	long* p_sem = RT_NULL;
	p_sem = va_arg(argp, long*);
	
	rt_sem_release((rt_sem_t)(*p_sem));
	
	return 0;
}

static int create_timer(APP_FUN_CODE code,va_list argp)
{
	char *name = 0;
	void *parameter = 0;
	rt_tick_t time;
	rt_uint8_t flag;
	long *p_time = RT_NULL;
	typedef void (*timeout)(void *parameter);
	
	p_time = va_arg(argp, long *);
	name = va_arg(argp, char*);
	timeout to = va_arg(argp, timeout);
	parameter = va_arg(argp, void*);
	time = va_arg(argp, rt_uint32_t);
	flag = 	va_arg(argp, rt_uint8_t);
	
	if(time == 0)
		time = 10;

	*p_time = (long)rt_timer_create(name,to,parameter,time,flag);
	
	return 0;
}

static int start_timer(APP_FUN_CODE code,va_list argp)
{
	long *p_time = RT_NULL;
	p_time = va_arg(argp, long *);
	
	rt_timer_start((rt_timer_t)(*p_time));
	
	return 0;
}

static int stop_timer(APP_FUN_CODE code,va_list argp)
{
	long *p_time = RT_NULL;
	p_time = va_arg(argp, long *);
	
	rt_timer_stop((rt_timer_t)(*p_time));
	
	return 0;
}

static int CPLD_Write_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* addr = RT_NULL;
	unsigned int date = 0;

	addr = va_arg(argp, unsigned char*);
	date = va_arg(argp, unsigned int );

	extern void CPLD_Write(unsigned char addr,unsigned int date);
	CPLD_Write(*addr,date);

	return 0;
}

static int CPLD_Read_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* addr = RT_NULL;
	addr = va_arg(argp, unsigned char*);

	extern unsigned int CPLD_Read(unsigned char addr);

	return CPLD_Read(*addr);
}

static int DO_Write_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* ch = RT_NULL;
	unsigned char* date = RT_NULL;

	ch = va_arg(argp, unsigned char*);
	date = va_arg(argp, unsigned char*);

	extern void DO_Write(unsigned char ch,unsigned char date);
	DO_Write(*ch,*date);
	
	return 0;
}

static int DO_10_Write_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned int date = 0;
	date = va_arg(argp, unsigned int);

	extern void DO_8_Write(unsigned int date);

	DO_8_Write(date);

	return 0;
}

static int DI_Read_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* ch = RT_NULL;

	ch = va_arg(argp, unsigned char*);

	extern unsigned char DI_Read(unsigned char ch);

	return DI_Read(*ch);
}

static int DO_16_Read_fun(APP_FUN_CODE code,va_list argp)
{
	extern unsigned int DI_16_Read(void);

	return DI_16_Read();
}

static int SW_Read_fun(APP_FUN_CODE code,va_list argp)
{
	extern unsigned int SW_Read(void);

	return SW_Read();
}

static int PTO_Freq_Set_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* ch = RT_NULL;
	unsigned int Freq = 0;

	ch = va_arg(argp, unsigned char*);
	Freq = va_arg(argp, unsigned int);

	extern void PTO_Freq_Set(unsigned char ch,unsigned int Freq);

	PTO_Freq_Set(*ch,Freq);

	return 0;
}

static int PTO_PlusSum_Set_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* ch = RT_NULL;
	unsigned int PlusSum = 0;

	ch = va_arg(argp, unsigned char*);
	PlusSum = va_arg(argp, unsigned int);

	extern void PTO_PlusSum_Set(unsigned char ch,unsigned int PlusSum);

	PTO_PlusSum_Set(*ch,PlusSum);

	return 0;
}

static int PTO_Ctr_Set_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* ch = RT_NULL;
	unsigned char* en = RT_NULL;
	unsigned char* dir = RT_NULL;

	ch = va_arg(argp, unsigned char*);
	en = va_arg(argp, unsigned char*);
	dir = va_arg(argp, unsigned char*);

	extern void PTO_Ctr_Set(unsigned char ch,unsigned char en,unsigned char dir);
	PTO_Ctr_Set(*ch,*en,*dir);

	return 0;
}

static int PTO_PlusCurNum_Read_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* ch = RT_NULL;

	ch = va_arg(argp, unsigned char*);

	extern unsigned int PTO_PlusCurNum_Read(unsigned char ch);

	return PTO_PlusCurNum_Read(*ch);

}

static int PTO_State_Read_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* ch = RT_NULL;

	ch = va_arg(argp, unsigned char*);

	extern unsigned int PTO_State_Read(unsigned char ch);

	return PTO_State_Read(*ch);
}

static int HC_PlusCurNum_Read_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* ch = RT_NULL;

	ch = va_arg(argp, unsigned char*);

	extern unsigned short HC_PlusCurNum_Read(unsigned char ch);

	return HC_PlusCurNum_Read(*ch);
}

static int HC_TimerEventValue_Read_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* ch = RT_NULL;

	ch = va_arg(argp, unsigned char*);

	extern unsigned short HC_TimerEventValue_Read(unsigned char ch);

	return HC_TimerEventValue_Read(*ch);
}

static int Init_CPLD_fun(APP_FUN_CODE code,va_list argp)
{
	extern void Init_CPLD(void);

	Init_CPLD();

	return 0;
}

static int Init_HC_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* no = RT_NULL;

	no = va_arg(argp, unsigned char*);

	extern void Init_HC(unsigned char no);

	Init_HC(*no);

	return 0;
}

static int myplsr_fun(APP_FUN_CODE code,va_list argp)
{
	extern int myplsr(short En, short No, short StartFreq, short EndFreq ,short ADRate, long PulsNum, short * State, long * Pon);

	short* En = RT_NULL;
	short* No = RT_NULL;
	short* StartFreq = RT_NULL;
	short* EndFreq = RT_NULL;
	short* ADRate = RT_NULL;
	long* PulsNum = RT_NULL;
	short* State = RT_NULL;
	long* Pon = RT_NULL;

	En = va_arg(argp, short*);
	No = va_arg(argp, short*);
	StartFreq = va_arg(argp, short*);
	EndFreq = va_arg(argp, short*);
	ADRate = va_arg(argp, short*);
	PulsNum = va_arg(argp, long*);
	State = va_arg(argp, short*);
	Pon = va_arg(argp, long*);

	return myplsr(*En, *No, *StartFreq, *EndFreq ,*ADRate, *PulsNum,  State, Pon);
}

static int myplsy_fun(APP_FUN_CODE code,va_list argp)
{
	extern int myplsy(short En, short No, short Freq, long PulsNum, short * State, long * Pon);

	short* En = RT_NULL;
	short* No = RT_NULL;
	short* Freq = RT_NULL;
	long* PulsNum = RT_NULL;
	short* State = RT_NULL;
	long* Pon = RT_NULL;

	En = va_arg(argp, short*);
	No = va_arg(argp, short*);
	Freq = va_arg(argp, short*);
	PulsNum = va_arg(argp, long*);
	State = va_arg(argp, short*);
	Pon = va_arg(argp, long*);

	return myplsy(*En, *No, *Freq, *PulsNum, State, Pon);
}

static int myorg_fun(APP_FUN_CODE code,va_list argp)
{
	extern int myorg(short En, short No, short StartFreq, short EndFreq ,short ADRate,long CurPlace,short * State, long * Pon);

	short* En = RT_NULL;
	short* No = RT_NULL;
	short* StartFreq = RT_NULL;
	short* EndFreq = RT_NULL;
	short* ADRate = RT_NULL;
	long* CurPlace = RT_NULL;
	short* State = RT_NULL;
	long* Pon = RT_NULL;

	En = va_arg(argp, short*);
	No = va_arg(argp, short*);
	StartFreq = va_arg(argp, short*);
	EndFreq = va_arg(argp, short*);
	ADRate = va_arg(argp, short*);
	CurPlace = va_arg(argp, long*);

	State = va_arg(argp, short*);
	Pon = va_arg(argp, long*);

	return myorg(*En, *No, *StartFreq, *EndFreq ,*ADRate, *CurPlace,  State, Pon);
}

static int mypwm_fun(APP_FUN_CODE code,va_list argp)
{
	extern int mypwm(short En, short No, short Freq, short Duty);

	short* En = RT_NULL;
	short* No = RT_NULL;
	short* Freq = RT_NULL;
	short* Duty = RT_NULL;

	En = va_arg(argp, short*);
	No = va_arg(argp, short*);
	Freq = va_arg(argp, short*);
	Duty = va_arg(argp, short*);

	return mypwm(*En, *No, *Freq, *Duty);
}

static int HC_Update_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* no = RT_NULL;

	no = va_arg(argp, unsigned char*);

	extern void HC_Update(unsigned char no);

	HC_Update(*no);

	return 0;
}

static int HC_Rst_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* no = RT_NULL;

	no = va_arg(argp, unsigned char*);

	extern void HC_Rst(unsigned char no);

	HC_Rst(*no);

	return 0;
}

static int HC_Start_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* no = RT_NULL;

	no = va_arg(argp, unsigned char*);

	extern void HC_Start(unsigned char no);

	HC_Start(*no);

	return 0;
}

static int HC_Stop_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* no = RT_NULL;

	no = va_arg(argp, unsigned char*);

	extern void HC_Stop(unsigned char no);

	HC_Stop(*no);

	return 0;
}

static int HC_Clear_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* no = RT_NULL;

	no = va_arg(argp, unsigned char*);

	extern void HC_Clear(unsigned char no);

	HC_Clear(*no);

	return 0;
}

static int HC_Refresh_fun(APP_FUN_CODE code,va_list argp)
{
	unsigned char* no = RT_NULL;

	no = va_arg(argp, unsigned char*);

	extern void HC_Refresh(unsigned char no);

	HC_Refresh(*no);

	return 0;
}
*/
/********************NA2000 interface func**************************/

static int get_varadr(APP_FUN_CODE code,int* errcode,va_list argp)
{
	int* pTmpDynamicData = RT_NULL;
	int* pTmpStaticData = RT_NULL;
	int* pTmpNvramData = RT_NULL;
	int* pTmpPrgData = RT_NULL;
	int* pTmpFBData = RT_NULL;
	int* pTmpfunstate = RT_NULL;
	int* pTmpPrgEntry = RT_NULL;
	int* pTmpFbEntry = RT_NULL;
	int* pTmpFBInfo = RT_NULL;

	pTmpDynamicData = va_arg(argp, int*);
	pTmpStaticData = va_arg(argp, int*);
	pTmpNvramData = va_arg(argp, int*);
	pTmpPrgData = va_arg(argp, int*);
	pTmpFBData = va_arg(argp, int*);
	pTmpfunstate = va_arg(argp, int*);
	pTmpPrgEntry = va_arg(argp, int*);
	pTmpFbEntry = va_arg(argp, int*);
	pTmpFBInfo = va_arg(argp, int*);
	
	
	*pTmpDynamicData =(int)pDynamicData;
	*pTmpStaticData = (int)pStaticData;
	*pTmpNvramData = (int)pNvramData;
	*pTmpPrgData = (int)pPrgData;
	*pTmpFBData = (int)pFBData;
	*pTmpfunstate = (int)funstate;
	*pTmpPrgEntry = (int)PrgEntry;
	*pTmpFbEntry = (int)FbEntry;
	*pTmpFBInfo = (int)pFBInfo;
	
	*errcode = 0;
	
	return 0;
}

static int mymalloc(APP_FUN_CODE code,int* errcode,va_list argp)
{
	int len = va_arg(argp, int);
	void *ret = RT_NULL;
	
	ret = rt_malloc(len);
	
	if(ret == RT_NULL)
		*errcode = -1;
	else
		*errcode = 0;
	
	return (int)ret;
}

static int myfree(APP_FUN_CODE code,int* errcode,va_list argp)
{
	void *ret = va_arg(argp,void *);
	
	rt_free(ret);
	
	*errcode = 0;
	
	return 0;
}

static int mymemset(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern void *	memset (void *__s, int __c, size_t __n);
	void *__s;
	int __c;
	int __n;

	__s =  va_arg(argp,void *);
	__c =  va_arg(argp,int);
	__n = va_arg(argp, int);
	
	rt_memset(__s,__c,__n);
	
	*errcode = 0;
	return 0;
}

static int mymemcpy(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern void *	memcpy (void *__s1, const void *__s2, size_t __n);
	void *__s1;
	void *__s2;
	int  __n;

	__s1 =  va_arg(argp,void *);
	__s2 =  va_arg(argp,void *);
	__n = va_arg(argp, int);

	rt_memcpy(__s1,__s2,__n);

	*errcode = 0;
	return 0;
}


static int Output_Q_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern int Output_Q( unsigned short occno, unsigned short value, unsigned int pulseTime );
	int result = 0;
	unsigned short occno;
	unsigned short value;
	unsigned int pulseTime ;
	
	occno = (unsigned short)va_arg(argp, int);
	value = (unsigned short)va_arg(argp, int);
	pulseTime = va_arg(argp, unsigned int);
	
	result = Output_Q(occno,value,pulseTime);
	
	*errcode = 0;
	return result;
}

static int MyDebug_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern int MyDebug(int LdNo, int StepNo);
	int result = 0;
	int LdNo;
	int StepNo;
	
	LdNo = va_arg(argp, int);
	StepNo = va_arg(argp, int);
	
	result = MyDebug(LdNo,StepNo);

	*errcode = 0;	
	return result;
}

static int Output_AQ_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern int Output_AQ( unsigned short occno, unsigned short value );
	int result = 0;
	unsigned short occno;
	unsigned short value;
	
	occno = (unsigned short)va_arg(argp, int);
	value = (unsigned short)va_arg(argp, int);
	
	result = Output_AQ(occno,value);

	*errcode = 0;
	return result;
}

static int Force_I_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern int Force_I( unsigned short occno, unsigned short value );
	int result = 0;
	unsigned short occno;
	unsigned short value;
	
	occno = (unsigned short)va_arg(argp, int);
	value = (unsigned short)va_arg(argp, int);
	
	result = Force_I(occno,value);

	*errcode = 0;
	return result;
	
}

static int Force_Q_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern int Force_Q( unsigned short occno, unsigned short value );
	int result = 0;
	unsigned short occno;
	unsigned short value;
	
	occno = (unsigned short)va_arg(argp, int);
	value = (unsigned short)va_arg(argp, int);
	
	result = Force_Q(occno,value);

	*errcode = 0;
	return result;
	
}

static int Force_AI_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern int Force_AI( unsigned short occno, unsigned short value );
	int result = 0;
	unsigned short occno;
	unsigned short value;
	
	occno = (unsigned short)va_arg(argp, int);
	value = (unsigned short)va_arg(argp, int);
	
	result = Force_AI(occno,value);

	*errcode = 0;
	return result;
}

static int Force_AQ_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern int Force_AQ( unsigned short occno, unsigned short value );

	int result = 0;
	unsigned short occno;
	unsigned short value;
	
	occno = (unsigned short)va_arg(argp, int);
	value = (unsigned short)va_arg(argp, int);
	
	result = Force_AQ(occno,value);

	*errcode = 0;
	return result;
}

static int UnForce_Data_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern int UnForce_Data( unsigned short para_type, unsigned short occno );
	int result = 0;
	unsigned short para_type;
	unsigned short occno;
	
	para_type = (unsigned short)va_arg(argp, int);
	occno = (unsigned short)va_arg(argp, int);
	
	result = UnForce_Data(para_type,occno);

	*errcode = 0;
	return result;
}

static int Move_Fuc_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	/*extern int  Move_Fuc( void* pdst, unsigned int dst_bit_start, 
			   void* psrc, unsigned int src_bit_start, 
			   unsigned int bit_len	);
	*/
	int  result = 0;
	void* pdst;
	unsigned int dst_bit_start;
	void* psrc;
	unsigned int src_bit_start;
	unsigned int bit_len	;
	
	pdst = va_arg(argp, void*);
	dst_bit_start = va_arg(argp, unsigned int);
	psrc = va_arg(argp, void*);
	src_bit_start = va_arg(argp, unsigned int);
	bit_len = va_arg(argp, unsigned int);
	
	result = Move_Fuc(pdst,dst_bit_start,psrc,src_bit_start,bit_len);

	*errcode = 0;
	return result;
}


static int Malloc_PN_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern unsigned short Malloc_PN(void);
	unsigned short result = 0;
	result = Malloc_PN();
	
	*errcode = 0;
	return (int)result;
}

static int Process_PN_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern unsigned int Process_PN(unsigned short occno, unsigned char state);
	unsigned int result = 0;
	unsigned short occno;
	unsigned short state;
	
	occno = (unsigned short)va_arg(argp, int);
	state = (unsigned short)va_arg(argp, int);
	
	result = Process_PN(occno,state);

	*errcode = 0;
	return result;
}

static int Free_PN_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern void Free_PN(unsigned short occno);
	
	unsigned short occno;
	occno = (unsigned short)va_arg(argp, int);
	
	Free_PN(occno);
	
	*errcode = 0;
	return 0;
	
}

static int LD_Call_T_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	/*extern void LD_Call_T( unsigned short occno, unsigned char ena, unsigned int preset,
	unsigned char * Q1, unsigned int *count, unsigned short mstype, unsigned short type );*/
	
	unsigned short occno;
	unsigned char ena;
	unsigned int preset;
	
	unsigned char* Q1;
	unsigned int*count;
	unsigned short mstype;
	unsigned short type ;
	
	occno = (unsigned short)va_arg(argp, int);
	ena = (unsigned char)va_arg(argp, int);
	preset = va_arg(argp, unsigned int);
	Q1 = va_arg(argp, unsigned char*);
	
	count = va_arg(argp, unsigned int*);
	mstype =(unsigned short)va_arg(argp, int);
	type = (unsigned short)va_arg(argp, int);
	
	LD_Call_T(occno,ena,preset,Q1,count,mstype,type);
		
	*errcode = 0;
	return 0;
	
}

static int LD_Call_C_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	/*extern void LD_Call_C(unsigned short direction, unsigned short occno,
	unsigned char cu_pin, unsigned char cd_pin, unsigned char r_pin, unsigned char ld_pin,
		unsigned int preset, unsigned char *qu, unsigned char *qd, unsigned int *value );*/
	
	unsigned short direction;
	unsigned short occno;
	unsigned char cu_pin;
	unsigned char cd_pin;
	unsigned char r_pin;
	
	unsigned char ld_pin;
	unsigned int preset;
	unsigned char *qu;
	unsigned char *qd;
	unsigned int *value ;
	
	direction = (unsigned short)va_arg(argp, int);
	occno = (unsigned short)va_arg(argp, int);
	cu_pin = (unsigned char)va_arg(argp, int);
	cd_pin =  (unsigned char)va_arg(argp, int);
	r_pin =  (unsigned char)va_arg(argp, int);
	
	ld_pin =  (unsigned char)va_arg(argp, int);
	preset = va_arg(argp, unsigned int);
	qu = va_arg(argp, unsigned char*);
	qd = va_arg(argp, unsigned char*);
	value = va_arg(argp, unsigned int*);
	
	LD_Call_C(direction, occno,cu_pin, cd_pin, r_pin,ld_pin,preset, qu, qd,value );
	
	*errcode = 0;
	return 0;
}

static int LD_Call_FBT_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	/*int LD_Call_FBT( unsigned short occno, unsigned char ena, unsigned int preset, 
	unsigned char * Q1, unsigned int *count, unsigned short mstype ,unsigned short type)  
	*/
	int result = 0;
	unsigned short occno;
	unsigned char ena;
	unsigned int preset;
	unsigned char * Q1;
	unsigned int *count;
	unsigned short mstype;
	unsigned short type;
	
	occno =  (unsigned short)va_arg(argp, int);
	ena =  (unsigned char)va_arg(argp, int);
	preset = va_arg(argp, unsigned int);
	Q1 = va_arg(argp, unsigned char*);
	count = va_arg(argp, unsigned int*);
	mstype = (unsigned short)va_arg(argp, int);
	type =(unsigned short)va_arg(argp, int);
	
	result = LD_Call_FBT( occno, ena, preset, Q1, count, mstype ,type);
	
	*errcode = 0;
	return result;
}

static int LD_Call_FBC_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	/*int LD_Call_FBC(unsigned short direction, unsigned short occno, unsigned char cu_pin,
	unsigned char cd_pin, unsigned char r_pin, unsigned char ld_pin, unsigned int preset, 
	unsigned char *qu, unsigned char *qd, unsigned int *value )
	*/
	int result = 0;
	unsigned short direction;
	unsigned short occno;
	unsigned char cu_pin;
	unsigned char cd_pin;
	unsigned char r_pin;
	unsigned char ld_pin;
	unsigned int preset; 
	unsigned char *qu;
	unsigned char *qd;
	unsigned int *value;
	
	direction =(unsigned short)va_arg(argp, int);
	occno = (unsigned char)va_arg(argp, int);
	cu_pin = (unsigned char)va_arg(argp, int);
	cd_pin = (unsigned char)va_arg(argp, int);
	r_pin = (unsigned char)va_arg(argp, int);
	ld_pin = (unsigned char)va_arg(argp, int);
	preset = va_arg(argp, unsigned int);
	qu = va_arg(argp, unsigned char*);
	qd = va_arg(argp, unsigned char*);
	value = va_arg(argp, unsigned int*);
	
	result = LD_Call_FBC(direction, occno,cu_pin,cd_pin, r_pin, ld_pin,preset, qu, qd,value );
	
	*errcode = 0;
	return result;
}

static int CRC16_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern unsigned short CRC16(unsigned char* buf, unsigned short len);
	unsigned short result = 0;
	
	unsigned char* buf;
	unsigned short len;
	
	buf = va_arg(argp, unsigned char*);
	len = (unsigned short)va_arg(argp, int);
		
	result = CRC16(buf,len);
	
	*errcode = 0;
	return result;
}

static int rLadToSeq_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern int rLadToSeq(unsigned short seqno, unsigned char mode);
	
	int result = 0;
	unsigned short seqno;
	unsigned char mode;
	
	seqno = (unsigned short)va_arg(argp, int);
	mode =(unsigned char)va_arg(argp, int);
		
	result = rLadToSeq(seqno,mode);
	
	*errcode = 0;
	return result;
}

static int rPID1_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	/*
	unsigned char rPID1(unsigned char En, unsigned int PointNo, unsigned char BIn, 
	unsigned char Man, float * FIn, float *FOut, unsigned int *IOut)
	*/
	unsigned char result = 0;

	unsigned char En;
	unsigned int PointNo;
	unsigned char BIn;	
	unsigned char Man;
	float * FIn;
	float *FOut;
	unsigned int *IOut;

	En = (unsigned char)va_arg(argp, int);
	PointNo = va_arg(argp, unsigned int);
	BIn = (unsigned char)va_arg(argp, int);
	Man =(unsigned char)va_arg(argp, int);
	FIn = va_arg(argp, float*);
	FOut = va_arg(argp, float*);
	IOut = va_arg(argp, unsigned int*);
	
	result = rPID1(En, PointNo,BIn, Man, FIn, FOut, IOut);
	
	*errcode = 0;
	return (int)result;
}

static int mymodrw_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	/*extern int mymodrw(unsigned int port, unsigned int adr, unsigned int code, unsigned int reg,
		unsigned int num, unsigned int pointtype, unsigned int array, unsigned int occno);*/
	
	int  result = 0;
	unsigned int port;
	unsigned int adr;
	unsigned int code2;
	unsigned int reg;
	unsigned int num;
	unsigned int pointtype;
	unsigned int array;
	unsigned int occno;
	
	port = va_arg(argp, unsigned int);
	adr = va_arg(argp, unsigned int);
	code2 = va_arg(argp, unsigned int);
	reg = va_arg(argp, unsigned int);
	num = va_arg(argp, unsigned int);
	pointtype = va_arg(argp, unsigned int);
	array = va_arg(argp, unsigned int);
	occno = va_arg(argp, unsigned int);
		
	result =  mymodrw(port,adr, code2,reg,num,pointtype, array, occno);

	*errcode = 0;
	return result;
}

static int MsgSend_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern int MsgSend(unsigned int TaskNo, unsigned char * buffer, unsigned int nBytes);
	int  result = 0;
	unsigned int TaskNo;
	unsigned char * buffer;
	unsigned int nBytes;
	
	TaskNo = va_arg(argp, unsigned int);
	buffer = va_arg(argp, unsigned char*);
	nBytes = va_arg(argp, unsigned int);
	
	result = MsgSend(TaskNo,buffer,nBytes);
	
	*errcode = 0;
	return result;
}

static int mallocebft_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern int Malloc_FBTimer();
	int  result = 0;
	result = Malloc_FBTimer();
	*errcode = 0;
	return result;
}

static int mallocebfc_fun(APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern int Malloc_FBCount();
	int  result = 0;
	result = Malloc_FBCount();
	*errcode = 0;
	return result;
}


/******************double interface functions************************/

static double fabs_fun(DOUBLE_APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern double 	fabs (double x);
	double result = 0.0;
	double value;

	value = va_arg(argp, double);
	result = fabs(value);

	*errcode = 0;
	return result;
}

static double sqrt_fun(DOUBLE_APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern double 	sqrt (double x);
	double result = 0.0;
	double value;

	value = va_arg(argp, double);
	result = sqrt(value);

	*errcode = 0;
	return result;
}

static double log10_fun(DOUBLE_APP_FUN_CODE code,int* errcode,va_list argp)
{
	//#define log10(p1)										entry(FC_LOG,&errcode,p1)
	double result = 0.0;
	double value = 0.0;

	value = va_arg(argp, double);
	result = log10(value);

	*errcode = 0;
	return result;
}

static double log_fun(DOUBLE_APP_FUN_CODE code,int* errcode,va_list argp)
{
	//#define log(p1)											entry(FC_LN,&errcode,p1)
	double result = 0.0;
	double value;

	value = va_arg(argp, double);
	result = log(value);

	*errcode = 0;
	return result;
}

static double exp_fun(DOUBLE_APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern double 	sqrt (double x);
	double result = 0.0;
	double value;

	value = va_arg(argp, double);
	result = exp(value);

	*errcode = 0;
	return result;
}

static double pow_fun(DOUBLE_APP_FUN_CODE code,int* errcode,va_list argp)
{
	//extern double 	pow (double x, double y);
	double result = 0.0;
	double value1;
	double value2;

	value1 = va_arg(argp, double);
	value2 = va_arg(argp, double);
	result = pow(value1,value2);

	*errcode = 0;
	return result;
}

static double sin_fun(DOUBLE_APP_FUN_CODE code,int* errcode,va_list argp)
{
	double result = 0.0;
	double value;

	value = va_arg(argp, double);
	result = sin(value);

	*errcode = 0;
	return result;
}

static double cos_fun(DOUBLE_APP_FUN_CODE code,int* errcode,va_list argp)
{
	double result = 0.0;
	double value;

	value = va_arg(argp, double);
	result = cos(value);

	*errcode = 0;
	return result;
}

static double tan_fun(DOUBLE_APP_FUN_CODE code,int* errcode,va_list argp)
{
	double result = 0.0;
	double value;

	value = va_arg(argp, double);
	result = tan(value);

	*errcode = 0;
	return result;
}

static double asin_fun(DOUBLE_APP_FUN_CODE code,int* errcode,va_list argp)
{
	double result = 0.0;
	double value;

	value = va_arg(argp, double);
	result = asin(value);

	*errcode = 0;
	return result;
}

static double acos_fun(DOUBLE_APP_FUN_CODE code,int* errcode,va_list argp)
{
	double result = 0.0;
	double value;

	value = va_arg(argp, double);
	result = acos(value);

	*errcode = 0;
	return result;
}


static double atan_fun(DOUBLE_APP_FUN_CODE code,int* errcode,va_list argp)
{
	double result = 0.0;
	double value;

	value = va_arg(argp, double);
	result = atan(value);

	*errcode = 0;
	return result;
}
/******************double interface functions************************/



//before call interface function,call this init first
void init_interface()
{
	register_func(FC_VARADR,get_varadr);
	register_func(FC_DEBUG,MyDebug_fun);
	register_func(FC_OUTPUTQ,Output_Q_fun);
	
	register_func(FC_OUTPUTAQ,Output_AQ_fun);
	register_func(FC_FORCEI,Force_I_fun);
	
	register_func(FC_FORCEQ,Force_Q_fun);
	register_func(FC_FORCEAI,Force_AI_fun);
	register_func(FC_FORCEAQ,Force_AQ_fun);
	register_func(FC_UNFORCE,UnForce_Data_fun);
	register_func(FC_BLKMOV,Move_Fuc_fun);//	FC_BLKMOV	=	10,
	
	register_func(FC_MALLOCPN,Malloc_PN_fun);
	register_func(FC_PROCPN,Process_PN_fun);
	register_func(FC_FREEPN,Free_PN_fun);
	register_func(FC_PRGT,LD_Call_T_fun);
	register_func(FC_PRGC,LD_Call_C_fun);

	register_func(FC_FBT,LD_Call_FBT_fun);
	register_func(FC_FBC,LD_Call_FBC_fun);
	
	register_func(FC_CRC,CRC16_fun);
	register_func(FC_EXESCC,rLadToSeq_fun);
	register_func(FC_PID,rPID1_fun);//	FC_PID =	20,
	
	register_func(FC_MODRW,mymodrw_fun);
	register_func(FC_FUNC,MsgSend_fun);
	register_func(FC_MALLOC,mymalloc);//	FC_MALLOC	=	23,
	register_func(FC_FREE,myfree);//	FC_FREE	=	24,

	register_func(FC_MEMCPY,mymemcpy);
	register_func(FC_MEMSET,mymemset);

	register_func(FC_MALLOCFBT,mallocebft_fun);
	register_func(FC_MALLOCFBC,mallocebfc_fun);


/******************double interface functions************************/
	register_double_func(FC_FABS,fabs_fun);
	register_double_func(FC_SQRT,sqrt_fun);
	register_double_func(FC_LOG,log10_fun);//#define log10(p1)
	register_double_func(FC_LN,log_fun);//#define log(p1)
	register_double_func(FC_EXP,exp_fun);
	register_double_func(FC_EXPT,pow_fun);

	register_double_func(FC_SIN,sin_fun);
	register_double_func(FC_COS,cos_fun);
	register_double_func(FC_TAN,tan_fun);

	register_double_func(FC_ASIN,asin_fun);
	register_double_func(FC_ACOS,acos_fun);
	register_double_func(FC_ATAN,atan_fun);//#define FC_ATAN				38

}

