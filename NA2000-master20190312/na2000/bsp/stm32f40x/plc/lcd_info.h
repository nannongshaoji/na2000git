#ifndef __H_LCD_INFO
#define __H_LCD_INFO

#include <rtthread.h>

#define MAX_SUPPORT_LCD_COMM_INFO_LEN		100

typedef struct
{
	rt_uint8_t		resv;
	
	rt_uint8_t 		reset_cmd_from_cpu:1;
	rt_uint8_t 		reset_cmd_from_lcd:1;
	rt_uint8_t 		cpu_state:2;
	
}lcd_cmd;

typedef struct
{
	rt_uint16_t		year;
	rt_uint16_t		month;
	rt_uint16_t		day;
	rt_uint16_t		hour;
	rt_uint16_t		min;
	rt_uint16_t		sec;
	
	rt_uint32_t		ipaddr;
	rt_uint32_t		mask;
	rt_uint32_t		gateway;
	
	float		temperature;
	
}lcd_data;

typedef struct
{
	lcd_cmd 	cmd;
	lcd_data 	data;
}lcd_comm_info;

extern lcd_comm_info *plcd_info;

#endif
