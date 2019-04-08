#ifndef APPCNST_H
#define APPCNST_H

#define CPU_VER					201

/* PLC filename*/
#define APP_COD					"auto.obj"
#define MYDEV					"/na"

#define LED_ON					1
#define LED_OFF					0

#ifndef TRUE
#define TRUE					1
#endif

#ifndef FALSE
#define FALSE					0
#endif


#define MAX_I					256
#define MAX_Q					256
#define MAX_AI					128
#define MAX_AQ					128
#define MAX_M					2048
#define MAX_R					2048
#define MAX_NM					1024
#define MAX_NR					1024
#define MAX_S					1024
#define MAX_SW					512
#define MAX_T					256
#define MAX_C					256
#define MAX_LD					32
#define MAX_SCF					32
#define MAX_FB					8
#define MAX_FBT					32
#define MAX_FBC					32

#define MAX_MDU					15

#define TYPE_CPU				1
#define TYPE_DIM				2
#define TYPE_DOM				3
#define TYPE_DIO				4
#define TYPE_IIM				5
#define TYPE_AIM				6
#define TYPE_AOM				7
#define TYPE_AIO				8
#define TYPE_HCM				9
#define TYPE_CMM				10
#define TYPE_SPM				11
#define TYPE_PWM				12
#define TYPE_FAIM				60

#define NAME_SIZE				24
#define MAX_CALL				16
#define MAX_PRGTASK				16
#define MAX_INT					256

/* for VAR_STRUCT  */
#define VAR_SYS					15        /* from PARAMTYPE_CNST to PARAMTYPE_V-1 */
#define VAR_MEM_LEN				(1024*4)  /* 4KB */


/* for PN_INFO  */
#define MAX_PN					1024
#define MAX_LDPN				256
#define MAX_FBPN				32
#define MAX_BREAK_POINT			10 


/* for Modbus protocol purpose */
/* Exception code */
#define ILLEGAL_FUNC			1
#define ILLEGAL_DATA_ADDR		2
#define ILLEGAL_DATA_VALUE		3
#define SLAVE_DEV_FAIL			4
#define ACKNOWLEDGE				5
#define SLAVE_DEV_BUSY			6
#define NEGATIVE_ACK			7
#define MEM_PARITY_ERR			8


/* 0x :  bit R(01) /W(05/15) */
#define MOD_Q_ADDR				1
#define MOD_Q_ADDR_END			MAX_Q 

#define MOD_M_ADDR				10001
#define MOD_M_ADDR_END			(MOD_M_ADDR+MAX_M-1)

#define MOD_NM_ADDR				30001
#define MOD_NM_ADDR_END			(MOD_NM_ADDR+MAX_NM-1)

/* 1x :  bit R(02) /W(--) */
#define MOD_I_ADDR				1
#define MOD_I_ADDR_END			MAX_I 

#define MOD_S_ADDR				10001
#define MOD_S_ADDR_END			(MOD_S_ADDR+MAX_S-1) 

/* 3x :  word R(04) /W(--) */
#define MOD_AI_ADDR				1
#define MOD_AI_ADDR_END			(MOD_AI_ADDR+MAX_AI-1) 

#define MOD_SW_ADDR				5001
#define MOD_SW_ADDR_END			(MOD_SW_ADDR+MAX_SW-1)

#define MOD_EVT_ADDR			10001
#define MOD_EVT_ADDR_END		(MOD_EVT_ADDR+EVT_OFF+MAX_EVT1*EVENT1_LEN+MAX_EVT2*EVENT2_LEN+MAX_EVT3*EVENT3_LEN-1) 

/* 4x :  word R(03) /W(06/16) */
#define MOD_R_ADDR				1
#define MOD_R_ADDR_END			(MOD_R_ADDR+MAX_R-1) 

#define MOD_AQ_ADDR				20001
#define MOD_AQ_ADDR_END			(MOD_AQ_ADDR+MAX_AQ-1) 

#define MOD_NR_ADDR				21001
#define MOD_NR_ADDR_END			(MOD_NR_ADDR+MAX_NR-1) 

#define MOD_CLOCK_ADDR			30001
#define MOD_CLOCK_ADDR_END		30005

#define MOD_CLK_ADDR			19991
#define MOD_CLK_ADDR_END		19997

/* 6x :  word R(20) /W(21) */
#define MOD_VAR_ADDR			31001
#define MOD_VAR_ADDR_END		(MOD_VAR_ADDR+VAR_MEM_LEN/2-1)

/******************************************************/
#define Q_NOWAIT				1
#define Q_WAIT					0 

#define SCAN_IN					1   
#define SCAN_OUT				0
#define DEFAULT_SCAN			SCAN_IN

#define INITED					1
#define UNINITED				0   
#define DEFAULT_INIT			UNINITED
                        
#define QUALITY_BAD				1
#define QUALITY_OK				0   
#define DEFAULT_QUA				QUALITY_OK

/***** Here "STATE" is only for module's *****/
#define STATE_COMMERR			0x01
#define STATE_DIAGERR			0x02
#define STATE_TYPEERR			0x04
#define STATE_TRANOK			0x20 /*CAN2 Module*/
#define STATE_NOTREADY			0x20 /*CAN1 Module*/
#define STATE_CODELOAD			0x40
#define STATE_IOLOAD			0x80
#define STATE_ALL				(STATE_COMMERR|STATE_DIAGERR|STATE_TYPEERR|STATE_CODELOAD|STATE_IOLOAD)
#define STATE_DRV				(STATE_COMMERR|STATE_DIAGERR|STATE_TYPEERR|STATE_CODELOAD)


/******************** S ***********/
#define FST_SCN					1
#define ALW_ON					2
#define ALW_OFF					3
#define T_SEC					4
#define LD_OVERRUN				6
#define LD_EXECERR				7
#define LD_OPERERR				8
#define IO_COMERR				33
#define IO_DIAGERR				34
#define IO_CFGERR				35
#define COM_COMERR				36
#define COM_DIAGERR				37
#define COM_CFGERR				38
#define IO_DOWNLOAD				41
#define COM_DOWNLOAD			42

#define MST_M1					97
#define MST_FLT1				98
#define MST_GPSLOST1			99
#define MST_SEC1				100
#define MST_CAN11FLT			101
#define MST_CAN12FLT			102
#define MST_ETH11FLT			103
#define MST_ETH12FLT			104
#define MST_TASK1FLT			105
#define MST_SELF_ON1			106
#define MST_FATALERR1			107
#define MST_STOP1				116
#define MST_DEBUG1				117
#define MST_NVRAMFLT1			118

#define MODBUSTCPFLT			145

#define MDU_S_FIRST				(513-1)


/********************* SW ************************/
#define TIME_INFO				1   /* 8 words */
#define ALARM_PTR				9
#define SOE_PTR					10
#define OVERRUN_INFO			11
#define EXECERR_INFO			14
#define CPU_TYPE				18
#define SOFT_VER				19
#define COM1_SEND				21
#define COM1_RECV				22
#define COM2_SEND				23
#define COM2_RECV				24
#define START_TIME				25   /* 7 words */
#define SCAN_TIME				32
#define LADEXE_TIME				33


#define DEFAULT_IP				0xc0a80142

/* Data Type */
#define DATATYPE_BOOL			0
#define DATATYPE_BYTE			1
#define DATATYPE_WORD			2
#define DATATYPE_DWORD			3
#define DATATYPE_SINT			4
#define DATATYPE_INT			5
#define DATATYPE_DINT			6
#define DATATYPE_REAL			7
#define DATATYPE_USINT			8
#define DATATYPE_UINT			9
#define DATATYPE_UDINT			10
#define DATATYPE_TIME			11
#define DATATYPE_DATE			12
#define DATATYPE_TOD			13
#define DATATYPE_DT				14
#define DATATYPE_STRING			15
#define DATATYPE_NULL			89
#define DATATYPE_ALL			90


/* Parameter Type */
#define PARAMTYPE_NULL			0
#define PARAMTYPE_CNST			1			/* 常数 */
#define PARAMTYPE_I				2			/* I */
#define PARAMTYPE_Q				3			/* Q */
#define PARAMTYPE_AI			4			/* IW */
#define PARAMTYPE_AQ			5			/* QW */
#define PARAMTYPE_M				6			/* M */
#define PARAMTYPE_R				7			/* MW */
#define PARAMTYPE_NM			8			/* N */
#define PARAMTYPE_NR			9			/* NW */
#define PARAMTYPE_S				10			/* S */
#define PARAMTYPE_SW			11			/* SW */
#define PARAMTYPE_T				12			/* T */
#define PARAMTYPE_C				13			/* C */
#define PARAMTYPE_V				14			/* V */
#define PARAMTYPE_TRUE			15
#define PARAMTYPE_BOOL			16
#define PARAMTYPE_DINT			17
#define PARAMTYPE_REAL			18
#define PARAMTYPE_IP			33			/* Ethernet IP Address */
#define PARAMTYPE_INPUT			37          /* 用户自定义功能模块 输入 */
#define PARAMTYPE_OUTPUT		38			/* 用户自定义功能模块 输出 */

typedef int 		(*FUNCPTR) ();


#define	UDP_PRI					22
#define	TCPC_PRI				23
#define	TCPS_PRI				24
#define	ETHRUN_PRI				25
#define	ETHMTM_PRI				26
#define	ETHDBG_PRI				30

#endif

