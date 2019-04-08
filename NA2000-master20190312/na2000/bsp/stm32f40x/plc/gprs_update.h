#ifndef __H_GPRS_UPDATE
#define __H_GPRS_UPDATE

#include "gprs.h"

rt_bool_t gprs_update_process(unsigned char *request,short req_len,unsigned char *ack,short* ack_len);

typedef struct
{
	unsigned char 	head[3];
	unsigned char 	func;
	unsigned short 	frame_len;
	
	unsigned short 	file_type;
	unsigned int		file_len;
	unsigned int 		file_frame_cnt;
	char						file_name[16];
	unsigned char		crc[2];
}gprs_down_request;


typedef enum
{
	ACK	=	0x01,
	NAK	=	0x02,
	STALL	=	0x04,
}RESULT;

#endif
