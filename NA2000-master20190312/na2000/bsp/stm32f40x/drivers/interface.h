#ifndef __H_INTERFACE_
#define __H_INTERFACE_

/***********************************************************************
*define APP_FUN_CODE enum for code between rt-thread and appliction
*AFC meaning (APP FUN CODE's) fisrt character
*AFC_DEBUG from 0
*AFC_UNKNOWN is 255,this is mean the max number of command is 255
*you can add you code between AFC_DEBUG and AFC_UNKNOWN
***********************************************************************/
//typedef enum APP_FUN_CODE
//{
//	FC_VARADR = 1,
//	FC_DEBUG,
//	FC_OUTPUTQ,
//	FC_MALLOC = 23,
//	FC_FREE = 24,
//	
//	AFC_UNKNOWN = 50,
//}APP_FUN_CODE;

typedef enum APP_FUN_CODE
{
	FC_VARADR	=	1,
	FC_DEBUG	=	2,
	FC_OUTPUTQ =	3,
	FC_OUTPUTAQ	=	4,
	FC_FORCEI	=	5,
	FC_FORCEQ	=	6,
	FC_FORCEAI	=	7,
	FC_FORCEAQ	=	8,
	FC_UNFORCE	=	9,
	FC_BLKMOV	=	10,
	FC_MALLOCPN	=	11,
	FC_PROCPN	=	12,
	FC_FREEPN	=	13,
	FC_PRGT	=	14,
	FC_PRGC	=	15,
	FC_FBT =	16,
	FC_FBC	=	17,
	FC_CRC	=	18,
	FC_EXESCC	=	19,
	FC_PID =	20,
	FC_MODRW	=	21,
	FC_FUNC	=	22,
	FC_MALLOC	=	23,
	FC_FREE	=	24,

	FC_MEMCPY	=	25,
	FC_MEMSET	=	26,

	FC_MALLOCFBT	=	27,
	FC_MALLOCFBC	=	28,


	AFC_UNKNOWN = 50,
}APP_FUN_CODE;

typedef enum DOUBLE_APP_FUN_CODE
{
	FC_FABS		=	1,
	FC_SQRT  	=	2,
	FC_LOG		=	3,
	FC_LN			=	4,
	FC_EXP		=	5,
	FC_EXPT		=	6,
	FC_SIN		=	7,
	FC_COS		=	8,
	FC_TAN		=	9,
	FC_ASIN		=	10,
	FC_ACOS		=	11,
	FC_ATAN		=	12,

	AFC_UNKNOWN_DOUBLE = 20,
}DOUBLE_APP_FUN_CODE;


/***********************************************************************
*define INTERFACR_ERROR_CODE for records interface internel error
*
************************************************************************/
typedef enum INTERFACR_ERROR_CODE
{
	INTERFACR_NO_ERROR = 0,
	INTERFACR_ERROR_OUTOF_RANGE = 1,	//code not in 0~255
	INTERFACR_ERROR_REG_FUNC,					//register function error
	
	
	INTERFACR_ERROR_UNKNOWN = 255,
}INTERFACR_ERROR_CODE;

//define a raw type of entry
typedef int (*entry_t)(APP_FUN_CODE code,...);
//real entry for 
extern int entry_f(APP_FUN_CODE code,...);

//called by rt-thread
extern void init_interface(void);

//this is raw type for format string like c sprintf
typedef int (*sprintf_f)(char* str,char *fmt,...);


//who want to use entry function,he/she must define entry
extern entry_t entry;


/*****************double entry***************************/
typedef double (*entry_double_t)(DOUBLE_APP_FUN_CODE code,...);

extern double entry_double_f(DOUBLE_APP_FUN_CODE code,...);

extern entry_double_t entry_double;
/*****************double entry***************************/

/***********************************************************************
*define multiple macros for application
*
************************************************************************/
//get rt_sprintf address



//CPLD
#define APP_CPLD_WRITE(addr,date) 	entry(AFC_CPLD_WRITE,addr,date)
#define APP_CPLD_READ(addr) 				entry(AFC_CPLD_READ,addr)
#define APP_DO_WRITE(ch,date) 			entry(AFC_DO_WRITE,ch,date)
#define APP_DO_10_WRITE(date) 			entry(AFC_DO_10_WRITE,date)

#define APP_DI_READ(ch) 						entry(AFC_DI_READ,ch)
#define APP_DI_14_READ() 						entry(AFC_DI_14_READ)
#define APP_SW_READ() 							entry(AFC_SW_READ)

#define APP_PTO_Freq_Set(ch,Freq) 				entry(AFC_PTO_Freq_Set,ch,Freq)
#define APP_PTO_PlusSum_Set(ch,PlusSum) 	entry(AFC_PTO_PlusSum_Set,ch,PlusSum)
#define APP_PTO_Ctr_Set(ch,en,dir) 				entry(AFC_PTO_Ctr_Set,ch,en,dir)

#define APP_PTO_PlusCurNum_Read(ch) 			entry(AFC_PTO_PlusCurNum_Read,ch)
#define APP_PTO_State_Read(ch) 						entry(AFC_PTO_State_Read,ch)
#define APP_HC_PlusCurNum_Read(ch) 				entry(AFC_HC_PlusCurNum_Read,ch)
#define APP_HC_TimerEventValue_Read(ch) 	entry(AFC_HC_TimerEventValue_Read,ch)

#define APP_Init_CPLD() 	entry(AFC_Init_CPLD)
#define APP_Init_HC(no) 	entry(AFC_Init_HC,no)

#define APP_myplsr(En,No,StartFreq,EndFreq,ADRate,PulsNum,State,Pon)\
					entry(AFC_myplsr,En,No,StartFreq,EndFreq,ADRate,PulsNum,State,Pon)

#define APP_myplsy(En,No,Freq,PulsNum,State,Pon)\
					entry(AFC_myplsy,En,No,Freq,PulsNum,State,Pon)

#define APP_myorg(En,No,StartFreq,EndFreq,ADRate,CurPlace,State,Pon)\
					entry(AFC_myorg,En,No,StartFreq,EndFreq,ADRate,CurPlace,State,Pon)

#define APP_mypwm(En,No,Freq,Duty) 	entry(AFC_mypwm,En,No,Freq,Duty)

#define APP_HC_Update(no)			entry(AFC_HC_Update,no)
#define APP_HC_Rst(no)				entry(AFC_HC_Rst,no)
#define APP_HC_Start(no)			entry(AFC_HC_Start,no)
#define APP_HC_Stop(no)				entry(AFC_HC_Stop,addr,date)
#define APP_HC_Clear(no)			entry(AFC_HC_Clear,no)
#define APP_HC_Refresh(no)		entry(AFC_HC_Refresh,no)

//CPLD
#endif
