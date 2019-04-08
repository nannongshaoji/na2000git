#include "spi_sd.h"

#define SPI_SPEED_1   1
#define SPI_SPEED_2   2
#define SPI_SPEED_4   4
#define SPI_SPEED_8   8
#define SPI_SPEED_16  16
#define SPI_SPEED_256 256

#define SPI_SD_WORK_HZ			24000000
#define SPI_SD_WORK_SPEED		SPI_SPEED_1
	
u8  SD_Type=0;//SD卡的类型
//Mini STM32开发板
//SD卡 驱动
//正点原子@ALIENTEK

//2010/5/13									   
//增加了一些延时,实测可以支持TF卡(1G/2G),金士顿2G,4G 16G SD卡
//2010/6/24
//加入了u8 SD_GetResponse(u8 Response)函数
//修改了u8 SD_WaitDataReady(void)函数
//增加了USB读卡器支持的u8 MSD_ReadBuffer(u8* pBuffer, u32 ReadAddr, u32 NumByteToRead);
//和u8 MSD_WriteBuffer(u8* pBuffer, u32 WriteAddr, u32 NumByteToWrite);两个函数
struct SD_CardInfo
{
  uint32_t CardCapacity;  /*!< Card Capacity */
  uint32_t CardBlockSize; /*!< Card Block Size */
  uint16_t RCA;
  uint8_t CardType;
};

struct spi_sd_device
{
    struct rt_device                sd_device;
    struct rt_spi_device *          rt_spi_device;
};

struct spi_sd_device spi_sd_device;

struct SD_CardInfo sdinfo;

void SPI_SetSpeed(u16 speed)
{
	struct rt_spi_configuration cfg;
	cfg.data_width = 8;
	cfg.mode = RT_SPI_MODE_3 | RT_SPI_MSB; /* SPI Compatible Modes 0 and 3 */
	cfg.max_hz = SPI_SD_WORK_HZ / speed; /* Atmel RapidS Serial Interface: 66MHz Maximum Clock Frequency */
	rt_spi_configure(spi_sd_device.rt_spi_device, &cfg);
}
//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPIx_ReadWriteByte(u8 TxData)
{		
	u8 ret;
	rt_spi_transfer(spi_sd_device.rt_spi_device,&TxData,&ret,1);
	return ret;
}

//等待SD卡回应
//Response:要得到的回应值
//返回值:0,成功得到了该回应值
//    其他,得到回应值失败
u8 SD_GetResponse(u8 Response)
{
	u16 Count=0xFFF;//等待次数	   						  
	while ((SPIx_ReadWriteByte(0XFF)!=Response)&&Count)Count--;//等待得到准确的回应  	  
	if (Count==0)return MSD_RESPONSE_FAILURE;//得到回应失败   
	else return MSD_RESPONSE_NO_ERROR;//正确回应
}
//等待SD卡写入完成
//返回值:0,成功;   
//    其他,错误代码;
u8 SD_WaitDataReady(void)
{
	u8 r1=MSD_DATA_OTHER_ERROR;
	u32 retry;
	retry=0;
	do
	{
		r1=SPIx_ReadWriteByte(0xFF)&0X1F;//读到回应
		if(retry==0xfffe)
			return 1; 
		retry++;
		switch (r1)
		{					   
		case MSD_DATA_OK://数据接收正确了	 
			r1=MSD_DATA_OK;
		break;  
		case MSD_DATA_CRC_ERROR:  //CRC校验错误
			return MSD_DATA_CRC_ERROR;  
		case MSD_DATA_WRITE_ERROR://数据写入错误
			return MSD_DATA_WRITE_ERROR;  
		default://未知错误    
			r1=MSD_DATA_OTHER_ERROR;
		break;	 
		}   
	}while(r1==MSD_DATA_OTHER_ERROR); //数据错误时一直等待
	
	retry=0;
	
	while(SPIx_ReadWriteByte(0XFF)==0)//读到数据为0,则数据还未写完成
	{
		retry++;
		//delay_us(10);//SD卡写等待需要较长的时间
		if(retry>=0XFFFFFFFE)return 0XFF;//等待失败了
	}  
	return 0;//成功了
}	 
//向SD卡发送一个命令
//输入: u8 cmd   命令 
//      u32 arg  命令参数
//      u8 crc   crc校验值	   
//返回值:SD卡返回的响应															  
u8 SD_SendCommand(u8 cmd, u32 arg, u8 crc)
{
	u8 r1;	
	u8 Retry=0;	         
	SPIx_ReadWriteByte(0xff);//高速写命令延时
	SPIx_ReadWriteByte(0xff);     
	SPIx_ReadWriteByte(0xff);  	 
	//发送
	SPIx_ReadWriteByte(cmd | 0x40);//分别写入命令
	SPIx_ReadWriteByte(arg >> 24);
	SPIx_ReadWriteByte(arg >> 16);
	SPIx_ReadWriteByte(arg >> 8);
	SPIx_ReadWriteByte(arg);
	SPIx_ReadWriteByte(crc); 
	//等待响应，或超时退出
	while((r1=SPIx_ReadWriteByte(0xFF))==0xFF)
	{
		Retry++;	    
		if(Retry>200)break; 
	}   
	//在总线上额外增加8个时钟，让SD卡完成剩下的工作
	SPIx_ReadWriteByte(0xFF);
	//返回状态值
	return r1;
}		  																				 
//向SD卡发送一个命令(结束是不失能片选，还有后续数据传来）
//输入:u8 cmd   命令 
//     u32 arg  命令参数
//     u8 crc   crc校验值	 
//返回值:SD卡返回的响应															  
u8 SD_SendCommand_NoDeassert(u8 cmd, u32 arg, u8 crc)
{
	u8 Retry=0;	         
	u8 r1;			   
	SPIx_ReadWriteByte(0xff);//高速写命令延时
	SPIx_ReadWriteByte(0xff);  	 	    
	//发送
	SPIx_ReadWriteByte(cmd | 0x40); //分别写入命令
	SPIx_ReadWriteByte(arg >> 24);
	SPIx_ReadWriteByte(arg >> 16);
	SPIx_ReadWriteByte(arg >> 8);
	SPIx_ReadWriteByte(arg);
	SPIx_ReadWriteByte(crc);   
	//等待响应，或超时退出
	while((r1=SPIx_ReadWriteByte(0xFF))==0xFF)
	{
		Retry++;	    
		if(Retry>200)break; 
	}  	  
	//返回响应值
	return r1;
}
//把SD卡设置到挂起模式
//返回值:0,成功设置
//       1,设置失败
u8 SD_Idle_Sta(void)
{
	u16 i;
	u8 retry;
	rt_thread_delay(1); 
	//先产生>74个脉冲，让SD卡自己初始化完成
	for(i=0;i<20;i++)SPIx_ReadWriteByte(0xFF); 
	//-----------------SD卡复位到idle开始-----------------
	//循环连续发送CMD0，直到SD卡返回0x01,进入IDLE状态
	//超时则直接退出
	retry = 0;
	do
	{	   
		//发送CMD0，让SD卡进入IDLE状态
		i = SD_SendCommand(CMD0, 0, 0x95);
		retry++;
	}while((i!=0x01)&&(retry<200));
	//跳出循环后，检查原因：初始化成功？or 重试超时？
	if(retry==200)return 1; //失败
	return 0;//成功	 						  
}														    
//初始化SD卡
//如果成功返回,则会自动设置SPI速度为18Mhz
//返回值:0：NO_ERR
//       1：TIME_OUT
//      99：NO_CARD																 
u8 SD_Init(void)
{			
	u8 r1;      // 存放SD卡的返回值
	u16 retry;  // 用来进行超时计数
	u8 buff[6];	
	SPI_SetSpeed(SPI_SPEED_256);//设置到低速模式
	if(SD_Idle_Sta()) 
	{
		r1 = 1;//超时返回1 设置到idle 模式失败
		goto out;
	}
	//-----------------SD卡复位到idle结束-----------------	 
	//获取卡片的SD版本信息
	r1 = SD_SendCommand_NoDeassert(8, 0x1aa,0x87);	     
	//如果卡片版本信息是v1.0版本的，即r1=0x05，则进行以下初始化
	if(r1 == 0x05)
	{
		//设置卡类型为SDV1.0，如果后面检测到为MMC卡，再修改为MMC
		SD_Type = SD_TYPE_V1;	   
		//如果是V1.0卡，CMD8指令后没有后续数据
		//多发8个CLK，让SD结束后续操作
		SPIx_ReadWriteByte(0xFF);	  
		//-----------------SD卡、MMC卡初始化开始-----------------	 
		//发卡初始化指令CMD55+ACMD41
		// 如果有应答，说明是SD卡，且初始化完成
		// 没有回应，说明是MMC卡，额外进行相应初始化
		retry = 0;
		do
		{
			//先发CMD55，应返回0x01；否则出错
			r1 = SD_SendCommand(CMD55, 0, 0);
			if(r1 == 0XFF)
				goto out;//只要不是0xff,就接着发送
			
			//得到正确响应后，发ACMD41，应得到返回值0x00，否则重试200次
			r1 = SD_SendCommand(ACMD41, 0, 0);
			retry++;
		}while((r1!=0x00) && (retry<400));
		
		// 判断是超时还是得到正确回应
		// 若有回应：是SD卡；没有回应：是MMC卡	  
		//----------MMC卡额外初始化操作开始------------
		if(retry==400)
		{
			retry = 0;
			//发送MMC卡初始化命令（没有测试）
			do
			{
				r1 = SD_SendCommand(1,0,0);
				retry++;
			}while((r1!=0x00)&& (retry<400));
			
			if(retry==400)
			{
				r1=1;   //MMC卡初始化超时
				goto out;
			}
			//写入卡类型
			SD_Type = SD_TYPE_MMC;
		}
		//----------MMC卡额外初始化操作结束------------	    
		//设置SPI为高速模式
		SPI_SetSpeed(SPI_SD_WORK_SPEED);   
		SPIx_ReadWriteByte(0xFF);	 
		//禁止CRC校验	   
		r1 = SD_SendCommand(CMD59, 0, 0x95);
		if(r1 != 0x00) 
			goto out;  //命令错误，返回r1   	   
		//设置Sector Size
		r1 = SD_SendCommand(CMD16, 512, 0x95);
		if(r1 != 0x00) 
			goto out;//命令错误，返回r1		 
		//-----------------SD卡、MMC卡初始化结束-----------------

	}
	//SD卡为V1.0版本的初始化结束	 
	//下面是V2.0卡的初始化
	//其中需要读取OCR数据，判断是SD2.0还是SD2.0HC卡
	else if(r1 == 0x01)
	{
		//V2.0的卡，CMD8命令后会传回4字节的数据，要跳过再结束本命令
		buff[0] = SPIx_ReadWriteByte(0xFF);  //should be 0x00
		buff[1] = SPIx_ReadWriteByte(0xFF);  //should be 0x00
		buff[2] = SPIx_ReadWriteByte(0xFF);  //should be 0x01
		buff[3] = SPIx_ReadWriteByte(0xFF);  //should be 0xAA	      
		SPIx_ReadWriteByte(0xFF);//the next 8 clocks			 
		//判断该卡是否支持2.7V-3.6V的电压范围
		//if(buff[2]==0x01 && buff[3]==0xAA) //不判断，让其支持的卡更多
		{	  
			retry = 0;
			//发卡初始化指令CMD55+ACMD41
			do
			{
				r1 = SD_SendCommand(CMD55, 0, 0);
				if(r1!=0x01)
					return r1;	   
				r1 = SD_SendCommand(ACMD41, 0x40000000, 0);
				if(retry>200)
					return r1;  //超时则返回r1状态  
			}while(r1!=0);		  
			//初始化指令发送完成，接下来获取OCR信息		   
			//-----------鉴别SD2.0卡版本开始-----------
			r1 = SD_SendCommand_NoDeassert(CMD58, 0, 0);
			if(r1!=0x00)
			{
				return r1;  //如果命令没有返回正确应答，直接退出，返回应答	 
			}//读OCR指令发出后，紧接着是4字节的OCR信息
			buff[0] = SPIx_ReadWriteByte(0xFF);
			buff[1] = SPIx_ReadWriteByte(0xFF); 
			buff[2] = SPIx_ReadWriteByte(0xFF);
			buff[3] = SPIx_ReadWriteByte(0xFF);		 
			//OCR接收完成，片选置高
			SPIx_ReadWriteByte(0xFF);	   
			//检查接收到的OCR中的bit30位（CCS），确定其为SD2.0还是SDHC
			//如果CCS=1：SDHC   CCS=0：SD2.0
			if(buff[0]&0x40)SD_Type = SD_TYPE_V2HC;    //检查CCS	 
			else SD_Type = SD_TYPE_V2;	
			//-----------鉴别SD2.0卡版本结束----------- 
		}	    
	}
	
out:
	//设置SPI为高速模式
	SPI_SetSpeed(SPI_SD_WORK_SPEED);
	return r1;
}	 																			   
//从SD卡中读回指定长度的数据，放置在给定位置
//输入: u8 *data(存放读回数据的内存>len)
//      u16 len(数据长度）
//      u8 release(传输完成后是否释放总线CS置高 0：不释放 1：释放）	 
//返回值:0：NO_ERR
//  	 other：错误信息														  
u8 SD_ReceiveData(u8 *data, u16 len, u8 release)
{
	// 启动一次传输				  	  
	if(SD_GetResponse(0xFE))//等待SD卡发回数据起始令牌0xFE
	{
		return 1;
	}
	while(len--)//开始接收数据
	{
		*data=SPIx_ReadWriteByte(0xFF);
		data++;
	}
	//下面是2个伪CRC（dummy CRC）
	SPIx_ReadWriteByte(0xFF);
	SPIx_ReadWriteByte(0xFF);
	if(release==RELEASE)//按需释放总线，将CS置高
	{
		SPIx_ReadWriteByte(0xFF);
	}											  					    
	return 0;
}

//获取SD卡的CID信息，包括制造商信息
//输入: u8 *cid_data(存放CID的内存，至少16Byte）	  
//返回值:0：NO_ERR
//		 1：TIME_OUT
//       other：错误信息														   
u8 SD_GetCID(u8 *cid_data)
{
	u8 r1;	   
	//发CMD10命令，读CID
	r1 = SD_SendCommand(CMD10,0,0xFF);
	if(r1 != 0x00)return r1;  //没返回正确应答，则退出，报错  
	SD_ReceiveData(cid_data,16,RELEASE);//接收16个字节的数据	 
	return 0;
}																				  
//获取SD卡的CSD信息，包括容量和速度信息
//输入:u8 *cid_data(存放CID的内存，至少16Byte）	    
//返回值:0：NO_ERR
//       1：TIME_OUT
//       other：错误信息														   
u8 SD_GetCSD(u8 *csd_data)
{
	u8 r1;	 
	r1=SD_SendCommand(CMD9,0,0xFF);//发CMD9命令，读CSD
	if(r1)return r1;  //没返回正确应答，则退出，报错  
	SD_ReceiveData(csd_data, 16, RELEASE);//接收16个字节的数据 
	return 0;
}  
//获取SD卡的容量（字节）   
//返回值:0： 取容量出错 
//       其他:SD卡的容量(字节)														  
u32 SD_GetCapacity(void)
{
	u8 csd[16];
	u32 Capacity;
	u8 r1;
	u16 i;
	u16 temp;  					    
//取CSD信息，如果期间出错，返回0
	if(SD_GetCSD(csd)!=0) return 0;	    
	//如果为SDHC卡，按照下面方式计算
	if((csd[0]&0xC0)==0x40)
	{									  
		Capacity=((u32)csd[8])<<8;
		Capacity+=(u32)csd[9]+1;	 
		Capacity = (Capacity)*1024;//得到扇区数
		Capacity*=512;//得到字节数
		sdinfo.CardCapacity = 	Capacity;	
		sdinfo.CardBlockSize = 512;
	}
	else
	{		    
		i = csd[6]&0x03;
		i<<=8;
		i += csd[7];
		i<<=2;
		i += ((csd[8]&0xc0)>>6);
		//C_SIZE_MULT
		r1 = csd[9]&0x03;
		r1<<=1;
		r1 += ((csd[10]&0x80)>>7);	 
		r1+=2;//BLOCKNR
		temp = 1;
		while(r1)
		{
			temp*=2;
			r1--;
		}
		Capacity = ((u32)(i+1))*((u32)temp);
		sdinfo.CardBlockSize = temp;			
		// READ_BL_LEN
		i = csd[5]&0x0f;
		//BLOCK_LEN
		temp = 1;
		while(i)
		{
			temp*=2;
			i--;
		}
		//The final result
		Capacity *= (u32)temp;//字节为单位 	
		sdinfo.CardCapacity = Capacity;			
	}
	return (u32)Capacity;
}	    																			    
//读SD卡的一个block
//输入:u32 sector 取地址（sector值，非物理地址） 
//     u8 *buffer 数据存储地址（大小至少512byte） 		   
//返回值:0： 成功
//       other：失败															  
u8 SD_ReadSingleBlock(u32 sector, u8 *buffer)
{
	u8 r1;	    
	//设置为高速模式
	SPI_SetSpeed(SPI_SD_WORK_SPEED);  		   
	//如果不是SDHC，给定的是sector地址，将其转换成byte地址
	if(SD_Type!=SD_TYPE_V2HC)
	{
		sector = sector<<9;
	} 
	r1 = SD_SendCommand(CMD17, sector, 0);//读命令	 		    
	if(r1 != 0x00)
	return r1; 		   							  
	r1 = SD_ReceiveData(buffer, 512, RELEASE);
	//	r1 = SD_DMA_ReceiveData(buffer, 512);
	if(r1 != 0)
		return r1;   //读数据出错！
	else 
		return 0; 
}

//////////////////////////////////////////////////////////////////////////
//写入SD卡的一个block(未实际测试过)										    
//输入:u32 sector 扇区地址（sector值，非物理地址） 
//     u8 *buffer 数据存储地址（大小至少512byte） 		   
//返回值:0： 成功
//       other：失败															  
u8 SD_WriteSingleBlock(u32 sector, const u8 *data)
{
	u8 r1;
	u16 i;
	u16 retry;
 
	//如果不是SDHC，给定的是sector地址，将其转换成byte地址
	if(SD_Type!=SD_TYPE_V2HC)
	{
			sector = sector<<9;
	}   
	r1 = SD_SendCommand(CMD24, sector, 0x00);
	if(r1 != 0x00)
	{
			return r1;  //应答不正确，直接返回
	}
	
	//先放3个空数据，等待SD卡准备好
	SPIx_ReadWriteByte(0xff);
	SPIx_ReadWriteByte(0xff);
	SPIx_ReadWriteByte(0xff);
	//放起始令牌0xFE
	SPIx_ReadWriteByte(0xFE);

//		SD_DMA_SendData((u8 *)data,512);
	//放一个sector的数据
	for(i=0;i<512;i++)
	{
			SPIx_ReadWriteByte(*data++);
	}
	//发2个Byte的dummy CRC
	SPIx_ReadWriteByte(0xff);
	SPIx_ReadWriteByte(0xff);
	
	//等待SD卡应答
	r1 = SPIx_ReadWriteByte(0xff);
	if((r1&0x1F)!=0x05)
	{
		return r1;
	}
	
	//等待操作完成
	retry = 0;
	while(!SPIx_ReadWriteByte(0xff))
	{
		rt_thread_delay(1);
		if(retry++>100)
			return 1;
	}
	
	//写入完成
	SPIx_ReadWriteByte(0xff);

	return 0;
}				           
//读SD卡的多个block(实际测试过)										    
//输入:u32 sector 扇区地址（sector值，非物理地址） 
//     u8 *buffer 数据存储地址（大小至少512byte）
//     u8 count 连续读count个block 		   
//返回值:0： 成功
//       other：失败															  
u8 SD_ReadMultiBlock(u32 sector, u8 *buffer, u8 count)
{
	u8 r1;	 			 
	//如果不是SDHC，将sector地址转成byte地址
	if(SD_Type!=SD_TYPE_V2HC)sector = sector<<9;  
	//SD_WaitDataReady();
	//发读多块命令
	r1 = SD_SendCommand(CMD18, sector, 0);//读命令
	if(r1 != 0x00)return r1;	 
	do//开始接收数据
	{
			if(SD_ReceiveData(buffer, 512, NO_RELEASE) != 0x00)break; 
			buffer += 512;
	} while(--count);		 
	//全部传输完毕，发送停止命令
	SD_SendCommand(CMD12, 0, 0);
	
	SPIx_ReadWriteByte(0xFF);    
	if(count != 0)return count;   //如果没有传完，返回剩余个数	 
	else return 0;	 
}											  
//写入SD卡的N个block(未实际测试过)									    
//输入:u32 sector 扇区地址（sector值，非物理地址） 
//     u8 *buffer 数据存储地址（大小至少512byte）
//     u8 count 写入的block数目		   
//返回值:0： 成功
//       other：失败															   
u8 SD_WriteMultiBlock(u32 sector, const u8 *data, u8 count)
{
	u8 r1;
	u16 i;	 		 
	if(SD_Type != SD_TYPE_V2HC)sector = sector<<9;//如果不是SDHC，给定的是sector地址，将其转换成byte地址  
	if(SD_Type != SD_TYPE_MMC) r1 = SD_SendCommand(ACMD23, count, 0x00);//如果目标卡不是MMC卡，启用ACMD23指令使能预擦除   
	r1 = SD_SendCommand(CMD25, sector, 0x00);//发多块写入指令
	if(r1 != 0x00)return r1;  //应答不正确，直接返回
	SPIx_ReadWriteByte(0xff);//先放3个空数据，等待SD卡准备好
	SPIx_ReadWriteByte(0xff);   
	//--------下面是N个sector写入的循环部分
	do
	{
		//放起始令牌0xFC 表明是多块写入
		SPIx_ReadWriteByte(0xFC);	  
		//放一个sector的数据
		for(i=0;i<512;i++)
		{
			SPIx_ReadWriteByte(*data++);
		}
		//发2个Byte的dummy CRC
		SPIx_ReadWriteByte(0xff);
		SPIx_ReadWriteByte(0xff);

		//等待SD卡应答
		r1 = SPIx_ReadWriteByte(0xff);
		if((r1&0x1F)!=0x05)
		{
			//如果应答为报错，则带错误代码直接退出
			return r1;
		}		   
		//等待SD卡写入完成
		if(SD_WaitDataReady()==1)
		{
			//等待SD卡写入完成超时，直接退出报错
			return 1;
		}	   
	}while(--count);//本sector数据传输完成  
	//发结束传输令牌0xFD
	r1 = SPIx_ReadWriteByte(0xFD);
	if(r1==0x00)
	{
		count =  0xfe;
	}		   
	if(SD_WaitDataReady()) //等待准备好
	{
		return 1;  
	}
	//写入完成
	SPIx_ReadWriteByte(0xff);  
	return count;   //返回count值，如果写完则count=0，否则count=1
}						  					  

rt_size_t spi_sd_read(rt_device_t dev, rt_off_t offset, void * buf, rt_size_t size)
{
	rt_size_t res = size;
	rt_size_t ret = 0;
	rt_off_t off = 0;
	char *buff = (char *)buf;
	
	while(res>0)
	{
		ret += (rt_size_t)SD_ReadSingleBlock((u32)(offset+off), (u8 *)buff);
		off++;
		res -= 1;
		buff += 512;
	}

	if(!res)
		return size;
	else
		return 0;
}

rt_size_t spi_sd_write(rt_device_t dev, rt_off_t offset,const void * buf, rt_size_t size)
{
	rt_size_t res = size;
	rt_size_t ret = 0;
	rt_size_t off = 0;
	char *buff = (char *)buf;

	while(res>0)
	{
		ret += (rt_size_t)SD_WriteSingleBlock((u32)offset+off, (u8 *)buff);
		off++;
		res -= 1;
		buff += 512;
	}
	
	if(!res)
		return size;
	else
		return 0;
}
static rt_err_t spi_sd_init(rt_device_t dev)
{
	u8 state;
	state=SD_Init();
	if(state)
	 return RT_EIO;
	
	return RT_EOK;
}
static rt_err_t spi_sd_open(rt_device_t dev, rt_uint16_t oflag)
{
	SD_GetCapacity();
	return RT_EOK;
}
static rt_err_t spi_sd_close(rt_device_t dev)
{

	return RT_EOK;
}

static rt_err_t spi_sd_control(rt_device_t dev, int cmd, void *args)
{
	RT_ASSERT(dev != RT_NULL);

	if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
	{
		struct rt_device_blk_geometry *geometry;

		geometry = (struct rt_device_blk_geometry *)args;
		if (geometry == RT_NULL) return -RT_ERROR;

		geometry->bytes_per_sector = 512;
		geometry->block_size = sdinfo.CardBlockSize;
		geometry->sector_count = sdinfo.CardCapacity/sdinfo.CardBlockSize;
	}

return RT_EOK;
}
	
void spi_sd_hw_init(const char * sd_device_name, const char * spi_device_name)
{
	struct rt_spi_device * rt_spi_device;
	rt_spi_device = (struct rt_spi_device *)rt_device_find(spi_device_name);
	if(rt_spi_device == RT_NULL)
			return;
	spi_sd_device.rt_spi_device = rt_spi_device;
	
 /* config spi */
	{
			struct rt_spi_configuration cfg;
			cfg.data_width = 8;
			cfg.mode = RT_SPI_MODE_0 | RT_SPI_MSB; /* SPI Compatible Modes 0 and 3 */
			cfg.max_hz = SPI_SD_WORK_HZ; 
			rt_spi_configure(spi_sd_device.rt_spi_device, &cfg);
	}

		
	spi_sd_device.sd_device.type    = RT_Device_Class_Block;
	spi_sd_device.sd_device.init    = spi_sd_init;
	spi_sd_device.sd_device.open    = spi_sd_open;
	spi_sd_device.sd_device.close   = spi_sd_close;
	spi_sd_device.sd_device.read 	 	= spi_sd_read;
	spi_sd_device.sd_device.write   = spi_sd_write;
	spi_sd_device.sd_device.control = spi_sd_control;
	/* no private */
	spi_sd_device.sd_device.user_data = RT_NULL;

	rt_device_register(&spi_sd_device.sd_device, sd_device_name,
										 RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);

}

void spi_sd_hw_uninit()
{
	rt_device_unregister(&spi_sd_device.sd_device);
}

#ifdef SD_HOT_PLUG_TEST

#ifdef RT_USING_DFS
/* dfs init */
//#include <dfs_init.h>
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#define SD_MOUNTPOINT		"/sd0"

#define SD_CD_PIN               GPIO_Pin_11
#define SD_CD_PORT          	  GPIOD
#define SD_CD_PIN_SOURCE				GPIO_PinSource11
#define SD_CD_CLK_PORT					RCC_AHB1Periph_GPIOD

extern void rt_spi_sd_device_init(void);
extern void rt_spi_sd_device_uninit(void);

void sd_hotplug_server(void *param)
{
	uint8_t sd_plugged_in = 0;
	uint8_t mounted = 0;			//sd has been mounted success flag
	
	while(1) 
	{
		// when no sd card, gpio read value is 1
		sd_plugged_in = !GPIO_ReadInputDataBit(SD_CD_PORT,SD_CD_PIN);
		
		// this for elimination of jitter
		rt_thread_delay(50*RT_TICK_PER_SECOND/1000);
		
		// when no sd card, gpio read value is 1
		sd_plugged_in &= !GPIO_ReadInputDataBit(SD_CD_PORT,SD_CD_PIN);
		
		// sd card plugged out
		if(sd_plugged_in)
		{
			if(!mounted)
			{
				#ifdef RT_USING_DFS
					spi_sd_hw_init("sd0","spi12");
				#endif
			
				#ifdef RT_USING_DFS_ELMFAT
				/* mount sd card fat partition 1 as /sd0 directory */
					mounted = !dfs_mount("sd0", SD_MOUNTPOINT, "elm", 0, 0);
				#endif
			}
		}
		else
		{
			if(mounted)
			{
				#ifdef RT_USING_DFS_ELMFAT
					dfs_unmount(SD_MOUNTPOINT); //we don't care unmount return value cause it's safe
				#endif
				#ifdef RT_USING_DFS
					spi_sd_hw_uninit();
				#endif
				mounted = 0;
			}
		}
	}
}

void sd_hotplug_init(void)
{
	rt_thread_t tid;

	tid = rt_thread_create("sd_hp",sd_hotplug_server,RT_NULL,256*3,20,20);
	
	if(tid != RT_NULL) 
	{
		rt_kprintf("create sd_hotplug_server thread successed\n");
		rt_thread_startup(tid);
	}
}

#endif
