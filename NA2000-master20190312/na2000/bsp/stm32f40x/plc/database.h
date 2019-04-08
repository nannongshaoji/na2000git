#ifndef DATABASE_H
#define DATABASE_H


/* For Normal Purpose */
#ifndef NOERR
#define NOERR					0
#endif

/* for EVENT       */
#define MAX_EVT_BOX				3   /* 4 EVT Queue */
#define EVT_OFF					30  /* 30 Word */
#define MAX_EVT1				0
#define EVENT1_LEN				0
#define MAX_EVT2				32
#define EVENT2_LEN				32
#define MAX_EVT3				0
#define EVENT3_LEN				0
#define EVENT_LEN				(MAX_EVT1*EVENT1_LEN+MAX_EVT2*EVENT2_LEN+MAX_EVT3*EVENT3_LEN)


/*   =============================  I  =============================================================*/
typedef struct
{   /* State.n= forceMark.n==FORCED?  : rawData.n */
	unsigned char   State[MAX_I/8];      /* DI value, occno--occno+7: State.0--State.7 */
	unsigned char   RawData[MAX_I/8];     /* DI raw data, occno--occno+7: rawData.0--rawData.7 */
	unsigned char   ForceMark[MAX_I/8];   /* DI raw data, occno--occno+7: forceMark.0--forceMark.7 */
} I_INFO;


/*   =============================  Q  =============================================================*/
#define HOLD_TYPE				0
#define HOLD_TYPE1				0xFFFFFFFF
#define MAX_PULSE				600000
typedef struct
{   /* State :DO real state.
	   Output, PulseTime, OutputMark only valid when ForceMark is UNFORCED.
	   Note: All DO is output together.
	*/
	unsigned char   State[MAX_Q/8];      /* DO value, occno--occno+7: State.0--State.7 */
	unsigned char   RawState[MAX_Q/8];   /* DO value, occno--occno+7: State.0--State.7 */
	unsigned char   ForceMark[MAX_Q/8];  /* DO Force mark, occno--occno+7: ForceMark.0--ForceMark.7 */
	unsigned char   Output[MAX_Q/8];     /* DO Output Image, occno--occno+7: Output.0--Output.7 */
	unsigned char   OutputEna[MAX_Q/8];  /* DO Output Enable, occno--occno+7: OutputEna.0--OutputEna.7 */
	unsigned char   CheckEna[MAX_Q/8];   /* DO Chena Enable, occno--occno+7: OutputEna.0--OutputEna.7 */
	unsigned int    PulseTime[MAX_Q];    /* DO Output Pulse Time in Ticks (10ms), 0xFFFF is HOLD_TYPE, others mean PULSE_TYPE. */
} Q_INFO;


/*   =============================  AI  =============================================================*/
typedef struct
{   /* output[n]= forceMark.n==FORCED?  : rawData[n] */
	unsigned short	Value[MAX_AI];       /* AI value */
	unsigned short	RawData[MAX_AI];     /* AI raw data */
	unsigned char	ForceMark[MAX_AI/8]; /* AI raw data, occno--occno+7: forceMark.0--forceMark.7 */
} AI_INFO;


/*   =============================  AQ  =============================================================*/
typedef struct
{   /* State :AO real value.
	   Output,  OutputMark only valid when ForceMark is UNFORCED.
	   Note: All AO is output together.
	*/
	unsigned short  Value[MAX_AQ];      /* AO value */
	unsigned char   ForceMark[MAX_AQ/8];  /* AO Force mark, occno--occno+7: ForceMark.0--ForceMark.7 */
	unsigned short  Output[MAX_AQ];     /* AO Output Image */
	unsigned short	RawData[MAX_AQ];     /* AO raw data */
	unsigned char   OutputEna[MAX_AQ/8];  /* AO Output Enable, occno--occno+7: OutputEna.0--OutputEna.7 */
} AQ_INFO;

		
/*   =============================  M  =============================================================*/
typedef struct
{   /* State :M real state.
	   PulseTime, OutputMark 
	   Note: All M is output immediately.
	*/
	unsigned char   State[MAX_M/8];      /* M value, occno--occno+7: State.0--State.7 */
} M_INFO;


/*   =============================  R  =============================================================*/
typedef struct
{   /*  R real value.
	*/
	unsigned short	Value[MAX_R];      /* R value */
} R_INFO;


/*   =============================  NM  =============================================================*/
typedef struct
{   /* State :NM real state.
	   PulseTime, OutputMark 
	   Note: All NM is output immediately.
	*/
	unsigned char   State[MAX_NM/8];      /* NM value, occno--occno+7: State.0--State.7 */
} NM_INFO;


/*   =============================  NR  =============================================================*/
typedef struct
{   /*  R real value.
	*/
	unsigned short	Value[MAX_NR];      /* NR value */
} NR_INFO;


/*   =============================  S  =============================================================*/
typedef struct
{   /*  system bit value . 1--1024
	    Details refer to "SJ600A Configuration Software Detail Design"
	*/
	unsigned char	State[MAX_S/8];     
} S_INFO;


/*   =============================  SW  =============================================================*/
typedef struct
{   
	unsigned short	Value[MAX_SW];     
} SW_INFO;

/*   =============================  T  =============================================================*/
typedef struct
{   /* CtrlWord: 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 
                  |  |  |  |  |  | | |   | | |     | |_ Type 0/1: Timer/HoldTimer(Reversed)
                  |  |  |  |  |  | | |	 | | |	   |___	Started	
                  |  |  |  |  |	 | | |	 | | |_________				
                  |  |  |  |  |	 | | |	 | |___________	ms/s/m/h type
                  |  |  |  |  |	 | | |	 |_____________
				  |  |  |  |  |  | | |_________________
				  |  |  |  |  |  | |___________________TON/TOF/TP type
				  |  |  |  |  |  |_____________________
                  |  |  |  |  |________________________ 							
                  |  |  |  |___________________________ Old Ena	Status 
                  |  |  |______________________________ Reset Status : 0/1 : Work  /Reset (Reversed)
                  |  |_________________________________ Q
				  |____________________________________ Ena	Status : 0/1 : DiaEna/Ena 
    */

	unsigned short	CtrlWord[MAX_T];
	unsigned int	PreValue[MAX_T];
	unsigned int	CurValue[MAX_T];
} T_INFO;


/*   =============================  C  =============================================================*/
typedef struct
{   /* CtrlWord: 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0 
                  |  |  |  |  |  |  |  |             
                  |  |  |  |  |  |	|  |		    	
                  |  |  |  |  |	 |	|  |_______________	QD				
                  |  |  |  |  |	 |	|__________________ QU 					
                  |  |  |  |  |	 |_____________________	Old CD	Status					
                  |  |  |  |  |________________________ Old CU	Status 							
                  |  |  |  |___________________________ LD Status : 0/1 : Work  /Reset 
                  |  |  |______________________________ R Status : 0/1 : Work  /Reset 
                  |  |_________________________________ CD	Status : 0/1 : DiaEna/Ena
				  |____________________________________ CU	Status : 0/1 : DiaEna/Ena 
    */

	unsigned short  CtrlWord[MAX_C];
	unsigned short	PreValue[MAX_C];
	unsigned short	CurValue[MAX_C];
} C_INFO;


/*   =============================  SYS_INFO  ========================================================*/
typedef struct
{
	unsigned char	Version[NAME_SIZE]; /* annex  Version */
	unsigned char	LadExecFirstTime;
	unsigned char	pad1;
	unsigned char	pad2;
	unsigned char	pad3;
} SYS_INFO;

/*   =============================  PN_INFO  ========================================================*/
typedef struct
{
    unsigned char	State[MAX_PN/8]; 
    unsigned char	OldState[MAX_PN/8]; 
    unsigned char	IsDefined[MAX_PN/8]; 
    unsigned char	FirstMark[MAX_PN/8]; 
} PN_INFO;

#define MAX_PID					32
typedef struct 
{
	unsigned char	ScanEna;        /* SCAN_OUT / SCAN_IN, DEFAULT_SCAN */   
	unsigned char	Init;           /* UNINITED / INITED ,DEFAULT_INIT  */
	unsigned char	IsDefined;      /* YES : is defined, NO: spare */
	unsigned char	OldState;
	float			SetPoint;
	float			Measure;
	float			Error;
    float			OldOutput;
    float			OldError;
	float			SumError;
    unsigned int	ScanCnt;
	unsigned char	Flag[4];
} PID;

/*   =============================  EVT_INFO  ========================================================*/

typedef struct
{
    unsigned int	GroupNo;/* 1,..4 mean valid,  0 means not used */
    unsigned int	StartPtr; /* Base is StartPtr. strat with StartPtr, such as 2021 */ 
    unsigned int	CurPtr;  /* The next Empty Evt Buffer, start with 3x2001 not 0 */
    unsigned int	PerEvtLen;  /* in words */
    unsigned int	MaxEvt; /*  for EVT1_INFO, this is EVENT1_LEN(8) */ 
} EVT_DEF;

typedef struct /* include 8 Words */
{
    unsigned char	Code;
    unsigned char	EvtCode; 
    unsigned char	Year;/* 2000+Year */
    unsigned char	Month;
    unsigned char	Day;
    unsigned char	Hour;
    unsigned char	Minute;
    unsigned char	Second;
    unsigned short	Ms;
    unsigned short	OccNo;
    unsigned short	DataL;
    unsigned short	DataH;
}EVT1_INFO;

typedef struct /* include 32 Words */
{
    unsigned char	SeqNo;
    unsigned char	YearL;
    unsigned char	YearH;
    unsigned char	Month;
    unsigned char	Day;
    unsigned char	Hour;
    unsigned char	Minute;
    unsigned char	Second;
    char			AlarmStr[56];
}EVT2_INFO;

typedef struct
{
	unsigned char	Year;
    unsigned char	Month;
    unsigned char	Day;
    unsigned char	Hour;
    unsigned char	Minute;
    unsigned char	Second;
    unsigned short	Ms;
	unsigned char	Pad[56];
}EVT3_INFO;

typedef struct
{
	EVT_DEF			pDef[MAX_EVT_BOX];/* 30 Word */
	unsigned short	p1[EVENT_LEN];
}EVENT_INFO;

/*   =============================  DYNAMIC_DATA  =============================================================*/
typedef struct
{	
	SYS_INFO		pSys;
	I_INFO			pI;      
	Q_INFO			pQ;   
	AI_INFO			pAI; 
	AQ_INFO			pAQ;  
	M_INFO			pM;   
	R_INFO			pR; 
	S_INFO			pS;  
	SW_INFO			pSW;      
	T_INFO			pT;    
	C_INFO			pC;
	PN_INFO			pPN;  
	PID				pPID[MAX_PID];
	unsigned char	pVAR[VAR_MEM_LEN];
	EVENT_INFO		pEVENT; 
} DYNAMIC_DATA;


typedef struct
{
	unsigned char   test[4];
	NM_INFO			pNM;        
	NR_INFO			pNR;  
} NVRAM_DATA;


typedef struct
{
	int				Baud;
	unsigned char	DataBit;
	unsigned char	Parity;
	unsigned char	StopBit;
	unsigned char	Protocol;
} COM_PARA;

typedef struct
{
	unsigned char	Start;
	unsigned char	End;
	unsigned char	MaxBytes;
	unsigned char	Pad;
	unsigned short  MaxInterval;
	unsigned short  TimeOut;
} COM_PARA1;

typedef struct 
{
	char			ModuleName[NAME_SIZE-4];
	unsigned long	NtpSrv;
	unsigned char	IsDef;
	unsigned char   Type;
	unsigned char	ModuleNo;	/* Module Type */
	unsigned char	Pad;
	unsigned short	INum;
	unsigned short	IRefAddr;
	unsigned short	QNum;
	unsigned short	QRefAddr;
	unsigned short	AINum;
	unsigned short	AIRefAddr;
	unsigned short	AQNum;
	unsigned short	AQRefAddr;
	unsigned long	EthAddr;
	unsigned long	Pad2;
	COM_PARA		ComDef[2];
	COM_PARA1		ComDef1[2];
} MODULE_DEF;

typedef struct
{
	unsigned char	IsDefined;
	unsigned char	ModuleAddr;
	unsigned char	MasterId;
	unsigned char	SlaveId;
	unsigned short	FiltTime;
	unsigned char   IsSoe;
	unsigned char	ExtPad;
} I_DEF;

typedef struct
{
	unsigned char	IsDefined;
	unsigned char	ModuleAddr;
	unsigned char	MasterId;
	unsigned char	SlaveId;
	unsigned char	FaultOutput;
	unsigned char	ExtPad[3];
} Q_DEF;

typedef struct
{
	unsigned char	IsDefined;
	unsigned char	ModuleAddr;
	unsigned char	MasterId;
	unsigned char	SlaveId;
	unsigned char	Pad;
	unsigned char	SignalType;
	unsigned short  ZeroOffset;
	float			UpperLimit;
	float			LowerLimit;
} AI_DEF;

typedef struct
{
	unsigned char	IsDefined;
	unsigned char	ModuleAddr;
	unsigned char	MasterId;
	unsigned char	SlaveId;
	unsigned char	Pad;
	unsigned char	SignalType;
	unsigned short	FaultOutput;
} AQ_DEF;


/************************************** STATIC_DATA ****************************************************/
typedef struct
{
	unsigned char	*p; /* data start ptr */
	unsigned short	StartPtr; /* start with 1, in ModBus DataType bit(0x,1x), word(3x,4x,6x) */
	unsigned short	WordLen;  /* in WORDs, VAR begin from pDynamicData->pVAR */
	unsigned short	BitLen;   /* in bits, VAR begin from pDynamicData->pVAR */
	unsigned short	Array;    /* in Structs, VAR begin from pDynamicData->pVAR */
	unsigned char	UnitWidth;/* 1(BOOL),8(C,UC),16(W,I),32(DW,DI,F) */
	unsigned char	ParamentType; /* PARAMTYPE_I,_Q,_AI,_AQ,_M,_R,_V,_S,_T,_C */
	unsigned char	DataType; /* DATATYPE_BOOL, ..., DATATYPE_REAL */
	unsigned char	ModType;  /* For, 0xFF(_T & _C, _V), 0,1,3,4,6(Others) */
} VAR_STRUCT;


typedef struct
{	
    MODULE_DEF		pMDU[MAX_MDU];
	I_DEF			pI[MAX_I];       
	Q_DEF			pQ[MAX_Q]; 
	AI_DEF			pAI[MAX_AI]; 
	AQ_DEF			pAQ[MAX_AQ]; 
	VAR_STRUCT		pVAR[VAR_SYS];
	unsigned short	MaxMDU;
	unsigned short	MaxI;
	unsigned short	MaxQ;
	unsigned short	MaxAI;
	unsigned short	MaxAQ;
	unsigned short	MaxM;
	unsigned short	MaxR;
	unsigned short	MaxNM;
	unsigned short	MaxNR;
	unsigned short	MaxS;
	unsigned short	MaxSW;
	unsigned short	MaxT;
	unsigned short	MaxC;
	unsigned short	MaxV;
	/* Station Info -----------------------*/
	
	unsigned short	NaproVer;
	unsigned char	EngFlag;
	unsigned char   StationNo2;
	unsigned char	StationNo;   /* 1,2,... 				*/
	unsigned char	DualNo;      /* 1/2 --- CCU2_1/CCU2_2 */
	unsigned short	Myticks;

	/* Seq */
	unsigned short	SeqNum;
	unsigned short	Pad3;
	FILE_INFO		FileInfo[MAXFCNUM];
	unsigned char	FbInstBool[256];

	unsigned int	Offset;
	unsigned char	ModtcpNum;
	unsigned char	net_state;
	unsigned char	int_enable_flag;
	unsigned char	FbNo;
	unsigned char	FbInstNo;
	unsigned char   FbInstState;
	short			TimeZoneBias;
	/* Program task */
	unsigned short  TaskPrgNo[MAX_PRGTASK];
	unsigned short  TaskPrgTime[MAX_PRGTASK];
	/* INT */
	unsigned short  IntPrgNo[MAX_INT];
	unsigned short  IntPrgTime[5];

} STATIC_DATA;


typedef struct
{
    char			m_ParamName[4];
	unsigned short  pad;
	unsigned char	m_PointType;
	unsigned char	sub_PointType;
	unsigned short	m_OccNo;
	unsigned short	sub_OccNo;
	unsigned char   m_DataType;
	unsigned char	sub_DataType;
	unsigned char	m_Bias;
	unsigned char	sub_Bias;
}LAD_PARA;


typedef struct
{
	/* PRG */
	unsigned short	PrgNum;
	unsigned short  LdNum;
	char            PrgName[MAX_LD][10];
	unsigned short  PrgVer[MAX_LD];
	unsigned char	*pBool[MAX_LD];
	int				*pDint[MAX_LD];
	float			*pReal[MAX_LD];
	unsigned char   *RSData[MAX_LD];
	unsigned short  *PNOccNo[MAX_LD];
	unsigned short	BoolCnt[MAX_LD];
	unsigned short	DintCnt[MAX_LD];
	unsigned short	RealCnt[MAX_LD];
	unsigned short	RSCnt[MAX_LD];
	unsigned short	PNCnt[MAX_LD];
	unsigned short	BreakPoint[MAX_LD][MAX_BREAK_POINT];
    unsigned short	BreakPointIndex[MAX_LD];
	unsigned short  CurrentStep[MAX_LD];
	unsigned short	CntFlag[MAX_LD];
} PRG_DATA;

typedef struct
{
	unsigned char	*pBool[MAX_LD];
	int				*pDint[MAX_LD];
	float			*pReal[MAX_LD];
	unsigned char   *RSData[MAX_LD];
	unsigned short  *PNOccNo[MAX_LD];
	unsigned short  *CurrentStep;
} LAD_STRUCT;


typedef struct
{ 
	unsigned char   IsDefined;
	unsigned char   Pad;
	unsigned short	CtrlWord;
	unsigned int	PreValue;
	unsigned int	CurValue;
} FBTC_INFO;

#define FB_VAR_LEN		(2*1024)
typedef struct
{
	unsigned short	FbNum;
	unsigned short	VarLen;
	char			FbName[MAX_FB][10];
	FBTC_INFO		pT[MAX_FBT];     
	FBTC_INFO		pC[MAX_FBC];
	unsigned char	FBVar[FB_VAR_LEN];
} FB_DATA;

typedef struct
{
	unsigned char	*FBVar;
} FB_STRUCT;

#define MAX_INST_NUM	32
#define MAX_FB_DATA		512
#define DYN_FB_INST		4
#define MAX_FB_PARA		32
typedef struct
{
	unsigned short	instnum;
	unsigned char	initflag;
	unsigned char	pad;
	int				nowinstnum;
	short			pinst[MAX_INST_NUM];
	short			cinst[MAX_INST_NUM];
	unsigned char	pad1;
	unsigned char   VarNum;
	unsigned short  DefInstNum;
	unsigned short  *VarAddr[MAX_FB_PARA];
	unsigned short	*FBPNNo[MAX_FBPN];
	unsigned char	*FBRSData[MAX_FBPN];
	unsigned short	*FBTimerNo[MAX_FBPN];
	unsigned short	*FBCountNo[MAX_FBPN];
	unsigned int	length;
	unsigned char	data[MAX_FB_DATA];
	unsigned int    *OutData[MAX_FB_PARA];
} FB_INFO;
                           
/**********************************************************/
typedef struct 
{
	unsigned short	Year;
	unsigned char	Month;
	unsigned char	Day;
	unsigned short	Hour;
	unsigned char	Minute;
	unsigned char	Second;
	unsigned short	Ms;
	unsigned char	Week;
	unsigned char	Pad;	
} CLOCK;



typedef struct
{
	unsigned char	m_PointType;
	unsigned char	sub_PointType;
	unsigned short	m_OccNo;
	unsigned short	sub_OccNo;
	unsigned char	m_DataType;
	unsigned char	m_Bias;
} POINT_DEF;

typedef union 
{
	unsigned char	Bool;
	unsigned char	Byte;
	unsigned short	Word;
	unsigned int	Dword;
	signed char		Sint;
	short			Int;
	int				Dint;
	float			Real;
} V_VALUE;

typedef struct
{
	unsigned int	Type;
	unsigned short	Timeout;
	unsigned short	Port;
	unsigned int	Ip;
	unsigned char	Res[4];
} ETH_PARAM;

#define	MAX_ETH_TABLE		4
typedef struct {
	unsigned char	Used;
	unsigned char	Type;
	unsigned short	Id;
	unsigned short	Timeout;
	unsigned short	Port;
	unsigned int	Ip;
	int             Sid;                     /* socket id */ 
	rt_mq_t			Qid;
	rt_thread_t		Tid;
} ETH_TABLE;


typedef struct
{
	unsigned char	Code;
	unsigned char	Addr;
	unsigned short	Reg;
	unsigned short	Num;
	unsigned char	Mode;
	unsigned char	Cfgno;
	unsigned short	Time;
	unsigned short	Cnt;
	int				Errcnt;
	unsigned char	*pData;
	unsigned int	Offset;
} MODBUSTCP_CMD;

typedef struct
{
	unsigned long	Ip;
	unsigned int	Num;
	MODBUSTCP_CMD	*Cmd;
	int				Sid;
	int				State;
} MODBUSTCP_DATA;

typedef struct
{
	unsigned char	IsDefined;
	unsigned char	MduAddr;   /* 1.. */
	unsigned char	ErrCnt_R;
	unsigned char	ErrCnt_S;
	unsigned char	ModuleType;/* TYPE_DIN / TYPE_DOUT / TYPE_AIN */
	unsigned char	pad1;
	unsigned short	S_Addr;    /* 1.. */
	unsigned short	IOSum;
	unsigned short	IOStart;   /* 1.. */
	unsigned short	Serial;    /* 1.. */
	unsigned short	pad2;	  /*  */
} IOMDU_STRU;

typedef struct
{
	unsigned char	Code;
	unsigned char	Addr;
	unsigned short  Reg;
	unsigned short  Num;
	unsigned short  Occno;
	POINT_DEF		Data;
} MODBUS_MASTER;

typedef struct
{
	int				Baud;
	unsigned char	DataBit;
	unsigned char	Parity;
	unsigned char	StopBit;
	unsigned char	Protocol;
	unsigned char	Start;
	unsigned char	End;
	unsigned char	MaxBytes;
	unsigned char	Addr;
	unsigned short  MaxInterval;
	unsigned short  TimeOut;

	int				ModNum;
	MODBUS_MASTER	*ModM;
} COM_DEF;

typedef struct
{
	unsigned char	Polar;			//方向极性，0-正极性，1-负极性
	unsigned char	Stop;			//脉冲紧急停止方式，0-急停，1-减速停
	unsigned char	Prt;			//极限开关保护使能，0-关闭，1-开启
	unsigned char	Pad;
	unsigned short	MinTime;		//最小加减速时间
	unsigned short	MaxTime;		//最大加减速时间
	unsigned int	MinFrq;			//最小运行频率
	unsigned int	MaxFrq;			//最大运行频率
} PTO_DEF;

typedef struct
{
	unsigned char	Mode;			//计数模式，0-无，1-测频模式，2-单相递增计数模式，3-脉冲+方向计数模式，4-AB相计数模式
	unsigned char	X1X4;			//倍率，1-4倍频，0-1倍频（计数模式4有效）
	unsigned char	Pad[2];
} HSC_DEF;

typedef struct						//数据中心
{
	unsigned char	Enable;			//使能，0-不使能，1-使能
	unsigned char	Mode;			//地址方式，0-IP地址，1-域名
	unsigned short	Port;			//端口号
	unsigned int	Ip;				//IP地址
	char			Dns[56];		//域名
} NA2000_GPRS_DATA;

typedef struct
{
	NA2000_GPRS_DATA Data[4];
	unsigned short	Interval;		//心跳包间隔
	unsigned short	Timeout;		//心跳包超时
	char			Register[20];	//自定义注册包
	char			Heart[20];		//自定义心跳包
	char			Apn[20];		//APN名称
	char			Login[20];		//登录名称
	char			Password[20];	//登录密码
	char			Pad[24];
} NA2000_GPRS_DEF;

typedef struct
{
	unsigned char	Enable;			//无线通讯使能，0-不使能，1-使能
	unsigned char	Type;			//无线网络类型，0-ZigBee，1-RoLa
	unsigned char	Master;			//站点类型，0-从站，1-主站
	unsigned char	Protocol;		//协议类型，0-南大傲拓，1-自定义
	unsigned short	Netid;			//网络号
	unsigned char	Chid;			//信道号
	unsigned char	Pad;
	unsigned short	Addr;			//本站地址
	unsigned short	Interval;		//主站命令间隔
	unsigned short	Timeout;		//主站命令超时
	unsigned short	Pad2;
	int				ModNum;			//主站命令数量
	MODBUS_MASTER	*ModM;			//主站命令报文
} NA2000_ROLA_DEF;

typedef struct
{
	int				TOccno;
	int				TNum;
	int				COccno;
	int				CNum;
	int				HscOccno;
	int				HscNum;
	int				PtoOccno;
	int				PtoNum;
	int				MWOccno;
	int				MWNum;	
	int				Pad[6];
} NA2000_RET_DEF;

typedef struct
{
	IOMDU_STRU		pIOMDU[MAX_MDU];
	unsigned short	CAN1MduSum;
	unsigned short	Pad;

	COM_DEF			Com[3];
	
	PTO_DEF			PtoDef[4];
	HSC_DEF			HscDef[2];
	unsigned char	PtoVType;
	unsigned char	HscVType;
	unsigned short	PtoVOccno;
	unsigned short	HscVOccno;
	unsigned short	Pad2;

	NA2000_GPRS_DEF Gprs;
	NA2000_ROLA_DEF Rola;

	NA2000_RET_DEF	RetDef;
} CONFIG_DATA;

#endif
