#ifndef ZIGBEE_H
#define ZIGBEE_H

#define REG_ECHO							0x24
#define REG_ECHO_ON						0x0001
#define REG_ECHO_OFF					0x0000

#define REG_MODE							0x00
#define REG_MODE_TT						0x0000
#define REG_MODE_AT						0x0001
#define REG_MODE_API					0x0002

#define REG_PHY_CHAN					0x01
#define REG_PHY_CHAN_BEGIN		0x000B
#define REG_PHY_CHAN_END			0x001A

#define REG_TYPE							0x02
#define REG_TYPE_COORD				0x0000
#define REG_TYPE_ROUTER				0x0001
#define REG_TYPE_TTY					0x0002

#define REG_NETWORK_ID				0x03
#define REG_NETWORK_ID_BEGIN	0x0000
#define REG_NETWORK_ID_END		0xFFFB

#define REG_NETWORK_ADDR			0x04
#define REG_NETWORK_ADDR_BEGIN	0x0000
#define REG_NETWORK_ADDR_END		0xFFF7

#define REG_AUTO_STARTUP_WHEN_BOOT			0x20
#define REG_AUTO_STARTUP_WHEN_BOOT_OFF	0x0000
#define REG_AUTO_STARTUP_WHEN_BOOT_ON		0x0001

#define MAX_WAIT_TIME					300//ms
#define MAX_INTERVAL_TIME			30//ms

#define MAX_DATA_LEN					80

typedef struct
{
	unsigned char head;
	unsigned char data_len;
	unsigned char cmd[2];
	unsigned char addr[2];
	unsigned char data[MAX_DATA_LEN];
}SZB_SEND_BLOCK;
	
typedef union
{
	SZB_SEND_BLOCK ssb;
	unsigned char msg[ sizeof(SZB_SEND_BLOCK) ];
}ZB_BUFF;

extern unsigned char calcFCS(unsigned char *pMsg, unsigned char len);

extern void zigbeethread(void *lp);

extern unsigned char send_zb_data(int fd,ZB_BUFF *buff,unsigned short addr);
extern unsigned char* recv_zb_data(int fd,ZB_BUFF *buff,unsigned short *addr,int maxwaittime,int maxinterval);

#endif
