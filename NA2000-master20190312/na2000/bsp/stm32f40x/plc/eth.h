/* -------------------------------------------------------------------------------
                                RTU Software 
                      
				 Version	:	1.0                                                      
				Filename	:	eth.h                             
   Originally Written By	:	Chen Sining                          
					Date	:	2013.03.19
       Application Notes	:	This file defines some macro definations and 
								structures, which may be used by Ethernet
								software.
         Updating Record	:	( Pls. Specify : Who , Where , When, Why , What )
    
--------------------------------------------------------------------------------- */
#ifndef   ETH_H
#define   ETH_H 


#define		ETHDBG_PORT		5555
#define		MDBTCP_PORT     502
#define		DATA_LEN		4200
#define		MAX_CONN		4	     
       
typedef struct {
	unsigned short Pad;
	unsigned short ITagNum;
	unsigned short QTagNum;
	unsigned short IWTagNum;
	unsigned short QWTagNum;
	unsigned short MTagNum;
	unsigned short RTagNum;
	unsigned short NMTagNum;
	unsigned short NRTagNum;
	unsigned short VTagNum;
	unsigned short *ITagOccno;
	unsigned short *QTagOccno;
	unsigned short *IWTagOccno;
	unsigned short *QWTagOccno;
	unsigned short *MTagOccno;
	unsigned short *RTagOccno;
	unsigned short *NMTagOccno;
	unsigned short *NRTagOccno;
	unsigned short *VTagOccno;
} POINT_TABLE;
                   
typedef struct {
	int					UseFlg;
	int                 SrvSid;                     /* socket id for server */ 
	rt_thread_t			SrvTid;                     /* thread id for server */
	struct sockaddr_in	SrvAdd;
	POINT_TABLE			*SrvPoint;
} ETHDBG_NODE_TABLE;


#endif
