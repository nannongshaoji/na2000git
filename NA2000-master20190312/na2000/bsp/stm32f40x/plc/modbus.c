#include "serial_app.h"
#include "sysapp.h"
#include <string.h>

int MODBUS_Read_CoilStatus(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval)
{
	char sendbuf[256],recvbuf[256];
	unsigned short crc16;
	unsigned short crctmp;
	int rc,i;
	
	sendbuf[0]= SlaveAdd;
	sendbuf[1]= 0x01;
	sendbuf[2]= (unsigned char)(RegAdd >> 8);
	sendbuf[3]= (unsigned char)(RegAdd);
	sendbuf[4]= (unsigned char)(RegNum >> 8);
	sendbuf[5]= (unsigned char)RegNum;
	crc16= CRC16((unsigned char*)sendbuf, 6);
	sendbuf[6]= (unsigned char)crc16;
	sendbuf[7]= (unsigned char)(crc16 >> 8);
	clearport(fd);
	writeport(fd,sendbuf,8);
	rc = readport(fd, recvbuf, (RegNum + 7) / 8 + 5, maxwaittime, maxinterval);
	if(rc==((RegNum+7)/8+5)) 
	{
		if(recvbuf[0]==SlaveAdd && recvbuf[1]==0x01 && recvbuf[2]==(unsigned char)((RegNum+7)/8) )
		{
			crc16= CRC16((unsigned char *)recvbuf, 3+(RegNum+7)/8);
			crctmp= (recvbuf[rc-1]<<8) | recvbuf[rc-2];
			if(crc16==crctmp )
			{
				for(i=0; i< (RegNum+7)/8; i++)
				{
					value[i]=recvbuf[3+i];
				}			
				return 0;					
			}
			else return -1;
		}
		else return -2;
	}
	else return -3;
}

int MODBUS_Read_InputStatus(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval)
{
	char sendbuf[256],recvbuf[256];
	unsigned short crc16;
	unsigned short crctmp;
	int rc,i;
	
	sendbuf[0]= SlaveAdd;
	sendbuf[1]= 0x02;
	sendbuf[2]= (unsigned char)(RegAdd >> 8);
	sendbuf[3]= (unsigned char)(RegAdd);
	sendbuf[4]= (unsigned char)(RegNum >> 8);
	sendbuf[5]= (unsigned char)RegNum;
	crc16= CRC16((unsigned char*)sendbuf, 6);
	sendbuf[6]= (unsigned char)crc16;
	sendbuf[7]= (unsigned char)(crc16 >> 8);
	clearport(fd);
	writeport(fd,sendbuf,8);
	rc = readport(fd, recvbuf, (RegNum + 7) / 8 + 5, maxwaittime, maxinterval);
	if(rc==((RegNum+7)/8+5)) 
	{
		if(recvbuf[0]==SlaveAdd && recvbuf[1]==0x02 && recvbuf[2]==(unsigned char)((RegNum+7)/8) )
		{
			crc16= CRC16((unsigned char*)recvbuf, 3+(RegNum+7)/8);
			crctmp= (recvbuf[rc-1]<<8) | recvbuf[rc-2];
			if(crc16==crctmp )
			{
				for(i=0; i< (RegNum+7)/8; i++)
				{
					value[i]=recvbuf[3+i];
				}			
				return 0;					
			}
			else return -1;
		}
		else return -2;
	}
	else return -3;
}

int MODBUS_Read_HoldingRegister(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval)
{
	char sendbuf[256],recvbuf[256];
	unsigned short crc16;
	unsigned short crctmp;
	int rc,i;
	
	sendbuf[0]= SlaveAdd;
	sendbuf[1]= 0x03;
	sendbuf[2]= (unsigned char)(RegAdd >> 8);
	sendbuf[3]= (unsigned char)(RegAdd);
	sendbuf[4]= (unsigned char)(RegNum >> 8);
	sendbuf[5]= (unsigned char)RegNum;
	crc16= CRC16((unsigned char*)sendbuf, 6);
	sendbuf[6]= (unsigned char)crc16;
	sendbuf[7]= (unsigned char)(crc16 >> 8);
	clearport(fd);
	writeport(fd,sendbuf,8);
	rc = readport(fd, recvbuf, RegNum * 2 + 5, maxwaittime, maxinterval);
	if(rc==(RegNum*2+5)) 
	{
		if(recvbuf[0]==SlaveAdd && recvbuf[1]==0x03 && recvbuf[2]==(unsigned char)(RegNum*2))
		{
			crc16= CRC16((unsigned char*)recvbuf, 3+(RegNum*2));
			crctmp= (recvbuf[rc-1]<<8) | recvbuf[rc-2];
			if(crc16==crctmp )
			{
				for(i=0; i< RegNum; i++)
				{
					value[i*2]=recvbuf[3+i*2+1];
					value[i*2+1]=recvbuf[3+i*2];
				}
				return 0;					
			}
			else return -1;
		}
		else return -2;
	}
	else return -3;
}

int MODBUS_Read_InputRegister(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval)
{
	char sendbuf[256],recvbuf[256];
	unsigned short crc16;
	unsigned short crctmp;
	int rc,i;
	
	sendbuf[0]= SlaveAdd;
	sendbuf[1]= 0x04;
	sendbuf[2]= (unsigned char)(RegAdd >> 8);
	sendbuf[3]= (unsigned char)(RegAdd);
	sendbuf[4]= (unsigned char)(RegNum >> 8);
	sendbuf[5]= (unsigned char)RegNum;
	crc16= CRC16((unsigned char*)sendbuf, 6);
	sendbuf[6]= (unsigned char)crc16;
	sendbuf[7]= (unsigned char)(crc16 >> 8);
	clearport(fd);
	writeport(fd,sendbuf,8);
	rc = readport(fd, recvbuf, RegNum * 2 + 5, maxwaittime, maxinterval);
	if(rc==(RegNum*2+5)) 
	{
		if(recvbuf[0]==SlaveAdd && recvbuf[1]==0x04 && recvbuf[2]==(unsigned char)(RegNum*2))
		{
			crc16= CRC16((unsigned char*)recvbuf, 3+(RegNum*2));
			crctmp= (recvbuf[rc-1]<<8) | recvbuf[rc-2];
			if(crc16==crctmp )
			{
				for(i=0; i< RegNum; i++)
				{
					value[i*2]=recvbuf[3+i*2+1];
					value[i*2+1]=recvbuf[3+i*2];
				}
				return 0;					
			}
			else return -1;
		}
		else return -2;
	}
	else return -3;
}

int MODBUS_Write_SingleCoil(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned char *value, int maxwaittime, int maxinterval)
{
	char sendbuf[256],recvbuf[256];
	unsigned short crc16;
	int rc;
	
	sendbuf[0]= SlaveAdd;
	sendbuf[1]= 0x05;
	sendbuf[2]= (unsigned char)(RegAdd >> 8);
	sendbuf[3]= (unsigned char)(RegAdd);
	if (value[0] &0x01)
		sendbuf[4]= 0xff;
	else
		sendbuf[4]= 0;
	sendbuf[5]= 0;
	crc16= CRC16((unsigned char*)sendbuf, 6);
	sendbuf[6]= (unsigned char)crc16;
	sendbuf[7]= (unsigned char)(crc16 >> 8);
	
	clearport(fd);
	writeport(fd,sendbuf,8);
	rc = readport(fd, recvbuf, 8, maxwaittime, maxinterval);
	if(rc==8) 
	{
		if(!memcmp(sendbuf, recvbuf, 8))
		{
			return 0;
		}
		else return -1;
	}
	else return -3;
}

int MODBUS_Write_SingleRegister(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned char *value, int maxwaittime, int maxinterval)
{
	char sendbuf[256],recvbuf[256];
	unsigned short crc16;
	int rc;
	
	sendbuf[0]= SlaveAdd;
	sendbuf[1]= 0x06;
	sendbuf[2]= (unsigned char)(RegAdd >> 8);
	sendbuf[3]= (unsigned char)(RegAdd);
	sendbuf[4]= value[1];
	sendbuf[5]= value[0];
	crc16= CRC16((unsigned char*)sendbuf, 6);
	sendbuf[6]= (unsigned char)crc16;
	sendbuf[7]= (unsigned char)(crc16 >> 8);
	
	clearport(fd);
	writeport(fd,sendbuf,8);
	rc = readport(fd, recvbuf, 8, maxwaittime, maxinterval);
	if(rc==8) 
	{
		if(!memcmp(sendbuf, recvbuf, 8))
		{
			return 0;
		}
		else return -1;
	}
	else return -3;
}

int MODBUS_Write_MultiCoil(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval)
{
	char sendbuf[256],recvbuf[256];
	unsigned short crc16;
	unsigned short crctmp;
	int rc,i;
	
	sendbuf[0]= SlaveAdd;
	sendbuf[1]= 0x0f;
	sendbuf[2]= (unsigned char)(RegAdd >> 8);
	sendbuf[3]= (unsigned char)(RegAdd);
	sendbuf[4]= (unsigned char)(RegNum >> 8);
	sendbuf[5]= (unsigned char)RegNum;
	sendbuf[6]= (unsigned char)((RegNum+7)/8);
	for(i=0;i<(RegNum+7)/8;i++)
		sendbuf[7+i]=value[i];
	crc16= CRC16((unsigned char*)sendbuf, (RegNum+7)/8+7);
	sendbuf[(RegNum+7)/8+7]= (unsigned char)crc16;
	sendbuf[(RegNum+7)/8+8]= (unsigned char)(crc16 >> 8);
	
	clearport(fd);
	writeport(fd,sendbuf,(RegNum+7)/8+9);
	rc = readport(fd, recvbuf, 8, maxwaittime, maxinterval);
	if(rc==8) 
	{
		if(!memcmp(sendbuf, recvbuf, 6))
		{
			crc16= CRC16((unsigned char*)recvbuf, 6);
			crctmp= (recvbuf[7]<<8) | recvbuf[6];
			if(crc16== crctmp )
				return 0;
			else return -1;
		}
		else return -2;
	}
	else return -3;
}

int MODBUS_Write_MultiRegister(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval)
{
	char sendbuf[256],recvbuf[256];
	unsigned short crc16;
	unsigned short crctmp;
	int rc,i;
	
	sendbuf[0]= SlaveAdd;
	sendbuf[1]= 0x10;
	sendbuf[2]= (unsigned char)(RegAdd >> 8);
	sendbuf[3]= (unsigned char)(RegAdd);
	sendbuf[4]= (unsigned char)(RegNum >> 8);
	sendbuf[5]= (unsigned char)RegNum;
	sendbuf[6]= (unsigned char)(RegNum*2);
	for(i=0;i<RegNum;i++)
	{
		sendbuf[7+i*2]=value[i*2+1];
		sendbuf[8+i*2]=value[i*2];
	}
	crc16= CRC16((unsigned char *)sendbuf, RegNum*2+7);
	sendbuf[RegNum*2+7]= (unsigned char)crc16;
	sendbuf[RegNum*2+8]= (unsigned char)(crc16 >> 8);
	
	clearport(fd);
	writeport(fd,sendbuf,RegNum*2+9);
	rc = readport(fd, recvbuf, 8, maxwaittime, maxinterval);
	if(rc==8) 
	{
		if(!memcmp(sendbuf, recvbuf, 6))
		{
			crc16= CRC16((unsigned char*)recvbuf, 6);
			crctmp= (recvbuf[7]<<8) | recvbuf[6];
			if(crc16== crctmp )
				return 0;
			else return -1;
		}
		else return -2;
	}
	else return -3;
}

