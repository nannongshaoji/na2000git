#include "gprs_update.h"

#define ROOT			"/na"
char TMP_FILE[20] = "";

static gprs_down_request *gdr = RT_NULL;

static int fd = -1;
static int curr_frame_no = 0;

extern void store_onlinemod_flag(char flag);

rt_bool_t handle_down_request(unsigned char *request,short req_len,unsigned char *ack,short* ack_len);
rt_bool_t handle_transfer_data(unsigned char *data,short data_len,unsigned char *ack,short* ack_len);
rt_bool_t handle_reset(unsigned char *data,short data_len,unsigned char *ack,short* ack_len);

static void init()
{
	if(gdr != RT_NULL)
	{
		rt_free(gdr);
		gdr = RT_NULL;
	}
	if(fd != -1)
	{
		close(fd);
		fd = -1;
	}
	curr_frame_no = 0;
}
     
static void packet_ack(unsigned char *ack,unsigned char func,unsigned char result,short* ack_len,int opt)
{
	int i=0,j=0;
	int optlen=sizeof(opt);
	
	if(opt < 0)
		optlen=0;
	
	ack[i++] = 0xAA;
	ack[i++] = 0xBB;
	ack[i++] = 0xCC;
	ack[i++] = 0x80 | func;
	ack[i++] = 0x00;
	ack[i++] = 0x00;
	
	for(j=0;j<optlen;j++)
	{
		ack[i++] = opt>>(j*8);
	}
	
	ack[i++] = result;
	ack[i++] = 0;
	ack[i++] = 0;
	
	ack[4] = i;
	ack[5] = i >> 8;
	
	*ack_len = i;
}

//return:1 need_rebooot
rt_bool_t gprs_update_process(unsigned char *request,short req_len,unsigned char *ack,short* ack_len)
{
	rt_bool_t ret = RT_FALSE;
	short frame_len = request[4] + (request[5] << 8);
	
	if(req_len != frame_len)
	{
		packet_ack(ack,request[3],STALL,ack_len,-1);
		return ret;
	}
	
	switch(request[3])
	{
		case 0x01:
			ret = handle_down_request(request,req_len,ack,ack_len);
		break;
		case 0x04:
			ret = handle_transfer_data(request,req_len,ack,ack_len);
		break;
		case 0x08:
			ret = handle_reset(request,req_len,ack,ack_len);
		break;
	}
	
	return !ret;
}

// if the return value is 1,then the ack will be fill
// otherwise this is not update packet
rt_bool_t handle_down_request(unsigned char *request,short req_len,unsigned char *ack,short* ack_len)
{
	init();
	
	gdr = rt_malloc(sizeof(gprs_down_request));
	if(gdr == RT_NULL)
	{
		packet_ack(ack,request[3],STALL,ack_len,-1);
		return RT_TRUE;
	}
	
	rt_memcpy(gdr,request,sizeof(gprs_down_request));

	if(rt_strlen(gdr->file_name) > 0)
	{
		char *pdot = rt_strstr(gdr->file_name,".");
		char filename[9] = "";
		rt_memset(TMP_FILE,0,sizeof(TMP_FILE));
		if(pdot)
		{
			char max = pdot - gdr->file_name;
			max = max > 8 ? 8: max;
			rt_memcpy(filename,gdr->file_name,max);
		}
		rt_sprintf(TMP_FILE,ROOT"/%s.tmp",filename);
		
		if((fd = open(TMP_FILE,O_RDWR | O_CREAT | O_TRUNC, 0)) < 0)
		{
			lseek(fd,0,SEEK_SET);
			packet_ack(ack,request[3],STALL,ack_len,-1);
			return RT_TRUE;
		}
	}
	
	packet_ack(ack,request[3],ACK,ack_len,-1);
	return RT_TRUE;
}

rt_bool_t handle_transfer_data(unsigned char *data,short len,unsigned char *ack,short* ack_len)
{
	short frame_len = data[4] + (data[5] << 8);
	int file_frame_no = data[6] + (data[7] << 8) + (data[8] << 16) + (data[9] << 24);
	short data_len = frame_len - 12;
	
	if(fd == -1)
	{
		packet_ack(ack,data[3],STALL,ack_len,-1);
		return RT_TRUE;
	}
	
	if(curr_frame_no != file_frame_no)
	{
		packet_ack(ack,data[3],NAK,ack_len,curr_frame_no+1);
		return RT_TRUE;
	}
	
	if(write(fd,&data[10],data_len) < 0 )
	{
		packet_ack(ack,data[3],STALL,ack_len,-1);
		return RT_TRUE;
	}
	
	if(curr_frame_no + 1 == gdr->file_frame_cnt )
	{
		char file_path[30] = "";
		rt_sprintf(file_path,ROOT"/%s",gdr->file_name);
		unlink(file_path);
		if(fd != -1)
		{
			close(fd);
			fd = -1;
		}
		
		rename(TMP_FILE,file_path);

		if(rt_strcmp(gdr->file_name,"na2000.bin") == 0)
			store_onlinemod_flag(1);
		
	}
	
	curr_frame_no++;
	
	packet_ack(ack,data[3],ACK,ack_len,file_frame_no);

	return RT_TRUE;
}

rt_bool_t handle_reset(unsigned char *data,short len,unsigned char *ack,short* ack_len)
{
	short frame_len = data[4] + (data[5] << 8);
		
	if(frame_len != 13)
	{
		packet_ack(ack,data[3],STALL,ack_len,-1);
		return RT_TRUE;
	}
	
	if(memcmp(&data[6],"reset",5) == 0)
	{
		packet_ack(ack,data[3],ACK,ack_len,-1);
		return RT_FALSE;
	}
	
	return RT_TRUE;
}

