#include "sysapp.h"
#include "serial_app.h"

#include "modbus.h"
#include "get_fd.h"
#include "commgr.h"

#define COM1_STAT 257
#define COM2_STAT COM1_STAT+4

static unsigned int frame_time_out[MAX_COM_NUM];//ms

extern void Swap4(unsigned char*,unsigned char*,int);

static int Com_ProcessLetter(unsigned int id,unsigned int no, unsigned char aByte, unsigned char *revBuf)
{
	unsigned char *p=(unsigned char*)revBuf;

	if(no==1 && p[0]!=pConfigData->Com[id].Addr) return 0;
	if(no==2 && p[1]!=1 && p[1]!=2 && p[1]!=3 && p[1]!=4 && p[1]!=5 && p[1]!=6 && p[1]!=15 && p[1]!=16)
		return 0;
	if(no>=8)
	{
		if(p[1]>=1 && p[1]<=6) return 2;
		else if(p[1]==15 || p[1]==16)
		{
			if(no>=(p[6]+9)) return 2;
		}
		else if(p[1]==20 || p[1]==21)
		{
			if(no>=(p[2]+5)) return 2;
		}
	}
	return 1;
}

//com1modrtu
static unsigned int Com_ReadLetter(unsigned int id,int fd, unsigned char *pbuf, unsigned short len, unsigned short* rcvlen)
{
	unsigned char temp;
	int rc,ret;
	
	serial_device* serial_dev = RT_NULL;
	rt_device_t device = RT_NULL;
	rt_int32_t sem_wait_time = 0;
	rt_int32_t sem_interval_time = 0;
	
	*rcvlen=0;
	if(pbuf==0 || len==0) return 3;
	
	device = fd_get_dev(fd);
	if (device == RT_NULL)
	{
		rt_kprintf("%s can not find device\n",__func__);
		return 0;
	}
	
	serial_dev = fd_get_serial_device(device);
	if (serial_dev == RT_NULL)
	{
		rt_kprintf("%s can not get serial_device\n",__func__);
		return 0;
	}
	
	sem_interval_time = 100 * RT_TICK_PER_SECOND/1000;
	sem_wait_time = 5 * 1000 * RT_TICK_PER_SECOND/1000;
	
	while (1)
	{
		if (*rcvlen>0)
		{
			if(rt_sem_take(&serial_dev->rx_sem, sem_interval_time) != RT_EOK) return 4;
		}
		else
		{
			if(rt_sem_take(&serial_dev->rx_sem, sem_wait_time) != RT_EOK) return 3;
		}
		
		rc = rt_device_read(device, 0, &temp, 1);
		
		if (rc==1)
		{
			pbuf[*rcvlen]=temp;
			(*rcvlen)++;
			if((*rcvlen)>=len) return 5;
			ret=Com_ProcessLetter(id,*rcvlen, temp, pbuf);
			if(ret==0)
				*rcvlen=0;
			else if(ret==2) 
				return 0;
			continue;
		}
	}
}

static int open_and_set_com(int id)
{
	int fd = 0,ret = 0;
	char *dev ="uart4";
	switch(id)
	{
		case 0:
			dev = "uart4";
			break;
		case 1:
			dev = "uart3";
			break;
		default:
			dev = "uart5";
			break;
	}
	fd = openport(dev);     //打开串口1 
	if (fd<0)
	{
		rt_kprintf("open com%d failed!!\n",id);
		return fd;
	}
	
	if(id < 2)
	{
		if (pConfigData->Com[id].Parity==1)
			ret=setport(fd,pConfigData->Com[id].Baud,pConfigData->Com[id].DataBit,pConfigData->Com[id].StopBit,'o');
		else if (pConfigData->Com[id].Parity==2)
			ret=setport(fd,pConfigData->Com[id].Baud,pConfigData->Com[id].DataBit,pConfigData->Com[id].StopBit,'e');
		else
			ret=setport(fd,pConfigData->Com[id].Baud,pConfigData->Com[id].DataBit,pConfigData->Com[id].StopBit,'n');  //设置串口，波特率，数据位，停止位，校验
	}
	else if(id == 2)
	{
		ret=setport(fd,115200,8,1,'n');
	}
	if (ret<0)
	{		
		rt_kprintf("set com%d failed!!\n",id);
		close(fd);
		return -1;
	}
	return fd;
}

void commodrtu(void *lp)
{
	int fd;
	unsigned char lastState=0;
	rt_device_t device = RT_NULL;
	unsigned char id = *(unsigned char *)lp;
	
	SNDDEF SndDef;
	RCVDEF RcvDef;
	
	fd = open_and_set_com(id);
	
	if(fd < 0)
		return;
	
	rt_thread_delay(4);

	clearport(fd);

	lastState = Get_S(MST_M1);
	while (1)
	{
		lastState= Get_S(MST_M1);

		if (lastState==1)
		{
			SndDef.SndSize=0;	
			if (Com_ReadLetter(id,fd, RcvDef.RcvBuf, MAX_RECV_BUF, &RcvDef.RcvCount)==0 )
			{  
				if (ModibusRtu(RcvDef.RcvBuf, SndDef.SndBuf, &SndDef.SndSize, 1) == 0 && SndDef.SndSize)
				{
					device = fd_get_dev(fd);
					rt_device_write(device,0,SndDef.SndBuf, SndDef.SndSize);
				}
			}
		}
		else
			rt_thread_delay(5);
	}

//	close(fd);
//	rt_kprintf("com%d Modbus RTU thread exit\n",id);
}

/***********************************com1free***********************************/
int Com_SendMsg(unsigned int id,int fd, unsigned char *pbuf, unsigned short len)
{
	char str[64];
	rt_device_t device = RT_NULL;
	
	device = fd_get_dev(fd);
	if (device == RT_NULL)
	{
		rt_kprintf("%s can not find device\n",__func__);
		return 0;
	}
	
	if(len==0 || pbuf==0) return 6;
	if(rt_device_write(device,0,pbuf, len) != len)
	{
		sprintf(str, "COM%d : send message error (len=%d)\n",id,len);
		Printff(str);
		return 5;
	}
	return 0;
}

//com1free
int Com_RecvMsg(unsigned int id,int fd, unsigned char *pbuf, unsigned short mustlen, unsigned short * len)
{
	unsigned char temp;
	int rc;

	serial_device* serial_dev = RT_NULL;
	rt_device_t device = RT_NULL;
	rt_int32_t sem_wait_time = 0;
	rt_int32_t sem_interval_time = 0;

	device = fd_get_dev(fd);
	if (device == RT_NULL)
	{
		rt_kprintf("%s can not find device\n",__func__);
		return 0;
	}
	
	serial_dev = fd_get_serial_device(device);
	if (serial_dev == RT_NULL)
	{
		rt_kprintf("%s can not get serial_device\n",__func__);
		return 0;
	}
	
	sem_interval_time = pConfigData->Com[id].MaxInterval * RT_TICK_PER_SECOND/1000;
	sem_wait_time = pConfigData->Com[id].TimeOut * RT_TICK_PER_SECOND/1000;

	*len=0;
	while (1)
	{
		if (*len>0)
		{
			if(rt_sem_take(&serial_dev->rx_sem, sem_wait_time) != RT_EOK) return 4;
		}
		else
		{
			if(rt_sem_take(&serial_dev->rx_sem, sem_interval_time) != RT_EOK) return 3;
		}

		rc = rt_device_read(device, 0, &temp, 1);
		if (rc!=1)
		{
			continue;
		}
		if (pConfigData->Com[id].Start!=0 && *len==0 && temp!=pConfigData->Com[id].Start)
			continue;
		pbuf[*len]=temp;
		(*len)++;
		if((*len)>=pConfigData->Com[id].MaxBytes) return 5;
		if((*len)>=mustlen) return 1;
		if (pConfigData->Com[id].End!=0 && temp==pConfigData->Com[id].End)  return 2;
	}
}

//com1free
int Com_RecvMsg2(unsigned int id,int fd, unsigned char *pbuf, unsigned char adr, unsigned char code, unsigned short mustlen, unsigned short * len)
{
	unsigned char temp;
	int rc;

	serial_device* serial_dev = RT_NULL;
	rt_device_t device = RT_NULL;
	rt_int32_t sem_wait_time = 0;
	rt_int32_t sem_interval_time = 0;

	device = fd_get_dev(fd);
	if (device == RT_NULL)
	{
		rt_kprintf("%s can not find device\n",__func__);
		return 0;
	}
	
	serial_dev = fd_get_serial_device(device);
	if (serial_dev == RT_NULL)
	{
		rt_kprintf("%s can not get serial_device\n",__func__);
		return 0;
	}
	
	sem_interval_time = pConfigData->Com[id].MaxInterval * RT_TICK_PER_SECOND/1000;
	sem_wait_time = pConfigData->Com[id].TimeOut * RT_TICK_PER_SECOND/1000;

	*len=0;
	while (1)
	{
		if (*len>0)
		{
			if(rt_sem_take(&serial_dev->rx_sem, sem_wait_time) != RT_EOK) return 4;
		}
		else
		{
			if(rt_sem_take(&serial_dev->rx_sem, sem_interval_time) != RT_EOK) return 3;
		}

		rc = rt_device_read(device, 0, &temp, 1);
		
		if (rc!=1)  continue;

		if ((*len)==0 && temp!=adr)  continue;
		if ((*len)==1)
		{
			if (temp!=code && temp!=(code|0x80))
			{
				(*len)=0;
				continue;
			}
			if (temp==(code|0x80))
				mustlen=5;
		}

		pbuf[*len]=temp;
		(*len)++;
		if((*len)>=pConfigData->Com[id].MaxBytes) return 5;
		if((*len)>=mustlen) return 1;
	}
}

void comfree(void *lp)
{
	int   fd,ret=0,len;
	unsigned short rcvlen;
	unsigned char id = *(unsigned char*)lp;
	unsigned char msgbuf[256 + 1];
	unsigned char rcvbuf[MAX_RECV_BUF],sndbuf[MAX_SEND_BUF];
	unsigned int   dst_bit_start;
	unsigned short num=0,i,mustlen=0;
	unsigned char  ctmp,ctmp1;
	int rc;
	int com_send = 0;
	int com_recv = 0;

	if(id == 0)
	{  
		com_recv = COM1_RECV;
		com_send = COM1_SEND;
	}
	else
	{
		com_recv = COM2_RECV;
		com_send = COM2_SEND;
	}

	rt_thread_delay(4);//usleep(38000);

	fd = open_and_set_com(id);
	
	if(fd < 0)
		return;

	clearport(fd);

	while (1)
	{
		rt_memset(msgbuf,0,sizeof(msgbuf));
		/* 从消息队列中接收消息 */
    if (rt_mq_recv(ComQid[id], &msgbuf[0], sizeof(msgbuf), RT_WAITING_FOREVER) == RT_EOK)
		{
			len = msgbuf[0];
			rt_memmove(&msgbuf[0],&msgbuf[1],len);
		}
		
		if (len>1)
		{
			len--;
			if (msgbuf[0]==1)
			{
				pDynamicData->pSW.Value[com_send-1] = 0;
				if (Com_SendMsg(id,fd, &msgbuf[1], len)==0)  pDynamicData->pSW.Value[com_send-1] = 1;
				else pDynamicData->pSW.Value[com_send-1] = 2;
			}
			else if (msgbuf[0]==2)
			{
				pDynamicData->pSW.Value[com_recv-1] = 0;
				ret=Com_RecvMsg(id, fd, rcvbuf, msgbuf[1], &rcvlen);
				if (ret>0 && rcvlen>0)
				{
					memcpy(&dst_bit_start,&msgbuf[2],4);
					memcpy((unsigned char *)dst_bit_start,rcvbuf,rcvlen);
				}

				pDynamicData->pSW.Value[com_recv-1]=ret;
			}
			else if (msgbuf[0]==3)
			{
				memset(sndbuf,0,MAX_SEND_BUF);
				switch (msgbuf[2]) /*code*/
				{
				case 1:case 2:case 3:case 4:
					sndbuf[0]=msgbuf[1];
					sndbuf[1]=msgbuf[2];
					memcpy(&sndbuf[2],&msgbuf[4],2);
					Swap4( &sndbuf[2],&sndbuf[2],2);
					memcpy(&sndbuf[4],&msgbuf[6],2);
					Swap4( &sndbuf[4],&sndbuf[4],2);
					len=6;
					memcpy(&num,&msgbuf[6],2);
					if (msgbuf[2]==1 || msgbuf[2]==2)
						mustlen=5+(num+7)/8;
					else
						mustlen=5+num*2;
					break;

				case 5:
					sndbuf[0]=msgbuf[1];
					sndbuf[1]=msgbuf[2];
					memcpy(&sndbuf[2],&msgbuf[4],2);
					Swap4( &sndbuf[2],&sndbuf[2],2);
					ctmp=0;
					memcpy(&dst_bit_start,&msgbuf[8],4);
					if (msgbuf[3]==PARAMTYPE_NULL)
					{
						memcpy(&ctmp,(unsigned char *)dst_bit_start,1);
					}
					else
					{
						Move_Fuc(&ctmp, 1, pStaticData->pVAR[msgbuf[3]].p, dst_bit_start, 1);
					}
					if (ctmp & 0x01)
						sndbuf[4]=0xff;
					else
						sndbuf[4]=0;
					sndbuf[5]=0;
					len=6;
					mustlen=8;
					break;

				case 6:
					sndbuf[0]=msgbuf[1];
					sndbuf[1]=msgbuf[2];
					memcpy(&sndbuf[2],&msgbuf[4],2);
					Swap4( &sndbuf[2],&sndbuf[2],2);
					memcpy(&dst_bit_start,&msgbuf[8],4);
					if (msgbuf[3]==PARAMTYPE_NULL)
					{
						memcpy(&sndbuf[4],(unsigned char *)dst_bit_start,2);
					}
					else
					{
						Move_Fuc(&sndbuf[4], 1, pStaticData->pVAR[msgbuf[3]].p, dst_bit_start, 16);
					}
					Swap4( &sndbuf[4],&sndbuf[4],2);
					len=6;
					mustlen=8;
					break;

				case 15:
					sndbuf[0]=msgbuf[1];
					sndbuf[1]=msgbuf[2];
					memcpy(&sndbuf[2],&msgbuf[4],2);
					Swap4( &sndbuf[2],&sndbuf[2],2);
					memcpy(&sndbuf[4],&msgbuf[6],2);
					Swap4( &sndbuf[4],&sndbuf[4],2);
					memcpy(&num,&msgbuf[6],2);
					sndbuf[6]=(num+7)/8;
					memcpy(&dst_bit_start,&msgbuf[8],4);
					if (msgbuf[3]==PARAMTYPE_NULL)
					{
						memcpy(&sndbuf[7],(unsigned char *)dst_bit_start,sndbuf[6]);
					}
					else
					{
						Move_Fuc(&sndbuf[7], 1, pStaticData->pVAR[msgbuf[3]].p, dst_bit_start, num);
					}
					len=sndbuf[6]+7;
					mustlen=8;
					break;

				case 16:
					sndbuf[0]=msgbuf[1];
					sndbuf[1]=msgbuf[2];
					memcpy(&sndbuf[2],&msgbuf[4],2);
					Swap4( &sndbuf[2],&sndbuf[2],2);
					memcpy(&sndbuf[4],&msgbuf[6],2);
					Swap4( &sndbuf[4],&sndbuf[4],2);
					memcpy(&num,&msgbuf[6],2);
					sndbuf[6]=num*2;
					memcpy(&dst_bit_start,&msgbuf[8],4);
					if (msgbuf[3]==PARAMTYPE_NULL)
					{
						memcpy(&sndbuf[7],(unsigned char *)dst_bit_start,sndbuf[6]);
					}
					else
					{
						Move_Fuc(&sndbuf[7], 1, pStaticData->pVAR[msgbuf[3]].p, dst_bit_start, num*16);
					}
					for(i=0;i<num;i++)
						Swap4( &sndbuf[7+i*2],&sndbuf[7+i*2],2);
					len=sndbuf[6]+7;
					mustlen=8;
					break;

				}
					
				MakeCRC(sndbuf, len);
				len+=2;
				pDynamicData->pSW.Value[com_send-1] = 0;
				if (Com_SendMsg(id,fd, sndbuf, len)==0) pDynamicData->pSW.Value[com_send-1] = 1;
				else pDynamicData->pSW.Value[com_send-1] = 2;

				pDynamicData->pSW.Value[com_recv-1] = 0;
				rc=Com_RecvMsg2(id, fd, rcvbuf, sndbuf[0], sndbuf[1], mustlen, &rcvlen);
				if (rc==1 && rcvlen>=5)
				{
					if (CheckCRC(rcvbuf,rcvlen-2))
					{
						pDynamicData->pSW.Value[com_recv-1]=9;
						continue;
					}
					if (rcvbuf[1]==(sndbuf[1]|0x80))
					{
						pDynamicData->pSW.Value[com_recv-1]=8;
						continue;
					}
					if (rcvbuf[1]==5 || rcvbuf[1]==6 || rcvbuf[1]==15 || rcvbuf[1]==16)
					{
						if (rcvlen==8 && !memcmp(rcvbuf,sndbuf,6))
							pDynamicData->pSW.Value[com_recv-1]=1;
						else
							pDynamicData->pSW.Value[com_recv-1]=7;
						continue;
					}
					if (rcvbuf[1]==3 || rcvbuf[1]==4)
					{
						if (num*2==rcvbuf[2] && !memcmp(rcvbuf,sndbuf,2))
						{
							for (i=0;i<num;i++)
								Swap4( &rcvbuf[3+i*2],&rcvbuf[3+i*2],2);
							memcpy(&dst_bit_start,&msgbuf[8],4);
							if (msgbuf[3]==PARAMTYPE_NULL)
							{
								memcpy((unsigned char *)dst_bit_start,&rcvbuf[3],rcvbuf[2]);
							}
							else
							{
								Move_Fuc(pStaticData->pVAR[msgbuf[3]].p, dst_bit_start, &rcvbuf[3], 1, num*16);
							}
							pDynamicData->pSW.Value[com_recv-1]=1;
						}
						else
							pDynamicData->pSW.Value[com_recv-1]=7;
						continue;
					}
					if (rcvbuf[1]==1 || rcvbuf[1]==2)
					{
						if ((num+7)/8==rcvbuf[2] && !memcmp(rcvbuf,sndbuf,2))
						{
							memcpy(&dst_bit_start,&msgbuf[8],4);
							if (msgbuf[3]==PARAMTYPE_NULL)
							{
								if (num%8==0)
									memcpy((unsigned char *)dst_bit_start,&rcvbuf[3],rcvbuf[2]);
								else
								{
									memcpy((unsigned char *)dst_bit_start,&rcvbuf[3],rcvbuf[2]-1);
									dst_bit_start+=(rcvbuf[2]-1);
									ctmp=0;
									for (i=0;i<num%8;i++)
										ctmp|=(0x01<<i);
									ctmp1=~ctmp;
									*((unsigned char*)dst_bit_start)&=ctmp1;
									*((unsigned char*)dst_bit_start)|=(rcvbuf[3+rcvbuf[2]-1]&ctmp);
								}
							}
							else
							{
								Move_Fuc(pStaticData->pVAR[msgbuf[3]].p, dst_bit_start, &rcvbuf[3], 1, num);
							}
							pDynamicData->pSW.Value[com_recv-1]=1;
						}
						else
							pDynamicData->pSW.Value[com_recv-1]=7;
						continue;
					}
				}
				else
					pDynamicData->pSW.Value[com_recv-1] = rc;
			}
		}
	}
}

static void settimeoutvalue(int id)
{
	int onebyte_time;
	if (pConfigData->Com[id].Baud == 0)
	{
		onebyte_time = (1000 * 1000) * (1 + pConfigData->Com[id].DataBit + (pConfigData->Com[id].Parity == 0 ? 0 : 1) \
						+ pConfigData->Com[id].StopBit) / 9600;
	}
	else
	{
		onebyte_time = (1000 * 1000) * (1 + pConfigData->Com[id].DataBit + (pConfigData->Com[id].Parity == 0 ? 0 : 1) \
						+ pConfigData->Com[id].StopBit) / pConfigData->Com[id].Baud;
	}
	frame_time_out[id] = onebyte_time * 7 / 2 + 3000;
	frame_time_out[id] /= 1000;
	if (pConfigData->Com[id].Baud > 19200)
	{
		frame_time_out[id] = 4;
	}
}

unsigned char comstate[MAX_COM_NUM][64];

void commodmst(void *lp)
{
	int   fd,ret=0,i,sleepms;
	unsigned char data[256];
	unsigned char *pdst;
	unsigned int   dst_bit_start;
	unsigned char errorcount[64];
	unsigned int state;
	int maxinterval, maxwaittime;
	POINT_DEF *para;
	int com_stat = COM1_STAT;
	unsigned char id = *(unsigned char *)lp;

	fd = open_and_set_com(id);
	
	if(fd < 0)
		return;
	
	clearport(fd);

	rt_thread_delay(4);

	settimeoutvalue(id);

	sleepms=500/pConfigData->Com[id].ModNum;
	if (sleepms < frame_time_out[id]) sleepms=frame_time_out[id];
	if (sleepms<20) sleepms=20;

	memset(errorcount,0,64);
	memset(comstate[id],0,64);

	maxinterval = pConfigData->Com[id].MaxInterval;
	maxwaittime = pConfigData->Com[id].TimeOut;

	while (1)
	{
		for (i=0;i<pConfigData->Com[id].ModNum;i++)
		{
			if (pConfigData->Com[id].ModM[i].Occno>0 && pConfigData->Com[id].ModM[i].Occno<=pStaticData->MaxM)
			{
				if (Get_M(pConfigData->Com[id].ModM[i].Occno)==0)
				{
					rt_thread_delay(sleepms/10);
					continue;
				}
			}
			para = (POINT_DEF *)&(pConfigData->Com[id].ModM[i].Data.m_PointType);
			if( (pdst=GetDataPtr_Fuc( *para, &dst_bit_start ))==((void*)0) ) 
			{
				rt_thread_delay(sleepms/10);
				continue;
			}
			
			switch (pConfigData->Com[id].ModM[i].Code)
			{
			case 0x01:
				ret = MODBUS_Read_CoilStatus(fd, pConfigData->Com[id].ModM[i].Addr, pConfigData->Com[id].ModM[i].Reg, pConfigData->Com[id].ModM[i].Num, data, maxwaittime, maxinterval);
				if (!ret)
				{
					Move_Fuc( pdst, dst_bit_start, data, 1, pConfigData->Com[id].ModM[i].Num );
				}
				break;

			case 0x02:
				ret = MODBUS_Read_InputStatus(fd, pConfigData->Com[id].ModM[i].Addr, pConfigData->Com[id].ModM[i].Reg, pConfigData->Com[id].ModM[i].Num, data, maxwaittime, maxinterval);
				if (!ret)
				{
					Move_Fuc( pdst, dst_bit_start, data, 1, pConfigData->Com[id].ModM[i].Num );
				}
				break;

			case 0x03:
				ret = MODBUS_Read_HoldingRegister(fd, pConfigData->Com[id].ModM[i].Addr, pConfigData->Com[id].ModM[i].Reg, pConfigData->Com[id].ModM[i].Num, data, maxwaittime, maxinterval);
				if (!ret)
				{
					Move_Fuc( pdst, dst_bit_start, data, 1, pConfigData->Com[id].ModM[i].Num*2*8 );
				}
				break;

			case 0x04:
				ret = MODBUS_Read_InputRegister(fd, pConfigData->Com[id].ModM[i].Addr, pConfigData->Com[id].ModM[i].Reg, pConfigData->Com[id].ModM[i].Num, data, maxwaittime, maxinterval);
				if (!ret)
				{
					Move_Fuc( pdst, dst_bit_start, data, 1, pConfigData->Com[id].ModM[i].Num*2*8 );
				}
				break;

			case 0x05:
				Move_Fuc( data, 1, pdst, dst_bit_start, 1 );
				ret = MODBUS_Write_SingleCoil(fd, pConfigData->Com[id].ModM[i].Addr, pConfigData->Com[id].ModM[i].Reg, data, maxwaittime, maxinterval);
				break;

			case 0x06:
				Move_Fuc( data, 1, pdst, dst_bit_start, 2*8 );
				ret = MODBUS_Write_SingleRegister(fd, pConfigData->Com[id].ModM[i].Addr, pConfigData->Com[id].ModM[i].Reg, data, maxwaittime, maxinterval);
				break;

			case 0x0f:
				Move_Fuc( data, 1, pdst, dst_bit_start, pConfigData->Com[id].ModM[i].Num );
				ret = MODBUS_Write_MultiCoil(fd, pConfigData->Com[id].ModM[i].Addr, pConfigData->Com[id].ModM[i].Reg, pConfigData->Com[id].ModM[i].Num, data, maxwaittime, maxinterval);
				break;

			case 0x10:
				Move_Fuc( data, 1, pdst, dst_bit_start, pConfigData->Com[id].ModM[i].Num*2*8 );
				ret = MODBUS_Write_MultiRegister(fd, pConfigData->Com[id].ModM[i].Addr, pConfigData->Com[id].ModM[i].Reg, pConfigData->Com[id].ModM[i].Num, data, maxwaittime, maxinterval);
				break;

			default:
				break;
			}

			if (!ret)
			{
				if (pConfigData->Com[id].ModM[i].Occno>0 && pConfigData->Com[id].ModM[i].Occno<=pStaticData->MaxM)
				{
					Output_M(pConfigData->Com[id].ModM[i].Occno,0);
				}

				errorcount[i]=0;
				if (comstate[id][i]==1)
					printf("COM1 Modbus Master No.%d OK\n",i+1);
				comstate[id][i]=0;
			}
			else
			{
				errorcount[i]++;
				if (errorcount[i]>=3)
				{
					if (comstate[id][i]==0)
						printf("COM1 Modbus Master No.%d error\n",i+1);
					errorcount[i]=3;
					comstate[id][i]=1;
				}
			}
			rt_thread_delay(sleepms/10);
		}

		state=0;
		for (i=0;i<32;i++)
		{
			if (comstate[id][i]==1)
				state|=(1<<i);
		}
		memcpy(&pDynamicData->pSW.Value[com_stat-1],&state,4);
		state=0;
		for (i=0;i<32;i++)
		{
			if (comstate[id][i+32]==1)
				state|=(1<<i);
		}
		memcpy(&pDynamicData->pSW.Value[com_stat+1],&state,4);
	}
}
