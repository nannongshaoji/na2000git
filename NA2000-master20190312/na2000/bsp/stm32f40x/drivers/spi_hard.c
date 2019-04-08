#include <board.h>
#include <rtthread.h>
#include "spi_flash_at45dbxx.h"
#include "stm32f20x_40x_spi.h"

/*
 * SPI1_MOSI: GPIOB5
 * SPI1_MISO: GPIOB4
 * SPI1_SCK : GPIOB3
 *
 * SPI Flash CE: GPIOG15 
 * SD Card CS: GPIOD12 
*/

void rt_hw_spi_init(int spix)
{
	/* register spi bus */
	if(spix == 0)
	{
		{
			static struct stm32_spi_bus stm32_spi;
			GPIO_InitTypeDef GPIO_InitStructure;
			/* Enable GPIO clock */
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB ,ENABLE);
			GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
			GPIO_Init(GPIOB, &GPIO_InitStructure);

			GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
			GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
			GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

			stm32_spi_register(SPI1, &stm32_spi, "spi1");
		}
		/* attach spi flash cs */

		#ifdef RT_USING_SPI_FLASH
		{
			static struct rt_spi_device spi_device;
			static struct stm32_spi_cs  spi_cs;
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
			/* spi11: PG15 */
			spi_cs.GPIOx = GPIOG;
			spi_cs.GPIO_Pin = GPIO_Pin_15;
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
			GPIO_InitStructure.GPIO_Pin = spi_cs.GPIO_Pin;
			GPIO_Init(spi_cs.GPIOx, &GPIO_InitStructure);

			GPIO_SetBits(spi_cs.GPIOx, spi_cs.GPIO_Pin);
			rt_spi_bus_attach_device(&spi_device, "spi11", "spi1",(void*)&spi_cs);
		}

		#endif /* RT_USING_SPI_FLASH */
		#ifdef RT_USING_SPI_SD
		/* attach touch panel cs */
		{
			static struct rt_spi_device spi_device;
			static struct stm32_spi_cs  spi_cs;
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
			/* spi21: PD12 */
			spi_cs.GPIOx = GPIOD;
			spi_cs.GPIO_Pin = GPIO_Pin_12;
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
			GPIO_InitStructure.GPIO_Pin = spi_cs.GPIO_Pin;
			GPIO_Init(spi_cs.GPIOx, &GPIO_InitStructure);
			GPIO_SetBits(spi_cs.GPIOx, spi_cs.GPIO_Pin);
			rt_spi_bus_attach_device(&spi_device, "spi12", "spi1", (void*)&spi_cs);
		}
		#endif /* RT_USING_SPI_SD */
	}
	else if(spix == 1)
	{
		{
			static struct stm32_spi_bus stm32_spi;
			GPIO_InitTypeDef GPIO_InitStructure;
			/* Enable GPIO clock */
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB ,ENABLE);
			GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
			GPIO_Init(GPIOB, &GPIO_InitStructure);

			GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_SPI2);
			GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
			GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

			stm32_spi_register(SPI2, &stm32_spi, "spi2");
		}
		#ifdef RT_USING_SPI_BD
		/* attach bd board */
		{
			static struct rt_spi_device spi_device;
			static struct stm32_spi_cs  spi_cs;
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
			/* spi21: PE15 */
			spi_cs.GPIOx = GPIOE;
			spi_cs.GPIO_Pin = GPIO_Pin_15;
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
			GPIO_InitStructure.GPIO_Pin = spi_cs.GPIO_Pin;
			GPIO_Init(spi_cs.GPIOx, &GPIO_InitStructure);
			GPIO_SetBits(spi_cs.GPIOx, spi_cs.GPIO_Pin);
			rt_spi_bus_attach_device(&spi_device, "spi21", "spi2", (void*)&spi_cs);
		}
		#endif /* RT_USING_SPI_SD */
	}
}

void rt_spi_flash_device_init(void)
{ 
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT) 
	at45dbxx_init("flash0", "spi11");
#endif /* RT_USING_DFS && RT_USING_DFS_ELMFAT */

}

