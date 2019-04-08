/* -------------------------------------------------------------------------------
                                RTU Software 
                      
				 Version	:	1.0                                                      
				Filename	:	ethdbg.c                             
   Originally Written By	:	Chen Sining                          
					Date	:	2013.03.19
       Application Notes	:	This file is for Ethernet debug task of single CPU.
         Updating Record	:	( Pls. Specify : Who , Where , When, Why , What )
    
--------------------------------------------------------------------------------- */
#include "sysapp.h"



ETHDBG_NODE_TABLE EthdbgNodeTable[MAX_CONN];

void subethdbg(void * arg);
int ethdbg_send_msg(int sid,unsigned char * msgp);
int ethdbg_recv_msg(int sid,unsigned char * msgp);


void ethdbg(void *arg)
{
	struct sockaddr_in ServerSocket,SenderSocket;     
	int ServerSid,LanSid;
	int i,opt;
	unsigned int length;
	rt_thread_t tid;
  
	memset(EthdbgNodeTable,0x0,sizeof(ETHDBG_NODE_TABLE)*MAX_CONN);

	/* create a new socket */
	if ((ServerSid=socket(AF_INET,SOCK_STREAM,0))==-1)  
	{
		printf("ETH_DBG : socket error\n");
		return;
	}  
  
	/* assign local address to the above socket */
	ServerSocket.sin_family=AF_INET;
	ServerSocket.sin_addr.s_addr=INADDR_ANY;
	ServerSocket.sin_port=htons(ETHDBG_PORT);

	opt = 1;
//	if (setsockopt(ServerSid, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
//	{
//		close(ServerSid);
//		printf("ETH_DBG : setsockopt error\n");
//		return;
//	}

	if (bind(ServerSid,(struct sockaddr *)&ServerSocket,sizeof(struct sockaddr))==-1)
	{
		closesocket(ServerSid);
		printf("ETH_DBG : bind error\n");
		return;
	}
  
	/* prepare to receive connections */
	if (listen(ServerSid,5)==-1)
	{
		closesocket(ServerSid);
		printf("ETH_DBG : listen error\n");
		return;
	}
 
	while(TRUE)
	{ 
		/* accept a connection request from a foreign socket */
		length=sizeof(SenderSocket);
		LanSid=accept(ServerSid,(struct sockaddr *)&SenderSocket,(socklen_t *)&length);
		if (LanSid>0)
		{ 
			for (i=0;i<MAX_CONN;i++)
			{
				if (EthdbgNodeTable[i].UseFlg==FALSE)
				{
					/* once receiving a connection request,create a new sub-task */       
					EthdbgNodeTable[i].UseFlg=TRUE;
					EthdbgNodeTable[i].SrvPoint=NULL;
					EthdbgNodeTable[i].SrvSid=LanSid;
					EthdbgNodeTable[i].SrvAdd=SenderSocket;

					//线程进入,执行程序
					tid=rt_thread_create("dbg",subethdbg,&i,1024+512,20,20);

					EthdbgNodeTable[i].SrvTid=tid;

					if (tid==RT_NULL)
					{
						EthdbgNodeTable[i].UseFlg=0;
						closesocket(LanSid);
						printf("ETH_DBG : rt_thread_create error\n");
					}
					else 
					{
						printf("ETH_DBG : Create thread, No=%d, Tid=%d, Sid=%d\n",i,tid,LanSid);
						rt_thread_startup(tid);
					}
					break;
				}
			}
			if (i==MAX_CONN)
			{
				Printff("ETH_DBG : too many connection requests\n");
				closesocket(LanSid);
			}			
		}   
	}
}

void MyExit(int exitno)
{
	char tt[40];
	int i;
	for (i=0;i<MAX_LD;i++)
		pPrgData->CntFlag[i]=0xffff;
	memset(pPrgData->BreakPointIndex,0,MAX_LD*2);
	sprintf(tt,"ETH_DBG : exit (%d)\n",exitno);
	printf(tt);
}
int g_tmp = 0;
void subethdbg(void * arg)
{ 
	unsigned char *msgp;
	int rcode; 
	int i;
	int opt=1;

	i=*((int *)arg);
	
	msgp=(unsigned char *)rt_malloc(DATA_LEN);
	if (msgp==NULL)
	{
		shutdown(EthdbgNodeTable[i].SrvSid,2);
		closesocket(EthdbgNodeTable[i].SrvSid);
		EthdbgNodeTable[i].UseFlg=FALSE;
		MyExit(-2);
		return;
	}

	if(setsockopt(EthdbgNodeTable[i].SrvSid, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt) )== -1)
	{
		close(EthdbgNodeTable[i].SrvSid);
		printf("ETH_DBG : setsockopt EthdbgNodeTable[%d].SrvSid) error\n",i);
		return;
	}	
	
	while (TRUE)
	{
		rcode=ethdbg_recv_msg(EthdbgNodeTable[i].SrvSid,msgp);

		if (rcode==-1)
		{
			shutdown(EthdbgNodeTable[i].SrvSid,2);
			closesocket(EthdbgNodeTable[i].SrvSid);
			EthdbgNodeTable[i].UseFlg=FALSE;
			rt_free(msgp);
			if (EthdbgNodeTable[i].SrvPoint!=NULL)
			{
				rt_free(EthdbgNodeTable[i].SrvPoint);
				EthdbgNodeTable[i].SrvPoint=NULL;
			}
			MyExit(-3);
			return;
		}
		else   
		{
			rcode=ethdbg_msg_handle(EthdbgNodeTable[i].SrvSid,msgp,&EthdbgNodeTable[i]);
			if (rcode==0)
				ethdbg_send_msg(EthdbgNodeTable[i].SrvSid,msgp);
			else if (rcode==-1)
			{
				shutdown(EthdbgNodeTable[i].SrvSid,2);
				closesocket(EthdbgNodeTable[i].SrvSid);
				EthdbgNodeTable[i].UseFlg=FALSE;
				rt_free(msgp);
				if (EthdbgNodeTable[i].SrvPoint!=NULL)
				{
					free(EthdbgNodeTable[i].SrvPoint);
					EthdbgNodeTable[i].SrvPoint=NULL;
				}
				MyExit(-4);
				return;
			}
			else if (rcode==-2)
				printf("ETH_DBG : receive letter error\n");
		}
	}
}

int ethdbg_recv_msg(int sid,unsigned char * msgp)
{
	int recv_count,length,k,rc;
	fd_set read_mask;
	struct timeval wait;
	/* receive message head */                                                 
	k=0;
	length=6;
	while(length>0)
	{
		wait.tv_sec=300;
		wait.tv_usec=0;
		FD_ZERO(&read_mask);
		FD_SET(sid,&read_mask);
		rc=select(FD_SETSIZE,&read_mask,(fd_set *) 0,(fd_set *) 0,&wait);
		if (rc<=0)
			return (-1);
		recv_count=recv(sid,(char *)&msgp[k],length,0);
		if (recv_count<=0)
			return (-1);
		k+=recv_count;
		length-=recv_count;
	}

	/* receive message data */
	k=0;
	if (msgp[0]!=0 || msgp[1]!=0 || msgp[2]!=0x55 || msgp[3]!=0x55)
		return (-1);
	length=(int)(((unsigned short)msgp[4]<<8)+msgp[5]);
	if (length==0 || length>DATA_LEN-6)
		return (-1);
	while(length>0)
	{
		wait.tv_sec=300;
		wait.tv_usec=0;
		FD_ZERO(&read_mask);
		FD_SET(sid,&read_mask);
		rc=select(FD_SETSIZE,&read_mask,(fd_set *) 0,(fd_set *) 0,&wait);
		if (rc<=0)
			return (-1);
		recv_count=recv(sid,(char *)&msgp[k+6],length,0);
		if (recv_count<=0)
			return (-1);
		k+=recv_count;
		length-=recv_count;
	}

	return (k+6);                                                 
}

int ethdbg_send_msg(int sid,unsigned char * msgp)
{
	int k,len,send_rc;
	//int k_tmp=0;
	k=0;
	
	len=(int)(((unsigned short)msgp[4]<<8)+msgp[5])+6;  

	while (len>0)
	{ 
        send_rc=send(sid,(char *)msgp+k,len,0);
	    if (send_rc<=0) 
			return -1;
	    k+=send_rc;
		len-=send_rc;
	}
	return(k);
}

