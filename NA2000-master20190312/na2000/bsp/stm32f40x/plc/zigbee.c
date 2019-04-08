#include "system.h"
#include "appcnst.h"
#include "sysapp.h"
#include <rtthread.h>

#include "zigbee.h"
#include "serial_app.h"

#include "lcd_info.h"
#include "commgr.h"

extern lcd_comm_info info;

#define ZB_STAT 265
unsigned char error_count[64]={0,};

#define ZB_DEBUG
//#undef ZB_DEBUG

#ifdef ZB_DEBUG
static void _zb_debug(const char *func,unsigned char *buff,unsigned char len)
{
	extern int GetHardSW(void);
	if(GetHardSW()==1)
	{
		rt_kprintf("[ %s ][%04d]:",func,rt_tick_get());
		int i=0;
		for(i=0;i<len;i++)
		{
			rt_kprintf("%02x ",buff[i]);
		}
		rt_kprintf("\n");
	}
}
#define zb_debug(buff,len)	_zb_debug(__func__,buff,len)
#else
#define zb_debug(buff,len)	
#endif


unsigned char calcFCS(unsigned char *pMsg, unsigned char len)
{ 
	unsigned char result = 0;
	while (len--)
	{ 
		result ^= *pMsg++;
	} 
	return result; 
}

unsigned char send_zb_data(int fd,ZB_BUFF* buff,unsigned short addr)
{
	buff->ssb.head = 0xFE;
	buff->ssb.data_len = buff->ssb.data_len + 2;
	buff->ssb.cmd[0] = 0x24;
	buff->ssb.cmd[1] = 0x5F;
	buff->ssb.addr[0] = addr & 0x00FF;
	buff->ssb.addr[1] = (addr >> 8) & 0x00FF;
	
	buff->msg[buff->ssb.data_len + 4] = calcFCS(&buff->msg[1],buff->ssb.data_len+3);
	
	zb_debug((unsigned char *)buff->ssb.data,buff->ssb.data_len-1);
	
	writeport(fd,(char *)buff->msg,buff->ssb.data_len + 5);
	return 0;
}

unsigned char* recv_zb_data(int fd,ZB_BUFF* buff,unsigned short *addr,int maxwaittime,int maxinterval)
{
	unsigned char recv_len;
	recv_len = readport(fd,(char *)buff->msg,sizeof(SZB_SEND_BLOCK),maxwaittime,maxinterval);
	
	if(recv_len > 7 && calcFCS(buff->msg+1,recv_len-2) == buff->msg[recv_len-1])
	{
		if(buff->ssb.head == 0xFE && buff->ssb.cmd[0] == 0x44 && buff->ssb.cmd[1] == 0x5F)
		{
			buff->ssb.data_len = buff->ssb.data_len - 2;
			if(addr)
				*addr = buff->ssb.addr[0] + (buff->ssb.addr[1] << 8);
			zb_debug((unsigned char *)buff->ssb.data,buff->ssb.data_len-1);
			return buff->ssb.data;
		}
	}
	return RT_NULL;
}

static unsigned char set_reg(int fd,unsigned char reg,unsigned short val)
{
	unsigned char buff[20] = {0,};
	short len = 0;
	if(pConfigData->Rola.Type == 0)	//zigbee mode
	{
		buff[0] = 0xFE;
		buff[1] = 0x03;
		buff[2] = 0x21;
		buff[3] = 0x2B;
		buff[4] = reg;
		buff[5] = val & 0x00FF;
		buff[6] = (val >> 8) & 0x00FF;
		buff[7] = calcFCS(buff+1,6);

		writeport(fd,(char*)buff,8);
	}
	else	//lora mode
	{
		buff[0] = 0xFE;
		buff[1] = 0x03;
		buff[2] = 0x20;
		buff[3] = 0x2B;

		switch(reg)
		{
			case REG_TYPE:
				sprintf((char *)(buff+4),"TYP=%d",val);
				break;
			case REG_NETWORK_ID:
				sprintf((char *)(buff+4),"PID=%d",val);
				break;
			case REG_NETWORK_ADDR:
				sprintf((char *)(buff+4),"NID=%d",val);
				break;
		}

		len = rt_strlen((char*)buff);
		buff[1] = len - 4;
		buff[len] = calcFCS(buff+1,len-1);

		writeport(fd,(char*)buff,len+1);
	}
	return 0;
}

static unsigned char is_set_reg_failed(int fd,unsigned char reg)
{
	unsigned char buff[100];
	
	readport(fd,(char *)buff,100,300,30);
	
	return 0;
}

static void entry_api_mode(int fd)
{
	writeport(fd,"===",3);
	rt_thread_delay(20);
	writeport(fd,"===",3);
	rt_thread_delay(1000);
}

static unsigned char save_reg(int fd)
{
	unsigned char buff[20];
	if(pConfigData->Rola.Type == 0)	//zigbee mode
	{
		buff[0] = 0xFE;
		buff[1] = 0x01;
		buff[2] = 0x21;
		buff[3] = 0x2C;
		buff[4] = 0x00;
		buff[5] = 0x0C;
		
		writeport(fd,(char*)buff,6);
	}
	else
	{
		buff[0] = 0xFE;
		buff[1] = 0x03;
		buff[2] = 0x20;
		buff[3] = 0x2B;
		sprintf((char *)(buff+4),"SAV");
		buff[7] = calcFCS(buff+1,6);
		
		writeport(fd,(char*)buff,8);
	}
	
	return 0;
}

static unsigned char reboot_zb(int fd)
{
	unsigned char buff[20];
	if(pConfigData->Rola.Type == 0)	//zigbee mode
	{
		buff[0] = 0xFE;
		buff[1] = 0x01;
		buff[2] = 0x21;
		buff[3] = 0x2C;
		buff[4] = 0x02;
		buff[5] = 0x0E;
		
		writeport(fd,(char*)buff,6);
	}
	else
	{
		buff[0] = 0xFE;
		buff[1] = 0x03;
		buff[2] = 0x20;
		buff[3] = 0x2B;
		sprintf((char *)(buff+4),"SRS");
		buff[7] = calcFCS(buff+1,6);
		
		writeport(fd,(char*)buff,8);
	}
	
	return 0;
}

unsigned char is_set_zb_echo_failed(int fd)
{
	return is_set_reg_failed(fd,REG_ECHO);
}

// return value:1 means success,0 failed
unsigned char set_zb_echo(int fd,unsigned short on)
{
	set_reg(fd,REG_ECHO,on);
	return !is_set_zb_echo_failed(fd);
}

unsigned char is_set_zb_mode_failed(int fd)
{
	return !is_set_reg_failed(fd,REG_MODE);
}

unsigned char set_zb_mode(int fd,unsigned short mode)
{
	set_reg(fd,REG_MODE,mode);
	return !is_set_zb_mode_failed(fd);
}

unsigned char is_set_zb_phy_channel_failed(int fd)
{
	return is_set_reg_failed(fd,REG_PHY_CHAN);
}

unsigned char set_zb_phy_channel(int fd,unsigned short channel)
{
	set_reg(fd,REG_PHY_CHAN,REG_PHY_CHAN_BEGIN+channel);
	return !is_set_zb_phy_channel_failed(fd);
}

unsigned char is_set_zb_type_failed(int fd)
{
	return is_set_reg_failed(fd,REG_TYPE);
}

unsigned char set_zb_type(int fd,unsigned short type)
{
	set_reg(fd,REG_TYPE,type);
	return !is_set_zb_type_failed(fd);
}

unsigned char is_set_zb_network_id_failed(int fd)
{
	return is_set_reg_failed(fd,REG_NETWORK_ID);
}

unsigned char set_zb_network_id(int fd,unsigned short id)
{
	set_reg(fd,REG_NETWORK_ID,REG_NETWORK_ID_BEGIN+id);
	return !is_set_zb_network_id_failed(fd);
}

unsigned char is_set_zb_network_addr_failed(int fd)
{
	return is_set_reg_failed(fd,REG_NETWORK_ADDR);;
}

unsigned char set_zb_network_addr(int fd,unsigned short addr)
{
	set_reg(fd,REG_NETWORK_ADDR,REG_NETWORK_ADDR_BEGIN+addr);
	return !is_set_zb_network_addr_failed(fd);
}

unsigned char is_auto_startup_zb_network_failed(int fd)
{
	return is_set_reg_failed(fd,REG_AUTO_STARTUP_WHEN_BOOT);
}

static int MODBUS_Read_CoilStatus(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval)
{
	ZB_BUFF sendbuf={0,},rb={0,};
	int i;
	char *recvbuf = (char *)rb.ssb.data;
	
	sendbuf.ssb.data[0]= SlaveAdd;
	sendbuf.ssb.data[1]= 0x01;
	sendbuf.ssb.data[2]= (unsigned char)(RegAdd >> 8);
	sendbuf.ssb.data[3]= (unsigned char)(RegAdd);
	sendbuf.ssb.data[4]= (unsigned char)(RegNum >> 8);
	sendbuf.ssb.data[5]= (unsigned char)RegNum;
	sendbuf.ssb.data_len = 6;
	
	clearport(fd);
	send_zb_data(fd,&sendbuf,SlaveAdd);
	
	recv_zb_data(fd,&rb,RT_NULL,maxwaittime,maxinterval);
	
	if(rb.ssb.data_len == ((RegNum+7)/8+3)) 
	{
		if(recvbuf[0]==SlaveAdd && recvbuf[1]==0x01 && recvbuf[2]==(unsigned char)((RegNum+7)/8) )
		{
				for(i=0; i< (RegNum+7)/8; i++)
				{
					value[i]=recvbuf[3+i];
				}			
				return 0;
		}
	}
	return -1;
}

static int MODBUS_Read_InputStatus(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval)
{
	ZB_BUFF sendbuf={0,},rb={0,};
	int i;
	char *recvbuf = (char *)rb.ssb.data;
	
	sendbuf.ssb.data[0]= SlaveAdd;
	sendbuf.ssb.data[1]= 0x02;
	sendbuf.ssb.data[2]= (unsigned char)(RegAdd >> 8);
	sendbuf.ssb.data[3]= (unsigned char)(RegAdd);
	sendbuf.ssb.data[4]= (unsigned char)(RegNum >> 8);
	sendbuf.ssb.data[5]= (unsigned char)RegNum;
	sendbuf.ssb.data_len = 6;
	
	clearport(fd);
	send_zb_data(fd,&sendbuf,SlaveAdd);
	
	recv_zb_data(fd,&rb,RT_NULL,maxwaittime,maxinterval);
	
	if(rb.ssb.data_len == ((RegNum+7)/8+3)) 
	{
		if(recvbuf[0]==SlaveAdd && recvbuf[1]==0x02 && recvbuf[2]==(unsigned char)((RegNum+7)/8) )
		{	
				for(i=0; i< (RegNum+7)/8; i++)
				{
					value[i]=recvbuf[3+i];
				}			
				return 0;
		}
	}
	return -1;
}

static int MODBUS_Read_HoldingRegister(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval)
{
	ZB_BUFF sendbuf={0,},rb={0,};
	int i;
	char *recvbuf = (char *)rb.ssb.data;
	
	sendbuf.ssb.data[0]= SlaveAdd;
	sendbuf.ssb.data[1]= 0x03;
	sendbuf.ssb.data[2]= (unsigned char)(RegAdd >> 8);
	sendbuf.ssb.data[3]= (unsigned char)(RegAdd);
	sendbuf.ssb.data[4]= (unsigned char)(RegNum >> 8);
	sendbuf.ssb.data[5]= (unsigned char)RegNum;
	sendbuf.ssb.data_len = 6;
	
	clearport(fd);
	send_zb_data(fd,&sendbuf,SlaveAdd);
	recv_zb_data(fd,&rb,RT_NULL,maxwaittime,maxinterval);
	
	if(rb.ssb.data_len == (RegNum*2+3)) 
	{
		if(recvbuf[0]==SlaveAdd && recvbuf[1]==0x03 && recvbuf[2]==(unsigned char)(RegNum*2))
		{
				for(i=0; i< RegNum; i++)
				{
					value[i*2]=recvbuf[3+i*2+1];
					value[i*2+1]=recvbuf[3+i*2];
				}
				return 0;		
		}
	}
	
	return -1;
}

static int MODBUS_Read_InputRegister(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval)
{
	ZB_BUFF sendbuf={0,},rb={0,};
	int i;
	char *recvbuf = (char *)rb.ssb.data;
	
	sendbuf.ssb.data[0]= SlaveAdd;
	sendbuf.ssb.data[1]= 0x04;
	sendbuf.ssb.data[2]= (unsigned char)(RegAdd >> 8);
	sendbuf.ssb.data[3]= (unsigned char)(RegAdd);
	sendbuf.ssb.data[4]= (unsigned char)(RegNum >> 8);
	sendbuf.ssb.data[5]= (unsigned char)RegNum;
	sendbuf.ssb.data_len = 6;
	
	clearport(fd);
	send_zb_data(fd,&sendbuf,SlaveAdd);
	
	recv_zb_data(fd,&rb,RT_NULL,maxwaittime,maxinterval);;
	
	if(rb.ssb.data_len == (RegNum*2+3)) 
	{
		if(recvbuf[0]==SlaveAdd && recvbuf[1]==0x04 && recvbuf[2]==(unsigned char)(RegNum*2))
		{
				for(i=0; i< RegNum; i++)
				{
					value[i*2]=recvbuf[3+i*2+1];
					value[i*2+1]=recvbuf[3+i*2];
				}
				return 0;		
		}
	}
	
	return -1;
}

static int MODBUS_Write_SingleCoil(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned char *value, int maxwaittime, int maxinterval)
{
	ZB_BUFF sendbuf={0,},rb={0,};
	
	sendbuf.ssb.data[0]= SlaveAdd;
	sendbuf.ssb.data[1]= 0x05;
	sendbuf.ssb.data[2]= (unsigned char)(RegAdd >> 8);
	sendbuf.ssb.data[3]= (unsigned char)(RegAdd);
	if (value[0] &0x01)
		sendbuf.ssb.data[4]= 0xff;
	else
		sendbuf.ssb.data[4]= 0;
	sendbuf.ssb.data[5]= 0;
	sendbuf.ssb.data_len = 6;
	
	clearport(fd);
	send_zb_data(fd,&sendbuf,SlaveAdd);
	
	recv_zb_data(fd,&rb,RT_NULL,maxwaittime,maxinterval);
	
	if(rb.ssb.data_len == 6)
	{
		if(!memcmp(sendbuf.ssb.data, rb.ssb.data, 6))
		{
			return 0;
		}
	}
	return -1;
}

static int MODBUS_Write_SingleRegister(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned char *value, int maxwaittime, int maxinterval)
{
	ZB_BUFF sendbuf={0,},rb={0,};
	
	sendbuf.ssb.data[0]= SlaveAdd;
	sendbuf.ssb.data[1]= 0x06;
	sendbuf.ssb.data[2]= (unsigned char)(RegAdd >> 8);
	sendbuf.ssb.data[3]= (unsigned char)(RegAdd);
	sendbuf.ssb.data[4]= value[1];
	sendbuf.ssb.data[5]= value[0];
	sendbuf.ssb.data_len = 6;
	
	clearport(fd);
	send_zb_data(fd,&sendbuf,SlaveAdd);
	
	recv_zb_data(fd,&rb,RT_NULL,maxwaittime,maxinterval);
	
	if(rb.ssb.data_len == 6) 
	{
		if(!memcmp(sendbuf.ssb.data, rb.ssb.data, 6))
		{
				return 0;
		}
	}
	
	return -1;
}

static int MODBUS_Write_MultiCoil(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval)
{
	ZB_BUFF sendbuf={0,},rb={0,};
	int i=0;
	
	sendbuf.ssb.data[0]= SlaveAdd;
	sendbuf.ssb.data[1]= 0x0f;
	sendbuf.ssb.data[2]= (unsigned char)(RegAdd >> 8);
	sendbuf.ssb.data[3]= (unsigned char)(RegAdd);
	sendbuf.ssb.data[4]= (unsigned char)(RegNum >> 8);
	sendbuf.ssb.data[5]= (unsigned char)RegNum;
	sendbuf.ssb.data[6]= (unsigned char)((RegNum+7)/8);
	for(i=0;i<(RegNum+7)/8;i++)
		sendbuf.ssb.data[7+i]=value[i];
	sendbuf.ssb.data_len = (RegNum+7)/8+7;
	
	clearport(fd);
	send_zb_data(fd,&sendbuf,SlaveAdd);
	
	recv_zb_data(fd,&rb,RT_NULL,maxwaittime,maxinterval);
	
	if(rb.ssb.data_len ==6) 
	{
		if(!memcmp(sendbuf.ssb.data, rb.ssb.data, 6))
		{
				return 0;
		}
	}
	
	return -1;
}

static int MODBUS_Write_MultiRegister(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval)
{
	ZB_BUFF sendbuf={0,},rb={0,};
	int i;
	
	sendbuf.ssb.data[0]= SlaveAdd;
	sendbuf.ssb.data[1]= 0x10;
	sendbuf.ssb.data[2]= (unsigned char)(RegAdd >> 8);
	sendbuf.ssb.data[3]= (unsigned char)(RegAdd);
	sendbuf.ssb.data[4]= (unsigned char)(RegNum >> 8);
	sendbuf.ssb.data[5]= (unsigned char)RegNum;
	sendbuf.ssb.data[6]= (unsigned char)(RegNum*2);
	for(i=0;i<RegNum;i++)
	{
		sendbuf.ssb.data[7+i*2]=value[i*2+1];
		sendbuf.ssb.data[8+i*2]=value[i*2];
	}
	sendbuf.ssb.data_len = RegNum*2+7;
	
	clearport(fd);
	send_zb_data(fd,&sendbuf,SlaveAdd);
	
	recv_zb_data(fd,&rb,RT_NULL,maxwaittime,maxinterval);
	
	if(rb.ssb.data_len == 6) 
	{
		if(!memcmp(sendbuf.ssb.data, rb.ssb.data, 6))
		{
				return 0;
		}
	}
	
	return -1;
}

void zb_master_process(int fd)
{
	unsigned char i = 0,ret = 0;
	int   sleepms;
	unsigned char data[80];
	unsigned char *pdst;
	int maxinterval, maxwaittime;
	POINT_DEF *para;
	unsigned int   dst_bit_start;
	sleepms=500/pConfigData->Rola.ModNum;

	maxinterval = pConfigData->Rola.Interval;
	maxwaittime = pConfigData->Rola.Timeout;
	
	for (i=0;i<pConfigData->Rola.ModNum;i++)
	{
		if (pConfigData->Rola.ModM[i].Occno>0 && pConfigData->Rola.ModM[i].Occno<=pStaticData->MaxM)
		{
			if (Get_M(pConfigData->Rola.ModM[i].Occno)==0)
			{
				rt_thread_delay(sleepms/10);
				continue;
			}
		}
		para = (POINT_DEF *)&(pConfigData->Rola.ModM[i].Data.m_PointType);
		if( (pdst=GetDataPtr_Fuc( *para, &dst_bit_start ))==((void*)0) ) 
		{
			rt_thread_delay(sleepms/10);
			continue;
		}
		
		switch (pConfigData->Rola.ModM[i].Code)
		{
		case 0x01:
			ret = MODBUS_Read_CoilStatus(fd, pConfigData->Rola.ModM[i].Addr, pConfigData->Rola.ModM[i].Reg, pConfigData->Rola.ModM[i].Num, data, maxwaittime, 100);
			if (!ret)
			{
				Move_Fuc( pdst, dst_bit_start, data, 1, pConfigData->Rola.ModM[i].Num );
			}
			break;

		case 0x02:
			ret = MODBUS_Read_InputStatus(fd, pConfigData->Rola.ModM[i].Addr, pConfigData->Rola.ModM[i].Reg, pConfigData->Rola.ModM[i].Num, data, maxwaittime, 100);
			if (!ret)
			{
				Move_Fuc( pdst, dst_bit_start, data, 1, pConfigData->Rola.ModM[i].Num );
			}
			break;

		case 0x03:
			ret = MODBUS_Read_HoldingRegister(fd, pConfigData->Rola.ModM[i].Addr, pConfigData->Rola.ModM[i].Reg, pConfigData->Rola.ModM[i].Num, data, maxwaittime, 100);
			if (!ret)
			{
				Move_Fuc( pdst, dst_bit_start, data, 1, pConfigData->Rola.ModM[i].Num*2*8 );
			}
			break;

		case 0x04:
			ret = MODBUS_Read_InputRegister(fd, pConfigData->Rola.ModM[i].Addr, pConfigData->Rola.ModM[i].Reg, pConfigData->Rola.ModM[i].Num, data, maxwaittime, 100);
			if (!ret)
			{
				Move_Fuc( pdst, dst_bit_start, data, 1, pConfigData->Rola.ModM[i].Num*2*8 );
			}
			break;

		case 0x05:
			Move_Fuc( data, 1, pdst, dst_bit_start, 1 );
			ret = MODBUS_Write_SingleCoil(fd, pConfigData->Rola.ModM[i].Addr, pConfigData->Rola.ModM[i].Reg, data, maxwaittime, 100);
			break;

		case 0x06:
			Move_Fuc( data, 1, pdst, dst_bit_start, 2*8 );
			ret = MODBUS_Write_SingleRegister(fd, pConfigData->Rola.ModM[i].Addr, pConfigData->Rola.ModM[i].Reg, data, maxwaittime, 100);
			break;

		case 0x0f:
			Move_Fuc( data, 1, pdst, dst_bit_start, pConfigData->Rola.ModM[i].Num );
			ret = MODBUS_Write_MultiCoil(fd, pConfigData->Rola.ModM[i].Addr, pConfigData->Rola.ModM[i].Reg, pConfigData->Rola.ModM[i].Num, data, maxwaittime, 100);
			break;

		case 0x10:
			Move_Fuc( data, 1, pdst, dst_bit_start, pConfigData->Rola.ModM[i].Num*2*8 );
			ret = MODBUS_Write_MultiRegister(fd, pConfigData->Rola.ModM[i].Addr, pConfigData->Rola.ModM[i].Reg, pConfigData->Rola.ModM[i].Num, data, maxwaittime, 100);
			break;

		default:
			break;
		}

		if (!ret)
		{
			if (pConfigData->Rola.ModM[i].Occno>0 && pConfigData->Rola.ModM[i].Occno<=pStaticData->MaxM)
			{
				Output_M(pConfigData->Rola.ModM[i].Occno,0);
			}

			error_count[i] = 0;
			pDynamicData->pSW.Value[ZB_STAT-1+i/16] &= ~(1 << i%16);
		}
		else
		{
			error_count[i]++;
			if (error_count[i]>=3)
			{
				error_count[i]=3;
				pDynamicData->pSW.Value[ZB_STAT-1+i/16] |= (1 << i%16);
			}
		}
		rt_thread_delay(maxinterval);
	}
}

void zb_slave_process(int fd)
{
	ZB_BUFF rcv_buff={0,};
	ZB_BUFF snd_buff={0,};
	unsigned char* pos = RT_NULL;
	unsigned short addr = 0;

	pos = recv_zb_data(fd,&rcv_buff,&addr,MAX_WAIT_TIME,MAX_INTERVAL_TIME);
	
	if(*pos != pConfigData->Rola.Addr)
		return;

	if(pos)
	{
		if (ModibusRtu(pos,snd_buff.ssb.data,(unsigned short *)&snd_buff.ssb.data_len, 0) == 0 && snd_buff.ssb.data_len)
		{
			if(snd_buff.ssb.data_len < MAX_DATA_LEN)
				send_zb_data(fd,&snd_buff,addr);
		}
	}
}

typedef void (*zb_process)(int fd);

void zigbeethread(void *lp)
{
	int   fd;
	zb_process process = RT_NULL;

	if((fd = openport("uart5")) < 0)
		return;

	if(setport(fd,115200,8,1,'n') < 0)
		return;
	
	entry_api_mode(fd);
	
	if(pConfigData->Rola.Type == 0)	//zigbee mode
	{
		set_zb_echo(fd,0);
		set_zb_phy_channel(fd,pConfigData->Rola.Chid);

		if(pConfigData->Rola.Protocol == 0)
		{
			switch(pConfigData->Rola.Master)
			{
				case 0:
					set_zb_type(fd,REG_TYPE_ROUTER);
					process = zb_slave_process;
					break;
				case 1:
					set_zb_type(fd,REG_TYPE_COORD);
					process = zb_master_process;
					break;
				default:
					return;
			}
		}
	}
	else	//lora mode
	{
		if(pConfigData->Rola.Protocol == 0)
		{
			switch(pConfigData->Rola.Master)
			{
				case 0:
					set_zb_type(fd,REG_TYPE_TTY);
					process = zb_slave_process;
					break;
				case 1:
					set_zb_type(fd,REG_TYPE_ROUTER);
					process = zb_master_process;
					break;
				default:
					return;
			}
		}
	}

	set_zb_network_id(fd,pConfigData->Rola.Netid);
	set_zb_network_addr(fd,pConfigData->Rola.Addr);
	save_reg(fd);
	reboot_zb(fd);
	
	rt_thread_delay(10000);
	
	while(1)
	{
		if(process)
			process(fd);
		rt_thread_delay(10);
	}
}

