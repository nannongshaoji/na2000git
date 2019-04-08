#include "system.h"
#include "drivers/serial.h"
#include "get_fd.h"

int openport(char *device_name)   
{
	int fd = -1;
	
	rt_device_t dev = RT_NULL;
	dev = rt_device_find(device_name);
	
	if (dev == RT_NULL){
			rt_kprintf(" can not find device: %s\n", device_name);
			return -1;
	}
	
	if (rt_device_open(dev, (RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX)) == RT_EOK)
	{
		fd = fd_put_dev(dev);
		return fd;
	}
	else{
		rt_kprintf("Cannot Open %s Serial Port!",device_name);
		return -1;
	}
}

void closeport(int fd)
{
	rt_device_t device = fd_get_dev(fd);
	if(device == RT_NULL)
		return;

	fd_close_dev(fd);
	rt_device_close(device);
}

int setport(int fd, int baud,int databits,int stopbits,int parity)
{
	struct serial_configure usart_config = RT_SERIAL_CONFIG_DEFAULT;
	rt_device_t device = RT_NULL;
	device = fd_get_dev(fd);

	if (device == RT_NULL)
	{
		rt_kprintf("%s can not find device\n",__func__);
		return -1;
	}

	switch(baud)
	{
	case 1200:
		usart_config.baud_rate = BAUD_RATE_1200;
		break;
	case 2400:
		usart_config.baud_rate = BAUD_RATE_2400;
		break;
	case 4800:
		usart_config.baud_rate = BAUD_RATE_4800;
		break;
	case 9600:
		usart_config.baud_rate = BAUD_RATE_9600;
		break;
	case 19200:
		usart_config.baud_rate = BAUD_RATE_19200;
		break;
	case 38400:
		usart_config.baud_rate = BAUD_RATE_38400;
		break;
	case 57600:
		usart_config.baud_rate = BAUD_RATE_57600;
		break;
	case 115200:
		usart_config.baud_rate = BAUD_RATE_115200;
		break;
	case 230400:
		usart_config.baud_rate = BAUD_RATE_230400;
		break;
	case 460800:
		usart_config.baud_rate = BAUD_RATE_460800;
		break;
	case 921600:
		usart_config.baud_rate = BAUD_RATE_921600;
		break;
	default :
		usart_config.baud_rate = BAUD_RATE_9600;
		break;
	}
	
	switch (databits) /*设置数据位数*/
	{
	case 5:
		usart_config.data_bits = DATA_BITS_5; //5位数据位
		break;
	case 6:
		usart_config.data_bits = DATA_BITS_6; //6位数据位
		break;
	case 7:
		usart_config.data_bits = DATA_BITS_7; //7位数据位
		break;
	case 8:
		usart_config.data_bits = DATA_BITS_8; //8位数据位
		break;
	case 9:
		usart_config.data_bits = DATA_BITS_9; //9位数据位
		break;
	default:
		usart_config.data_bits = DATA_BITS_8;
		break;
	}
	
	switch (parity) //设置校验
	{
	case 'n':
	case 'N':
		usart_config.parity = PARITY_NONE;
		break;
	case 'o':
	case 'O':
		usart_config.parity = PARITY_ODD;
		usart_config.data_bits++;
		break;
	case 'e':
	case 'E':
		usart_config.parity = PARITY_EVEN;
		usart_config.data_bits++;
		break;
	default:
		usart_config.parity = PARITY_NONE;
		break;
	}
	
	switch (stopbits)//设置停止位
	{
	case 1:
		usart_config.stop_bits = STOP_BITS_1;
		break;
	case 2:
		usart_config.stop_bits = STOP_BITS_2;
		break;
	case 3:
		usart_config.stop_bits = STOP_BITS_3;
		break;
	case 4:
		usart_config.stop_bits = STOP_BITS_4;
		break;
	default:
		usart_config.stop_bits = STOP_BITS_1;
		break;
	}

	rt_device_control(device,RT_DEVICE_CTRL_CONFIG,(void*)(&usart_config));

	return 0;
}

int readport(int fd, char *buf, int len, int maxwaittime, int maxinterval)//读数据，参数为串口，BUF，长度，超时时间
{
	rt_device_t device = RT_NULL;
	
	serial_device* serial_dev = RT_NULL;

	int no = 0;
	int rc = 1;
	rt_int32_t sem_wait_time = 0;
	rt_int32_t sem_interval_time = 0;
	
	if (maxwaittime == 0)
	{
		maxwaittime = 300;
	}
	if (maxinterval == 0)
	{
		maxinterval = 300;
	}
	
	sem_wait_time = maxwaittime * RT_TICK_PER_SECOND/1000;
	sem_interval_time = maxinterval * RT_TICK_PER_SECOND/1000;
	
	device = fd_get_dev(fd);

	serial_dev = fd_get_serial_device(device);

	if (device == RT_NULL){
		rt_kprintf(" can not find device\n");
		return -1;
	}
	
	if (serial_dev == RT_NULL){
		rt_kprintf(" can not get serial_device\n");
		return -1;
	}

	while (1)
	{
		if (no == 0)
		{
			if(rt_sem_take(&serial_dev->rx_sem, sem_wait_time) != RT_EOK)
				rc = -1;
		}
		else
		{
			if(rt_sem_take(&serial_dev->rx_sem, sem_interval_time) != RT_EOK)
				rc = -1;
		}
		if (rc<=0) return no;
		
		rc = rt_device_read(device, 0, &buf[no], 1);	//rc=read(fd,&buf[no],1);

		if(rc==1)
		{
			no=no+1;
			if(no>=len)
				return no;
		}
	}
	//return 0;
}

int writeport(int fd,char *buf,int len)  //发送数据
{
	int ret;
	rt_device_t device = RT_NULL;
	device = fd_get_dev(fd);

	if (device == RT_NULL){
		rt_kprintf(" can not find device: %s\n");
		return -1;
	}

	ret = rt_device_write(device,0,buf, len);
 	return ret; 
}

void clearport(int fd)
{
	rt_device_t device = RT_NULL;
	serial_device* serial_dev = RT_NULL;
	device = fd_get_dev(fd);
	serial_dev = fd_get_serial_device(device);
	rt_sem_control(&serial_dev->rx_sem,RT_IPC_CMD_RESET,0);
	device->control(device,TCFLSH,(void*)TCIOFLUSH);
}
