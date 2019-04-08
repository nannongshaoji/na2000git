
#include <rtthread.h>
#include "stm32f4xx.h"
#include "sysapp.h"
#include "sys1ms.h"
#include "manage.h"

int Light(int LightNo, int OnOff)
{
	switch (LightNo) {
		case SYS_RUN_LED:
				if(OnOff == 1)
					DO_Write(CPLD_RUN,1);
				else
					DO_Write(CPLD_RUN,0);
		break;
		case SYS_ACTIVE_LED:
				if(OnOff == 1)
					DO_Write(CPLD_COM,1);
				else
					DO_Write(CPLD_COM,0);
		break;
		case SYS_FAULT_LED:
				if(OnOff == 1)
					DO_Write(CPLD_ERR,1);
				else
					DO_Write(CPLD_ERR,0);
		break;
		default:
		break;
	}
return 0;
}

void watchdog_int()
{
/*****************watch dog***************************/
//WDG_EN:PC3 WDG_Plus PA5

#define WDG_EN_PIN              GPIO_Pin_3
#define WDG_EN_PORT             GPIOC
#define WDG_EN_PIN_SOURCE				GPIO_PinSource3
#define WDG_EN_CLK_PORT					RCC_AHB1Periph_GPIOC

#define WDG_Plus_PIN            GPIO_Pin_5
#define WDG_Plus_PORT           GPIOA
#define WDG_Plus_PIN_SOURCE			GPIO_PinSource5
#define WDG_Plus_CLK_PORT				RCC_AHB1Periph_GPIOA
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure2;
	/* Enable GPIOD clock */
	RCC_AHB1PeriphClockCmd(WDG_EN_CLK_PORT | WDG_Plus_CLK_PORT,ENABLE);

	/*WDG_EN*/
	GPIO_InitStructure.GPIO_Pin = WDG_EN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(WDG_EN_PORT, &GPIO_InitStructure);

	/*WDG_Plus*/
	GPIO_InitStructure2.GPIO_Pin = WDG_Plus_PIN;
	GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure2.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure2.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure2.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(WDG_Plus_PORT, &GPIO_InitStructure2);

	GPIO_SetBits(WDG_EN_PORT , WDG_EN_PIN);
}

void wdgenable(void)
{
	GPIO_ResetBits(WDG_EN_PORT , WDG_EN_PIN);
}

void wdgdisable(void)
{
	GPIO_SetBits(WDG_EN_PORT , WDG_EN_PIN);
}

void wdgreset(void)
{
	static char feed = 0;
	feed == 0? GPIO_SetBits(WDG_Plus_PORT , WDG_Plus_PIN): GPIO_ResetBits(WDG_Plus_PORT , WDG_Plus_PIN);
	feed = !feed;
}

void feed_dog(void *lp)
{
	while(1)
	{
		wdgreset();
		rt_thread_delay(50*RT_TICK_PER_SECOND/1000);
	}
}

void watchdog_dem()
{
	rt_thread_t tid;
	watchdog_int();

	tid = rt_thread_create("watchdog",feed_dog, RT_NULL,256, 12, 20);

	if (tid != RT_NULL)
		rt_thread_startup(tid);
}

int ResetCPU(void)
{
	extern void Sys_Soft_Reset();
	extern void reset_lcd();
	reset_lcd();
	Sys_Soft_Reset();
	return 0;
}


void SW_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*使能SW引脚使用的GPIO时钟*/
	RCC_AHB1PeriphClockCmd(SW_GPIO_CLK_PORT, ENABLE);

	GPIO_InitStructure.GPIO_Pin = SW_DEBUG_PIN | SW_RUN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(SW_GPIO_PORT, &GPIO_InitStructure);
	
	watchdog_dem();
}

//读取=1时为上档debug，=3时是中间run，=2时是stop为下档，2017-10-31
int GetHardSW()
{
	uint16_t SW_value = 0 ;
	uint16_t value = 0 ;

	value = GPIO_ReadInputDataBit(SW_GPIO_PORT,SW_RUN_PIN);
	SW_value = (value & 0x01);
	SW_value = SW_value << 1;
	value = GPIO_ReadInputDataBit(SW_GPIO_PORT,SW_DEBUG_PIN);
	SW_value = SW_value | (value & 0x01);

	return SW_value;
}
