#include "bd_board.h"
#include <rtthread.h>
#include <drivers/spi.h>
#include <sysapp.h>

#define SPI_BD_BOARD_MASTER_ID			1

#define SAMP_CNT 				8
#define MAX_AI_CHANNEL	4
#define RANGE_REGISTER  0xA000

#define CAI2001_0201		1
#define CAI2001_0402		2
#define CAI2001_0203		3
#define CAI2001_0204		4
#define CAO2001_0201		5

#define SGT_NULL			0
#define SGT_420mA			1
#define SGT_020mA			2
#define SGT_010mA			3

//Set the configuration to AIN1, FS=+/-0.512, SS, DR=128sps, PULLUP on DOUT
#define ADSCON_CH0 		0xd98A
//Set the configuration to AIN2, FS=+/-0.512, SS, DR=128sps, PULLUP on DOUT
#define ADSCON_CH1 		0xe98A

static int bd_type;
static rt_device_t bd_dev = RT_NULL;

const static unsigned short pt100table[] = {
	6026,		//-100
	6833,		//-80
	7633,		//-60
	8427,
	9216,
	10000, 	//0
	10779,
	11554,
	12324,
	13090,
	13851,
	14607,
	15358,
	16105,
	16848,
	17586,
	18319,
	19047,
	19771,
	20490,
	21205,
	21915,
	22621,
	23321,
	24018,
	24709,	//400
};

struct spi_bd
{
    struct rt_device                spi_device;
    struct rt_spi_device *          rt_spi_device;
};

static struct spi_bd spi_dev;

extern void rt_hw_spi_init(int);

static short trans_to_temperature(unsigned short val)
{
	char table_len = sizeof(pt100table)/sizeof(unsigned short);
	char i=0;
	float tmp = 0;
	short ret = 0;
	val = (int)(val * 125)/10;
	
	for(i=0;i<table_len-1;i++)
	{
		if(pt100table[i] < val && pt100table[i+1] > val)
			break;
	}
	
	if(i>=table_len-1)
		return 0;

	tmp = val - pt100table[i];
	tmp = tmp/((pt100table[i+1] - pt100table[i]));
	tmp *= 20;
	tmp += (i*2-10)*10;
	
	ret = (short)(tmp*10);
	return ret;
}

/* RT-Thread Device Driver Interface */
static rt_err_t ad_init(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t ad_open(rt_device_t dev, rt_uint16_t oflag)
{

    return RT_EOK;
}

static rt_err_t ad_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t ad_control(rt_device_t dev, int cmd, void *args)
{
    RT_ASSERT(dev != RT_NULL);

    if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
    {
    }

    return RT_EOK;
}

static rt_size_t ad_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	return rt_spi_transfer(spi_dev.rt_spi_device,&pos,buffer,1);
}

static rt_size_t ad_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	return rt_spi_transfer(spi_dev.rt_spi_device,buffer,RT_NULL,1);
}

rt_err_t bd_init(const char * ad_name, const char * spi_device_name)
{
	struct rt_spi_device * rt_spi_device;
	rt_spi_device = (struct rt_spi_device *)rt_device_find(spi_device_name);
	if(rt_spi_device == RT_NULL)
	{
			return -RT_ENOSYS;
	}
	
	spi_dev.rt_spi_device = rt_spi_device;
	/* config spi */
	{
		struct rt_spi_configuration cfg;
		cfg.data_width = 16;
		cfg.mode = RT_SPI_MODE_1 | RT_SPI_MSB; /* SPI Compatible Modes 0 and 3 */
		cfg.max_hz = 1000000; /* Atmel RapidS Serial Interface: 66MHz Maximum Clock Frequency */
		rt_spi_configure(spi_dev.rt_spi_device, &cfg);
	}
	
/* register device */
	spi_dev.spi_device.type    = RT_Device_Class_Block;
	spi_dev.spi_device.init    = ad_init;
	spi_dev.spi_device.open    = ad_open;
	spi_dev.spi_device.close   = ad_close;
	spi_dev.spi_device.control = ad_control;
	spi_dev.spi_device.read = ad_read;
	spi_dev.spi_device.write = ad_write;
	spi_dev.spi_device.user_data = RT_NULL;
	
	rt_device_register(&spi_dev.spi_device, ad_name,RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
	return RT_EOK;
}

static void process(void *lp)
{
	rt_uint16_t 	value[MAX_AI_CHANNEL] = {0,};
	rt_uint16_t		txData[MAX_AI_CHANNEL] = {0x0800,0x1000,0x1800,0x0000};
	rt_uint8_t		bd_type = *(rt_uint8_t *)(lp);
	char 					i = 0, j = 0;
	long					tmp = 0;
	long 					tmp_sample[MAX_AI_CHANNEL][SAMP_CNT] = {{0,},};
	
	bd_dev = rt_device_find("bd");

	if(bd_dev && rt_device_open(bd_dev,RT_DEVICE_OFLAG_RDWR) != RT_EOK)
	{
		return;
	}
		
	while(1)
	{
		rt_thread_delay(10);
		switch(bd_type)
		{
			case CAI2001_0201:
				break;
			case CAI2001_0402:
				for(j=0;j<SAMP_CNT;j++)
				{
					for(i=0;i<pStaticData->pMDU[0].AINum;i++)
					{
						#define TRANS	(5*20000/(4096*220*0.02))
						rt_device_read(bd_dev,txData[i],&value[i],2);
						tmp = value[i]*TRANS;
						switch(pStaticData->pAI[i].SignalType)
						{
							case SGT_420mA:
							case SGT_020mA:
									value[i] = tmp;
								if(value[i] > 20000)
									value[i] = 20000;
								tmp_sample[i][j] = value[i];
								break;
							case SGT_010mA:
								value[i] = tmp * 2;
								if(value[i] > 20000)
									value[i] = 20000;
								tmp_sample[i][j] = value[i];
								break;
							default:
								break;
						}
					}
					rt_thread_delay(10);
				}

				for(i=0;i<MAX_AI_CHANNEL;i++)
				{
					rt_uint32_t min = 20000;
					rt_uint32_t max = 0;
					rt_uint32_t total = 0;

					for(j=0;j<SAMP_CNT;j++)
					{
						if( tmp_sample[i][j] < min )
						{
								min = tmp_sample[i][j];
						}
						if( tmp_sample[i][j] > max )
						{
								max = tmp_sample[i][j];
						}
						total += tmp_sample[i][j];
					}
					value[i] = (total - min - max) / (SAMP_CNT-2);
				}

				Move_Fuc( &pDynamicData->pAI.RawData[0], (unsigned int)pStaticData->pMDU[0].AIRefAddr,
										value, 1,
										(unsigned int)pStaticData->pMDU[0].AINum*2*8);
				Drv_Multi_AI(pStaticData->pMDU[0].AIRefAddr, pStaticData->pMDU[0].AINum);
				break;
			case CAI2001_0203:
				
				txData[0] = ADSCON_CH0;
				txData[1] = ADSCON_CH1;
				
				for(i=0;i<pStaticData->pMDU[0].AINum;i++)
				{
					rt_device_read(bd_dev,txData[i],&value[i],2);
					rt_thread_delay(10);
					txData[i] = txData[i] & 0xFFF8;
					rt_device_read(bd_dev,txData[i],&value[i],2);
					value[i] = value[i] >> 4;
					rt_thread_delay(10);
					value[i] = trans_to_temperature(value[i]);
				}
				Move_Fuc( &pDynamicData->pAI.RawData[0], (unsigned int)pStaticData->pMDU[0].AIRefAddr,
										value, 1,
										(unsigned int)pStaticData->pMDU[0].AINum*2*8);
				Drv_Multi_AI(pStaticData->pMDU[0].AIRefAddr, pStaticData->pMDU[0].AINum);
				break;
			case CAI2001_0204:
			break;
			case CAO2001_0201:
				break;
			default:
				break;
		}
	}
}

void init_bd_board(int type)
{
	bd_type = type;
	rt_hw_spi_init(1);
	bd_init("bd","spi21");
	{
		rt_thread_t tid;
		char name[10] = "";
		rt_sprintf(name,"bd%d",type);
		tid = rt_thread_create(name,process,&bd_type,768,20,20);
		if(tid)
			rt_thread_startup(tid);
	}
	
}

