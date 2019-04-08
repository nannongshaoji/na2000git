
#include "get_fd.h"

#include "sysapp.h"


#define SERIAL_MAX_NUM 4

serial_device serial_dev[SERIAL_MAX_NUM];

int serial_fd_init()
{
	char i = 0;
	char sem_name[8];
	
	for(i = 0;i<SERIAL_MAX_NUM;i++)
	{
		sprintf(sem_name, "ssem%d",i);
		
		serial_dev[i].used = NO_USED;	
		serial_dev[i].dev = RT_NULL;
		rt_sem_init(&serial_dev[i].rx_sem,sem_name,0,RT_IPC_FLAG_FIFO);
	}
	return 0;
}

serial_device* fd_get_serial_device(rt_device_t dev)
{
	char i = 0;
	for(i=0;i<SERIAL_MAX_NUM;i++)
	{
		if(serial_dev[i].dev == dev && serial_dev[i].used == BE_USED)
		{
			return &serial_dev[i];
		}
	}
	return RT_NULL;
}


rt_device_t fd_get_dev(int fd)
{
	rt_device_t dev =RT_NULL;
	
	if(serial_dev[fd].dev != RT_NULL && serial_dev[fd].used == BE_USED)
	{
		dev = serial_dev[fd].dev;
		return dev;
	}
	return RT_NULL;
}



int fd_put_dev(rt_device_t dev)
{
	char i = 0;
	for(i = 0;i<SERIAL_MAX_NUM;i++)
	{
		if(serial_dev[i].used == NO_USED )
		{
			serial_dev[i].dev = dev;
			serial_dev[i].used = BE_USED;
			return i;
		}
	}
	return -1;
}

void fd_close_dev(int fd)
{
	serial_dev[fd].used = NO_USED;
	serial_dev[fd].dev = RT_NULL;
}
