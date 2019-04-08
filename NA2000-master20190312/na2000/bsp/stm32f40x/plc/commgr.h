#ifndef __H_COM_MGR
#define __H_COM_MGR

#define MAX_SEND_BUF    512//256
#define MAX_RECV_BUF    512//256

#define MAX_COM_NUM			4

typedef struct 
{
	unsigned short SndSize;
	unsigned short Pad;
	unsigned char  SndBuf[MAX_SEND_BUF];
} SNDDEF;

typedef struct 
{
	unsigned short  RcvCount;
	unsigned short  pad;
	unsigned char  RcvBuf[MAX_RECV_BUF];
} RCVDEF;

#endif
