#ifndef __H_HISTORY
#define __H_HISTORY

#include <rtthread.h>

#define MAX_CHAR_ONE_LINE_NUM							1300

typedef struct
{
	char							where;
	unsigned char 		mode;	//0--period,1--event
	union
	{
		unsigned short 	event_index;
		unsigned short 	time;
	}arg;
	
	unsigned short		start_index;	//mw start index
	unsigned short		num;					//number of MW to be store
	
	unsigned short		shift;
	
	char 	buff[MAX_CHAR_ONE_LINE_NUM];
	struct rt_mutex		mutex;
}his_config;

							
void start_history_dem(unsigned char where,	//where:0-spi flash,1-spi sd
											unsigned char mode,		//mode:0-period,1-event trigger
											unsigned short arg,		//mode parameter
											unsigned short start_index,	// start of MW register 
											unsigned short num,		//number of MW register
											unsigned short shift,	//the start address of modbus 04 function
											unsigned char clear_flag //remove his.ind and his.log
											);
rt_err_t restore_data_from_disk(unsigned char* buff,unsigned short start,int num);

#endif
