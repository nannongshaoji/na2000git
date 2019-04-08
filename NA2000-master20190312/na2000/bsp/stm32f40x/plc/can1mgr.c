/*  ----------------------------------------------------------------------
                                CPU Software 
                      
              Version   :  1.0                                                     
              Filename  :  can1mgr.c
   Originally Written By:  Chen Sining                         
	   	   Date     	:  2015.1.30
      Application Notes :  This file defines basic macro definations, and may
                           be used by both Config.Software and CPU target
                           application. So, in case of updating, care must
                           be taken that all indivisuals should be informed
                           to conform with the changes. 
        Updating Record :  ( Pls. Specify : Who , Where , When, Why , What )

   -----------------------------------------------------------------------*/

#include "sysapp.h"
#include "candrv.h"
#include "sys1ms.h"
#include "cpld.h"

//#ifdef NA400
//#define		AOM401_0401			121
//#endif

#define		MAX_ACK_TIME			1
#define		MAX_LETTER_TIME		2
#define		MAX_WAIT_TIME			1
#define		MAX_ERR_R			6
#define		MAX_ERR_S			3

static CLOCK ClkLast,ClkCopy,ClkNow;

static void rSetTime( Send_Msg_Struct *pSnd , unsigned char addr );
static void rDownload( Send_Msg_Struct *pSnd, unsigned char addr );
static void rIOPoll( Send_Msg_Struct *pSnd, unsigned char addr );
static unsigned long rIOOutput( Send_Msg_Struct *pSnd, unsigned char addr );
static void rClearIOOutput(unsigned char addr);
//static void rCAN1Heartbeat( Send_Msg_Struct *pSnd, unsigned char addr );

static unsigned long IsIOBoard( unsigned char addr);
static void IOStateAlarm(unsigned char addr, unsigned char alarmstate, unsigned char newstate);
static unsigned long rProcessMsg( Receive_Msg_Struct *pRev, unsigned char AckCode, Send_Msg_Struct *pSnd );

unsigned char SlowNumPerScan;  /* slow module number per scantime */
extern unsigned char can1_slow_io;
extern struct rt_completion can1,can1ok;
extern void set_mscnt(int ms);

extern char i2c_write_addr(unsigned char can_addr);
extern char set_submodule_addr_flag(char);
extern char get_submodule_addr_flag(void);

#define PORT1 1

/**** CAN1 NA-A Protocol Code   **************/
/* Down */
#define  CODE_IOLOAD			1
#define  CODE_IOPOLL			2
#define  CODE_DOSET				3
#define  CODE_TIMESET			4
#define  CODE_AOSET				5
#define  CODE_HEARTBEAT			11
#define	 CODE_READ				21
#define	 CODE_WRITE				22
/* Up   */
#define  CODE_DIVAL				31
#define  CODE_SOE				32
#define  CODE_AIVAL				33
#define  CODE_DOVAL				34
#define  CODE_ACK				35
#define  CODE_MIXVAL			36
#define  CODE_AOVAL				37

#define MST_PEERON1				114
#define MST_PEERMST1			115
#define MST_SELFON1				106

void sync_time()
{
	Send_Msg_Struct  	sndbuf;
	rSetTime(&sndbuf,can_board_addr);
	CANAppWrite(PORT1,&sndbuf);
}

/* Detemine whether the board is IO */
static unsigned long IsIOBoard( unsigned char addr)
{
	if(	   (addr<=pConfigData->CAN1MduSum)
		&& (pConfigData->pIOMDU[addr-1].IsDefined)  
		&& (   (pConfigData->pIOMDU[addr-1].ModuleType==TYPE_DIM)
		     ||(pConfigData->pIOMDU[addr-1].ModuleType==TYPE_DOM)
		     ||(pConfigData->pIOMDU[addr-1].ModuleType==TYPE_AIM)  
			 ||(pConfigData->pIOMDU[addr-1].ModuleType==TYPE_FAIM)  
			 ||(pConfigData->pIOMDU[addr-1].ModuleType==TYPE_AOM)
			 ||(pConfigData->pIOMDU[addr-1].ModuleType==TYPE_HCM)
			 ||(pConfigData->pIOMDU[addr-1].ModuleType==TYPE_SPM)
			 )   )
		return 1;

	else return 0;
}

///* Set slave to master */
//static void rCAN1Heartbeat( Send_Msg_Struct *pSnd, unsigned char addr )
//{
//	unsigned char *SendBuf= pSnd->data;
//    SendBuf[0]=addr;
//    SendBuf[1]=CODE_HEARTBEAT;
//    SendBuf[2]=Get_S(MST_M1);
//    SendBuf[3]=Get_S(MST_SELFON1);
//    SendBuf[4]=pDynamicData->pS.State[(MST_M1-1)/8 + (pStaticData->DualNo-1)*3];
//    SendBuf[5]=pDynamicData->pS.State[(MST_M1-1)/8 + (pStaticData->DualNo-1)*3 +1];
//    SendBuf[6]=pDynamicData->pS.State[(MST_M1-1)/8 + (pStaticData->DualNo-1)*3 +2];
//    SendBuf[7]=0;
//    
//    pSnd->msg_size 			= 8;
//    pSnd->destination_node 	= SendBuf[0];
//}

/* Set Time of IO board */
static void rSetTime( Send_Msg_Struct *pSnd, unsigned char addr )
{
	unsigned char *SendBuf= pSnd->data;
	SendBuf[0]=addr;
	SendBuf[1]=CODE_TIMESET;
	SendBuf[2]=pCCUClock->Day;
	SendBuf[3]=pCCUClock->Hour;
	SendBuf[4]=pCCUClock->Minute;
	SendBuf[5]=pCCUClock->Second;
	SendBuf[6] = (pCCUClock->Ms & 0xFF);
	SendBuf[7] = (pCCUClock->Ms>>8) & 0xFF;

	pSnd->msg_size 			= 8;
	pSnd->destination_node 	= SendBuf[0];
}

/* Download the parameters */
static void rDownload( Send_Msg_Struct *pSnd, unsigned char addr )
{
	unsigned char *SendBuf= pSnd->data;
	unsigned char i;
	
	memset( pSnd, 0, sizeof(Send_Msg_Struct) );
	
	switch(pConfigData->pIOMDU[addr-1].ModuleType)
	{
		case TYPE_DIM:
		    SendBuf[0]=addr;
			SendBuf[1]=CODE_IOLOAD;
			SendBuf[2]=pStaticData->pMDU[addr-1].ModuleNo;
			for(i=0;i<pConfigData->pIOMDU[addr-1].IOSum;i++)
			{
				memcpy( &SendBuf[3+i*2], &pStaticData->pI[ pConfigData->pIOMDU[addr-1].IOStart+i-1 ].FiltTime, 2);
			}
			pSnd->msg_size 			= 3+32*2;
			pSnd->destination_node 	= SendBuf[0];
			break;

		case TYPE_AIM:
		case TYPE_FAIM:
		    SendBuf[0]=addr;
			SendBuf[1]=CODE_IOLOAD;
			SendBuf[2]=pStaticData->pMDU[addr-1].ModuleNo;
			for(i=0;i<pConfigData->pIOMDU[addr-1].IOSum;i++)
			{
				memcpy( &SendBuf[3+i], &pStaticData->pAI[ pConfigData->pIOMDU[addr-1].IOStart+i-1 ].SignalType, 1);
			}
			if (pConfigData->pIOMDU[addr-1].IOSum<16){
				//SendBuf[18]=pStaticData->pMDU[addr-1].EthAddr2;
			}
			pSnd->msg_size 			= 3+16;
			pSnd->destination_node 	= SendBuf[0];
			break;

		case TYPE_AOM:
			SendBuf[0]=addr;
			SendBuf[1]=CODE_IOLOAD;
			SendBuf[2]=pStaticData->pMDU[addr-1].ModuleNo;
			SendBuf[3]=0xaa;
			for(i=0;i<pConfigData->pIOMDU[addr-1].IOSum;i++)
			{
				memcpy( &SendBuf[4+2*i], &pStaticData->pAQ[ pConfigData->pIOMDU[addr-1].IOStart+i-1 ].FaultOutput, 2);
			}
			pSnd->msg_size 			= 4+2*pConfigData->pIOMDU[addr-1].IOSum;
        #ifdef NA400
            if (pStaticData->pMDU[addr-1].ModuleNo!=AOM401_0401)
        #endif
            {
                for(i=0;i<pConfigData->pIOMDU[addr-1].IOSum;i++)
                SendBuf[pSnd->msg_size+i]=pStaticData->pAQ[ pConfigData->pIOMDU[addr-1].IOStart+i-1 ].SignalType;
                pSnd->msg_size 			= 4+2*pConfigData->pIOMDU[addr-1].IOSum+pConfigData->pIOMDU[addr-1].IOSum;
            }
            pSnd->destination_node 	= SendBuf[0];
			break;

		case TYPE_DOM:
			SendBuf[0]=addr;
			SendBuf[1]=CODE_IOLOAD;
			SendBuf[2]=pStaticData->pMDU[addr-1].ModuleNo;
			for(i=0;i<pConfigData->pIOMDU[addr-1].IOSum;i++)
				SendBuf[3+i]=pStaticData->pQ[ pConfigData->pIOMDU[addr-1].IOStart+i-1 ].FaultOutput;	/* 故障输出值，0xff - 保持， 0 - 清0，1 - 置1 */
			pSnd->msg_size 			= 3+pConfigData->pIOMDU[addr-1].IOSum;
			pSnd->destination_node 	= SendBuf[0];
			break;

		case TYPE_HCM:
		case TYPE_SPM:
		    SendBuf[0]=addr;
			SendBuf[1]=CODE_IOLOAD;
			SendBuf[2]=pStaticData->pMDU[addr-1].ModuleNo;
			pSnd->msg_size 			= 8;
			pSnd->destination_node 	= SendBuf[0];
			break;

	}
}

/* polling IO data */ 
static void rIOPoll( Send_Msg_Struct *pSnd, unsigned char addr )
{
	unsigned char *SendBuf= pSnd->data;
	
	memset( pSnd, 0, sizeof(Send_Msg_Struct) );

	SendBuf[0]=addr;
	SendBuf[1]=CODE_IOPOLL;
	SendBuf[2]=pStaticData->pMDU[addr-1].ModuleNo;
	SendBuf[3]=0;
	SendBuf[4]=0;
	SendBuf[5]=0;
	SendBuf[6]=0;
	SendBuf[7]=0;
	pSnd->msg_size 			= 8;
	pSnd->destination_node 	= SendBuf[0];
}


/* Ack IO data */ 
static void rAck( Send_Msg_Struct *pSnd, unsigned char addr, unsigned char srcCode, unsigned short *serialNo)
{
	unsigned char *SendBuf= pSnd->data;
	
	SendBuf[0]=addr;
	SendBuf[1]=CODE_ACK;
	SendBuf[2]=srcCode;
	memcpy( SendBuf+3, serialNo, 2 );
	SendBuf[5]=0;
	pSnd->msg_size 			= 8;
	pSnd->destination_node 	= SendBuf[0];
}

/* Download the parameters */
static unsigned long rIOOutput( Send_Msg_Struct *pSnd, unsigned char addr )
{
	unsigned char *SendBuf= pSnd->data;
	unsigned char i;
	unsigned short occno,no,bias,tmp;
	char buf[128];
	
	memset( pSnd, 0, sizeof(Send_Msg_Struct) );
	
	switch(pConfigData->pIOMDU[addr-1].ModuleType)
	{
		case TYPE_DIM:
		case TYPE_AIM:
		case TYPE_FAIM:
			return 1;

		case TYPE_DOM:
		  SendBuf[0]=addr;
			SendBuf[1]=CODE_DOSET;
			SendBuf[2]=pStaticData->pMDU[addr-1].ModuleNo;
			SendBuf[3]=0;
			for(i=0;i<pConfigData->pIOMDU[addr-1].IOSum;i++)
			{
				occno= pConfigData->pIOMDU[addr-1].IOStart+i;
		        no= (occno-1)/8; 
				bias=(occno-1)%8;
				
				if(		(pDynamicData->pQ.OutputEna[no] & (1<<bias))
					&&	(	((pDynamicData->pQ.Output[no]^pDynamicData->pQ.RawState[no]) & (1<<bias))
						 || pDynamicData->pQ.PulseTime[occno-1] )	)
				{
					SendBuf[4+3*SendBuf[3]]= (( !!(pDynamicData->pQ.Output[no]&(1<<bias)) )<<7) | (i+1);
					if(pDynamicData->pQ.PulseTime[occno-1]==0)
					{
						SendBuf[5+3*SendBuf[3]]=0xff;
						SendBuf[6+3*SendBuf[3]]=0xff;
					}
					else
					{
						tmp=(pDynamicData->pQ.PulseTime[occno-1]+9)/10;
						memcpy(&SendBuf[5+3*SendBuf[3]],&tmp,2);
					}
			        pDynamicData->pQ.OutputEna[no] = pDynamicData->pQ.OutputEna[no] & (~(1<<bias)); 
					/*sprintf(buf,"DO-1 Occno=%02d Val=%d Pulse=%04x\n", SendBuf[4+3*SendBuf[3]]&0x7f, SendBuf[4+3*SendBuf[3]]>>7, tmp );
					Printff(buf);*/
					SendBuf[3]++;
				}
				else if(	Get_Q_CheckEna(occno) 
						&& ( (pDynamicData->pQ.Output[no]^pDynamicData->pQ.RawState[no]) & (1<<bias) )   )
				{
					SendBuf[4+3*SendBuf[3]]= (( !!(pDynamicData->pQ.Output[no]&(1<<bias)) )<<7) | (i+1);
					if(pDynamicData->pQ.PulseTime[occno-1]==0)
					{
						SendBuf[5+3*SendBuf[3]]=0xff;
						SendBuf[6+3*SendBuf[3]]=0xff;
					}
					else
					{
						tmp=(pDynamicData->pQ.PulseTime[occno-1]+9)/10;
						memcpy(&SendBuf[5+3*SendBuf[3]],&tmp,2);
					}
			        pDynamicData->pQ.OutputEna[no] = pDynamicData->pQ.OutputEna[no] & (~(1<<bias)); 
					sprintf(buf,"DO-%d Occno=%02d Val=%d Pulse=%04x\n", addr,SendBuf[4+3*SendBuf[3]]&0x7f, SendBuf[4+3*SendBuf[3]]>>7, tmp );
					Printff(buf);
					SendBuf[3]++;
				}
				else
					pDynamicData->pQ.OutputEna[no] = pDynamicData->pQ.OutputEna[no] & (~(1<<bias)); 
			}

			pSnd->msg_size 			= 4+SendBuf[3]*3;
			pSnd->destination_node 	= SendBuf[0];
			if(SendBuf[3])
				return 0;
			break;

		case TYPE_SPM:
		  SendBuf[0]=addr;
			SendBuf[1]=CODE_DOSET;
			SendBuf[2]=pStaticData->pMDU[addr-1].ModuleNo;
			SendBuf[3]=0;
			for(i=0;i<pStaticData->pMDU[addr-1].QNum;i++)
			{
				occno= pStaticData->pMDU[addr-1].QRefAddr+i;
		        no= (occno-1)/8; 
				bias=(occno-1)%8;
				
				if ((pDynamicData->pQ.OutputEna[no] & (1<<bias)) && (pDynamicData->pQ.Output[no] & (1<<bias)))
				{
					SendBuf[4+3*SendBuf[3]]= (( !!(pDynamicData->pQ.Output[no]&(1<<bias)) )<<7) | (i+1);
					SendBuf[5+3*SendBuf[3]]=0xff;
					SendBuf[6+3*SendBuf[3]]=0xff;
					SendBuf[3]++;
				}
				pDynamicData->pQ.OutputEna[no] = pDynamicData->pQ.OutputEna[no] & (~(1<<bias)); 
				pDynamicData->pQ.State[no] = pDynamicData->pQ.State[no] & (~(1<<bias));
			}

			pSnd->msg_size 			= 4+SendBuf[3]*3;
			pSnd->destination_node 	= SendBuf[0];
			if(SendBuf[3])
				return 0;
			break;

		case TYPE_AOM:
		  SendBuf[0]=addr;
			SendBuf[1]=CODE_AOSET;
			SendBuf[2]=pStaticData->pMDU[addr-1].ModuleNo;
			SendBuf[3]=0;

			for(i=0;i<pStaticData->pMDU[addr-1].AQNum;i++)
			{
				occno= pStaticData->pMDU[addr-1].AQRefAddr+i;
		        no= (occno-1)/8; 
				bias=(occno-1)%8;
				
				if ( (pDynamicData->pAQ.OutputEna[no] & (1<<bias)) && (pDynamicData->pAQ.RawData[occno-1]!=pDynamicData->pAQ.Output[occno-1]) )
				{
					SendBuf[4+3*SendBuf[3]]= (i+1);
					memcpy(&SendBuf[5+3*SendBuf[3]], &pDynamicData->pAQ.Output[occno-1], 2);
					/*pDynamicData->pAQ.Value[occno-1]=pDynamicData->pAQ.Output[occno-1];*/
					SendBuf[3]++;
				}
				pDynamicData->pAQ.OutputEna[no] = pDynamicData->pAQ.OutputEna[no] & (~(1<<bias)); 
			}

			pSnd->msg_size 			= 4+SendBuf[3]*3;
			pSnd->destination_node 	= SendBuf[0];
			if(SendBuf[3])
				return 0;
			break;

	}
	return 1;
}

static void rClearIOOutput(unsigned char addr)
{
	unsigned char i;
	unsigned short occno,no,bias;
	switch(pConfigData->pIOMDU[addr-1].ModuleType)
	{
		case TYPE_DOM:
			for(i=0;i<pConfigData->pIOMDU[addr-1].IOSum;i++)
			{
				occno= pConfigData->pIOMDU[addr-1].IOStart+i;
		        no= (occno-1)/8; 
				bias=(occno-1)%8;
				
				pDynamicData->pQ.OutputEna[no] = pDynamicData->pQ.OutputEna[no] & (~(1<<bias)); 
			}
			break;

		case TYPE_AOM:
			for(i=0;i<pStaticData->pMDU[addr-1].AQNum;i++)
			{
				occno= pStaticData->pMDU[addr-1].AQRefAddr+i;
		        no= (occno-1)/8; 
				bias=(occno-1)%8;
				
				pDynamicData->pAQ.OutputEna[no] = pDynamicData->pAQ.OutputEna[no] & (~(1<<bias)); 
			}
			break;

		case TYPE_SPM:
			for(i=0;i<pStaticData->pMDU[addr-1].QNum;i++)
			{
				occno= pStaticData->pMDU[addr-1].QRefAddr+i;
		        no= (occno-1)/8; 
				bias=(occno-1)%8;
				
				pDynamicData->pQ.OutputEna[no] = pDynamicData->pQ.OutputEna[no] & (~(1<<bias)); 
				pDynamicData->pQ.State[no] = pDynamicData->pQ.State[no] & (~(1<<bias));
			}
			break;

	}
	return ;
}

static void IOStateAlarm(unsigned char addr, unsigned char alarmstate, unsigned char newstate)
{
	char ls[128];
	if( alarmstate & STATE_COMMERR )
	{
		if( newstate & STATE_COMMERR )
			sprintf( ls, "Module Addr %xH communition error\n", addr );
		else
			sprintf( ls, "Module Addr %xH communition recovery\n", addr );
		Printff( ls );
		/*if (pStaticData->pMDU[addr-1].ModuleNo==TIM_212)
		{
			for (i=0;i<pStaticData->pMDU[addr-1].AINum;i++)
				pDynamicData->pAI.RawData[(pStaticData->pMDU[addr-1].AIRefAddr-1+i)]=32767;
			Drv_Multi_AI(pStaticData->pMDU[addr-1].AIRefAddr, pStaticData->pMDU[addr-1].AINum);
		}???*/
	}
	if( alarmstate & STATE_DIAGERR )
	{
		if( newstate & STATE_DIAGERR )
			sprintf( ls, "Module Addr %xH diagnose error\n", addr );
		else
			sprintf( ls, "Module Addr %xH diagnose Ok\n", addr );
		Printff( ls );
	}
	if( alarmstate & STATE_TYPEERR )
	{
		if( newstate & STATE_TYPEERR )
			sprintf( ls, "Module Addr %xH type dismatch\n", addr );
		else
			sprintf( ls, "Module Addr %xH type Ok\n", addr );
		Printff( ls );
	}
	if( alarmstate & STATE_IOLOAD )
	{
		if( newstate & STATE_IOLOAD )
			sprintf( ls, "Module Addr %xH download parameter finished\n", addr );
		else
			sprintf( ls, "Module Addr %xH require to download parameter\n", addr );
		Printff( ls );
	}
}



/*Process receive message*/
static unsigned long rProcessMsg( Receive_Msg_Struct *pRev, unsigned char AckCode, Send_Msg_Struct *pSnd )
{
	unsigned char index,len; 
	unsigned char *ps, cTmp, cVar;
	unsigned char c1;
	unsigned char tmpss[32], state, state1, writeIO;
	unsigned short sTmp;
	unsigned int   dst_bit_start;
	EVT1_INFO evt1;

//	if (pRev->data[1]==CODE_HEARTBEAT)
//	{
//		can_err_mark=0; 
//		can_heartbeat_ok=1;
////		pStaticData->peer_state=1;
////		pStaticData->red_err_mark=0;

//		Output_S( MST_PEERON1, pRev->data[3]);
//		Output_S( MST_PEERMST1, pRev->data[2] );

//		pDynamicData->pS.State[(MST_M1-1)/8 + (!(pStaticData->DualNo-1))*3]=pRev->data[4];
//		pDynamicData->pS.State[(MST_M1-1)/8 + (!(pStaticData->DualNo-1))*3 +1]=pRev->data[5];
//		pDynamicData->pS.State[(MST_M1-1)/8 + (!(pStaticData->DualNo-1))*3 +2]=pRev->data[6];
//		return 0;
//	}

	if (pRev->data[1]==CODE_TIMESET)
	{
		if( !Get_S(MST_M1) )
		{
			memcpy(&sTmp,&pRev->data[6],2);
			set_mscnt(sTmp);
		}
		return 0;
	}

	if( !Get_S(MST_M1) )  return 0;

	if( !IsIOBoard(pRev->data[0]) ) 
	{
		Printff("Bus1 Error Ctrl Msg.\n");
		//LOG4C((LOG_ERROR, "errno:%02x",CAN1_BUS_CTRL_MSG_ERR));
	 	return 1;
	}
	index= pRev->data[0];
	if(pSnd) pSnd->msg_size=0;
	else pSnd=(Send_Msg_Struct *)tmpss;
	memset(pSnd,0,32);

	ps=	&pDynamicData->pS.State[pConfigData->pIOMDU[index-1].S_Addr-1];
	pConfigData->pIOMDU[index-1].ErrCnt_R=0;
	pConfigData->pIOMDU[index-1].ErrCnt_S=0;
	pRev->data[2] &= (~STATE_COMMERR);

	state=  pDynamicData->pS.State[pConfigData->pIOMDU[index-1].S_Addr-1];
	state1= pRev->data[2];
 	if(    (state&STATE_IOLOAD) 
		&& (state1&STATE_IOLOAD) 
 		&&((state & (STATE_TYPEERR))==0) 
		&&((state1& (STATE_TYPEERR))==0)
		&&((state1&STATE_NOTREADY)==0) )  writeIO=1;
	else writeIO=0;
	
	switch( pRev->data[1] )
	{
		case CODE_DIVAL:
			if(writeIO)
			{
				Move_Fuc( &pDynamicData->pI.RawData[0], (unsigned int)pConfigData->pIOMDU[index-1].IOStart, 
						  &pRev->data[3], 1, 
						  (unsigned int)pConfigData->pIOMDU[index-1].IOSum	);
				Drv_Multi_I(pConfigData->pIOMDU[index-1].IOStart, pConfigData->pIOMDU[index-1].IOSum);
			}
			if( !(pRev->data[2] & STATE_IOLOAD) )
					cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV))) & (~STATE_IOLOAD);
			else	cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV)));
			cVar= cTmp^ps[0];
			ps[0]=cTmp;
			if(cVar) IOStateAlarm( pRev->data[0], cVar, cTmp);
			break;

		case CODE_SOE:
			if(writeIO)
			{
				Move_Fuc( &pDynamicData->pI.RawData[0], (unsigned int)pConfigData->pIOMDU[index-1].IOStart, 
						  &pRev->data[3], 1, 
						  (unsigned int)pConfigData->pIOMDU[index-1].IOSum	);
				Drv_Multi_I(pConfigData->pIOMDU[index-1].IOStart, pConfigData->pIOMDU[index-1].IOSum);
			}
			if( !(pRev->data[2] & STATE_IOLOAD) )
					cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV))) & (~STATE_IOLOAD);
			else	cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV)));
			cVar= cTmp^ps[0];
			ps[0]=cTmp;
			if(cVar) IOStateAlarm( pRev->data[0], cVar, cTmp);

			/* determine whether to send Ack*/
			if(pRev->data[7] && (pRev->msg_size==(10+pRev->data[7]*6)) )
			{
				memcpy(&sTmp,&pRev->data[8],2);
				if(writeIO && (pConfigData->pIOMDU[pRev->data[0]-1].Serial!=sTmp))
				{
					for(c1=0;c1<pRev->data[7];c1++)
					{
						memset(&evt1,0,sizeof(evt1));
						evt1.Code	=1;
						evt1.Hour	=pRev->data[11+c1*6];
						evt1.Minute	=pRev->data[12+c1*6];
						evt1.Second	=pRev->data[13+c1*6];
						evt1.EvtCode=(!!(pRev->data[10+c1*6]&0x80));
						evt1.Ms	    =pRev->data[15+c1*6];
						evt1.Ms	    =(evt1.Ms<<8) | pRev->data[14+c1*6];
						SysTimerRead(&ClkNow);
						if(evt1.Hour==23 && ClkNow.Hour==0)
						{
							evt1.Year	=ClkLast.Year-2000;
							evt1.Month	=ClkLast.Month;
							evt1.Day	=ClkLast.Day;
						}
						else
						{
							evt1.Year	=ClkNow.Year-2000;
							evt1.Month	=ClkNow.Month;
							evt1.Day	=ClkNow.Day;
						}
						evt1.OccNo  =pConfigData->pIOMDU[index-1].IOStart + (pRev->data[10+c1*6]&0x7f) -1;
						evt1.DataL  =(!!(pRev->data[10+c1*6]&0x80));
						PutEvt( 1, (char*)&evt1, EVENT1_LEN/*in words*/);
					}
					pConfigData->pIOMDU[pRev->data[0]-1].Serial=sTmp;
				}
				/*send Ack*/
				rAck( pSnd, pRev->data[0],  pRev->data[1], &sTmp);
				CANAppWrite(PORT1, pSnd);
				/*sprintf(tmpstr,"Send SOE ack %d\n", *((unsigned short *)&pRev->data[8]));
				Printff(tmpstr);*/
			}
			break;

		case CODE_DOVAL:
			if(writeIO)
			{
				Drv_Multi_Q(pConfigData->pIOMDU[index-1].IOStart, pConfigData->pIOMDU[index-1].IOSum, &pRev->data[3]);
			}

			if( !(pRev->data[2] & STATE_IOLOAD) )
					cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV))) & (~STATE_IOLOAD);
			else	cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV)));
			cVar= cTmp^ps[0];
			ps[0]=cTmp;
			if(cVar) IOStateAlarm( pRev->data[0], cVar, cTmp);
			break;

		case CODE_AOVAL:
			if(writeIO)
			{
				memcpy( & pDynamicData->pAQ.Value[(pConfigData->pIOMDU[index-1].IOStart-1)],
						& pRev->data[3], 
						pConfigData->pIOMDU[index-1].IOSum*2	);
				memcpy( & pDynamicData->pAQ.RawData[(pConfigData->pIOMDU[index-1].IOStart-1)],
						& pRev->data[3], 
						pConfigData->pIOMDU[index-1].IOSum*2	);
			}
			if( !(pRev->data[2] & STATE_IOLOAD) )
					cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV))) & (~STATE_IOLOAD);
			else	cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV)));
			cVar= cTmp^ps[0];
			ps[0]=cTmp;
			if(cVar) IOStateAlarm( pRev->data[0], cVar, cTmp);
			break;

		case CODE_AIVAL:
			if(writeIO)
			{
				memcpy( & pDynamicData->pAI.RawData[(pConfigData->pIOMDU[index-1].IOStart-1)],
						& pRev->data[3], 
						pConfigData->pIOMDU[index-1].IOSum*2	);
				Drv_Multi_AI(pConfigData->pIOMDU[index-1].IOStart, pConfigData->pIOMDU[index-1].IOSum);
			}

			if( !(pRev->data[2] & STATE_IOLOAD) )
					cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV))) & (~STATE_IOLOAD);
			else	cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV)));
			cVar= cTmp^ps[0];
			ps[0]=cTmp;
			if(cVar) IOStateAlarm( pRev->data[0], cVar, cTmp);
			break;

		case CODE_MIXVAL:
			if(writeIO)
			{
				if (pRev->data[3]!=pStaticData->pMDU[index-1].INum)  return 3;
				Move_Fuc( &pDynamicData->pI.RawData[0], (unsigned int)pStaticData->pMDU[index-1].IRefAddr, 
						  &pRev->data[4], 1, 
						  (unsigned int)pStaticData->pMDU[index-1].INum	);
				Drv_Multi_I(pStaticData->pMDU[index-1].IRefAddr, pStaticData->pMDU[index-1].INum);
				if (pRev->data[4+(pStaticData->pMDU[index-1].INum+7)/8]!=pStaticData->pMDU[index-1].QNum)  return 3;
				Drv_Multi_Q(pStaticData->pMDU[index-1].QRefAddr, pStaticData->pMDU[index-1].QNum, &pRev->data[5+(pStaticData->pMDU[index-1].INum+7)/8]);
				if (pRev->data[5+(pStaticData->pMDU[index-1].INum+7)/8+(pStaticData->pMDU[index-1].QNum+7)/8]!=pStaticData->pMDU[index-1].AINum)  return 3;
				memcpy( & pDynamicData->pAI.RawData[(pStaticData->pMDU[index-1].AIRefAddr-1)],
						& pRev->data[6+(pStaticData->pMDU[index-1].INum+7)/8+(pStaticData->pMDU[index-1].QNum+7)/8], 
						pStaticData->pMDU[index-1].AINum*2	);
				Drv_Multi_AI(pStaticData->pMDU[index-1].AIRefAddr, pStaticData->pMDU[index-1].AINum);
			}

			if( !(pRev->data[2] & STATE_IOLOAD) )
					cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV))) & (~STATE_IOLOAD);
			else	cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV)));
			cVar= cTmp^ps[0];
			ps[0]=cTmp;
			if(cVar) IOStateAlarm( pRev->data[0], cVar, cTmp);

			len=6+(pStaticData->pMDU[index-1].INum+7)/8+(pStaticData->pMDU[index-1].QNum+7)/8+pStaticData->pMDU[index-1].AINum*2;
			if (pRev->data[len] && (pRev->msg_size==(len+1+pRev->data[len]*8)))
			{
				if(writeIO)
				{
					for(c1=0;c1<pRev->data[len];c1++)
					{
						memset(&evt1,0,sizeof(evt1));
						evt1.Code	=1;
						evt1.Hour	=pRev->data[len+4+c1*8];
						evt1.Minute	=pRev->data[len+5+c1*8];
						evt1.Second	=pRev->data[len+6+c1*8];
						evt1.EvtCode=(!!(pRev->data[len+3+c1*8]));
						evt1.Ms	    =pRev->data[len+8+c1*8];
						evt1.Ms	    =(evt1.Ms<<8) | pRev->data[len+7+c1*8];
						SysTimerRead(&ClkNow);
						if(evt1.Hour==23 && ClkNow.Hour==0)
						{
							evt1.Year	=ClkLast.Year-2000;
							evt1.Month	=ClkLast.Month;
							evt1.Day	=ClkLast.Day;
						}
						else
						{
							evt1.Year	=ClkNow.Year-2000;
							evt1.Month	=ClkNow.Month;
							evt1.Day	=ClkNow.Day;
						}
						evt1.OccNo  =pStaticData->pMDU[index-1].IRefAddr + (pRev->data[len+1+c1*8]) -1;
						evt1.DataL  =(!!(pRev->data[len+3+c1*8]));
						PutEvt( 1, (char*)&evt1, EVENT1_LEN/*in words*/);
					}
				}
			}
			break;

		case CODE_READ:
			if(writeIO)
			{
				if (pRev->msg_size!=(pRev->data[4]*2+10))  return 4;

				memcpy(&dst_bit_start,&pRev->data[6],4);
				memcpy((unsigned char *)dst_bit_start,&pRev->data[10],pRev->data[4]*2);
			}

			if( !(pRev->data[2] & STATE_IOLOAD) )
					cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV))) & (~STATE_IOLOAD);
			else	cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV)));
			cVar= cTmp^ps[0];
			ps[0]=cTmp;
			if(cVar) IOStateAlarm( pRev->data[0], cVar, cTmp);
			AckCode=0;
			break;

		case CODE_ACK:
			switch(pRev->data[3])
			{
				case CODE_IOLOAD:
					if(!pRev->data[4])
						cTmp= (pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV)) | STATE_IOLOAD;
					else
						cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV))) & (~STATE_IOLOAD);
					break;
				case CODE_DOSET:
				case CODE_AOSET:
				case CODE_WRITE:
				default:
					if( !(pRev->data[2] & STATE_IOLOAD) )
							cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV))) & (~STATE_IOLOAD);
					else	cTmp= ((pRev->data[2]&STATE_DRV) | (ps[0]&(~STATE_DRV)));
					break;
			}
								
			cVar= cTmp^ps[0];
			ps[0]=cTmp;
			if(cVar) IOStateAlarm( pRev->data[0], cVar, cTmp);
			if(AckCode)
			{
				if(pRev->data[3]==AckCode)
					return 0;/*pRev->data[4];*/
				else return 2;
			}
			else return 2;
	}

	if(AckCode) return 2;
	return 0;
}
// if return value < 0,then submodules states isn't right
static short check_submodule_state()
{
	short ret = 0;
	if(has_sub_modules())
	{
		if(!get_submodule_addr_flag())
		{
			if(i2c_write_addr(0x02) !=0)
			{
				set_submodule_addr_flag(0);
				ret = -1;
			}
			else
			{
				set_submodule_addr_flag(1);
			}
		}
	}
	else
	{
		set_submodule_addr_flag(0);
		return -1;
	}
	return ret;
}

void can1mgr(void *lp)
{
	char str[128];
	unsigned short i;
	unsigned long  rc,len;
	int  wait_time;
	unsigned short  ErrMduIndex=0;
	unsigned short  SlowMduIndex=0, SlowVisitMark=0, SlowEvent=0, MaxSlowMduIndex=0;
	unsigned char lastCode, mystate;
	static	Send_Msg_Struct  	sndbuf;
	static	Receive_Msg_Struct  rcvbuf;
	unsigned long rc1,rc2;
	
	rc = CANInit(PORT1);
	if (rc!=0)
	{
	 	Printff("Bus1 init failed\n");
		Output_S(MST_CAN11FLT, 1);
		return;
	}
//	set_CANAppWrite_cb(rt_sem_release,&can1ok);
	Printff("Bus1 init Successfully\n");

	{
		rt_uint16_t can1_slow_io=0;
		for( i=0;i<pConfigData->CAN1MduSum;i++)
		{
			if( pConfigData->pIOMDU[i].IsDefined
				&&  (pConfigData->pIOMDU[i].ModuleType==TYPE_AIM || pConfigData->pIOMDU[i].ModuleType==TYPE_AOM))
			{
				MaxSlowMduIndex=i+1;
				can1_slow_io++;
			}
		}

		SlowNumPerScan=(can1_slow_io+9)/10;
	}

	for( i=0;i<pConfigData->CAN1MduSum;i++)
 	if(     pConfigData->pIOMDU[i].IsDefined
		&&  (pConfigData->pIOMDU[i].ModuleType==TYPE_AIM || pConfigData->pIOMDU[i].ModuleType==TYPE_AOM || pConfigData->pIOMDU[i].ModuleType==TYPE_HCM || pConfigData->pIOMDU[i].ModuleType==TYPE_SPM))
		MaxSlowMduIndex=i+1;

	InitSoftTimerEx(CAN1_1_TIMER, CAN1_1_TIMER_VALUE, CAN1_1_TIMER_VALUE);
	InitSoftTimerEx(CAN1_2_TIMER, CAN1_2_TIMER_VALUE, 0);
	InitSoftTimerEx(CAN1_3_TIMER, CAN1_3_TIMER_VALUE, 0);

	/* Clear Receive Buffer */
	while( (rc=CANAppRead(
						PORT1, 			/* CAN1/CAN2: PORT1/PORT2 */
						Q_NOWAIT, 		/* Q_NOWAIT / Q_WAIT */
						0,		 		/* when flags is Q_WAIT, timeout is valid */
						(Receive_Msg_Struct  *)&rcvbuf,   /* message buffer */
						&len /* length of message */    ))==NOERR && len );

	rc = 0;

	SysTimerRead(&ClkLast);
	ClkCopy=ClkLast;
	while(1)
	{	
		if(check_submodule_state()<0)
		{
			Output_S(MST_CAN11FLT, 1);
			Output_S( IO_COMERR, 1 );
			for( i=0;i<pConfigData->CAN1MduSum;i++ )
			{
				if(pConfigData->pIOMDU[i].IsDefined)
				{
					pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] = STATE_COMMERR;
					IOStateAlarm(pConfigData->pIOMDU[i].MduAddr, STATE_COMMERR, STATE_COMMERR);
				}
			}
			rt_completion_done(&can1ok);
			rt_thread_delay(4);
			continue;
		}
		
		SysTimerRead(&ClkNow);
		if(ClkNow.Day!=ClkCopy.Day)
		{
			ClkLast=ClkCopy;
		}
		ClkCopy=ClkNow;

		mystate=0;
		for( i=0;i<pConfigData->CAN1MduSum;i++ )
		{
			if(pConfigData->pIOMDU[i].IsDefined)
			{
				mystate|= pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1];
			}
		}
		Output_S( IO_COMERR, !!(mystate & STATE_COMMERR) );
		Output_S( IO_DIAGERR,!!(mystate & STATE_DIAGERR) );
		Output_S( IO_CFGERR, !!(mystate & STATE_TYPEERR) );
		
		Output_S(MST_CAN11FLT, (mystate & STATE_COMMERR)); 
		
		if (Get_S(MST_M1))
		{	/* Set Time & Download */
			if(pConfigData->CAN1MduSum && Get_S(IO_DOWNLOAD)==0)
			{ /* This is the first time */
				/* Set Time */
				rSetTime(&sndbuf, can_board_addr);
				rc=CANAppWrite(PORT1, &sndbuf);
				
				if(rc!=NOERR)
				{     
					for( i=0;i<pConfigData->CAN1MduSum;i++ )
						if(pConfigData->pIOMDU[i].IsDefined)
						{
								pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] |= STATE_COMMERR;
						}
						
					sprintf(str,"1--[ %d ] CAN1 send message Addr=%xH, Code=%xH, Len=%d, error (%04xH)\n",rt_tick_get(),sndbuf.data[0],sndbuf.data[1],sndbuf.msg_size,(unsigned int)rc);
					Printff(str);
					continue;
				}
				
				//usleep(50000);//
				
				/* Download IO Board Parameter */
				for( i=0;i<pConfigData->CAN1MduSum;i++ )
				{
					if( (pConfigData->pIOMDU[i].IsDefined==0)
							|| (pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & STATE_IOLOAD) )
						continue;
					
					if(!IsIOBoard( pConfigData->pIOMDU[i].MduAddr ))
						continue;
					
					/* IO Board */
					while(pConfigData->pIOMDU[i].ErrCnt_S<MAX_ERR_S && pConfigData->pIOMDU[i].ErrCnt_R<MAX_ERR_R)
					{
						rDownload( &sndbuf, pConfigData->pIOMDU[i].MduAddr );
						pConfigData->pIOMDU[pConfigData->pIOMDU[i].MduAddr-1].Serial=0;
						rc=CANAppWrite(PORT1, &sndbuf);
						if(rc!=NOERR)
						{
							sprintf(str,"2--[ %d ] CAN1 send message Addr=%xH, Code=%xH, Len=%d, error (%04xH)\n",rt_tick_get(),sndbuf.data[0],sndbuf.data[1],sndbuf.msg_size,(unsigned int)rc);
							Printff(str);
							pConfigData->pIOMDU[i].ErrCnt_S++;
							if(pConfigData->pIOMDU[i].ErrCnt_S>=MAX_ERR_S)
							{
								pConfigData->pIOMDU[i].ErrCnt_S=MAX_ERR_S;
								if((pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & STATE_COMMERR) ==0)
								{
										IOStateAlarm(pConfigData->pIOMDU[i].MduAddr, STATE_COMMERR, STATE_COMMERR);
										pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] |= STATE_COMMERR;
								}
							}
							continue;
						}
						/* wait for response */
						else
						{
							while( (rc2=CANAppRead(
													PORT1, 			/* CAN1/CAN2: PORT1/PORT2 */
													Q_WAIT, 		/* Q_NOWAIT / Q_WAIT */
													MAX_ACK_TIME,	/* when flags is Q_WAIT, timeout is valid */
													(Receive_Msg_Struct  *)&rcvbuf,   /* message buffer */
													&len /* length of message */    ))==NOERR 
													&& (len==rcvbuf.msg_size+4) )
								{
									rc1=rProcessMsg(&rcvbuf,CODE_IOLOAD,0);
									if(rc1==0 && rcvbuf.data[0]==pConfigData->pIOMDU[i].MduAddr) 
										break;
								}
								if(rc2!=NOERR)
								{
									pConfigData->pIOMDU[i].ErrCnt_R++;
									if(pConfigData->pIOMDU[i].ErrCnt_R>=MAX_ERR_R)
									{
										pConfigData->pIOMDU[i].ErrCnt_R=MAX_ERR_R;
										if((pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & STATE_COMMERR)==0)
										{
												IOStateAlarm(pConfigData->pIOMDU[i].MduAddr, STATE_COMMERR, STATE_COMMERR);
												pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] |= STATE_COMMERR;
										}
									}
								}
								else 
									break;
						}/* else */
					}
				}/* for*/
				Output_S( IO_DOWNLOAD, 1 );
				rt_completion_done(&can1ok);
				continue;
			}/* if(Get_S(IO_DOWNLOAD)==0) */
			/* main loop ... */
			if (GetSoftTimerStatus(CAN1_3_TIMER) == TIMER_FLAG_TRIGGER)
			{
				InitSoftTimer(CAN1_3_TIMER, CAN1_3_TIMER_VALUE);
				sync_time();
			}
			
			/* Write to IO board info etc. */
			if (GetSoftTimerStatus(CAN1_1_TIMER) == TIMER_FLAG_TRIGGER)
			{
				InitSoftTimer(CAN1_1_TIMER, CAN1_1_TIMER_VALUE);
				SlowEvent= 0x04;
			}
			
			if (rt_completion_wait(&can1,RT_WAITING_FOREVER) == RT_EOK)
			{
				while( (rc2=CANAppRead(
										PORT1, 			
										Q_NOWAIT,
										0,	
										(Receive_Msg_Struct  *)&rcvbuf,  
										&len    ))==NOERR 
										&& (len==rcvbuf.msg_size+4) )
				{
						rc1=rProcessMsg(&rcvbuf,0,0);
				}
					
				for( i=0;i<pConfigData->CAN1MduSum;i++)
				{
				 	if( pConfigData->pIOMDU[i].IsDefined
							&&  pConfigData->pIOMDU[i].MduAddr 
							&&  (	(pConfigData->pIOMDU[i].ModuleType==TYPE_DOM)
										||(pConfigData->pIOMDU[i].ModuleType==TYPE_AOM)
										||(pConfigData->pIOMDU[i].ModuleType==TYPE_SPM))
							&&  IsIOBoard( pConfigData->pIOMDU[i].MduAddr ))
					{
			 			if( ( pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & STATE_IOLOAD)   
								&&((pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & (STATE_COMMERR|STATE_TYPEERR))==0) )
						{
							if( rIOOutput(&sndbuf, pConfigData->pIOMDU[i].MduAddr) ) 
								continue;
							lastCode=sndbuf.data[1];/*CODE_DOSET/CODE_AOSET*/
							rc=CANAppWrite(PORT1, &sndbuf);
							if(rc!=NOERR)
							{     
								sprintf(str,"3--[ %d ] CAN1 send message Addr=%xH, Code=%xH, Len=%d, error (%04xH)\n",rt_tick_get(),sndbuf.data[0],sndbuf.data[1],sndbuf.msg_size,(unsigned int)rc);
								Printff(str);
								pConfigData->pIOMDU[i].ErrCnt_S++;
								if(pConfigData->pIOMDU[i].ErrCnt_S>=MAX_ERR_S)
								{
									pConfigData->pIOMDU[i].ErrCnt_S=MAX_ERR_S;
									if((pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & STATE_COMMERR)==0)
									{
										IOStateAlarm(pConfigData->pIOMDU[i].MduAddr, STATE_COMMERR, STATE_COMMERR);
										pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] |= STATE_COMMERR;
									}
								}
								continue;
							}
							else
							{
								unsigned long rc1,rc2;
								int wait_time=MAX_WAIT_TIME; 
								while( (rc2=CANAppRead(
													PORT1, 			/* CAN1/CAN2: PORT1/PORT2 */
													wait_time!=0?Q_WAIT:Q_NOWAIT,/* Q_NOWAIT / Q_WAIT */
													wait_time,		/* when flags is Q_WAIT, timeout is valid */
													(Receive_Msg_Struct  *)&rcvbuf,   /* message buffer */
													&len /* length of message */    ))==NOERR 
													&& (len==rcvbuf.msg_size+4) )
								{
									rc1=rProcessMsg(&rcvbuf,lastCode,0);
									if(rc1==0 && rcvbuf.data[0]==pConfigData->pIOMDU[i].MduAddr) break;
								}
								if(rc2!=NOERR)
								{
									pConfigData->pIOMDU[i].ErrCnt_R++;
									if(pConfigData->pIOMDU[i].ErrCnt_R>=MAX_ERR_R)
									{
										pConfigData->pIOMDU[i].ErrCnt_R=MAX_ERR_R;
										if((pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & STATE_COMMERR)==0)
										{
											IOStateAlarm(pConfigData->pIOMDU[i].MduAddr, STATE_COMMERR, STATE_COMMERR);
											pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] |= STATE_COMMERR;
										}
									}
								}
							}/* else */
						}/* if */
						else
						{
							rClearIOOutput(pConfigData->pIOMDU[i].MduAddr);
						}
					}
				}/* for(i) */

				/* Call for IO board info etc. */
				{
					while( (rc2=CANAppRead(
										PORT1, 			
									Q_NOWAIT,
										0,	
									(Receive_Msg_Struct  *)&rcvbuf,  
												&len    ))==NOERR 
								 && (len==rcvbuf.msg_size+4) )
					{
							rc1=rProcessMsg(&rcvbuf,0,0);
					}

					SlowVisitMark=0;
					for( i=0;i<pConfigData->CAN1MduSum;i++)
					{
						if(	(i+1)==MaxSlowMduIndex
								&&	pConfigData->pIOMDU[i].IsDefined
								&&  IsIOBoard( pConfigData->pIOMDU[i].MduAddr )
								&&( ((pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & STATE_IOLOAD)==0)
										||(pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & (STATE_COMMERR|STATE_TYPEERR)) )	)
						{
							if(SlowEvent && SlowVisitMark<SlowNumPerScan)
							{
								SlowEvent=0; 
								SlowMduIndex=0; 
								SlowVisitMark++;
							}
						}
					 	if( pConfigData->pIOMDU[i].IsDefined
								&&  IsIOBoard( pConfigData->pIOMDU[i].MduAddr )
								&&( pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & STATE_IOLOAD) 
								&&((pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & (STATE_COMMERR|STATE_TYPEERR))==0) ) 
						{
							/*if( (pThisState[0] & 0x01)==RED_SLAVE )
								break;*/
							if(pConfigData->pIOMDU[i].ModuleType==TYPE_DIM 
								|| pConfigData->pIOMDU[i].ModuleType==TYPE_DOM 
								|| pConfigData->pIOMDU[i].ModuleType==TYPE_FAIM)
							{
								rIOPoll(&sndbuf, pConfigData->pIOMDU[i].MduAddr);
							}
							else if (pConfigData->pIOMDU[i].ModuleType==TYPE_AIM 
											|| pConfigData->pIOMDU[i].ModuleType==TYPE_AOM 
											|| pConfigData->pIOMDU[i].ModuleType==TYPE_HCM 
											|| pConfigData->pIOMDU[i].ModuleType==TYPE_SPM)
							{	
								if(SlowEvent==0 || SlowVisitMark>=SlowNumPerScan)
									continue;
								if(i<SlowMduIndex && SlowMduIndex)
									continue;
								rIOPoll(&sndbuf, pConfigData->pIOMDU[i].MduAddr);
								SlowVisitMark++;
								SlowMduIndex=i+1;
								if(SlowMduIndex>=MaxSlowMduIndex)
								{
									SlowEvent=0; 
									SlowMduIndex=0;
								}
							}
							else 
							{
//								rt_sem_release(&can1ok);
								continue;
							}
							lastCode=0;
							rc=CANAppWrite(PORT1, &sndbuf);
							if(rc!=NOERR)
							{     
   							sprintf(str,"4--[ %d ] CAN1 send message Addr=%xH, Code=%xH, Len=%d, error (%04xH)\n",rt_tick_get(),sndbuf.data[0],sndbuf.data[1],sndbuf.msg_size,(unsigned int)rc); 
								Printff(str);
								pConfigData->pIOMDU[i].ErrCnt_S++;
								if(pConfigData->pIOMDU[i].ErrCnt_S>=MAX_ERR_S)
								{
									pConfigData->pIOMDU[i].ErrCnt_S=MAX_ERR_S;
									if((pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & STATE_COMMERR)==0)
									{
   										IOStateAlarm(pConfigData->pIOMDU[i].MduAddr, STATE_COMMERR, STATE_COMMERR);
   										pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] |= STATE_COMMERR;
									}
								}
								continue;
							}
							else
							{
								unsigned long rc1,rc2;
								int wait_time=MAX_LETTER_TIME; 
								while( (rc2=CANAppRead(
														PORT1, 			/* CAN1/CAN2: PORT1/PORT2 */
														wait_time!=0?Q_WAIT:Q_NOWAIT,/* Q_NOWAIT / Q_WAIT */
														wait_time,		/* when flags is Q_WAIT, timeout is valid */
														(Receive_Msg_Struct  *)&rcvbuf,   /* message buffer */
														&len /* length of message */    ))==NOERR 
														&& (len==rcvbuf.msg_size+4) )
			 					{
     								rc1=rProcessMsg(&rcvbuf,0,0);
										if(rc1==0 && rcvbuf.data[0]==pConfigData->pIOMDU[i].MduAddr) 
											break;
								}
								if(rc2!=NOERR)
								{
									pConfigData->pIOMDU[i].ErrCnt_R++;
									if(pConfigData->pIOMDU[i].ErrCnt_R>=MAX_ERR_R)
									{
										pConfigData->pIOMDU[i].ErrCnt_R=MAX_ERR_R;
										if((pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] & STATE_COMMERR)==0)
										{
    										IOStateAlarm(pConfigData->pIOMDU[i].MduAddr, STATE_COMMERR, STATE_COMMERR);
	   										pDynamicData->pS.State[pConfigData->pIOMDU[i].S_Addr-1] |= STATE_COMMERR;
										}
									}
								}	
							}/* else */
						}/* if */
					}/* for(i) */

				}
				rt_completion_done(&can1ok);
			}
			
			if (GetSoftTimerStatus(CAN1_2_TIMER) == TIMER_FLAG_TRIGGER)
			{
				InitSoftTimer(CAN1_2_TIMER, CAN1_2_TIMER_VALUE);
				//printf("CAN1_2_TIMER  is up \n");

				if(ErrMduIndex>=pConfigData->CAN1MduSum) ErrMduIndex=0;
				for( ;ErrMduIndex<pConfigData->CAN1MduSum; ErrMduIndex++ )
				{
					if(pConfigData->pIOMDU[ErrMduIndex].IsDefined==0) 
						continue;
					if( !IsIOBoard(pConfigData->pIOMDU[ErrMduIndex].MduAddr) ) 
						continue;
				 	if((pDynamicData->pS.State[pConfigData->pIOMDU[ErrMduIndex].S_Addr-1] & STATE_IOLOAD) 
							&&((pDynamicData->pS.State[pConfigData->pIOMDU[ErrMduIndex].S_Addr-1] & (STATE_COMMERR|STATE_TYPEERR))==0) )
				 		continue;
				 		
				 	if(pDynamicData->pS.State[pConfigData->pIOMDU[ErrMduIndex].S_Addr-1] & STATE_COMMERR)
				 	{
			 			rIOPoll(&sndbuf, pConfigData->pIOMDU[ErrMduIndex].MduAddr);
						lastCode=0;
						rc=CANAppWrite(PORT1, &sndbuf);
				 		ErrMduIndex++;
				 		break;
				 	}
				 	else
				 	{
						rDownload( &sndbuf, pConfigData->pIOMDU[ErrMduIndex].MduAddr );	
						pConfigData->pIOMDU[pConfigData->pIOMDU[ErrMduIndex].MduAddr-1].Serial=0;

						lastCode=0;
						rc=CANAppWrite(PORT1, &sndbuf);
						if(rc!=NOERR)
						{     
							sprintf(str,"5--[ %d ] CAN1 send message Addr=%xH, Code=%xH, Len=%d, error (%04xH)\n",rt_tick_get(),sndbuf.data[0],sndbuf.data[1],sndbuf.msg_size,(unsigned int)rc); 
							Printff(str);
							pConfigData->pIOMDU[ErrMduIndex].ErrCnt_S++;
							if(pConfigData->pIOMDU[ErrMduIndex].ErrCnt_S>=MAX_ERR_S)
							{
								pConfigData->pIOMDU[ErrMduIndex].ErrCnt_S=MAX_ERR_S;
								if((pDynamicData->pS.State[pConfigData->pIOMDU[ErrMduIndex].S_Addr-1] & STATE_COMMERR)==0)
								{
										IOStateAlarm(pConfigData->pIOMDU[ErrMduIndex].MduAddr, STATE_COMMERR, STATE_COMMERR);
										pDynamicData->pS.State[pConfigData->pIOMDU[ErrMduIndex].S_Addr-1] |= STATE_COMMERR;
								}
							}
							continue;
						}
						else
						{
							while( (rc2=CANAppRead(
													PORT1, 			/* CAN1/CAN2: PORT1/PORT2 */
													Q_WAIT, 		/* Q_NOWAIT / Q_WAIT */
													MAX_ACK_TIME,	/* when flags is Q_WAIT, timeout is valid */
													(Receive_Msg_Struct  *)&rcvbuf,   /* message buffer */
													&len /* length of message */    ))==NOERR 
													&& (len==rcvbuf.msg_size+4) )
							{
								rc1=rProcessMsg(&rcvbuf,CODE_IOLOAD,0);
								if(rc1==0 && rcvbuf.data[0]==pConfigData->pIOMDU[ErrMduIndex].MduAddr) 
									break;
							}
							
							if(rc2!=NOERR)
							{
								pConfigData->pIOMDU[ErrMduIndex].ErrCnt_R++;
								if(pConfigData->pIOMDU[ErrMduIndex].ErrCnt_R>=MAX_ERR_R)
								{
									pConfigData->pIOMDU[ErrMduIndex].ErrCnt_R=MAX_ERR_R;
									if((pDynamicData->pS.State[pConfigData->pIOMDU[ErrMduIndex].S_Addr-1] & STATE_COMMERR)==0)
									{
											IOStateAlarm(pConfigData->pIOMDU[ErrMduIndex].MduAddr, STATE_COMMERR, STATE_COMMERR);
											pDynamicData->pS.State[pConfigData->pIOMDU[ErrMduIndex].S_Addr-1] |= STATE_COMMERR;
									}
								}
							}
							else 
							{
								rSetTime(&sndbuf, pConfigData->pIOMDU[ErrMduIndex].MduAddr);
								rc=CANAppWrite(PORT1, &sndbuf);
							}
						}
				 		ErrMduIndex++;
				 		break;
				 	}
				}/* for(ErrMduIndex) */
				wait_time=0;
				while( (rc2=CANAppRead(
										PORT1, 			
										wait_time!=0?Q_WAIT:Q_NOWAIT,
										wait_time,		
										(Receive_Msg_Struct  *)&rcvbuf,
										&len     ))==NOERR 
										&& (len==rcvbuf.msg_size+4) )
				{
     				rc1=rProcessMsg(&rcvbuf,0,0);
				}

			}/* if( event & 0x1 ) */
		}/* if( (pThisState[0] & 0x01)==RED_MASTER ) */
	}
}
