#include "sysapp.h"
#include "history.h"

#define HISTORY_INDEX_FILE_PATH_SD		"/sd0/his.ind"
#define HISTORY_FILE_SD								"/sd0/his.log"

#define HISTORY_INDEX_FILE_PATH				"/na/his.ind"
#define HISTORY_FILE									"/na/his.log"

#define TIME_PREFIX								"[%02u-%02u-%02u %02u:%02u:%02u]"

#define MAX_RECORD_NUM						5									

static his_config *config = RT_NULL;

static void save_current_index(unsigned index)
{
	int fd = -1;
	char *filepath = RT_NULL;
	
	if(config->where == 0)
	{
		filepath = HISTORY_INDEX_FILE_PATH;
	}
	else
	{
		filepath = HISTORY_INDEX_FILE_PATH_SD;
	}
	
	if((fd = open(filepath,O_RDWR | O_CREAT | O_TRUNC, 0))< 0 )
		return;
	
	lseek(fd,0,SEEK_SET);
	write(fd,&index,sizeof(index));
	close(fd);
}

static unsigned get_next_index()
{
	int 			len = 0;
	unsigned 	index = 0;
	int 			fd = -1;
	char *filepath = RT_NULL;
	
	if(config->where == 0)
	{
		filepath = HISTORY_INDEX_FILE_PATH;
	}
	else
	{
		filepath = HISTORY_INDEX_FILE_PATH_SD;
	}
	
	if((fd = open(filepath,O_RDONLY, 0)) < 0)
	{
		index = 0;
		close(fd);
		save_current_index(index);
		return index;
	}
	
	len = lseek(fd,0L,SEEK_END);
	if(len <= 0)
	{
		close(fd);
		save_current_index(index);
		return 0;
	}
	
	lseek(fd,0,SEEK_SET);
	read(fd,&index,sizeof(index));
	index += 1;
	
	if(index > MAX_RECORD_NUM-1)
		index = 0;
	
	close(fd);
	save_current_index(index);

	return index;
}

static rt_bool_t init_his_config(unsigned char where,unsigned char mode,
										unsigned short arg,unsigned short start_index,
										unsigned short num,unsigned int shift)
{
	config = rt_calloc(1,sizeof(his_config));
	if(config == RT_NULL)
		return RT_ERROR;
	
	config->mode = mode;
	config->arg.time = arg;
	config->start_index = start_index;
	config->num = num;
	config->shift = shift;
	config->where = where;

	rt_mutex_init(&config->mutex,"hismutex",RT_IPC_FLAG_PRIO);
	
	return RT_EOK;
}

static void store_data_to_disk()
{
	int 						fd = -1;
	CLOCK						cl;
	unsigned short 	i=0;
	unsigned 				index = 0;
	unsigned int 		offset = 0;
	char *					filepath = RT_NULL;
	
	if(config == RT_NULL)
		return;
	
	index = get_next_index();
	offset = index * (config->num*6+21);

	SysTimerRead(&cl);
	
	rt_mutex_take(&config->mutex,RT_WAITING_FOREVER);
	
	rt_memset(config->buff,0,MAX_CHAR_ONE_LINE_NUM);
	sprintf(config->buff,TIME_PREFIX,cl.Year%100,cl.Month,cl.Day,cl.Hour,cl.Minute,cl.Second);
	
	for(i=0;i<config->num;i++)
	{
		char tmp[20] = "";
		sprintf(tmp,"%5hu,",pDynamicData->pR.Value[config->start_index + i]);
		strcat(config->buff,tmp);
	}
	strcat(config->buff,"\r\n");
	
	if(config->where == 0)
	{
		filepath = HISTORY_FILE;
	}
	else
	{
		filepath = HISTORY_FILE_SD;
	}
	
	if((fd = open(filepath,O_RDWR | O_CREAT, 0)) < 0)
			goto out;
	
	lseek(fd,offset,SEEK_SET);
	write(fd,config->buff,rt_strlen(config->buff));
	
out:
	if(fd >= 0)
		close(fd);
	
	rt_mutex_release(&config->mutex);
}

rt_err_t restore_data_from_disk(unsigned char* buff,unsigned short start,int num)
{
	rt_err_t				ret = RT_EOK;
	unsigned short 	reg_num = 3 + config->num;
	unsigned char 	row = 0;
	unsigned char 	col = 0;
	unsigned int		offset = 0;
	unsigned char 	i=0;
	unsigned char 	act_read_num = 0;
	char* 					tmp = RT_NULL;
	unsigned int 		len = 0;
	int 						fd = 0;
	char *					filepath = RT_NULL;

	if(config == RT_NULL)
		return -RT_EINVAL;
	
	tmp = rt_calloc(1,MAX_CHAR_ONE_LINE_NUM);
	if(tmp == RT_NULL)
		return -RT_ENOMEM;
	
	start = start - config->shift;
	row = start / reg_num;
	col = start % reg_num;
	
	rt_mutex_take(&config->mutex,RT_WAITING_FOREVER);
	
	if(config->where == 0)
	{
		filepath = HISTORY_FILE;
	}
	else
	{
		filepath = HISTORY_FILE_SD;
	}
	
	if( (fd = open(filepath,O_RDONLY, 0))<0 )
	{
		ret = -RT_EIO;
		goto out;
	}
		
	len = lseek(fd,0L,SEEK_END);
	
	while(num > 0)
	{
		offset = row*(config->num*6+21);
		
		if(num > reg_num)
			act_read_num = reg_num - col;
		else
			act_read_num = num;
		
		if(offset > len)
				goto out;
		
		lseek(fd,offset,SEEK_SET);
		read(fd,tmp,MAX_CHAR_ONE_LINE_NUM);
		sscanf(tmp,TIME_PREFIX,
			(unsigned int *)&config->buff[0],(unsigned int *)&config->buff[1],
			(unsigned int *)&config->buff[2],(unsigned int *)&config->buff[3],
			(unsigned int *)&config->buff[4],(unsigned int *)&config->buff[5]);
		
		for(i=0;i<config->num;i++)
			sscanf(tmp+19+i*6,"%5hu,",(unsigned short*)(&config->buff[6+i*2]));

		memcpy(buff,config->buff + col*2,act_read_num*2);
		num -= act_read_num;
		row += 1;col = 0;
		buff += act_read_num*2;
	}
	
out:
	if(fd >= 0)
		close(fd);
	if(tmp)
		rt_free(tmp);
	rt_mutex_release(&config->mutex);
	return ret;
}

void history_thread(void *param)
{
	extern int GetHardSW();
	if(config == RT_NULL)
		return;

	while(1)
	{
		if(GetHardSW()!=2)
		{
			if(config->mode == 0)
			{
				store_data_to_disk();
				rt_thread_delay(config->arg.time);
			}
			else if(config->mode == 1)
			{
				if(Get_M(config->arg.event_index) == 1)
				{
					store_data_to_disk();
					Output_M(config->arg.event_index,0);
					rt_thread_delay(100);
				}
			}
		}
		else
			rt_thread_delay(100);
	}
}

void start_history_dem(unsigned char where,unsigned char mode,
										unsigned short arg,unsigned short start_index,
										unsigned short num,unsigned short shift,
										unsigned char clear_flag)
{
	rt_thread_t tid;
	
	init_his_config(where,mode,arg,start_index,num,shift);
	
	if(clear_flag)
	{
		if(where)
		{
			unlink(HISTORY_INDEX_FILE_PATH_SD);
			unlink(HISTORY_FILE_SD);
		}
		else
		{
			unlink(HISTORY_INDEX_FILE_PATH);
			unlink(HISTORY_FILE);
		}
	}
	tid = rt_thread_create("history",history_thread,RT_NULL,1024,20,20);
	if(tid != RT_NULL)
	{
		rt_thread_startup(tid);
	}
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void restore_data(int start,int num)
{
	short *buff =rt_calloc(1,1024);
	int 	i=0;
	
	restore_data_from_disk((unsigned char *)buff,start,num);
	
	for(i=0;i<num;i++)
	{
		rt_kprintf("%hu,",buff[i]);
	}
	rt_kprintf("\r\n");
	
	if(buff)
		rt_free(buff);
	return;
}
FINSH_FUNCTION_EXPORT(restore_data, restore data from disk test);
#endif
