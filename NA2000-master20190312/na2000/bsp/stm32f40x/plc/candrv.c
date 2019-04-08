#include "candrv.h"
#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>

struct can_dev
{
	rt_device_t candev;
	Receive_Buf_Struc receive_buf[rec_buf_cont];
	struct rt_semaphore sem;
	BYTE serial;
	struct rt_can_msg msg;
//	pthread_t timeout_thread_id;
};

static struct can_dev candev;
	
void rt_can_timeout_thread(void *arg);

static rt_err_t can1ind(rt_device_t dev,rt_size_t size)
{
    rt_sem_release(&candev.sem);
    return RT_EOK;
}

int CANInit(int id)
{
	rt_thread_t tid;
	
	candev.candev = rt_device_find("bxcan1");
	RT_ASSERT(candev.candev);
	rt_sem_init(&candev.sem, "bxcan1", 0, RT_IPC_FLAG_FIFO);
	rt_device_open(candev.candev, (RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX));
	rt_device_set_rx_indicate(candev.candev, can1ind);
	
	tid = rt_thread_create("canto",
												 rt_can_timeout_thread, RT_NULL,
												 256, RT_THREAD_PRIORITY_MAX / 3 - 1, 20);
	if (tid != RT_NULL) rt_thread_startup(tid);

	return 0;
}

static US find_empty(Receive_Buf_Struc *receive_buf,BYTE serial,BYTE source_node)
{
	US i = 0;
	US  empty_num = 0xffff;
	for (i = 0; i < rec_buf_cont; i++)
	{
		if ((receive_buf[i].status == assembling)
				&& (receive_buf[i].serial == serial)
				&& (receive_buf[i].source_node == source_node))
		{
			empty_num = i; break;
		}
	}
	return empty_num;
}

static int packet(int port,Receive_Msg_Struct* buff)
{
	BYTE len = 0;
	BYTE frag_type = 0;
	BYTE serial,dest_node,source_node;
	US  empty_num;
	US  frag_ptr=0,frag_count = 0;
	int i=0;
	
	rt_can_msg_t msg = &candev.msg;
	
	Receive_Buf_Struc *receive_buf = candev.receive_buf;

	serial 			= (msg->id & 0x0000000F);
	dest_node 	= (msg->id & 0x0FE00000) >> 21;
	source_node = (msg->id & 0x000FE000) >> 13;
	frag_type 	= ((msg->id & 0x10000000)>>27) + ((msg->id&0x00100000)>>20);
	frag_count 	= (msg->id & 0x00001FF0) >> 4;
	
	len = msg->len;

	if(frag_type == 0x00 && frag_count == 0x1ff)
	{
		buff->msg_size =  len;
		buff->source_node = source_node;
		rt_memcpy(buff->data,msg->data,len);

		return 1;
	}

	if(frag_type == 1 && frag_count == 0)
	{
		empty_num = find_empty(receive_buf,serial,source_node);

		if (empty_num == 0xffff)
		{
			for (i = 0; i < rec_buf_cont; i++)
				if (receive_buf[i].status == nul)
				{
					empty_num = i;
					break;
				}
		}

		if (empty_num >= rec_buf_cont)
		{
			return 0;
		}

		receive_buf[empty_num].port 				= port;
		receive_buf[empty_num].target_node 	= dest_node;
		receive_buf[empty_num].source_node 	= source_node;
		receive_buf[empty_num].serial 			= serial;
		receive_buf[empty_num].msg_size 		= len;
		receive_buf[empty_num].status 			= assembling;
		receive_buf[empty_num].frag_count 	= 0;
		receive_buf[empty_num].timer 				= 5;
		
		rt_memcpy(&receive_buf[empty_num].data[0],msg->data, len);

		return 0;
	}

	if(frag_type == 2 )
	{
		empty_num = find_empty(receive_buf,serial,source_node);

		if (empty_num >= rec_buf_cont)
		{
			return 0;
		}
		
		if (frag_count != receive_buf[empty_num].frag_count+1)
		{
			return 0;
		}

		frag_ptr = receive_buf[empty_num].msg_size;
		receive_buf[empty_num].msg_size 	+= len;
		receive_buf[empty_num].frag_count = frag_count;
		
		rt_memcpy(&receive_buf[empty_num].data[frag_ptr], msg->data, len);

		return 0;
	}

	if(frag_type == 3) 
	{
		empty_num = find_empty(receive_buf,serial,source_node);
		
		if (empty_num >= rec_buf_cont)
		{
			return 0;
		}
		if (frag_count != receive_buf[empty_num].frag_count+1)
		{
			return 0;
		}

		frag_ptr = receive_buf[empty_num].msg_size;
		receive_buf[empty_num].msg_size += len;
		receive_buf[empty_num].status 	= completed;
		receive_buf[empty_num].timer 		= 0xff;
		
		rt_memcpy(&receive_buf[empty_num].data[frag_ptr], msg->data, len);

		buff->source_node 							= receive_buf[empty_num].source_node;
		buff->msg_size 									= receive_buf[empty_num].msg_size;
		receive_buf[empty_num].timer 		= 0xff;
		receive_buf[empty_num].status 	= nul;
		
		rt_memcpy(buff->data,receive_buf[empty_num].data,receive_buf[empty_num].msg_size);

		return 1;
	}

	return 1;
}

int CANAppRead(
	int port, 						/* CAN1/CAN2: PORT1/PORT2 */
	unsigned long flags, 			/* Q_NOWAIT / Q_WAIT */
	int		 timeout, 				/* when flags is Q_WAIT, timeout is valid */
	Receive_Msg_Struct  *msgbuf,	/* message buffer */
	unsigned long *msg_len			/* length of message */
	)
{
	int read_completed = 0;

	*msg_len = 0;
	rt_memset(msgbuf,0,sizeof(*msgbuf));

	if(flags == 1)
	{
		while(rt_device_read(candev.candev, 0, &candev.msg, sizeof(candev.msg)) > 0)
			read_completed=packet(port,msgbuf);
	}
	else
	{
		while(rt_sem_take(&candev.sem, timeout) == RT_EOK)
		{
			while(rt_device_read(candev.candev, 0, &candev.msg, sizeof(candev.msg)) > 0)
				read_completed=packet(port,msgbuf);
			if(read_completed)
				goto out;
		}
	}

out:
	rt_sem_init(&candev.sem, "bxcan1", 0, RT_IPC_FLAG_FIFO);
	*msg_len = msgbuf->msg_size + 4;
	return !read_completed;
}

#define CAN_RESEND_NUM 1

int CANAppWrite(int port, Send_Msg_Struct *pSendMsg)
{
	int ret;
	struct rt_can_msg msg;

	BYTE can_addr = 1;  //TODO
	BYTE serial = candev.serial++;

	int index = pSendMsg->msg_size;

	int i_frame=0;
	msg.ide = 1;

	while(index > 0)
	{
		if(pSendMsg->msg_size > 8)
		{
			if(index == pSendMsg->msg_size)
			{ //first frame
				msg.len = 8;
				
				BYTE b1 = (pSendMsg->destination_node & 0x7f) ;
				BYTE b2 = 0x80 | can_addr;
				BYTE b3 = 0x00;
				BYTE b4 = ((serial & 0x0f) << 3);
				msg.id = (b1 << 21) | b2 <<13 | b3 << 5 | b4 >> 3;
				msg.id |= 0x80000000;
				msg.rtr = 0;
				
				rt_memcpy(msg.data,pSendMsg->data,8);
			}
			else if(index<=8) 
			{
				msg.len = index;
				
				BYTE b1 = (pSendMsg->destination_node & 0x7f) | 0x80;
				BYTE b2 = 0x80 | can_addr;
				BYTE b3 = (i_frame>>1); 
				BYTE b4 = (((serial & 0x0f) << 3 ) | (( i_frame & 0x1) << 7));
				msg.id = (b1 << 21) | b2 <<13 | b3 << 5 | b4 >> 3;
				msg.id |= 0x80000000;
				msg.rtr = 0;
				
				rt_memcpy(msg.data,pSendMsg->data+i_frame*8,index);
			}
			else 
			{
				msg.len = 8;
				
				BYTE b1 = (pSendMsg->destination_node & 0x7f) | 0x80;
				BYTE b2 = 0x0 | can_addr;
				BYTE b3 = (i_frame>>1); 
				BYTE b4 = (((serial & 0x0f) << 3 ) | (( i_frame & 0x1) << 7)) ;
				msg.id = (b1 << 21) | b2 <<13 | b3 << 5 | b4 >> 3;
				msg.id |= 0x80000000;
				msg.rtr = 0;
				
				rt_memcpy(msg.data,pSendMsg->data+i_frame*8,8);
			}
		}
		else
		{
			msg.len = index;
			
			BYTE b1 = (pSendMsg->destination_node& 0x7f) ;
			BYTE b2 = 0x0 | can_addr;
			BYTE b3 = 0xFF;
			BYTE b4 = 0x80;
			msg.id = (b1 << 21) | b2 <<13 | b3 << 5 | b4 >> 3;
			msg.id |= 0x80000000;
			msg.rtr = 0;

			rt_memcpy(msg.data,pSendMsg->data,pSendMsg->msg_size);
		}

		ret = rt_device_write(candev.candev, 0, &msg, sizeof(msg));
		if(ret > 0) 
		{
			i_frame += 1;
			index -= 8;
		}
		else
		{
			rt_device_close(candev.candev);
			candev.candev = rt_device_find("bxcan1");
			rt_sem_init(&candev.sem, "bxcan1", 0, RT_IPC_FLAG_FIFO);
			rt_device_open(candev.candev, (RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX));
			rt_device_set_rx_indicate(candev.candev, can1ind);
			break;
		}
	}
	
	return ret > 0?0:1;
}

void rt_can_timeout_thread(void *arg)
{
	int i = 0;

	while (1)
	{
		rt_thread_delay(10);

		for (i = 0; i < rec_buf_cont; i++)
		{
			if ((candev.receive_buf[i].status & (assembling | completed)))
			{
				(candev.receive_buf[i].timer)--;
				if (candev.receive_buf[i].timer == 0)
				{
					candev.receive_buf[i].status = nul;
				}
			}
		}
	}
}


