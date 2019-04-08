#ifndef   SEQEXT_H
#define   SEQEXT_H 

#define MAX_FILE_STATE   8
#define MAX_OP_STACK     10
#define MAX_BREAK_POINT  10
#define MAX_USER_TIMER   10
#define MAX_USER_IID     10
#define MAX_STRING       64
#define MAX_DESCPT       48
#define MAX_TAGPATH      40
#define MAX_SEQ_NAME     8

#define noRUN            0x00
/* MSG_TO_SEQ_TSK */
#define rRUN             0x01
#define rWATCH           0x02
#define rDEBUG           0x03
#define rKILL            0x04
#define rLOCK            0x05
#define rUNLOCK          0x06

#define mREADY			 0x00
#define mEXECUTE		 0x01
#define mSUSPEND		 0x02
#define mNEXT 			 0x03
#define mFINISH 		 0x04

/* MSG_TO_SEQ_EXE */
#define rNEXT            0x01
#define rRESTART         0x02
#define rSTOP            0x03
#define rCONTINUE        0x04
#define rSTOPAT          0x05
#define rDELETE          0x06
#define rDELALL          0x07
#define rPRINT           0x08
#define rASSIGN          0x09

/* MSG_TO_FC_MAN */
#define sRUN             0x01
#define sWATCH           0x02
#define sDEBUG           0x03
#define sKILL            0x04
#define sLOCK            0x05
#define sUNLOCK          0x06
#define sRETURN          0x07
#define sDISP            0x08
#define sDBGDISP         0x09
#define sDIAG            0x0a
#define sSTEP            0x0b
#define sSSTEP           0x0c
#define sPRINT           0x0d
#define sVERSION         0x0e

/* MSG_TO_WS_DRV */
#define sLOG             0x01
#define sAUDIO           0x02
#define sOPENDISP        0x03

typedef char STR_T[MAX_STRING];


typedef union
{
	int          iData;
	float        fData;
} DATA_DEF;

typedef struct
{
    int          DataType;
	DATA_DEF     DataValue;
} DATA_STRUCT;

typedef struct
{
    unsigned short Flag;
    unsigned short InstNo;
} TIMEOUT_FLAG;  

typedef struct
{
  unsigned long  RemainTime;
  unsigned short InstNo;
  unsigned short Use;
} TIMER_STRUCT;

typedef struct
{
	unsigned char  Use;
	unsigned char  Lock;	
	unsigned char  State;
	unsigned char  SeqNo;
	unsigned short Mode;
	unsigned short RootNo;
	unsigned short PrevNo;
	unsigned short NextNo;
	char	       SeqFile[MAX_SEQ_NAME];
	char           Version[NAME_SIZE];
    DATA_STRUCT    OpStack[MAX_OP_STACK];
    int            OpStackIndex;
    unsigned short BreakPoint[MAX_BREAK_POINT];
    int            BreakPointIndex;
    unsigned int   TimeOut;
    TIMER_STRUCT   UserTimer[MAX_USER_TIMER];
    unsigned short ContFlag;
    unsigned short StepNo;
	unsigned short InstNo;    
	unsigned short DelayTime;    
	unsigned short TrapInstNo;
	unsigned short TrapFlag;
	DATA_STRUCT    Temp[1];
	DATA_STRUCT    ReturnValue;/*Reversed*/
	DATA_STRUCT    pId[MAX_USER_IID];
} FILE_STATE;

typedef struct
{
	unsigned char  Len;
	unsigned char  Code;
	unsigned char  SeqNo;
	unsigned char  Mode;
	unsigned short StepNo;
	unsigned short Pad;
} SEQ_BUF;

typedef struct
{
	char	       SeqFile[MAX_SEQ_NAME];
	char           Descript[MAX_DESCPT];
	char           Version[NAME_SIZE];
	unsigned char  MsgIn;
	unsigned char  MsgOut;
	unsigned char  StepIn;
	unsigned char  StepOut;
	SEQ_BUF		   MsgBuf[MAX_SEQ_BUF];
	unsigned int   StepBuf[MAX_SEQ_BUF];
	unsigned short LoadState;
	unsigned short ErrNum;
	unsigned short Pad;
	unsigned short FileNo;
	unsigned short InstNum;
	unsigned short StrNum;
	unsigned short TagNum;
	unsigned short IdNum;
	INST_DEF       *pInst;
	STR_T          *pStr;
	unsigned long  DoNum;  
	//unsigned short DoOccno[64];
} FILE_ENTRY;

typedef struct
{
	char	       SeqFile[MAX_SEQ_NAME];
} FILE_INFO;


#endif
