

#ifndef __SERIAL_FD__
#define __SERIAL_FD__

//#include "sysapp.h"

#include <rtthread.h>


#define offsetof1(s, m)   (long)&(((s *)0)->m)
	
#define container_of(ptr, type, member) (type *)( (long)ptr - offsetof1(type,member) )
	

#define NO_USED 0
#define BE_USED 1

typedef struct _serial_dev{
	rt_device_t dev;
	rt_uint8_t used;
	struct rt_semaphore  rx_sem;
}serial_device;


extern int serial_fd_init(void);
extern rt_device_t fd_get_dev(int fd);
extern int fd_put_dev(rt_device_t dev);
extern void fd_close_dev(int fd);
extern serial_device* fd_get_serial_device(rt_device_t dev);

#endif

