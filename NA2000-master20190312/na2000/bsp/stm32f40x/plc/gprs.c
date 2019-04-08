#include "system.h"
#include "appcnst.h"
#include "sysapp.h"
#include <rtthread.h>

#include "serial_app.h"
#include "commgr.h"
#include "gprs.h"
#include "gprs_update.h"
#include <stm32f4xx.h>
#include "zigbee.h"
#include "manage.h"

/**************************************************************/
/*																														*/
/**************return value equal 0 mean success***************/
/*																														*/
/**************************************************************/

#define GPRS_DEBUG_ON			1

#if GPRS_DEBUG_ON
#define gprs_dbg(x)				do {\
if(GetHardSW()==1) \
	rt_kprintf x;\
} while(0)

int ResetCPU(void);

static char channel_cnt = 0;
static char *g_recv = RT_NULL;
EGPRS_ERR_CODE gprs_errcode = GPRS_NO_ERROR;

void _dbg_hex(const char *func,unsigned char *src,int src_len)
{
	gprs_dbg(("[%s]:",func));
	if(GetHardSW()==1)
	{
		int i=0;
		for(i=0;i<src_len;i++)
		{
			gprs_dbg(("%02x ",src[i]));
		}
		gprs_dbg(("\n"));
	}
}

#define dbg_hex(data,len)	_dbg_hex(__func__,data,len)
#else
#define gprs_dbg(...)			
#define dbg_hex(...)		
#endif

#define GPRS_ECHO_ON			GPRS_DEBUG_ON

extern char* send_tcpip_cmd(int fd,char sid,char *cmd,int cmd_len,int response_time,short *len);
extern int na_create_thread(const char* name,
														void (*entry)(void *parameter),
                            void       *parameter,
                            rt_uint32_t stack_size,
                            rt_uint8_t  priority,
                            rt_uint32_t tick);
														
void gprs_power(char onoff)
{
	extern void DO_Write(unsigned char ch,unsigned char date);
	DO_Write(11,!!onoff);
}

char *mystrstr(const char *s1, int l1,const char *s2)
{
    int l2;

    l2 = rt_strlen(s2);
    if (!l2)
        return (char *)s1;
    while (l1 >= l2)
    {
        l1 --;
        if (!rt_memcmp(s1, s2, l2))
            return (char *)s1;
        s1 ++;
    }

    return RT_NULL;
}

char *mystrtok(const char *s1, int l1,const char *s2)
{
    int l2;

    l2 = rt_strlen(s2);
    if (!l2)
        return (char *)s1;
    while (l1 >= l2)
    {
        l1 --;
        if (!rt_memcmp(s1, s2, l2))
				{
            return (char *)s1 + l2;
				}
        s1 ++;
    }

    return RT_NULL;
}

#define INIT_GPRS_STATE	{\
		RT_NULL,\
		RT_NULL,\
		0,\
		0,\
		{GPRS_MAX_RETRY_CONN_CNT,GPRS_MAX_RETRY_CONN_CNT,GPRS_TCPIP_CONNECT_FAILED,NOT_CONNECTED,1,TCPIP_NO_EVENT,0,0},\
	}

static GPRS_STATE	gprs_state[GPRS_TCPIP_MAX_CONN_CNT] =
{
	INIT_GPRS_STATE,
	INIT_GPRS_STATE,
	INIT_GPRS_STATE,
	INIT_GPRS_STATE,
};

rt_bool_t gprs_config_ip_or_dns_mode(int fd,char mode);

static void init_gprs_conn_state(char index)
{
	gprs_state[index].conn_state.qiopen_retry_cnt = GPRS_MAX_RETRY_CONN_CNT;
	gprs_state[index].conn_state.tcp_reconn_period= GPRS_MAX_RETRY_CONN_CNT;
	gprs_state[index].conn_state.errcode 					= GPRS_NO_ERROR;
	gprs_state[index].conn_state.state 						= NOT_CONNECTED;
	gprs_state[index].conn_state.start_conn_time 	= 0;
	gprs_state[index].conn_state.lastalivetime 		= 0;
	gprs_state[index].conn_state.event 						= TCPIP_NO_EVENT;
	gprs_state[index].conn_state.first						= 1;
}
	
EGPRS_ERR_CODE gprs_get_errcode()
{
	return gprs_errcode;
}

void gprs_set_errcode(EGPRS_ERR_CODE errcode)
{
	gprs_errcode = errcode;
}

rt_err_t gprs_init(int fd)
{
	char i=0;
	
	clearport(fd);
	
	channel_cnt = 0;
	if(g_recv == RT_NULL)
	{
		g_recv = (char *)rt_calloc(MAX_RECV_BUFF_SIZE,1);
		if(g_recv == RT_NULL)
			goto out;
	}
	
	for(i=0;i<GPRS_TCPIP_MAX_CONN_CNT;i++)
	{
		if(!pConfigData->Gprs.Data[i].Enable)
				continue;
		
		if(gprs_state[i].send_buff == RT_NULL)
		{
			gprs_state[i].send_buff = rt_calloc(MAX_SEND_BUFF_SIZE,1);
			if(gprs_state[i].send_buff == RT_NULL)
			{
				gprs_set_errcode(GPRS_NO_MEM);
				goto out;
			}
		}
		
		if(gprs_state[i].recv_buff == RT_NULL)
		{
			gprs_state[i].recv_buff = rt_calloc(MAX_RECV_BUFF_SIZE,1);
			if(gprs_state[i].recv_buff == RT_NULL)
			{
				gprs_set_errcode(GPRS_NO_MEM);
				goto out;
			}
		}
		channel_cnt++;
	}
	return RT_EOK;
	
out:
	if(g_recv)
	{
		rt_free(g_recv);
		g_recv = RT_NULL;
	}
	
	for(i=0;i<GPRS_TCPIP_MAX_CONN_CNT;i++)
	{
		if(gprs_state[i].recv_buff)
		{
			rt_free(gprs_state[i].recv_buff);
			gprs_state[i].recv_buff = RT_NULL;
		}
		if(gprs_state[i].send_buff)
		{
			rt_free(gprs_state[i].send_buff);
			gprs_state[i].send_buff = RT_NULL;
		}
	}
	
	return RT_EFULL;
}

EGPRS_ERR_CODE gprs_get_conn_errcode(unsigned char sid)
{
	return gprs_state[sid].conn_state.errcode;
}

static void gprs_set_conn_errcode(unsigned char sid,EGPRS_ERR_CODE errcode)
{
	gprs_state[sid].conn_state.errcode = errcode;
}

static void gprs_set_conn_state(unsigned char sid,ECONN_STATE state)
{
	gprs_state[sid].conn_state.state = state;
}

static ECONN_STATE gprs_get_conn_state(unsigned char sid)
{
	return gprs_state[sid].conn_state.state;
}

static void gprs_set_start_conn_time(unsigned char sid,int time)
{
	gprs_state[sid].conn_state.start_conn_time = time;
}

static int gprs_left_conn_time(unsigned char sid)
{
	int t = rt_tick_get() - gprs_state[sid].conn_state.start_conn_time;
	return GPRS_TCPIP_CONN_MAX_LEFT_TIME - t;
}

static void gprs_set_tcpip_event(unsigned char sid,ETCPIP_EVENT event)
{
	gprs_state[sid].conn_state.event = event;
}

ETCPIP_EVENT gprs_get_tcpip_event(unsigned char sid)
{
	ETCPIP_EVENT old = gprs_state[sid].conn_state.event;
	gprs_state[sid].conn_state.event = TCPIP_NO_EVENT;
	return old;
}

void gprs_tcpip_set_lastalivetime(unsigned char sid,int time)
{
	gprs_state[sid].conn_state.lastalivetime = time;
}

static unsigned char gprs_get_first_flag(unsigned char sid)
{
	return gprs_state[sid].conn_state.first;
}

static void gprs_set_first_flag(unsigned char sid,unsigned char first)
{
	gprs_state[sid].conn_state.first = first;
}

static void gprs_filter_events(int fd,char *recv,int len)
{
	char *ptmp = RT_NULL;
	char *pSub = RT_NULL;
	char sid = 0;
	char id = 0;
	char sc = 0;
	int next_process_len = len;
	int i=0;
	
	for(i=0;i<len;i++)
	{
		gprs_dbg(("%c",recv[i]));
	}
	
	next_process_len = len;
	ptmp=mystrtok(recv,next_process_len,CRLF);
	while(ptmp)
	{
		if((pSub = mystrstr(ptmp,next_process_len,"+QIRDI:")) != RT_NULL)
		{
			sscanf(pSub,"+QIRDI: %d,%d,%d",(int *)&id,(int *)&sc,(int *)&sid);
			gprs_dbg(("+QIRDI: sid=%d\n",sid));
			gprs_set_tcpip_event(sid,TCPIP_HAS_DATA);
		}
		next_process_len = len - (ptmp - recv);
		ptmp=mystrtok(ptmp,next_process_len,"+QIRDI:");
	}
	
	next_process_len = len;
	ptmp=mystrtok(recv,next_process_len,CRLF);
	while(ptmp)
	{
		if((pSub = mystrstr(ptmp,next_process_len,", CLOSED"))!= RT_NULL)
		{
			sscanf(pSub-1,"%d, CLOSED",(int *)&sid);
			gprs_dbg(("%d, CLOSED\n",sid));
			gprs_set_tcpip_event(sid,TCPIP_CLOSED);
		}
		next_process_len = len - (ptmp - recv);
		ptmp=mystrtok(ptmp,next_process_len,", CLOSED");
	}
	
	next_process_len = len;
	ptmp=mystrtok(recv,next_process_len,CRLF);
	while(ptmp)
	{
		if((pSub = mystrstr(ptmp,next_process_len,", CONNECT OK"))!= RT_NULL)
		{
			sscanf(pSub-1,"%d, CONNECT OK",(int *)&sid);
			gprs_set_tcpip_event(sid,TCPIP_CONNECTED);
		}
		next_process_len = len - (ptmp - recv);
		ptmp=mystrtok(ptmp,next_process_len,", CONNECT OK");
	}
		
	next_process_len = len;
	ptmp=mystrtok(recv,next_process_len,CRLF);
	while(ptmp)
	{
		if((pSub = mystrstr(ptmp,next_process_len,", CONNECT FAIL"))!= RT_NULL)
		{
			sscanf(pSub-1,"%d, CONNECT FAIL",(int *)&sid);
			gprs_set_tcpip_event(sid,TCPIP_CONNECT_FAILED);
		}
		next_process_len = len - (ptmp - recv);
		ptmp=mystrtok(ptmp,next_process_len,", CONNECT FAIL");
	}
}

int gprs_readport(int fd,char *recv,int maxlen,int response_time)
{
	int len = 0;

	rt_memset(recv,0,maxlen);
	
	len = readport(fd,recv,maxlen,response_time,GPRS_MAX_INTERVAL);
	if(len <= 0)
		return len;
	
	gprs_filter_events(fd,recv,len);
	
	return len;
}

static char* send_cmd(int fd,char *cmd,int response_time,short *len)
{

	writeport(fd,cmd,rt_strlen(cmd));
	*len = gprs_readport(fd,g_recv,MAX_RECV_BUFF_SIZE,response_time);
	if(*len <= 0)
		return RT_NULL;
	
	return g_recv;
}

static char* send_cmd_expect_string(int fd,char *cmd,const char *expect,int response_time)
{
	char *pRet = RT_NULL;
	short len = 0;
	pRet = send_cmd(fd,cmd,response_time,&len);
	if(!pRet)
		return pRet;
	
	return mystrstr(pRet,len,expect);
}

rt_bool_t execute_cmd_expect_ok(int fd,char *cmd,int response_time)
{
	if(send_cmd_expect_string(fd,cmd,"OK",response_time))
		return RT_TRUE;
	return RT_FALSE;
}

char* send_tcpip_cmd(int fd,char sid,char *cmd,int cmd_len,int response_time,short *len)
{
	writeport(fd,cmd,cmd_len);
	gprs_state[sid].recv_len = 0;
	*len = gprs_readport(fd,gprs_state[sid].recv_buff,MAX_RECV_BUFF_SIZE,response_time);
	if(*len <= 0)
		return RT_NULL;
	
	gprs_state[sid].recv_len = *len;
	
	return gprs_state[sid].recv_buff;
}

char* send_tcpip_cmd_expect_string(int fd,char sid,char *cmd,int cmd_len,const char *expect,int response_time)
{
	char *pRet = RT_NULL;
	short len = 0;
	pRet = send_tcpip_cmd(fd,sid,cmd,cmd_len,response_time,&len);
	if(!pRet)
		return pRet;
	
	return mystrstr(pRet,len,expect);
}

rt_bool_t execute_tcpip_cmd_expect_ok(int fd,char sid,char *cmd,int cmd_len,int response_time)
{
	if(send_tcpip_cmd_expect_string(fd,sid,cmd,cmd_len,"OK",response_time))
		return RT_TRUE;
	return RT_FALSE;
}

rt_bool_t gprs_tcpip_close(int fd,char sid)
{
	char buff[20] = "";
	char *recv = RT_NULL;

	sprintf(buff,"AT+QICLOSE=%d"CRLF,sid);

	recv = send_tcpip_cmd_expect_string(fd,sid,buff,strlen(buff),"CLOSE OK",GPRS_MAX_WAITTIME);
	if(!recv)
	{
		gprs_set_errcode(GPRS_TCPIP_CLOSED_FAILED);
		return RT_FALSE;
	}

	return RT_TRUE;
}

rt_bool_t gprs_powup(int fd)
{
	rt_bool_t ret = execute_cmd_expect_ok(fd,"AT"CRLF,GPRS_MAX_WAITTIME);
	if(!ret)
		gprs_set_errcode(GPRS_NO_GPRS_MODULE);

	return ret;
}

void gprs_startup(int fd)
{
#define GPRS_POWER_ON			1
	gprs_power(GPRS_POWER_ON);
	rt_thread_delay(2000);
	gprs_power(!GPRS_POWER_ON);
}

void gprs_shutdown(int fd)
{
	char sid = 0;
	for(sid=0;sid<GPRS_TCPIP_MAX_CONN_CNT;sid++)
		gprs_tcpip_close(fd,sid);
	gprs_power(GPRS_POWER_ON);
	rt_thread_delay(1000);
	gprs_power(!GPRS_POWER_ON);
}

rt_bool_t gprs_set_cmd_echo_on(int fd,char on)
{
	char buff[20];
	sprintf(buff,"ATE%d"CRLF,on);
	
	return execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
}

rt_bool_t gprs_set_tcpip_echo_on(int fd,char on)
{
	char buff[20];
	sprintf(buff,"AT+QISDE=%d"CRLF,on);
	
	return execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
}

rt_bool_t gprs_set_baudrate(int fd) 
{
	rt_bool_t ret = RT_FALSE;
	
	ret = execute_cmd_expect_ok(fd,"AT+ICF=3"CRLF,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_SET_BAUDRATE_FAILED);
		return ret;
	}
	
	ret = execute_cmd_expect_ok(fd,"AT+IPR=115200"CRLF,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_SET_BAUDRATE_FAILED);
		return ret;
	}
	
	ret = execute_cmd_expect_ok(fd,"AT&W"CRLF,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_SET_BAUDRATE_FAILED);
		return ret;
	}
	
	return ret;
}

void gprs_current_network_strength(int fd,int *strength,int *errrate)
{
	int a=0,b=100;
	char* recv = send_cmd_expect_string(fd,"AT+CSQ"CRLF,"+CSQ:",GPRS_MAX_WAITTIME);
	
	if(!recv)
	{
		if(strength)
			*strength = a;
		if(errrate)
			*errrate = b;
		gprs_set_errcode(GPRS_WEAK_STRENGTH);
		return;
	}
	
	if(strength || errrate)
	{
		sscanf(recv,"+CSQ: %d,%d",&a,&b);
	}
	
	if(strength)
			*strength = a;
	if(errrate)
		*errrate = b;
}

rt_bool_t gprs_has_sim_card(int fd)
{ 
	char* recv = send_cmd_expect_string(fd,"AT+CPIN?"CRLF,"+CPIN: READY",GPRS_MAX_WAITTIME);

	if(!recv)
	{
		gprs_set_errcode(GPRS_NO_SIMCARD);
		return RT_FALSE;
	}
	
	return RT_TRUE;
}

rt_bool_t gprs_gsm_connected(int fd)
{
	int tmp1,tmp2;
	
	char* recv = send_cmd_expect_string(fd,"AT+CREG?"CRLF,"+CREG:",GPRS_MAX_WAITTIME);
	if(!recv)
	{
		gprs_set_errcode(GPRS_NO_GSM);
		return RT_FALSE;
	}
	
	sscanf(recv,"+CREG: %d,%d",&tmp1,&tmp2);
	if(!(tmp2 == 1 || tmp2 == 5))
	{
		gprs_set_errcode(GPRS_NO_GSM);
		return RT_FALSE;
	}
	
	return RT_TRUE;
}

rt_bool_t gprs_gprs_connected(int fd)
{
	int tmp1,tmp2;
	
	char* recv = send_cmd_expect_string(fd,"AT+CGREG?"CRLF,"+CGREG:",GPRS_MAX_WAITTIME);
	if(!recv)
	{
		gprs_set_errcode(GPRS_NO_GPRS);
		return RT_FALSE;
	}
	
	sscanf(recv,"+CGREG: %d,%d",&tmp1,&tmp2);
	if(!(tmp2 == 1 || tmp2 == 5))
	{
		gprs_set_errcode(GPRS_NO_GPRS);
		return RT_FALSE;
	}

	return RT_TRUE;
}

rt_bool_t gprs_show_the_other_side_ip_port(int fd,rt_bool_t enable)
{
	char buff[20];
	rt_bool_t ret = RT_FALSE;
	sprintf(buff,"AT+QISHOWRA=%d"CRLF,enable);
	
	ret = execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_ENABLE_INFO_HEAD_FAILED);
	}
	return ret;
}

rt_bool_t gprs_show_ip_head(int fd,rt_bool_t enable)
{
	char buff[20];
	rt_bool_t ret = RT_FALSE;
	sprintf(buff,"AT+QIHEAD=%d"CRLF,enable);
	
	ret = execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_ENABLE_INFO_HEAD_FAILED);
	}
	return ret;
}

rt_bool_t gprs_show_ip_head_proto(int fd,rt_bool_t enable)
{
	char buff[20];
	rt_bool_t ret = RT_FALSE;
	sprintf(buff,"AT+QISHOWPT=%d"CRLF,enable);
	
	ret = execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_ENABLE_INFO_HEAD_FAILED);
	}
	return ret;
}

rt_bool_t gprs_cache_recv_data(int fd,char mode)
{
	char buff[20];
	rt_bool_t ret = RT_FALSE;
	sprintf(buff,"AT+QINDI=%d"CRLF,mode);
	
	ret = execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_CANNOT_CACHE_DATA);
	}
	return ret;
}

rt_bool_t gprs_config_fg(int fd,char id)
{
	char buff[20];
	rt_bool_t ret = RT_FALSE;
	sprintf(buff,"AT+QIFGCNT=%d"CRLF,id);
	
	ret = execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_CONFIG_FG_FAILED);
	}
	return ret;
}

rt_bool_t gprs_config_ip_or_dns_mode(int fd,char mode)
{
	char buff[20];
	rt_bool_t ret = RT_FALSE;
	sprintf(buff,"AT+QIDNSIP=%d"CRLF,mode);
	
	ret = execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_CONFIG_FG_FAILED);
	}
	return ret;
}

rt_bool_t gprs_config_gain(int fd,char mode) 
{
	char buff[20];
	rt_bool_t ret = RT_FALSE;
	sprintf(buff,"AT+CGAT=%d"CRLF,mode);
	
	ret = execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_CGATT_FAILED);
	}
	return ret;
}

rt_bool_t gprs_set_tcpip_mode(int fd,char mode)
{
	char buff[20];
	rt_bool_t ret = RT_FALSE;
	sprintf(buff,"AT+QIMODE=%d"CRLF,mode);
	
	ret = execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_TCPIP_CONFIG_MODE_FAILED);
	}
	return ret;
}

rt_bool_t gprs_get_tcpip_mode(int fd,char* mode)
{
	char buff[20];
	rt_bool_t ret = RT_FALSE;
	sprintf(buff,"AT+QIMODE?"CRLF,mode);
	
	ret = execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_TCPIP_CONFIG_MODE_FAILED);
	}
	return ret;
}

rt_bool_t gprs_enable_multi_conn(int fd,rt_bool_t enable)
{
	char buff[20];
	rt_bool_t ret = RT_FALSE;
	sprintf(buff,"AT+QIMUX=%d"CRLF,enable);
	
	ret = execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_TCPIP_CONFIG_MUL_CONN_FAILED);
	}
	return ret;
}

rt_bool_t gprs_deact(int fd)
{
	if(!send_cmd_expect_string(fd,"AT+QIDEACT"CRLF,"DEACT OK",40*1000))
	{
		gprs_set_errcode(GPRS_DEACT_PDP_FAILED);
		return RT_FALSE;
	}
	return RT_TRUE;
}


rt_bool_t gprs_is_ip_init_state(int fd)
{
	char buff[20];
	char *pTmp = RT_NULL;
	short len = 0;
	sprintf(buff,"AT+QISTAT"CRLF);
	
	if((pTmp = send_cmd(fd,buff,GPRS_MAX_WAITTIME,&len))!=RT_NULL )
	{
		if(mystrstr(pTmp,len,"OK"))
		{
			if(mystrstr(pTmp,len,"STATE: IP INITIAL"))
				return RT_TRUE;
		}
	}
	return RT_FALSE;
}

rt_bool_t gprs_set_apn(int fd) 
{
	char buff[100]="";
	rt_bool_t ret = RT_TRUE;
	
	if(!gprs_is_ip_init_state(fd))
	{
		if(!gprs_deact(fd))
			return RT_FALSE;
	}
		
	sprintf(buff,"AT+QIREGAPP=\"%s\",\"%s\",\"%s\""CRLF,
		pConfigData->Gprs.Apn,pConfigData->Gprs.Login,pConfigData->Gprs.Password);
	
	ret = execute_cmd_expect_ok(fd,buff,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_CONFIG_APN_FAILED);
	}

	return ret;
}

rt_bool_t gprs_enable_apn(int fd)
{
	rt_bool_t ret = RT_FALSE;
	
	if(!gprs_is_ip_init_state(fd))
	{
		if(!gprs_deact(fd))
			return RT_FALSE;
	}
	
	ret = execute_cmd_expect_ok(fd,"AT+QIREGAPP"CRLF,GPRS_MAX_WAITTIME);
	if(!ret)
	{
		gprs_set_errcode(GPRS_CONFIG_APN_FAILED);
	}
	return ret;
}

rt_bool_t gprs_act(int fd)
{
	rt_bool_t ret = RT_FALSE;
	
	ret = execute_cmd_expect_ok(fd,"AT+QIACT"CRLF,150*1000);
	if(!ret)
	{
		gprs_set_errcode(GPRS_ACT_PDP_FAILED);
	}
	return ret;
}

rt_bool_t gprs_get_ip_addr(int fd,char *addr)
{
	char *recv = send_cmd_expect_string(fd,"AT+QILOCIP"CRLF,CRLF,GPRS_MAX_WAITTIME);
	
	if(rt_strstr(recv,"ERROR"))
	{
		gprs_set_errcode(GPRS_CANNOT_GET_IP);
		return RT_FALSE;
	}
	
	rt_memcpy(addr,recv,rt_strlen(recv));
	gprs_dbg(("gprs ip:%s",addr));
	return RT_TRUE;
}


PACKET_TYPE get_packet_type(unsigned char *packet,short len)
{
	if(packet[0] == 0xAA && packet[1] == 0xBB && packet[2] == 0xCC)
	{
		return GPRS_UPDATE_PACKET;
	}
	return GPRS_OTHER_PACKET;
}

rt_bool_t gprs_tcpip_connect(int fd,char sid,char *addr,unsigned short port)
{
	char buff[100] = "";
	char *recv = RT_NULL;
	
	if(gprs_get_conn_state(sid) == CONNECTED)
		return RT_TRUE;
	
	if(gprs_get_conn_state(sid) == CONNECTING)
	{
		if(gprs_left_conn_time(sid) <= 0)
		{
			if(!gprs_tcpip_close(fd,sid))
				gprs_set_errcode(GPRS_TCPIP_CLOSED_FAILED);
			
			gprs_set_conn_state(sid,NOT_CONNECTED);
			return RT_FALSE;
		}
		return RT_TRUE;
	}
	
	gprs_state[sid].conn_state.tcp_reconn_period--;				//todo:may be has bug there
	
	if(gprs_state[sid].conn_state.tcp_reconn_period <= 0)
	{
		sprintf(buff,"AT+QIOPEN=%d,\"TCP\",\"%s\",\"%d\""CRLF,sid,addr,port);
		
		init_gprs_conn_state(sid);
		gprs_set_start_conn_time(sid,rt_tick_get());
		gprs_set_conn_state(sid,CONNECTING);
		
		recv = send_tcpip_cmd_expect_string(fd,sid,buff,strlen(buff),"OK",GPRS_MAX_WAITTIME);
		if(!recv)
		{
			gprs_set_conn_errcode(sid,GPRS_TCPIP_CONNECT_FAILED);
			return RT_FALSE;
		}
	}

	return RT_TRUE;
}

short gprs_has_tcpip_data(int fd,char *sid)
{
	return gprs_readport(fd,g_recv,MAX_RECV_BUFF_SIZE,GPRS_MAX_CYCLE);
}

rt_bool_t gprs_send_tcpip_data(int fd,char sid,char *data,short len)
{
	char send[20] = "";
	
	char *pTmp = RT_NULL;
	int nsend=0,nAcked=0,nNacked=0x00FFFF;
	rt_tick_t timeout = rt_tick_from_millisecond(20000);
	rt_tick_t last_time = rt_tick_get();
	
	sprintf(send,"AT+QISEND=%d,%d"CRLF,sid,len);
	
	if(send_tcpip_cmd_expect_string(fd,sid,send,strlen(send),">",GPRS_MAX_WAITTIME) == RT_NULL)
	{
		goto out;
	}
	
	if(send_tcpip_cmd_expect_string(fd,sid,data,len,"SEND OK",GPRS_MAX_WAITTIME) == RT_NULL)
	{
		goto out;
	}
	
	dbg_hex((unsigned char*)data,len);
	
	rt_memset(send,0,sizeof(send));
	sprintf(send,"AT+QISACK=%d"CRLF,sid);
	
	while( rt_tick_get() - last_time < timeout)
	{
		if((pTmp=send_tcpip_cmd_expect_string(fd,sid,send,strlen(send),"+QISACK:",GPRS_MAX_WAITTIME)) != RT_NULL)
		{
			if( sscanf(pTmp,"+QISACK: %d,%d,%d",&nsend,&nAcked,&nNacked) >0 && (nsend - nAcked < nNacked || nNacked==0 ) )
			{
				gprs_set_errcode(GPRS_NO_ERROR);
				return RT_TRUE;
			}
		}
		rt_thread_delay(rt_tick_from_millisecond(500));						//may be not to long,500ms is enough
	}
	
out:
	gprs_tcpip_close(fd,sid);
	gprs_set_conn_errcode(sid,GPRS_TCPIP_CONNECT_FAILED);

	gprs_set_errcode(GPRS_TCPIP_SEND_FAILED);
	return RT_FALSE;
}

static rt_bool_t modbus_tcp_process(unsigned char* src,unsigned short src_len,unsigned char* dest,unsigned short *dest_len)
{
	short len = (src[4] << 8) + src[5];
	
	if(len == src_len - 6)
	{
		dest[0] = src[0];
		dest[1] = src[1];
		dest[2] = src[2];
		dest[3] = src[3];
	
		if(ModibusRtu((unsigned char*)src+6,(unsigned char*)dest+6,(unsigned short *)dest_len, 0) == 0 && *dest_len > 0)
		{
			dest[4] = (*dest_len >> 8) & 0xFF;
			dest[5] = *dest_len & 0xFF;
			*dest_len += 6;
			return RT_TRUE;
		}
	}
	return RT_FALSE;
}

void gprs_clear_tcpip_data(int fd,char sid)
{
	char buff[30] = "";
	short len = 0;
	sprintf(buff,"AT+QIRD=%d,%d,%d,%d"CRLF,0,1,sid,MAX_RECV_BUFF_SIZE);
	
	send_tcpip_cmd(fd,sid,buff,strlen(buff),GPRS_MAX_WAITTIME,&len);
}
	
rt_bool_t gprs_process_tcpip_data(int fd,char sid)
{
	char buff[30] = "";
	char *precv = RT_NULL,*ptmp = RT_NULL,*pbegin = RT_NULL,*pend = RT_NULL;
	short recv_len =0;
	short send_len=0;
	rt_bool_t ret = RT_TRUE;
	rt_bool_t reset_flag = 0;

	sprintf(buff,"AT+QIRD=%d,%d,%d,%d"CRLF,0,1,sid,MAX_RECV_BUFF_SIZE);
	
clear:
	precv = send_tcpip_cmd(fd,sid,buff,strlen(buff),GPRS_MAX_WAITTIME,&recv_len);	
	precv = gprs_state[sid].recv_buff;
	
	if(recv_len <= 0)
	{
		gprs_set_errcode(GPRS_TCPIP_RECV_TIMEOUT);
		ret = RT_FALSE;
		goto out;
	}
	
	if((ptmp = mystrtok(precv,recv_len,"TCP,")) == RT_NULL)
		goto out;

	pbegin = mystrtok(ptmp,recv_len - (ptmp-precv),CRLF);
	pend = mystrtok(pbegin,recv_len - (pbegin - precv),CRLF"OK"CRLF);
	pend -= 6;

	gprs_dbg(("sid:%d,pend:0x%4x,pbegin:0x%4x,pend - pbegin:%d\n",sid,pend,pbegin,pend - pbegin));
	dbg_hex((unsigned char*)pbegin,pend - pbegin);
	
	if(pbegin && pend && pend - pbegin > 3) 
	{
		switch(get_packet_type((unsigned char *)pbegin,pend - pbegin))
		{
		case GPRS_UPDATE_PACKET:
			reset_flag = gprs_update_process((unsigned char *)pbegin,pend - pbegin,(unsigned char*)gprs_state[sid].send_buff,&send_len);
			dbg_hex((unsigned char*)gprs_state[sid].send_buff,send_len);
			gprs_send_tcpip_data(fd,sid,(char *)gprs_state[sid].send_buff,send_len);
			if(reset_flag)
				ResetCPU();
			break;
		default:
			if(modbus_tcp_process((unsigned char *)pbegin,pend - pbegin,(unsigned char*)gprs_state[sid].send_buff,(unsigned short *)&send_len))
			{
				gprs_send_tcpip_data(fd,sid,(char *)gprs_state[sid].send_buff,send_len);
			}
			break;
		}
	}
out:
	if(recv_len == MAX_RECV_BUFF_SIZE)
		goto clear;
	return ret;
}

static void heartbeat_process(int fd,char sid)
{
	char len = 0;

	if(gprs_get_conn_state(sid) != CONNECTED) return;

	if(rt_tick_get() - gprs_state[sid].conn_state.lastalivetime < pConfigData->Gprs.Interval * 1000) return;

	len = strlen(pConfigData->Gprs.Heart);
	if(len > sizeof(pConfigData->Gprs.Heart))
		len = sizeof(pConfigData->Gprs.Heart);
	
	if(len <= 0)
		return;

	if(!gprs_send_tcpip_data(fd,sid,pConfigData->Gprs.Heart,len))
	{
		gprs_set_conn_state(sid,NOT_CONNECTED);
	}
	else
	{
		gprs_tcpip_set_lastalivetime(sid,rt_tick_get());
	}

}

static void send_register_packet(int fd,char sid)
{
	char buff[50] = {0,};
	
	if(gprs_get_conn_state(sid) != CONNECTED) return;

	if(gprs_get_first_flag(sid) == 1)
	{
		sprintf(buff,"%s",pConfigData->Gprs.Register);
		
		if(!gprs_send_tcpip_data(fd,sid,buff,strlen(buff)))
			gprs_set_conn_state(sid,NOT_CONNECTED);
		gprs_set_first_flag(sid,0);
	}
}
			
static void tcpip_conn_request(int fd,char sid)
{
	if(pConfigData->Gprs.Data[sid].Mode == 0)
	{
		unsigned int ip = htonl(pConfigData->Gprs.Data[sid].Ip);
		if(gprs_tcpip_connect(fd,sid,inet_ntoa(ip),pConfigData->Gprs.Data[sid].Port))
		{
			gprs_state[sid].conn_state.qiopen_retry_cnt = GPRS_MAX_RETRY_CONN_CNT;
			gprs_set_conn_errcode(sid,GPRS_NO_ERROR);
		}
		else
		{
			gprs_state[sid].conn_state.qiopen_retry_cnt--;
			gprs_set_conn_errcode(sid,GPRS_TCPIP_CONNECT_FAILED);
		}
	}
	else
	{
		if(gprs_tcpip_connect(fd,sid,pConfigData->Gprs.Data[sid].Dns,pConfigData->Gprs.Data[sid].Port))
		{
			gprs_state[sid].conn_state.qiopen_retry_cnt = GPRS_MAX_RETRY_CONN_CNT;
			gprs_set_conn_errcode(sid,GPRS_NO_ERROR);
		}
		else
		{
			gprs_state[sid].conn_state.qiopen_retry_cnt--;
			gprs_set_conn_errcode(sid,GPRS_TCPIP_CONNECT_FAILED);
		}
	}
}

void tcpip_process(int fd)
{
	char sid = 0;
	
	while(gprs_has_tcpip_data(fd,&sid) > 0);
	
	for(sid=0;sid<GPRS_TCPIP_MAX_CONN_CNT;sid++)
	{
		switch(gprs_get_tcpip_event(sid))
		{
			case TCPIP_HAS_DATA:
				if(gprs_process_tcpip_data(fd,sid))
					gprs_tcpip_set_lastalivetime(sid,rt_tick_get());
			break;
			case TCPIP_CLOSED:
				gprs_clear_tcpip_data(fd,sid);
				gprs_set_conn_errcode(sid,GPRS_TCPIP_CONNECT_FAILED);
				gprs_set_conn_state(sid,NOT_CONNECTED);
			break;
			case TCPIP_CONNECT_FAILED:
				gprs_set_conn_errcode(sid,GPRS_TCPIP_CONNECT_FAILED);
				if(!gprs_tcpip_close(fd,sid))
				{
					gprs_set_errcode(GPRS_TCPIP_CLOSED_FAILED);
				}
				gprs_set_conn_state(sid,NOT_CONNECTED);
			break;
			case TCPIP_CONNECTED:
				gprs_set_errcode(GPRS_NO_ERROR);
				gprs_set_conn_state(sid,CONNECTED);
				gprs_state[sid].conn_state.tcp_reconn_period = GPRS_MAX_RETRY_CONN_CNT;
				gprs_tcpip_set_lastalivetime(sid,rt_tick_get());
			break;
			default:
				break;
		}
	}
}

char get_one_num(char num)
{
	char ret = 0;
	while(num)
	{
		ret++;
		num = num & (num-1);
	}
	return ret;
}

void gprs_thread(void *lp)
{
	int fd = -1;
	int timecnt = 0;
	char ip[20] = "";
	int strength=0,errrate=100;
	char need_gprs = 0;
	char all_connect_failed_flag = 0;
	int reboot_cnt= 0;
	char sid = 0;
	
	if((fd = openport("uart5")) < 0)
	{
		gprs_set_errcode(GPRS_NO_GPRS_MODULE);
		return;
	}
	
	if(setport(fd,115200,8,1,'n') < 0)
	{
		gprs_set_errcode(GPRS_NO_GPRS_MODULE);
		return;
	}

	for(sid=0;sid<GPRS_TCPIP_MAX_CONN_CNT;sid++)
		need_gprs += pConfigData->Gprs.Data[sid].Enable;

	if(!need_gprs)
	{
		gprs_shutdown(fd);
		closeport(fd);
		
		if(pConfigData->Rola.Enable)
		{
			if(na_create_thread("zigbee",zigbeethread,RT_NULL,1024, 20, 20)<0)
				return;
		}
	
		return;
	}

reboot:
	
	reboot_cnt = reboot_cnt<60? ++reboot_cnt:reboot_cnt;
	rt_thread_delay(reboot_cnt*1000*10);
	
	gprs_init(fd);
	
	if(gprs_powup(fd))
	{
		gprs_shutdown(fd);
		rt_thread_delay(12000);
	}

	gprs_startup(fd);

	gprs_set_cmd_echo_on(fd,!GPRS_ECHO_ON);
	
	gprs_set_baudrate(fd);
	
	all_connect_failed_flag = 0;
	
	timecnt = 10;
	while(!gprs_has_sim_card(fd) && timecnt-- > 0)
	{
		rt_thread_delay(1000);
		if(timecnt <= 0)
			goto reboot;
	}
	
	timecnt = 10;
	gprs_current_network_strength(fd,&strength,&errrate);
	while(!(strength>=15 && strength< 99) && timecnt-- > 0)
	{
		rt_thread_delay(1000);
		gprs_current_network_strength(fd,&strength,&errrate);
		if(timecnt <= 0)
			goto reboot;
	}
	
	timecnt = 30;
	while(!gprs_gsm_connected(fd) && timecnt-- > 0)
	{
		rt_thread_delay(1000);
		if(timecnt <= 0)
			goto reboot;
	}
	
	timecnt = 30;
	while(!gprs_gprs_connected(fd) && timecnt-- > 0)
	{
		rt_thread_delay(1000);
		if(timecnt <= 0)
			goto reboot;
	}
	
	for(sid=0;sid<GPRS_TCPIP_MAX_CONN_CNT;sid++)
	{
		if(!pConfigData->Gprs.Data[sid].Enable)
			continue;
		gprs_config_ip_or_dns_mode(fd,pConfigData->Gprs.Data[sid].Mode);
	}
	
	gprs_show_the_other_side_ip_port(fd,0);
	
	gprs_show_ip_head(fd,0);
	
	gprs_show_ip_head_proto(fd,0);
	
	gprs_cache_recv_data(fd,1);
	
	gprs_config_fg(fd,0);
	
	gprs_set_tcpip_mode(fd,0);
	
	gprs_set_tcpip_echo_on(fd,GPRS_ECHO_ON);
	
	gprs_enable_multi_conn(fd,1);

	if(strlen(pConfigData->Gprs.Apn) > 0)
	{
		if(!gprs_set_apn(fd))
		{
			rt_thread_delay(2000);
			goto reboot;
		}
		
		if(!gprs_enable_apn(fd))
		{
			rt_thread_delay(2000);
			goto reboot;
		}
		
		timecnt = 3;
		while(!gprs_act(fd) && timecnt-- > 0)
		{
			rt_thread_delay(1000);
		}
		if(timecnt<=0)
			goto reboot;
		
		timecnt=10;
		while(!gprs_get_ip_addr(fd,ip) && timecnt-- > 0)
		{
			rt_thread_delay(2000);
		}
		if(timecnt<=0)
			goto reboot;
	}
	
	for(sid=0;sid<GPRS_TCPIP_MAX_CONN_CNT;sid++)
	{
		init_gprs_conn_state(sid);
	}
	
	while(1)
	{
		for(sid=0;sid<GPRS_TCPIP_MAX_CONN_CNT;sid++)
		{
			if(!(pConfigData->Gprs.Data[sid].Enable))
				continue;
			
			send_register_packet(fd,sid);
			
			heartbeat_process(fd,sid);
			
			tcpip_conn_request(fd,sid);
			
			if(gprs_get_errcode() == GPRS_TCPIP_CLOSED_FAILED)
				goto reboot;
		}

		for(sid=0;sid<GPRS_TCPIP_MAX_CONN_CNT;sid++)
		{
			if(gprs_get_conn_state(sid)!= CONNECTED && gprs_state[sid].conn_state.qiopen_retry_cnt <= 0)
			{
				all_connect_failed_flag |= (1<<sid);
			}
			else if(gprs_get_conn_state(sid)== CONNECTED)
			{
				all_connect_failed_flag &= (~(1<<sid));
			}
		}
		
		if(get_one_num(all_connect_failed_flag) == channel_cnt)
			goto reboot;
		
		tcpip_process(fd);
	}
	
}
