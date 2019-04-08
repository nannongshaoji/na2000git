/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2014-04-27     Bernard      make code cleanup. 
 */

#include <board.h>
#include <rtthread.h>

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#include "stm32f4xx_eth.h"
#endif

#ifdef RT_USING_GDB
#include <gdb_stub.h>
#endif

#ifdef RT_USING_DFS
/* dfs init */
//#include <dfs_init.h>
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#include <dfs.h>
#include <dfs_file.h>
#include <dfs_posix.h>
#endif

#include "spi_flash_at45dbxx.h"

#undef TCP_SERVER_ECHO_TEST

#ifdef TCP_SERVER_ECHO_TEST
void tcp_demo_init(void);
#endif

void static make_na_rootfs()
{
	#ifdef RT_USING_DFS_ELMFAT
	rt_uint8_t has_rootfs = 0;
	/* init the elm chan FatFs filesystam*/
	elm_init();

	if (dfs_mount("flash0", "/", "elm", 0, 0) != 0)
	{
		rt_kprintf("Flash0 mount to / failed! try to format flash...\n");
		if( dfs_mkfs("elm","flash0") != 0)
		{
			rt_kprintf("Try to format flash failed\n");
		}
		else
		{
			if (dfs_mount("flash0", "/", "elm", 0, 0) != 0)
			{
				rt_kprintf("Flash0 mount to / failed!");
			}
			else
			{
				has_rootfs = 1;
			}
		}
	}
	else
	{
		has_rootfs = 1;
	}

	if(has_rootfs && access("/na",0) < 0)
	{
		mkdir("/na",0);
		mkdir("/na/scc",0);
		mkdir("/na/fbk",0);
		mkdir("/na/prg",0);
	}
#endif
}
void rt_init_thread_entry(void* parameter)
{
/* Filesystem Initialization */
#ifdef RT_USING_DFS
	{
		extern void sd_hotplug_init(void);
		extern void rt_spi_flash_device_init(void);
		extern void rt_hw_spi_init(int);
		
		rt_hw_spi_init(0);
		sd_hotplug_init();
		rt_spi_flash_device_init();
		/* init the device filesystem */
		dfs_init();
		make_na_rootfs();
	}
#endif

    /* GDB STUB */
#ifdef RT_USING_GDB
    gdb_set_device("uart6");
    gdb_start();
#endif

    /* LwIP Initialization */
#ifdef RT_USING_LWIP
    {
        extern void lwip_sys_init(void);

        /* register ethernetif device */
        eth_system_device_init();

        /* init lwip system */
        lwip_sys_init();
        rt_kprintf("TCP/IP initialized!\n");

				rt_hw_stm32_eth_init();
    }
#endif
		
#ifdef TCP_SERVER_ECHO_TEST
		tcp_demo_init();
#endif

#ifdef RT_USING_USB_HOST
    /* register stm32 usb host controller driver */
		{
			extern void rt_hw_susb_init(void);
			rt_hw_susb_init();
		}
#endif
		
		{
			rt_thread_t tid;
			extern void manage_main(void* parameter);
			/*manage thread*/
			tid = rt_thread_create("manage",manage_main, RT_NULL,1024*3, 16, 20);
			if (tid != RT_NULL)
				rt_thread_startup(tid);
		}
}

extern void rt_thread_cpld(void* parameter);
extern void RS485_EN_init(void);
extern void FSMC_SRAM_Init(void);
extern int rt_can_app_init(void);

int backup_init(void);
void init_interface(void);
void watchdog_test(void);

extern int stm32_bxcan_init(void);

int rt_application_init()
{
	rt_thread_t tid;

	/*CPLD init*/
	FSMC_SRAM_Init();
	/*485 serial3*/
	RS485_EN_init();
	/*CAN driver*/
	//rt_can_app_init();
	stm32_bxcan_init();
	/*4k back up sram init*/
	backup_init();
	/*kernel interface*/
	init_interface();

	tid = rt_thread_create("init",rt_init_thread_entry, RT_NULL,512, RT_THREAD_PRIORITY_MAX/3, 20);
	if (tid != RT_NULL)
			rt_thread_startup(tid);

	return 0;
}

#ifdef TCP_SERVER_ECHO_TEST

#include <rtthread.h>
#include <lwip/sockets.h>
#include "flash_if.h"

#define SERV_PORT		1234
#define BACKLOG 		5
#define BUFF_SIZE		128

typedef  int (*pFunction)(void);

void tcp_server(void *param)
{
	int nbytes;
	rt_uint32_t sin_size;
	int listenfd,connfd;
	struct sockaddr_in server_addr,client_addr;
	rt_bool_t stop = RT_FALSE;
	int opt;
	int i=0;
	unsigned char recv_data[BUFF_SIZE+50]={0};
	struct fd_set fds,wfds;
	struct timeval timeout={300,0};
	int ret;

	if((listenfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		rt_kprintf("create server socket falied\n");
		return;
	}

	opt = 1;
	setsockopt(listenfd,IPPROTO_TCP,TCP_NODELAY,&opt,sizeof(opt));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERV_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	rt_memset(&server_addr.sin_zero,0,sizeof(server_addr.sin_zero));

	if(bind(listenfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr)) == -1)
	{
		rt_kprintf("unable to bind\n");
		return;
	}

	if(listen(listenfd,BACKLOG) == -1)
	{
		rt_kprintf("listen error\n");
		return;
	}
	rt_kprintf("\nTCP Server waiting for client on port %4d...\n",SERV_PORT);

  while(stop != RT_TRUE)
	{
		sin_size = sizeof(struct sockaddr_in);
		if((connfd = accept(listenfd,(struct sockaddr*)&client_addr,&sin_size))<0)
		{
			rt_kprintf("failed accept:%d\n",connfd);
			continue;
		}
		else
		{
			rt_kprintf("CPU got a connection from (IP:%s,PORT:%d)\n",
				inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

			while(1)
			{
				rt_thread_delay(5);
				FD_ZERO(&fds);
				FD_ZERO(&wfds);
				FD_SET(connfd,&fds);
				FD_SET(connfd,&wfds);
				ret = select(connfd+1,&fds,&wfds,NULL,&timeout);
				if(ret < 0)
				{
					rt_kprintf("Select Error: Close Socket!!\n");
					closesocket(connfd);
					break;
				}
				else if(ret == 0)
				{
					rt_kprintf("\nTCP Server Timeout: Close Socket!!\n(IP:%s,PORT:%d)\n",
						inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
					closesocket(connfd);

					break;
				}
//				if(FD_ISSET(connfd, &wfds))
//				{
//					nbytes = send(connfd,"12345",sizeof("12345"),MSG_DONTWAIT);
//					rt_kprintf("send:%d\n",nbytes);
//				}
				if(FD_ISSET(connfd, &fds))
				{
					nbytes = recv(connfd,recv_data,1024+50,MSG_DONTWAIT);
					if(nbytes <= 0 || nbytes >1024+50)
					{
						rt_kprintf("Connection Fault:Close Socket\n");
						closesocket(connfd);
						break;
					}else{
						rt_kprintf("recv: \n");
						for(i=0;i<nbytes;i++){
							rt_kprintf("%0x ",recv_data[i]);
						}
						rt_kprintf("\n");
					}
					send(connfd,recv_data,nbytes,MSG_DONTWAIT);
				}
			}

		}
	}
	closesocket(listenfd);
}

void tcp_demo_init(void)
{
	rt_thread_t demo_thread;
	rt_kprintf("tcp_demo_init\n");
	demo_thread = rt_thread_create("tcp_demo",
		tcp_server,RT_NULL,1024,20,20);

	if(demo_thread != RT_NULL) {
		rt_kprintf("create tcp demo thread successed\n");
		rt_thread_startup(demo_thread);
	}
	return;
}

#endif



/*****************back_up 4k***************************/

int backup_init(void)
{	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM,ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	
	PWR_BackupAccessCmd(ENABLE);
	
	PWR_BackupRegulatorCmd(ENABLE);
	while(PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET);

	return 0;
}


/*@}*/
