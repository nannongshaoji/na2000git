#include "lcd_info.h"
#include "sysapp.h"
#include "serial_app.h"

#include "modbus.h"

#define LCD_MAX_INTERVAL				10		//10ms
#define LCD_MAX_WAITTIME				50		//50ms

lcd_comm_info info;
static int   fd = -1;
static unsigned char reset_flag = 0;
static unsigned char lcd_thread_stopped = 1;
	
void reset_lcd()
{
	char reset_cnt = 20;
	lcd_cmd 	cmd = {0,};
	
	info.cmd.reset_cmd_from_cpu = 1;
	reset_flag = 1;
	
	while(!lcd_thread_stopped)
	{
		rt_thread_delay(10);
	}
	
	if(fd >= 0)
	{
		MODBUS_Write_MultiRegister(fd,1,0,1,(unsigned char *)&info.cmd.resv,LCD_MAX_WAITTIME,LCD_MAX_INTERVAL);
		while(reset_cnt-- > 0)
		{
			rt_thread_delay(20);
			MODBUS_Read_HoldingRegister(fd,1,0,1,(unsigned char *)&cmd,LCD_MAX_WAITTIME,LCD_MAX_INTERVAL);
			if(cmd.reset_cmd_from_cpu == 0)
				break;
		}
		clearport(fd);
		fd = -1;
	}
}

void commodmst_for_lcd(void *lp)
{
	unsigned char id = *(unsigned char *)lp;
	lcd_cmd 	cmd = {0,};
	/*
	temp_arr[0~1] = year
	temp_arr[2~3] = month 
	temp_arr[4~5] = day
	temp_arr[6~7] = hour
	temp_arr[8~9] = min
	temp_arr[10~11] = sec
	temp_arr[12~15] = 温度
	*/
	
	char *dev ="uart1";
	fd = openport(dev);     //打开串口1 
	if (fd<0)
	{
		rt_kprintf("open com%d failed!!\n",id);
		return ;
	}
	
	setport(fd,115200,8,1,'n');

	info.cmd.reset_cmd_from_lcd = 0;
	info.cmd.reset_cmd_from_cpu = 0;
	
	lcd_thread_stopped = 0;
	reset_flag = 0;
	
	while (!reset_flag)
	{
		info.data.year 	= pDynamicData->pSW.Value[0];
		info.data.month 	= pDynamicData->pSW.Value[1];
		info.data.day 		= pDynamicData->pSW.Value[2];
		info.data.hour 	= pDynamicData->pSW.Value[3];
		info.data.min 		= pDynamicData->pSW.Value[4];
		info.data.sec 		= pDynamicData->pSW.Value[5];
	
		info.data.ipaddr = pStaticData->pMDU[0].EthAddr;
		memcpy(&info.data.mask,&pStaticData->pMDU[0].ComDef[0],4);
		memcpy(&info.data.gateway,&pStaticData->pMDU[0].ComDef[1],4);
	
		MODBUS_Read_HoldingRegister(fd,1,0,1,(unsigned char *)&cmd,LCD_MAX_WAITTIME,LCD_MAX_INTERVAL);
		
		if(cmd.reset_cmd_from_lcd == 1)
		{
			cmd.reset_cmd_from_lcd = 0;
			MODBUS_Write_MultiRegister(fd,1,0,1,(unsigned char *)&cmd,LCD_MAX_WAITTIME,LCD_MAX_INTERVAL);
			rt_thread_delay(1);
			MODBUS_Write_MultiRegister(fd,1,0,1,(unsigned char *)&cmd,LCD_MAX_WAITTIME,LCD_MAX_INTERVAL);
			reset_flag = 1;
		}
	
		MODBUS_Write_MultiRegister(fd,1,2,sizeof(info.data),(unsigned char *)&info.data,LCD_MAX_WAITTIME,LCD_MAX_INTERVAL);

		if(reset_flag)
		{
			char buff[2] = {0,0x81};
			rt_mq_send(MsgQid, buff, sizeof(buff));
			rt_thread_delay(1);
		}
		else
		{
			rt_thread_delay(300 * RT_TICK_PER_SECOND/1000);
		}
	}

	lcd_thread_stopped = 1;
	printf("com%d Modbus Master thread exit\n",id+1);
	return;
}
