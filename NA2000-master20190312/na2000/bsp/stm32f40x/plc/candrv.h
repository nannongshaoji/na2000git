/*  ----------------------------------------------------------------------
                                CPU Software 
                      
              Version   :  1.0                                                     
              Filename  :  candrv.h
   Originally Written By:  Chen Sining                         
	   	   Date     	:  2007.11.24
      Application Notes :  This file defines basic macro definations, and may
                           be used by both Config.Software and CPU target
                           application. So, in case of updating, care must
                           be taken that all indivisuals should be informed
                           to conform with the changes. 
        Updating Record :  ( Pls. Specify : Who , Where , When, Why , What )

   -----------------------------------------------------------------------*/
#ifndef CANDRV_H
#define CANDRV_H

#define BYTE 	 				unsigned char
#define UC 	 					unsigned char
#define US 	 					unsigned short
#define UI    					unsigned int
#define UL    					unsigned long

#ifndef YES
#define YES						1
#endif

#ifndef NO
#define NO						0
#endif

#ifndef NOERR
#define NOERR					0
#endif

#define BUF_LEN					512
#define DlBufCnt    			(4096/8)
#define rec_buf_cont		    8


#define can_board_addr			0x7f
                   	
#define	Fail					0
#define	FAIL			 		Fail
#define	Success					1 
#define nulg 			      	0x00

typedef struct 
{
        BYTE dlen;
        BYTE did1;
        BYTE did2;
        BYTE did3;
        BYTE did4;
        BYTE data[8];
        BYTE dpad1;
        BYTE dpad2;
        BYTE dpad3;
} Msg_Struct;

typedef struct 
{
		BYTE		Irq;
		BYTE		Pad;
		short	    GetNo; 
		short		PutNo;
		short       Pads;
		Msg_Struct  Msg[DlBufCnt];
}ISR_STRUCT;

typedef struct 
{
        BYTE addr;
        BYTE acc_mask;
        BYTE bt0;
        BYTE bt1;
		BYTE serial;
		BYTE pad1;
		BYTE pad2;
		BYTE pad3;
}Can_Struct;

typedef struct
{
		BYTE	MOD;     /* 0 */
		BYTE	CMR;     /* 1 */
		BYTE	SR;      /* 2 */
		BYTE	IR;      /* 3 */
		BYTE	IER;     /* 4 */ 
		BYTE	RVS1;    /* 5 */
		BYTE	BTR0;    /* 6 */
		BYTE	BTR1;    /* 7 */
		BYTE	OCR;     /* 8 */		
		BYTE	TEST;    /* 9 */
		BYTE	RVS2;    /* 10 */
		BYTE	ALC;     /* 11 */
		BYTE	ECC;     /* 12 */
		BYTE	EWLR;    /* 13 */
		BYTE	RXERR;   /* 14 */
		BYTE	TXERR;   /* 15 */
		BYTE	EFF;     /* 16 --Reset: ACR0 */
		BYTE	IDC1;     /* 17 --Reset: ACR1 */
		BYTE	IDC2;     /* 18 --Reset: ACR2 */
		BYTE	IDC3;     /* 19 --Reset: ACR3 */
		BYTE	IDC4;     /* 20 --Reset: AMR0 */
		BYTE	DATA1;   /* 21 --Reset: AMR1 */
		BYTE	DATA2;   /* 22 --Reset: AMR2 */
		BYTE	DATA3;   /* 23 --Reset: AMR3 */
		BYTE	DATA4;   /* 24 */
		BYTE	DATA5;   /* 25 */
		BYTE	DATA6;   /* 26 */
		BYTE	DATA7;   /* 27 */
		BYTE	DATA8;   /* 28 */
		BYTE	RMC;     /* 29 */
		BYTE	RMSA;    /* 30 */
		BYTE	CDR;     /* 31 */
		BYTE	RXFIFO[64];/* 32-95 */
		BYTE    RVS[32];   /* 96-127 */
} CanBuf;


/*===============  functions declaration ==========================*/
/* set can card hardware setting , if successful return 0, else return err code*/
/*----------------------------------------------------------------------
input:
   segment :set memory decode segment value, 0xC000 to 0xDF00
   irq1    : can port#1 irq number
   irq2    : can port#2 irq number
----------------------------------------------------------------------*/
int canInitHW( BYTE port);

/* initiallize and configure can controller */
/*if success return(0) else return error code*/
int  canConfig( BYTE port, Can_Struct can);

/* reset can controller */
/*if success return(0) else return error code*/
int  canNormalRun( BYTE port ) ;

/* reset can controller */
void can_reset( BYTE port );

/* send a message */
/*if success return(1) else return(0)*/
int  canSendMsg( BYTE  port, Msg_Struct *send_msg);

/* receive a message */
/*if valid msg return(1) else return(0)*/
int  canReceiveMsg( BYTE port, Msg_Struct *msg_ptr);

int receive_frag(BYTE	port);
/*--------------------------------------------*/
     
/*--------------------- for pck trn and rec --------*/               
typedef struct 
{
        BYTE status;
        BYTE timer;
        BYTE destination_node;
        BYTE pad;     
		US   index;
		US   pad1;
        US	 frag_count;   /*if status=sending, frag_count=next frame to send*/
        US   frag_tot;     /* tot frames =from 0 to frag_num*/
        Msg_Struct  smsg_pck[DlBufCnt];	/*must >=42 */
}SEND_BUF_STRUCT;

typedef struct 
{
        BYTE status;
        BYTE timer;
        BYTE port;		/*added 98-1-6, used in display */
        BYTE target_node;	/*added 98-1-6, used in display */
        BYTE serial;       
		BYTE pad1;
        US frag_count;
        US msg_size;
        BYTE source_node;
		BYTE pad2;
        BYTE data[BUF_LEN];
}Receive_Buf_Struc ;

/* Send message structure */
typedef struct 
{
	US		msg_size;
	BYTE	destination_node;
	BYTE	pad;
	BYTE	data[BUF_LEN];
} Send_Msg_Struct;

/* Recv message structure */
typedef struct 
{
	US		msg_size;
	BYTE	source_node;
	BYTE	pad;
	BYTE	data[BUF_LEN];
} Receive_Msg_Struct;

/* ----------------- Send roution------------------------ 
	Return	 0 --- Ok
            -1 --- error
--------------------------------------------------*/            
/* send message buffer fragment handle status */
#define nul 			         0x00
#define completed		         0x80
#define overtime		         0x40    
#define timer_on		         0x20
#define assembling		         0x10


int send_request( BYTE Port, Send_Msg_Struct * send_appl_ptr, SEND_BUF_STRUCT *pSendBuf);
                                                         
int CANInit(int port);

int CANAppWrite(int port, Send_Msg_Struct *pSendMsg);


int CANAppRead(
	int port,						/* CAN1/CAN2: PORT1/PORT2 */
	unsigned long flags,			/* Q_NOWAIT / Q_WAIT */
	int		 timeout,				/* when flags is Q_WAIT, timeout is valid */
	Receive_Msg_Struct  *msgbuf,	/* message buffer */
	unsigned long *msg_len			/* length of message */
	);

typedef void (* CANAppWrite_cb)(void *param);
void set_CANAppWrite_cb(void *func,void *arg);
#endif
