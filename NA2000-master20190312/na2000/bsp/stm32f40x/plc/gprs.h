#ifndef __H_GPRS
#define __H_GPRS

#include "system.h"

#define MAX_RETRY_INTERVAL							60*60*1000
#define MAX_RECV_BUFF_SIZE							700
#define MAX_SEND_BUFF_SIZE							600

#define GPRS_MAX_WAITTIME								300	//ms
#define GPRS_MAX_INTERVAL								30	//ms

#define GPRS_MAX_QUERYTIME							90	//ms

#define GPRS_MAX_CYCLE									30	//ms

#define GPRS_MAX_RETRY_CONN_CNT					30
#define CRLF														"\r\n"

#define GPRS_MAX_KEEP_ALIVE_TIME				10*60*1000	//ms

#define GPRS_TCPIP_MAX_CONN_CNT					4
#define GPRS_TCPIP_CONN_TIMEOUT					1		//s
#define GPRS_TCPIP_CONN_MAX_LEFT_TIME		75*60*1000	//ms

typedef enum
{
	GPRS_NO_ERROR = 0,
	GPRS_NO_MEM,
	GPRS_NO_GPRS_MODULE,
	GPRS_SET_BAUDRATE_FAILED,
	GPRS_NO_SIMCARD,
	GPRS_WEAK_STRENGTH,
	GPRS_NO_GSM,
	GPRS_NO_GPRS,
	GPRS_CGATT_FAILED,
	GPRS_CONFIG_APN_FAILED,
	GPRS_ACT_PDP_FAILED,
	GPRS_DEACT_PDP_FAILED,
	GPRS_CANNOT_CACHE_DATA,
	GPRS_CANNOT_GET_IP,
	GPRS_CONFIG_FG_FAILED,
	GPRS_ENABLE_INFO_HEAD_FAILED,
	GPRS_TCPIP_CONFIG_MODE_FAILED,
	GPRS_TCPIP_CONFIG_MUL_CONN_FAILED,
	GPRS_TCPIP_CONNECT_FAILED,
	GPRS_TCPIP_RECV_TIMEOUT,
	GPRS_TCPIP_SEND_FAILED,
	GPRS_TCPIP_CLOSED_FAILED,
}EGPRS_ERR_CODE;

typedef enum
{
	NOT_CONNECTED=0,
	CONNECTING,
	CONNECTED,
}ECONN_STATE;

typedef enum
{
	TCPIP_NO_EVENT = 0,
	TCPIP_HAS_DATA,
	TCPIP_CONNECTED,
	TCPIP_CLOSED,
	TCPIP_CONNECT_FAILED,
}ETCPIP_EVENT;

typedef enum
{
	GPRS_UPDATE_PACKET,
	GPRS_UPDATE_PACKET_BUT_NOT_COMPLETE,
	GPRS_OTHER_PACKET
}PACKET_TYPE;

typedef struct
{
	signed char 		qiopen_retry_cnt;
	signed char 		tcp_reconn_period;
	EGPRS_ERR_CODE 	errcode;
	ECONN_STATE			state;
	unsigned char		first;
	ETCPIP_EVENT		event;
	int 						start_conn_time;	//ms
	int 						lastalivetime;	//ms
}GPRS_CONN_STATE;

typedef struct
{
	char* 					send_buff;
	char*						recv_buff;
	short						send_len;
	short						recv_len;
	GPRS_CONN_STATE conn_state;
}GPRS_STATE;

extern void gprs_thread(void *lp);
EGPRS_ERR_CODE gprs_get_errcode(void);
void gprs_set_errcode(EGPRS_ERR_CODE errcode);

EGPRS_ERR_CODE gprs_get_conn_errcode(unsigned char sid);
void gprs_set_conn_errcode(unsigned char sid,EGPRS_ERR_CODE errcode);

#endif
