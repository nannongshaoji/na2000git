#ifndef SYSAPP_H
#define SYSAPP_H

#include "system.h"
#include "appcnst.h" 
#include "seq.h"
#include "seqext.h"
#include "database.h" 
#include "eth.h"
#include "threaddef.h"
#include "stm32f4xx.h"

#define RT_THREAD
#ifdef RT_THREAD
#define printf rt_kprintf
#define sprintf rt_sprintf
#define malloc rt_malloc
#define free rt_free
#define memcpy rt_memcpy
#define memset rt_memset
#endif

/* extern variables */
extern unsigned char BitCnt[];  
extern unsigned char	int_enable_flag;
extern unsigned char	mon_module;
extern unsigned char	debug_flag;

#define PQ1 (0)    //Q1
#define PQ2 (1)	//Q2
#define PQ3 (2)	//Q3
#define PQ4 (3)	//Q4
#define Q1Q2 (0) //两轴指令使用Q1/Q2
#define Q1Q3 (1)
#define Q1Q4 (2)
#define Q2Q3 (3)
#define Q2Q4 (4)
#define Q3Q4 (5)

#define OFF (0)
#define ON  (1)
#define ABS (0)
#define INC  (1)
extern uint8_t gbiax;

extern const unsigned char ORBIT_ARRAY[8];
extern const unsigned char CLRBIT_ARRAY[8];
#define ORBIT(bias)   ORBIT_ARRAY[bias]
#define CLRBIT(bias)  CLRBIT_ARRAY[bias]
extern const unsigned short WORBIT_ARRAY[16];
extern const unsigned short WCLRBIT_ARRAY[16];
#define WORBIT(bias)   WORBIT_ARRAY[bias]
#define WCLRBIT(bias)  WCLRBIT_ARRAY[bias]

extern CONFIG_DATA *pConfigData;
extern DYNAMIC_DATA *pDynamicData;
extern STATIC_DATA *pStaticData;
extern NVRAM_DATA *pNvramData;
extern PRG_DATA *pPrgData;
extern FB_DATA *pFBData;
extern CLOCK    *pCCUClock;
extern unsigned char *funstate;
extern FILE_STATE *FileState;
extern FILE_ENTRY *FileEntry;
extern FUNCPTR *PrgEntry;
extern FUNCPTR *FbEntry;
extern FB_INFO *pFBInfo;

extern LAD_STRUCT m_LD;
extern FB_STRUCT m_FB;

extern rt_sem_t	sem_prg;
extern rt_sem_t	sem_lineprg;
extern rt_mq_t MsgQid,SeqQid,ComQid[2],UdpQid,TcpcQid,TcpsQid;
//extern void * memcpy (void *__s1, const void *__s2, size_t __n);
extern void InitSeq(void);
extern void SeqTsk(void);

extern void Printff(char* str);
extern int DefaultSysInit(unsigned char cputype);
extern int SysInit(unsigned char cputype);

extern int WriteConvert( void* dst, unsigned char dstType, void* src, unsigned char srcType );
/*******************************************************************************/
/*                                                                             */
/*   Here are Operators                                                        */
/*                                                                             */
/*******************************************************************************/

/* ---------------------------------  I  ----------------------------------------
   Input :
       occno : 1 -- MAX_I
   State :
       value of I[occno-1].----------------------------------------------------*/
/* return FORCED/UNFORCED (1/0) */
#define Get_I_ForceMark( occno ) \
	( !!( pDynamicData->pI.ForceMark[(occno-1)/8] & ORBIT((occno-1)%8) ) )
#define Get_I( occno ) \
	( !!( pDynamicData->pI.State[(occno-1)/8] & ORBIT((occno-1)%8) ) )
extern int Force_I( unsigned short occno, unsigned short value );
extern int UnForce_I( unsigned short occno );
extern int Drv_Multi_I(unsigned short occno, unsigned short num);
extern int SetForceMark_I( unsigned short occno );

/* -------------------------------  Q  -----------------------------------------
   Input :
       occno : 1 -- MAX_Q
   ----------------------------------------------------------------------------*/
#define Get_Q( occno ) \
	( !!( pDynamicData->pQ.State[(occno-1)/8] & ORBIT((occno-1)%8) ) )
extern int Output_Q( unsigned short occno, unsigned short value, unsigned int pulseTime );
extern int Force_Q( unsigned short occno, unsigned short value );
extern int UnForce_Q( unsigned short occno );
extern int Drv_Multi_Q( unsigned short occno, unsigned short num, unsigned char *value );
extern int SetForceMark_Q( unsigned short occno );
/* return FORCED/UNFORCED (1/0) */
#define Get_Q_ForceMark( occno ) \
	( !!( pDynamicData->pQ.ForceMark[(occno-1)/8] & ORBIT((occno-1)%8) ) )
#define Get_Q_CheckEna( occno ) \
	( !!( pDynamicData->pQ.CheckEna[(occno-1)/8] & ORBIT((occno-1)%8) ) )

/* ---------------------------------  AI  ----------------------------------------
   Input :
       occno : 1 -- MAX_AI
   Output :
       value of AI[occno-1].----------------------------------------------------*/
#define Get_AI( occno ) \
	pDynamicData->pAI.Value[occno-1]
extern int Force_AI( unsigned short occno, unsigned short value );
extern int UnForce_AI( unsigned short occno );
extern int Drv_Multi_AI(unsigned short occno, unsigned short num);
extern int SetForceMark_AI( unsigned short occno );
/* return FORCED/UNFORCED (1/0) */
#define Get_AI_ForceMark( occno ) \
	( !!( pDynamicData->pAI.ForceMark[(occno-1)/8] & ORBIT((occno-1)%8) ) )

/* -------------------------------  AQ  -----------------------------------------
   Input :
       occno : 1 -- MAX_AQ
   ----------------------------------------------------------------------------*/
#define Get_AQ( occno ) \
	(pDynamicData->pAQ.Value[occno-1])
extern int Output_AQ( unsigned short occno, unsigned short value );
extern int Force_AQ( unsigned short occno, unsigned short value );
extern int UnForce_AQ( unsigned short occno );
extern int SetForceMark_AQ( unsigned short occno );
/* return FORCED/UNFORCED (1/0) */
#define Get_AQ_ForceMark( occno ) \
	( !!( pDynamicData->pAQ.ForceMark[(occno-1)/8] & ORBIT((occno-1)%8) ) )

/* -------------------------------  M  -----------------------------------------
   Input :
       occno : 1 -- MAX_M
   ----------------------------------------------------------------------------*/
#define Get_M( occno ) \
	( !!( pDynamicData->pM.State[(occno-1)/8] & ORBIT((occno-1)%8) ) )
#define Get_N_M( occno ) \
	( !( pDynamicData->pM.State[(occno-1)/8] & ORBIT((occno-1)%8) ) )
#define Output_M(occno, value) \
	( pDynamicData->pM.State[(occno-1)/8]=(  ( pDynamicData->pM.State[(occno-1)/8] & (CLRBIT((occno-1)%8)) ) | (((value)&0x01)<<(occno-1)%8)  ) )

/* -------------------------------  NM  -----------------------------------------
   Input :
       occno : 1 -- MAX_NM
   ----------------------------------------------------------------------------*/
#define Get_NM( occno ) \
	( !!( pNvramData->pNM.State[(occno-1)/8] & ORBIT((occno-1)%8) ) )
#define Output_NM(occno, value) \
	( pNvramData->pNM.State[(occno-1)/8]=(  ( pNvramData->pNM.State[(occno-1)/8] & (CLRBIT((occno-1)%8)) ) | (((value)&0x01)<<(occno-1)%8)  ) )

/* -------------------------------  R  -----------------------------------------
   Input :
       occno : 1 -- MAX_R
	   bit   : 1 -- 16 (32)
   ----------------------------------------------------------------------------*/
#define Get_R( occno ) \
	( pDynamicData->pR.Value[occno-1] )
#define Get_NR( occno ) \
	( pNvramData->pNR.Value[occno-1] )
extern unsigned short Get_R_Bit( unsigned short occno, unsigned short bit ) ;
#define Output_R(occno, value) \
	( pDynamicData->pR.Value[occno-1]=value )
#define Output_NR(occno, value) \
	( pNvramData->pNR.Value[occno-1]=value )

/* -------------------------------  S  -----------------------------------------
   Input :
       occno : 1 -- MAX_S
   ----------------------------------------------------------------------------*/
#define  Get_S( occno ) \
	( !!( pDynamicData->pS.State[(occno-1)/8] & ORBIT((occno-1)%8) ) )
#define Output_S(occno, value) \
	( pDynamicData->pS.State[(occno-1)/8]=(  ( pDynamicData->pS.State[(occno-1)/8] & (CLRBIT((occno-1)%8)) ) | (((value&0x01))<<(occno-1)%8)  ) )

#define  Get_SW( occno ) \
	( pDynamicData->pSW.Value[occno-1])
#define Output_SW(occno, value) \
	( pDynamicData->pSW.Value[occno-1]=value )
	

/* -------------------------------  T  -----------------------------------------
   Input :
       occno : 1 -- MAX_T
   ----------------------------------------------------------------------------*/
/* Call this Function per Tick (10ms)*/
extern void Check_T_PerTick( unsigned int ticksum);
extern void Check_PID_PerTick( unsigned int ticksum);
/* This is Called in LD Exec */
extern void LD_Call_T( unsigned short occno, unsigned char ena, unsigned int preset, unsigned char * Q1, unsigned int *count, unsigned short mstype, unsigned short type );

/* -------------------------------  C  -----------------------------------------
   Input :
       occno : 1 -- MAX_C
   ----------------------------------------------------------------------------*/
/* This is Called in LD Exec */
extern void LD_Call_C(unsigned short direction, unsigned short occno, unsigned char cu_pin, unsigned char cd_pin, unsigned char r_pin, unsigned char ld_pin, unsigned int preset, unsigned char *qu, unsigned char *qd, unsigned int *value );

/* -------------------------------  EVT  -----------------------------------------
   Input :
   ----------------------------------------------------------------------------*/
/* Put one Event into Event Buffer */
extern int  PutEvt( unsigned short GroupNo, char* pEvt1, unsigned short Len /*in words*/) ;

extern void FreeAll_PN(void);

extern void Free_PN(unsigned short occno);

extern unsigned int Process_PN(unsigned short occno, unsigned char state);

extern unsigned short Malloc_PN(void);


/* -------------------------------  Force Data to Database  -----------------------------------------
   Input :
	   para_type:
				PARAMTYPE_Q, PARAMTYPE_AQ, PARAMTYPE_I, PARAMTYPE_Q
       occno : 1 -- MAX_XX
	   ext_data: Reversed
	   pbuf  : src buf
	   data_type: src data_type
			DATATYPE_BOOL, DATATYPE_BYTE, DATATYPE_WORD, DATATYPE_DWORD, 
			DATATYPE_SINT, DATATYPE_INT, DATATYPE_DINT, DATATYPE_REAL
   Return:    0:  success
           else:  fail
   ----------------------------------------------------------------------------*/
extern int Force_Data( unsigned short para_type, unsigned short occno, unsigned short ext_data, unsigned char *pbuf, unsigned char data_type );


/* -------------------------------  SetForceMark Data to Database  -----------------------------------------
   Input :
	   para_type:
				PARAMTYPE_Q, PARAMTYPE_AQ, PARAMTYPE_I, PARAMTYPE_Q
       occno : 1 -- MAX_XX
   Return:    0:  success
           else:  fail
   ----------------------------------------------------------------------------*/
extern int SetForceMark_Data( unsigned short para_type, unsigned short occno );

/* -------------------------------  GetForceMark Data to Database  -----------------------------------------
   Input :
	   para_type:
				PARAMTYPE_Q, PARAMTYPE_AQ, PARAMTYPE_I, PARAMTYPE_Q
       occno : 1 -- MAX_XX
   Return:    1:  forced
              0:  unfoced
   ----------------------------------------------------------------------------*/
extern int GetForceMark( unsigned short para_type, unsigned short occno );

/* -------------------------------  UnForce Data to Database  -----------------------------------------
   Input :
	   para_type:
				PARAMTYPE_Q, PARAMTYPE_AQ, PARAMTYPE_I, PARAMTYPE_Q
       occno : 1 -- MAX_XX
   Return:    0:  success
           else:  fail
   ----------------------------------------------------------------------------*/
extern int UnForce_Data( unsigned short para_type, unsigned short occno );

extern void UnForce_All(void);

/* -------------------------------  Move Func  -----------------------------------------
   Input :
	   pdst:     dst data buf ptr
	   dst_bit_start: src_bit_start: 
				 1---
	   psrc:     src data buf ptr
   Return:
              0:  success
           else:  fail
   ----------------------------------------------------------------------------*/
extern int  Move_Fuc( void* pdst, unsigned int dst_bit_start, 
			   void* psrc, unsigned int src_bit_start, 
			   unsigned int bit_len	);

/* -------------------------------  GetDataPtr Fuc  -----------------------------------------
   Input :
	   para_type:
				PARAMTYPE_I, PARAMTYPE_Q, PARAMTYPE_AI,	PARAMTYPE_AQ, 
				PARAMTYPE_M, PARAMTYPE_R, PARAMTYPE_S, PARAMTYPE_V
       v_occno : valid para_type=PARAMTYPE_V
	   unit_bit_width: return the given data width
   Return:
              0:  fail
	       else: return data ptr ( *unit_bit_width: return the given data width )
   ----------------------------------------------------------------------------*/
extern void* GetDataPtr_Fuc( POINT_DEF para, unsigned int* start_bit );


/* -------------------------------  ModibusProcess Func  ----------------------
   Input :
		 rcvbuf:  256 bytes buffer, require msg
		 sndbuf:  256 bytes buffer, reponse msg
		reallen:  reponse msg length
		crcmark:  0- non crc check  /   else- crc check
   Return:
              0:  success, need send reponse msg
           else:  fail
		          1- Input invalid parameter
				  2- CRC error if needed
				  3- Bad msg
   ----------------------------------------------------------------------------*/
extern unsigned long	ModibusRtu( unsigned char  *rcvbuf, unsigned char *sndbuf, 
					    unsigned short *reallen, unsigned char crcmark );
extern int GetItoQOccNo(LAD_PARA *para);
extern int GetValue(LAD_PARA *para, V_VALUE *Value);
extern int SetValue(LAD_PARA *para, V_VALUE *Value, unsigned int ext_data);
extern int GetValueType(LAD_PARA *para, unsigned char* data_type);
extern int SeqExecLad(int i);
extern int rLadToSeq(unsigned short seqno, unsigned char mode);
/*----------------------------------------------------------------------------*/

extern void MGR_ProcessWT(unsigned char  *rcvbuf);

extern unsigned short CRC16(unsigned char* buf, unsigned short len);

extern int rExecute(unsigned long MyTime, unsigned short iFileNo);
extern void rSeqRcv(void);
extern void rManageTimer(void);
extern void rSeqExe(void);
extern void rSeqMan(void);

extern int Light(int LightNo, int OnOff);
extern int ResetCPU(void);
extern int MyDebug(int LdNo, int StepNo);

extern int MsgSend(unsigned int TaskNo, unsigned char * buffer, unsigned int nBytes);
extern int Can2MsgSend(char * buffer, unsigned int nBytes);

extern float LinearConversion(unsigned short InValue, float UpperLimit, float LowerLimit, float InUpper, float InLower);

extern void MakeCRC(unsigned char* buf, unsigned short len);
extern unsigned short CheckCRC(unsigned char* buf, unsigned short len);
extern void CalenderCheck(void);
extern int CalculateWeekDay(int year, int month, int day);

extern int SysTimerRead(CLOCK *p);
extern int SysTimerWrite(CLOCK *p);

extern int ethdbg_msg_handle(int sid,unsigned char * msgp,ETHDBG_NODE_TABLE *pNode);
extern int DBG_Output_Data( unsigned short para_type, unsigned short occno, unsigned int ext_data, unsigned char *pbuf, unsigned char data_type );

extern int mymodrw(unsigned int port, unsigned int adr, unsigned int code, unsigned int reg, unsigned int num, unsigned int pointtype, unsigned int array, unsigned int occno);
extern unsigned char rPID1(unsigned char En, unsigned int PointNo, unsigned char BIn, unsigned char Man, float * FIn, float *FOut, unsigned int *IOut);
extern int LD_Call_FBC(unsigned short direction, unsigned short occno, unsigned char cu_pin, unsigned char cd_pin, unsigned char r_pin, unsigned char ld_pin, unsigned int preset, unsigned char *qu, unsigned char *qd, unsigned int *value );
extern int LD_Call_FBT( unsigned short occno, unsigned char ena, unsigned int preset, unsigned char * Q1, unsigned int *count, unsigned short mstype ,unsigned short type);

extern int Malloc_FBTimer(void);
extern int Malloc_FBCount(void);

extern int myresh(unsigned char pointtype, unsigned short occno, unsigned char num);
extern int mypwm(int Index, unsigned char Ch, unsigned char En, int Frq, int Duty);
extern int myplsy(int Index, unsigned char Ch, unsigned char En, int Frq, int Pn, unsigned char *State, int *Pon, int *Ps);
extern int myplsr(int Index, unsigned char Ch, unsigned char En, int Frq, int Pn, int Time, unsigned char *State, int *Pon, int *Ps);
extern int mypto(int Index, unsigned char Ch, unsigned char En, int * Tbl, int Num, int Time, unsigned char *State, int *Pon, int *Ps);
extern int myplsf(int Index, unsigned char Ch, unsigned char En, int Frq, int Time);
extern int myzrn(int Index, unsigned char Ch, unsigned char En, int Frq, int Time, unsigned char *State);
extern int mypstop(int Index, unsigned char Ch, unsigned char En, int Time, unsigned char *State);
extern int mypabs(unsigned char En);
extern int mypinc(unsigned char En);
extern int32_t mypline(uint32_t index, uint8_t Ch0,  uint8_t Ch1,uint8_t En,uint8_t Inc, int32_t PulsNum0, int32_t PulsNum1,
				uint8_t *State, int32_t *Pon0, int32_t *Pon1, int32_t *Pos0, int32_t *Pos1);

#endif
